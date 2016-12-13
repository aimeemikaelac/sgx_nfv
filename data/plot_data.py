#!/usr/bin/python

import pylab
import numpy
import csv
import os
import sys

TIMESTAMP="2016-12-11:01:31:48"
DATA_DIR=TIMESTAMP


#read data from files
FILE_LIST = ["experiment-baseline",
             "experiment-basic-nosgx-simulation",
             "experiment-basic-sgx-debug",
             "experiment-basic-sgx-prerelease",
             "experiment-basic-sgx-simulation",
             "experiment-sha256-nosgx-simulation",
             "experiment-sha256-sgx-debug",
             "experiment-sha256-sgx-prerelease",
             "experiment-sha256-sgx-simulation"]

EXPERIMENT_MAP = {"experiment-baseline":"baseline",
                  "experiment-basic":"basic",
                  "experiment-sha256":"sha256"}

EXPERIMENT_TITLES = {"basic":"Pattern Match Experiment",
                     "sha256":"SHA256 Hash Calculation Experiment"}



def plot_experiment(EXPERIMENT_DATA, experiment):
    baseline_bitrates = []
    nosgx_bitrates = []
    sgx_simulation_bitrates = []
    sgx_debug_bitrates = []
    sgx_prerelease_bitrates = []

    for row in EXPERIMENT_DATA["experiment-baseline"]["experiment-baseline"]:
        baseline_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

    for row in EXPERIMENT_DATA[experiment]["{}-nosgx-simulation".format(experiment)]:
        nosgx_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

    for row in EXPERIMENT_DATA[experiment]["{}-sgx-simulation".format(experiment)]:
        sgx_simulation_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

    for row in EXPERIMENT_DATA[experiment]["{}-sgx-debug".format(experiment)]:
        sgx_debug_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

    for row in EXPERIMENT_DATA[experiment]["{}-sgx-prerelease".format(experiment)]:
        sgx_prerelease_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

    baseline_average = sum(baseline_bitrates)/float(len(baseline_bitrates))
    nosgx_average = sum(nosgx_bitrates)/float(len(nosgx_bitrates))
    sgx_simulation_average = sum(sgx_simulation_bitrates)/float(len(sgx_simulation_bitrates))
    sgx_debug_average = sum(sgx_debug_bitrates)/float(len(sgx_debug_bitrates))
    sgx_prerelease_average = sum(sgx_prerelease_bitrates)/float(len(sgx_prerelease_bitrates))

    #convert to GBit/s
    baseline_average = baseline_average/1000.0/1000.0/1000.0
    nosgx_average = nosgx_average/1000.0/1000.0/1000.0
    sgx_simulation_average = sgx_simulation_average/1000.0/1000.0/1000.0
    sgx_debug_average = sgx_debug_average/1000.0/1000.0/1000.0
    sgx_prerelease_average = sgx_prerelease_average/1000.0/1000.0/1000.0

    print "{}: baseline average: {} GBit/s".format(experiment, baseline_average)
    print "{}: nosgx average: {} GBit/s".format(experiment, nosgx_average)
    print "{}: sgx simulation average: {} GBit/s".format(experiment, sgx_simulation_average)
    print "{}: sgx debug average: {} GBit/s".format(experiment, sgx_debug_average)
    print "{}: sgx prerelease average: {} GBit/s".format(experiment, sgx_prerelease_average)

    pylab.figure()
    X = range(4)
    Y = [nosgx_average, sgx_simulation_average, sgx_debug_average, sgx_prerelease_average]
    rects = pylab.bar(X, Y, tick_label=["No-SGX", "Simulation", "HW-Debug", "HW-Prerelease"], align='center', color='orange')
    pylab.xlabel("Experiment")
    pylab.ylabel("Bandwidth (GBit/s)")
    offset = 0
    for rect in rects:
        curr_offset = rect.get_height()*.05
        if curr_offset > offset:
            offset = curr_offset
    for rect in rects:
        height = rect.get_height()
        pylab.text(rect.get_x() + rect.get_width()/2., height - offset,
                "{}".format(round(height, 2)),
                ha='center', va='bottom')
    # pylab.show()
    pylab.savefig("{}.pdf".format(experiment))

if __name__ == "__main__":
    EXPERIMENT_DATA = {}
    EXPERIMENT_DATA_FILES = {}
    for fileentry in FILE_LIST:
        filename = "{}-{}.csv".format(fileentry, TIMESTAMP)
        filepath = "{}/{}".format(DATA_DIR, filename)
        if not os.path.isfile(filepath):
            print "Cannot open file: {}".format(filepath)
            sys.exit(-1)
        current_file = open(filepath)
        EXPERIMENT_DATA_FILES[fileentry] = current_file
        current_reader = csv.DictReader(current_file)
        experiment = ""
        for experiment_key in EXPERIMENT_MAP:
            if filename.startswith(experiment_key):
                experiment = experiment_key
                break
        if experiment == "":
            print "Encountered unlisted experiment"
            sys.exit(-1)
        if experiment not in EXPERIMENT_DATA:
            EXPERIMENT_DATA[experiment] = {}
        EXPERIMENT_DATA[experiment][fileentry] = list(current_reader)
    plot_experiment(EXPERIMENT_DATA, "experiment-basic")

    plot_experiment(EXPERIMENT_DATA, "experiment-sha256")
