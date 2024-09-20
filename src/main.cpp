#include <Arduino.h>
#include "esp_system.h"
#include "esp_task_wdt.h"
// Undefine the DEFAULT macro to avoid conflicts
#ifdef DEFAULT
#undef DEFAULT
#endif

#include <stdlib.h> 
#include <time.h> 
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/micro/kernels/micro_ops.h> // Include for built-in micro operators
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/version.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include "autoencoder.h" // Include your model header

namespace {
  using AnomalyDetectionOpResolver = tflite::MicroMutableOpResolver<2>;

  TfLiteStatus RegisterOps(AnomalyDetectionOpResolver& op_resolver) {
    TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
    TF_LITE_ENSURE_STATUS(op_resolver.AddRelu());
    // Add other operators if needed
    return kTfLiteOk;
  }
}

constexpr int kTensorArenaSize = 10 * 1024; // 4KB
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
tflite::MicroErrorReporter error_reporter;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
AnomalyDetectionOpResolver op_resolver;

// Arrays of test data
const int kDataSize = 20;
float accelerometer_data[kDataSize] = {2.944814, 114.085717, 87.839273, 71.839018, 18.722237, 18.719342, 6.970033, 103.941137, 72.133801, 84.968709, 2.470139, 116.389182, 99.893117, 25.480693, 21.818996, 22.008541, 36.509069, 62.970772, 51.833402, 34.947497};
float heart_rate_data[kDataSize] = {105.066761, 62.554447, 76.293018, 82.972566, 91.046299, 120.665837, 67.970640, 96.281099, 103.317311, 54.180537, 104.679037, 65.347171, 55.854643, 135.399698, 136.906883, 122.755761, 77.415239, 58.790490, 111.580972, 89.613724};
float temperature_data[kDataSize];

// Define min and max values based on your training data statistics
const float heart_rate_min = 57.0;  // Example min heart rate
const float heart_rate_max = 155.0; // Example max heart rate
const float accel_min = 0.0;        // Example min accelerometer
const float accel_max = 140.0;       // Example max accelerometer

float normalize(float value, float min, float max) {
  return (value - min) / (max - min);
}

float input_data[kDataSize][3];

void Scale2D(float *accelerometer_data, float *heart_rate_data){
  for (int i = 0; i < kDataSize; i++) {
      // Normalize heart rate and accelerometer data
      float normalized_heart_rate = normalize(heart_rate_data[i], heart_rate_min, heart_rate_max);
      float normalized_accel = normalize(accelerometer_data[i], accel_min, accel_max);
      
      // Uncomment if including temperature data
      // float normalized_temperature = normalize(temperature_data[i], temp_min, temp_max);

      // Load normalized values into the input array
      input_data[i][0] = normalized_heart_rate; // Heart rate
      input_data[i][1] = normalized_accel;      // Accelerometer
      // input_data[i][2] = normalized_temperature; // Temperature data
  }
}

void LoadModel() {
  model = tflite::GetModel(anomaly_detection_model_tflite);
  if (model == nullptr || model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Failed to load model or version mismatch");
    return;
  }
  Serial.println("Loaded Model!");
}

void RegisterOps() {
  TfLiteStatus status = RegisterOps(op_resolver);
  if (status != kTfLiteOk) {
    Serial.println("Failed to register operators");
  }
  Serial.println("Registered Ops!");
}

bool Interpreter() {
  interpreter = new tflite::MicroInterpreter(model, op_resolver, tensor_arena, kTensorArenaSize, &error_reporter);
  if (interpreter == nullptr) {
    Serial.println("Failed to create interpreter");
    return false;
  }

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("Failed to allocate tensors");
    return false;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.printf("Input tensor size: %d bytes\n", input->bytes);
  Serial.printf("Output tensor size: %d bytes\n", output->bytes);

  return true;
}

void SetupModel() {
  LoadModel();
  RegisterOps();
  if (!Interpreter()) {
    Serial.println("Failed to setup interpreter.");
    return;
  }
  Serial.println("Setup model completed!");
}

