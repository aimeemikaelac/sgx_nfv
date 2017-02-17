#!/usr/bin/python

import pylab
import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42
import numpy
import csv
import os
import sys

TIMESTAMP_LIST=["2016-12-11:01:31:48","2016-12-18:19:10:57"]
# DATA_DIR=TIMESTAMP


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



def plot_experiment(EXPERIMENT_DATA, experiment, keep_figure=False, save_separate=False, colors=[], legend_labels=[]):
    column_width = .35
    width = 0
    index = 0

    for timestamp in EXPERIMENT_DATA:
        baseline_bitrates = []
        nosgx_bitrates = []
        sgx_simulation_bitrates = []
        sgx_debug_bitrates = []
        sgx_prerelease_bitrates = []
        current = EXPERIMENT_DATA[timestamp]
        for row in current["experiment-baseline"]["experiment-baseline"]:
            baseline_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

        for row in current[experiment]["{}-nosgx-simulation".format(experiment)]:
            nosgx_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

        for row in current[experiment]["{}-sgx-simulation".format(experiment)]:
            sgx_simulation_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

        for row in current[experiment]["{}-sgx-debug".format(experiment)]:
            sgx_debug_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

        for row in current[experiment]["{}-sgx-prerelease".format(experiment)]:
            sgx_prerelease_bitrates.append(float(row["Counter Bit Rate (bit/s)"]))

        baseline_average = sum(baseline_bitrates)/float(len(baseline_bitrates))
        nosgx_average = sum(nosgx_bitrates)/float(len(nosgx_bitrates))
        print "timestamp: {}".format(timestamp)
        print "filepath: {}".format(EXPERIMENT_DATA[timestamp]["filepath"])
        print "No sgx average: {}".format(nosgx_average)
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

        if not keep_figure:
            pylab.figure()
        X = range(4)
        for x in range(4):
            X[x] = X[x] + width
        Y = [nosgx_average, sgx_simulation_average, sgx_debug_average, sgx_prerelease_average]
        if index < len(colors):
            color = colors[index]
        else:
            color ='orange'
        if index < len(legend_labels):
            label = legend_labels[index]
        else:
            label = None
        rects = pylab.bar(X,
                          Y,
                          column_width,
                          align='edge',
                          label=label,
                          color=color)
        pylab.xlabel("Experiment")
        pylab.ylabel("Bandwidth (GBit/s)")
        offset = 0
        # for rect in rects:
        #     curr_offset = rect.get_height()
        #     if curr_offset > offset:
        #         offset = curr_offset
        for rect in rects:
            height = rect.get_height()
            pylab.text(rect.get_x() + rect.get_width()/2., height,
                    "{}".format(round(height, 2)),
                    ha='center', va='bottom')
        # pylab.show()
        if save_separate:
            pylab.savefig("{}.pdf".format(experiment))
            pylab.savefig("{}.png".format(experiment))
        width += column_width
        index = index + 1
        pylab.legend()
    tick_x = range(4)
    tick_label=["No-SGX",
                "Simulation",
                "HW-Debug",
                "HW-Prerelease"]
    all_columns_width = (len(EXPERIMENT_DATA)*column_width)/2
    print all_columns_width
    for x in tick_x:
        tick_x[x] = tick_x[x] + all_columns_width
    pylab.xticks(tick_x, tick_label)
    if not save_separate:
        pylab.savefig("{}.pdf".format(experiment))
        pylab.savefig("{}.png".format(experiment))
    pylab.show()

if __name__ == "__main__":
    EXPERIMENT_DATA = {}
    # EXPERIMENT_DATA_FILES = {}
    for timestamp in TIMESTAMP_LIST:
        for fileentry in FILE_LIST:
            filename = "{}-{}.csv".format(fileentry, timestamp)
            filepath = "{}/{}".format(timestamp, filename)
            if not os.path.isfile(filepath):
                print "Cannot open file: {}".format(filepath)
                sys.exit(-1)
            current_file = open(filepath)
            # EXPERIMENT_DATA_FILES[timestamp][fileentry] = current_file
            current_reader = csv.DictReader(current_file)
            experiment = ""
            for experiment_key in EXPERIMENT_MAP:
                if filename.startswith(experiment_key):
                    experiment = experiment_key
                    break
            if experiment == "":
                print "Encountered unlisted experiment"
                sys.exit(-1)
            if timestamp not in EXPERIMENT_DATA:
                EXPERIMENT_DATA[timestamp] = {}
            if experiment not in EXPERIMENT_DATA[timestamp]:
                EXPERIMENT_DATA[timestamp][experiment] = {}
            print "Filepath 1: {}".format(filepath)
            EXPERIMENT_DATA[timestamp]["filepath"] = filepath
            EXPERIMENT_DATA[timestamp][experiment][fileentry] = list(current_reader)
    plot_experiment(EXPERIMENT_DATA, "experiment-basic", keep_figure=True, colors=['orange','green'], legend_labels=["Libraries", "No Libraries"])

    plot_experiment(EXPERIMENT_DATA, "experiment-sha256", keep_figure=True, colors=['orange','green'], legend_labels=["Libraries", "No Libraries"])
