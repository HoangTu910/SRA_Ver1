#!/bin/bash

# Function to run nvm and npm in a directory
run_service() {
    local dir=$1
    echo "Starting service in $dir..."
    (
        cd "$dir" || exit
        source ~/.nvm/nvm.sh  # Load nvm
        nvm use 16
        npm start
    )
}

# Start backend service
run_service "./backend" &

# Start broker service
run_service "./broker" &

# Wait for all background jobs to finish
wait
echo "All services started."