void Cleanup() {
  // Print memory before cleanup
  Serial.printf("Memory before cleanup: %d bytes\n", ESP.getFreeHeap());

  if (interpreter != nullptr) {
    delete interpreter;
    interpreter = nullptr;
    input = nullptr;
    output = nullptr;
  }

  // Print memory after cleanup
  Serial.printf("Memory after cleanup: %d bytes\n", ESP.getFreeHeap());
}

TfLiteStatus PerformInference() {
    Scale2D(accelerometer_data, heart_rate_data);

    for (int i = 0; i < kDataSize; ++i) {
        // Prepare a single sample
        float normalized_heart_rate = normalize(heart_rate_data[i], heart_rate_min, heart_rate_max);
        float normalized_steps = normalize(accelerometer_data[i], accel_min, accel_max);
        Serial.print("Normalized HR: ");
        Serial.print(normalized_heart_rate, 7);  // Print with 4 decimal places
        Serial.print(" | Normalized Steps: ");
        Serial.println(normalized_steps, 7);     // Print with 4 decimal places

        Serial.print("HRate: ");
        Serial.print(heart_rate_data[i], 7);  // Print with 4 decimal places
        Serial.print(" | Steps: ");
        Serial.println(accelerometer_data[i], 7);
        // Set the single sample as input
        input->data.f[0] = normalized_heart_rate; // Normalized heart rate
        input->data.f[1] = normalized_steps; // Normalized steps

        // Run inference
        TF_LITE_ENSURE_STATUS(interpreter->Invoke());

        // Get the output data from the model
        float output_value_hr = output->data.f[0]; // First output for heart rate
        float output_value_steps = output->data.f[1]; // Second output for steps

        // Print the output for debugging
        Serial.print("Output HR: ");
        Serial.print(output_value_hr, 7); // Print with 4 decimal places
        Serial.print(" | Output Steps: ");
        Serial.println(output_value_steps, 7); // Print with 4 decimal places

        // Calculate Mean Squared Error (MSE) for anomaly detection
        float mse_hr = pow(output_value_hr - normalized_heart_rate, 2);
        float mse_steps = pow(output_value_steps - normalized_steps, 2);
        float reconstruction_error = (mse_hr + mse_steps) / 2.0f;

        // Print MSE for debugging
        Serial.print("MSE HR: ");
        Serial.print(mse_hr, 4); // Print with 4 decimal places
        Serial.print(" | MSE Steps: ");
        Serial.print(mse_steps, 4); // Print with 4 decimal places
        Serial.print(" | Reconstruction Error: ");
        Serial.println(reconstruction_error, 7); // Print with 4 decimal places

        // Anomaly detection based on combined reconstruction error
        if (reconstruction_error > 0.0002) { // Adjust threshold as needed
            Serial.println("Anomaly detected!");
        } else {
            Serial.println("No anomaly detected.");
        }

        delay(1000); // Adjust delay as needed
    }

    return kTfLiteOk;
}




void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  SetupModel();

  // Generate random data for testing
  for (int i = 0; i < kDataSize; ++i) {
    // heart_rate_data[i] = random(5000, 18000) / 100.0;  // Random heart rate between 50.00 and 180.00
    // accelerometer_data[i] = random(0, 2000) / 100.0;   // Random accelerometer value between 0.00 and 20.00
    // temperature_data[i] = random(3500, 4000) / 100.0;  // Random temperature between 35.00 and 40.00

    // Print the generated data
    Serial.print("Heart Rate: ");
    Serial.print(heart_rate_data[i]);
    Serial.print(", Steps/min: ");
    Serial.println(accelerometer_data[i]);
    // Serial.print(", Temperature: ");
    // Serial.println(temperature_data[i]);
  }
}

void loop() {
  if (interpreter == nullptr) {
    Serial.println("Re-initializing interpreter...");
    if (!Interpreter()) {
      Serial.println("Failed to re-setup interpreter.");
      return;
    }
  }

  if (PerformInference() != kTfLiteOk) {
    Serial.println("Inference failed");
  }
  delay(5000); // Delay before the next batch of readings
}
