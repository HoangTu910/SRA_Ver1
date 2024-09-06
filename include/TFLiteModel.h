#ifndef TFLITE_MODEL_H
#define TFLITE_MODEL_H

#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/micro/kernels/micro_ops.h> // Include for built-in micro operators
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/version.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>


#ifdef DEFAULT
#undef DEFAULT
#endif

class TFLiteModel {
public:
    TFLiteModel();
    bool Initialize();
    void Cleanup();
    bool PerformInference(float hr, float ac, float te, bool* res);
    bool Interpreter();
    tflite::MicroInterpreter* interpreter;
private:
    void LoadModel();
    using AnomalyDetectionOpResolver = tflite::MicroMutableOpResolver<2>;
    void RegisterOPS(AnomalyDetectionOpResolver& op_resolver);
    AnomalyDetectionOpResolver op_resolver;
    TfLiteStatus RegisterOps(AnomalyDetectionOpResolver& op_resolver);
    static constexpr int kTensorArenaSize = 10 * 1024; // 10KB
    alignas(16) uint8_t tensor_arena[kTensorArenaSize];
    tflite::MicroErrorReporter error_reporter;
    const tflite::Model* model;
    TfLiteTensor* input;
    TfLiteTensor* output;
};

#endif // TFLITE_MODEL_H
