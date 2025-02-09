#!/bin/bash

# Define the source directory
SRC_DIR="/home/iot-bts2/HHT_AIT/backend/src/app/diffie-hellman"

# Define the source files and the output executable
EXEC="$SRC_DIR/exec-ecdh-secret"
ECdh_CPP="$SRC_DIR/ecdh.cpp"
EXEC_ECDH_SECRET_CPP="$SRC_DIR/exec-ecdh-secret.cpp"

# Compile the C++ files using g++
g++ -o "$EXEC" "$ECdh_CPP" "$EXEC_ECDH_SECRET_CPP"

# Check if the compilation succeeded
if [ $? -eq 0 ]; then
    echo "✅ Compilation successful. Executable created at: $EXEC"
else
    echo "Compilation failed."
    exit 1
fi
