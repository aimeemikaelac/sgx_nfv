#!/bin/bash

DIR=$(pwd)
TIMESTAMP=$(date +%Y-%m-%d:%H:%M:%S)
DATA_DIR=$DIR/data/
EXPERIMENT_DIR=$DIR/click_experiments/

GLOBAL_ITERATIONS=20
BASELINE_ITERATIONS=$GLOBAL_ITERATIONS
BASIC_SGX_ITERATIONS=$GLOBAL_ITERATIONS
BASIC_NO_SGX_ITERATIONS=$GLOBAL_ITERATIONS
SHA256_SGX_ITERATIONS=$GLOBAL_ITERATIONS
SHA256_NO_SGX_ITERATIONS=$GLOBAL_ITERATIONS

CSV_HEADER="Counter Count,Source Count,Sink Count,Counter Byte Count,Counter Bit Rate (bit/s),Counter Byte Rate (bytes/s)"

echo "Make sure to have run build_sgx.sh before running this script"
echo "Otherwise the SGX module won't build"

source /opt/intel/sgxsdk/environment

echo "Making current experiment run directory"
mkdir -p $DATA_DIR/$TIMESTAMP

BASELINE_FILE="$DATA_DIR/$TIMESTAMP/experiment-baseline-$TIMESTAMP.csv"
$EXPERIMENT_DIR/baseline_experiment.sh $BASELINE_FILE $BASELINE_ITERATIONS $EXPERIMENT_DIR
