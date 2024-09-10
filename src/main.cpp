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
#include "model_test.h" // Include your model header

namespace {
using AnomalyDetectionOpResolver = tflite::MicroMutableOpResolver<2>;

TfLiteStatus RegisterOps(AnomalyDetectionOpResolver& op_resolver) {
  TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
  TF_LITE_ENSURE_STATUS(op_resolver.AddRelu());
  // Add other operators if needed
  return kTfLiteOk;
}

constexpr int kTensorArenaSize = 4 * 1024; // 4KB
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
tflite::MicroErrorReporter error_reporter;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
AnomalyDetectionOpResolver op_resolver;
// Arrays of test data
const int kDataSize = 10;

float heart_rate_data[kDataSize];
float accelerometer_data[kDataSize];
float temperature_data[kDataSize];


void LoadModel(){
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
  // model = tflite::GetModel(anomaly_detection_model_tflite);
  // if (model == nullptr || model->version() != TFLITE_SCHEMA_VERSION) {
  //   Serial.println("Failed to load model or version mismatch");
  //   return;
  // }

  // AnomalyDetectionOpResolver op_resolver;
  // TfLiteStatus status = RegisterOps(op_resolver);

  // interpreter = new tflite::MicroInterpreter(model, op_resolver, tensor_arena, kTensorArenaSize, &error_reporter);
  // if (interpreter == nullptr) {
  //   Serial.println("Failed to create interpreter");
  //   return;
  // }

  // if (interpreter->AllocateTensors() != kTfLiteOk) {
  //   Serial.println("Failed to allocate tensors");
  //   return;
  // }

  // input = interpreter->input(0);
  // output = interpreter->output(0);

  // Serial.printf("Input tensor size: %d bytes\n", input->bytes);
  // Serial.printf("Output tensor size: %d bytes\n", output->bytes);
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

}  // namespace

TfLiteStatus PerformInference() {
  for (int i = 0; i < kDataSize; ++i) {
    // Load the input data
    input->data.f[0] = heart_rate_data[i];
    input->data.f[1] = accelerometer_data[i];
    input->data.f[2] = temperature_data[i];

    TF_LITE_ENSURE_STATUS(interpreter->Invoke());

    float anomaly_score = output->data.f[0];
    Serial.print("Data index: ");
    Serial.print(i);
    Serial.print(" | Anomaly Score: ");
    Serial.println(anomaly_score);
    if (anomaly_score > 0.5) {
      Serial.println("Anomaly detected!");
    } else {
      Serial.println("No anomaly detected.");
    }
    delay(1000);
  }
  return kTfLiteOk;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  // LoadModel();
  // RegisterOps();
  // if (!Interpreter()) {
  //   Serial.println("Failed to setup interpreter.");
  // }
  SetupModel();
  for (int i = 0; i < kDataSize; ++i) {
      heart_rate_data[i] = random(5000, 18000) / 100.0;  // Random heart rate between 50.00 and 180.00
      accelerometer_data[i] = random(0, 2000) / 100.0;   // Random accelerometer value between 0.00 and 20.00
      temperature_data[i] = random(3500, 4000) / 100.0;   // Random temperature between 35.00 and 40.00

      // Print the generated data
      Serial.print("Heart Rate: ");
      Serial.print(heart_rate_data[i]);
      Serial.print(", Accelerometer: ");
      Serial.print(accelerometer_data[i]);
      Serial.print(", Temperature: ");
      Serial.println(temperature_data[i]);
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
  //Serial.printf("Free heap size: %d bytes\n", ESP.getFreeHeap());
  if (PerformInference() != kTfLiteOk) {
    Serial.println("Inference failed");
  }
  delay(5000); // Delay before the next batch of readings
}
