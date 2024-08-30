#ifndef TFLITEMODEL_H
#define TFLITEMODEL_H

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model.h" 

class TFLiteModel {
public:
    TFLiteModel();
    void setup();
    bool getAnomalyPreditction(float v1, float v2, float v3);
private:
    tflite::MicroErrorReporter* error_reporter;
    const tflite::Model *model;
    tflite::MicroInterpreter* interpreter;
    TfLiteTensor* input;
    TfLiteTensor* output;

    bool initializeErrorReporting();
    bool loadModel();
    bool initializeInterpreter();
    bool allocateTensors();
};

#endif