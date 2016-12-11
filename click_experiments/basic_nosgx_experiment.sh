#!/bin/bash

CLICK=$1
OUTPUT_FILE=$2
ITERATIONS=$3
EXPERIMENT_DIR=$4

echo "Running the basic nosgx test"

CSV_HEADER="Counter Count,Source Count,Sink Count,Counter Byte Count,Counter Bit Rate (bit/s),Counter Byte Rate (bytes/s)"
echo "$CSV_HEADER" > $OUTPUT_FILE

for ((i=1;i<$ITERATIONS;i++));
do
  echo "Baseline experiment $i"
  $CLICK $EXPERIMENT_DIR/wire_infinite_source_nosgx.click | tee -a $OUTPUT_FILE
done
