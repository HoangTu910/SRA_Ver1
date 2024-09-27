# Version 2
## Data distribution
https://colab.research.google.com/drive/1_iEmS1FvLW4jn5GnI2wRjCiDt_QvFxT4#scrollTo=-_ES5x4aRO28

## Autoencoder model

The objective of autoencoder is it will try to reconstruct the input data as similar as training data. So basically we just need to train the autoencoder model with full of normal data, after that we can calculate the reconstruction error (RE) in both normal dataset and anomaly dataset. If RE is small, it mean autoencoder reconstruct correctly and it normal, otherwise we will consider it anomaly. Now we need to find the threshold for classifying if it normal or anomaly. 
