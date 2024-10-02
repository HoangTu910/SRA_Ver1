# Version 2
## Data distribution
https://colab.research.google.com/drive/1_iEmS1FvLW4jn5GnI2wRjCiDt_QvFxT4#scrollTo=-_ES5x4aRO28

## Autoencoder model

The objective of autoencoder is it will try to reconstruct the input data as similar as training data. So basically we just need to train the autoencoder model with full of normal data, after that we can calculate the reconstruction error (RE) in both normal dataset and anomaly dataset. If RE is small, it mean autoencoder reconstruct correctly and it normal, otherwise we will consider it anomaly. Now we need to find the threshold for classifying if it normal or anomaly. 


## New dataset definition
* pleth_1: MAX30101 red wavelength PPG from the distal phalanx (first segment) of the left index finger palmar side (arbitrary units, 500Hz)
* pleth_2: MAX30101 infrared wavelength PPG from the distal phalanx (first segment) of the left index finger palmar side (arbitrary units, 500Hz)
* pleth_3: MAX30101 green wavelength PPG from the distal phalanx (first segment) of the left index finger palmar side (arbitrary units, 500Hz)
* pleth_4: MAX30101 red wavelength PPG from the proximal phalanx (base segment) of the left index finger palmar side (arbitrary units, 500Hz)
* pleth_5: MAX30101 infrared wavelength PPG from the proximal phalanx (base segment) of the left index finger palmar side (arbitrary units, 500Hz)
* pleth_6: MAX30101 green wavelength PPG from the proximal phalanx (base segment) of the left index finger palmar side (arbitrary units, 500Hz)
* temp_1: distal phalanx (first segment) PPG sensor temperature (°C, 10Hz)
* temp_2: proximal phalanx (base segment) PPG sensor temperature in (°C, 10Hz)
* temp_3: InvenSenseMPU-9250 IMU temperature (°C, 500Hz)
* a_x: InvenSenseMPU-9250 IMU acceleration in x-direction (g, 500Hz)
* a_y: InvenSenseMPU-9250 IMU acceleration in y-direction (g, 500Hz)
* a_z: InvenSenseMPU-9250 IMU acceleration in z-direction (g, 500Hz)
* g_x: InvenSenseMPU-9250 IMU angular velocity around x-axis (°/s, 500Hz)
* g_y: InvenSenseMPU-9250 IMU angular velocity around y-axis (°/s, 500Hz)
* g_z: InvenSenseMPU-9250 IMU angular velocity around z-axis (°/s, 500Hz)

### Data input dimension

Input_data = (pleth_6, temp_2, [a_x, a_y, a_z, g_x, g_y, g_x])
