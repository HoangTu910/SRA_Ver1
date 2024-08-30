#include "TFLiteModel.h"

TFLiteModel::TFLiteModel() : error_reporter(nullptr), model(nullptr), interpreter(nullptr), input(nullptr), output(nullptr) {}

namespace {
    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;
    constexpr int kTensorArenaSize = 2000;
    uint8_t tensor_arena[kTensorArenaSize];
}

void TFLiteModel::setup()
{
    if (!initializeErrorReporting()) return;
    if (!loadModel()) return;
    if (!initializeInterpreter()) return;
    if (!allocateTensors()) return;

    input = interpreter->input(0);
    output = interpreter->output(0);
}

bool TFLiteModel::initializeErrorReporting()
{
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    return true; 
}

bool TFLiteModel::loadModel()
{
    model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter, "Model version mismatch.");
        return false;
    }
    return true;
}

bool TFLiteModel::initializeInterpreter()
{
    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;
    return true;
}

bool TFLiteModel::allocateTensors()
{
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return false;
    }
    return true;
}

bool TFLiteModel::getAnomalyPreditction(float v1, float v2, float v3)
{
    input->data.f[0] = v1;
    input->data.f[1] = v2;
    input->data.f[2] = v3;

    if (interpreter->Invoke() != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
        return false;
    }
    float anomaly_prediction = output->data.f[0];
    if(anomaly_prediction > 0.5) return true;
    else false;
}
