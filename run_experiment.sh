#!/bin/bash

set -e

log_string () {
  echo $1 | tee -a $2
}

SCRIPT=$(realpath $0)
SCRIPTPATH=$(dirname $SCRIPT)
DIR=$SCRIPTPATH
cd $DIR

TIMESTAMP=$(date +%Y-%m-%d:%H:%M:%S)
DATA_DIR=$DIR/data/
EXPERIMENT_DIR=$DIR/click_experiments/
LOG_DIR=$DIR/logs
LOG_FILE=$LOG_DIR/log-$TIMESTAMP.log

mkdir -p $LOG_DIR

CLICK=$DIR/click_out/bin/click

GLOBAL_ITERATIONS=10
BASELINE_ITERATIONS=$GLOBAL_ITERATIONS
BASIC_SGX_ITERATIONS=$GLOBAL_ITERATIONS
BASIC_NOSGX_ITERATIONS=$GLOBAL_ITERATIONS
SHA256_SGX_ITERATIONS=$GLOBAL_ITERATIONS
SHA256_NOSGX_ITERATIONS=$GLOBAL_ITERATIONS

CSV_HEADER="Counter Count,Source Count,Sink Count,Counter Byte Count,Counter Bit Rate (bit/s),Counter Byte Rate (bytes/s)"

log_string "Make sure to have run build_sgx.sh before running this script" $LOG_FILE
log_string "Otherwise the SGX module won't build" $LOG_FILE

log_string "Making current experiment run directory: $DATA_DIR/$TIMESTAMP" $LOG_FILE
mkdir -p $DATA_DIR/$TIMESTAMP

#-------------------------------------------------
#Baseline experiment

INSTALL_CLICK_FILE=$LOG_DIR/install-click-default-$TIMESTAMP.log
log_string "Building default click. Logging to $INSTALL_CLICK_FILE" $LOG_FILE
$DIR/scripts/build_click.sh >> $INSTALL_CLICK_FILE 2>&1

BASELINE_FILE="$DATA_DIR/$TIMESTAMP/experiment-baseline-$TIMESTAMP.csv"
$EXPERIMENT_DIR/baseline_experiment.sh $CLICK $BASELINE_FILE $BASELINE_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE

#-----------------------------------------------------
#SGX Simulation experiment

source /opt/intel/sgxsdk/environment
INSTALL_CLICK_SGX_SIMULATION_FILE=$LOG_DIR/install-click-sgx-simulation-$TIMESTAMP.log
log_string "Building click w/ SGX in simulation mode. Logging to $INSTALL_CLICK_SGX_SIMULATION_FILE" $LOG_FILE
$DIR/scripts/build_click_simulation.sh >> $INSTALL_CLICK_SGX_SIMULATION_FILE 2>&1

SGX_BASIC_SIMULATION_FILE=$DATA_DIR/$TIMESTAMP/experiment-basic-sgx-simulation-$TIMESTAMP.csv
$EXPERIMENT_DIR/basic_sgx_experiment.sh $CLICK $SGX_BASIC_SIMULATION_FILE $BASIC_SGX_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE

NOSGX_BASIC_SIMULATION_FILE=$DATA_DIR/$TIMESTAMP/experiment-basic-nosgx-simulation-$TIMESTAMP.csv
$EXPERIMENT_DIR/basic_nosgx_experiment.sh $CLICK $NOSGX_BASIC_SIMULATION_FILE $BASIC_NOSGX_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE

SGX_SHA256_SIMULATION_FILE=$DATA_DIR/$TIMESTAMP/experiment-sha256-sgx-simulation-$TIMESTAMP.csv
$EXPERIMENT_DIR/sha256_sgx_experiment.sh $CLICK $SGX_SHA256_SIMULATION_FILE $SHA256_SGX_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE

NOSGX_SHA256_SIMULATION_FILE=$DATA_DIR/$TIMESTAMP/experiment-sha256-nosgx-simulation-$TIMESTAMP.csv
$EXPERIMENT_DIR/sha256_nosgx_experiment.sh $CLICK $NOSGX_SHA256_SIMULATION_FILE $SHA256_NOSGX_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE

#------------------------------------------------------------------------------
#SGX HW DEBUG experiment


INSTALL_CLICK_SGX_DEBUG_FILE=$LOG_DIR/install-click-sgx-debug-$TIMESTAMP.log
log_string "Building click w/ SGX in debug mode. Logging to $INSTALL_CLICK_SGX_DEBUG_FILE" $LOG_FILE
$DIR/scripts/build_click_hardware_debug.sh >> $INSTALL_CLICK_SGX_DEBUG_FILE 2>&1

SGX_BASIC_DEBUG_FILE=$DATA_DIR/$TIMESTAMP/experiment-basic-sgx-debug-$TIMESTAMP.csv
$EXPERIMENT_DIR/basic_sgx_experiment.sh $CLICK $SGX_BASIC_DEBUG_FILE $BASIC_SGX_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE

SGX_SHA256_DEBUG_FILE=$DATA_DIR/$TIMESTAMP/experiment-sha256-sgx-debug-$TIMESTAMP.csv
$EXPERIMENT_DIR/sha256_sgx_experiment.sh $CLICK $SGX_SHA256_DEBUG_FILE $SHA256_SGX_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE


#----------------------------------------------
#SGX HW Prerelease experiment

INSTALL_CLICK_SGX_PRERELEASE_FILE=$LOG_DIR/install-click-sgx-prerelease-$TIMESTAMP.log
log_string "Building click w/ SGX in prerelease mode. Logging to $INSTALL_CLICK_SGX_PRERELEASE_FILE" $LOG_FILE
$DIR/scripts/build_click_hardware_prerelease.sh >> $INSTALL_CLICK_SGX_PRERELEASE_FILE 2>&1

SGX_BASIC_PRERELEASE_FILE=$DATA_DIR/$TIMESTAMP/experiment-basic-sgx-prerelease-$TIMESTAMP.csv
$EXPERIMENT_DIR/basic_sgx_experiment.sh $CLICK $SGX_BASIC_PRERELEASE_FILE $BASIC_SGX_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE

SGX_SHA256_PRERELEASE_FILE=$DATA_DIR/$TIMESTAMP/experiment-sha256-sgx-prerelease-$TIMESTAMP.csv
$EXPERIMENT_DIR/sha256_sgx_experiment.sh $CLICK $SGX_SHA256_PRERELEASE_FILE $SHA256_SGX_ITERATIONS $EXPERIMENT_DIR | tee -a $LOG_FILE
