#include <Arduino.h>

// Undefine the DEFAULT macro to avoid conflicts
#ifdef DEFAULT
#undef DEFAULT
#endif

#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/micro/kernels/micro_ops.h> // Include for built-in micro operators
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/version.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include "TFLiteModel.h"
#include "model.h" // Include your model header

constexpr int kTensorArenaSize = 10 * 1024; // 10KB
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
TFLiteModel::TFLiteModel() : model(nullptr), interpreter(nullptr), input(nullptr), output(nullptr) {}

TfLiteStatus TFLiteModel::RegisterOps(AnomalyDetectionOpResolver& op_resolver) {
    TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
    TF_LITE_ENSURE_STATUS(op_resolver.AddRelu());
    return kTfLiteOk;
}

bool TFLiteModel::Initialize() {
    this->LoadModel();
    this->RegisterOPS(this->op_resolver);
    if (!this->Interpreter()) {
        Serial.println("Failed to setup interpreter.");
        return false;
    }
    Serial.println("Setup model completed!");
    return true;
    // model = tflite::GetModel(anomaly_detection_model_tflite);
    // if (model == nullptr || model->version() != TFLITE_SCHEMA_VERSION) {
    //     Serial.println("Failed to load model or version mismatch");
    //     return false;
    // }

    // AnomalyDetectionOpResolver op_resolver;
    // if (RegisterOps(op_resolver) != kTfLiteOk) {
    //     Serial.println("Failed to register operators");
    //     return false;
    // }

    // interpreter = new tflite::MicroInterpreter(model, op_resolver, tensor_arena, kTensorArenaSize, &error_reporter);
    // if (interpreter == nullptr) {
    //     Serial.println("Failed to create interpreter");
    //     return false;
    // }

    // if (interpreter->AllocateTensors() != kTfLiteOk) {
    //     Serial.println("Failed to allocate tensors");
    //     return false;
    // }

    // input = interpreter->input(0);
    // output = interpreter->output(0);

    // // Serial.printf("Input tensor size: %d bytes\n", input->bytes);
    // // Serial.printf("Output tensor size: %d bytes\n", output->bytes);

    // return true;
}

void TFLiteModel::Cleanup() {
    if (interpreter != nullptr) {
        delete interpreter;
        interpreter = nullptr;
    }
}

bool TFLiteModel::PerformInference(float hr, float ac, float te, bool* res) {
    float normalized_heart_rate = this->normalize(hr, this->heart_rate_min, this->heart_rate_max);
    float normalized_steps = this->normalize(ac, this->accel_min, this->accel_max);
    input->data.f[0] = normalized_heart_rate;
    input->data.f[1] = normalized_steps;
    Serial.print("Data 1: ");
    Serial.print(hr);
    Serial.print(", Data 2: ");
    Serial.print(ac);
    Serial.print(", Data 3: ");
    Serial.println(te);
    TF_LITE_ENSURE_STATUS(interpreter->Invoke());

    float output_value_hr = output->data.f[0]; 
    float output_value_steps = output->data.f[1];
    float mse_hr = pow(output_value_hr - normalized_heart_rate, 2);
    float mse_steps = pow(output_value_steps - normalized_steps, 2);
    float reconstruction_error = (mse_hr + mse_steps) / 2.0f;
    Serial.print("MSE HR: ");
    Serial.print(mse_hr, 4); // Print with 4 decimal places
    Serial.print(" | MSE Steps: ");
    Serial.print(mse_steps, 4); // Print with 4 decimal places
    Serial.print(" | Reconstruction Error: ");
    Serial.println(reconstruction_error, 7);
    if (reconstruction_error > 0.0002) {
        //Serial.println("Anomaly detected!");
        *res = true;
    } else {
        //Serial.println("No anomaly detected.");
        *res = false;
    }

    return true;
}

bool TFLiteModel::Interpreter()
{
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

    return true;
}

float TFLiteModel::normalize(int value, float min, float max)
{
    return (value - min) / (max - min);
}

void TFLiteModel::LoadModel()
{
    model = tflite::GetModel(anomaly_detection_model_tflite);
    if (model == nullptr || model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("Failed to load model or version mismatch");
        return;
    }
}

void TFLiteModel::RegisterOPS(AnomalyDetectionOpResolver& op_resolver)
{
    if (RegisterOps(op_resolver) != kTfLiteOk) {
        Serial.println("Failed to register operators");
        return;
    }
    Serial.println("Registered operators!");
}
