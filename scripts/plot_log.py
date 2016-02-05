#!/usr/bin/python2
"""Reads and plots the data in a simulation log protocol buffer file."""

import argparse
try:
    import simulationresults_pb2
except ImportError:
    print("Run 'make' in the directory one level above this one before using this script.")
    exit(1)
import matplotlib.pyplot as plt

def make_plot(attrname, ylabel, times, num_senders):
    figfilename = "{}.png".format(attrname)
    print("Generating {}...".format(figfilename))
    plt.figure()
    for i in range(num_senders):
        values = [getattr(point.sender_data[i], attrname) for point in run_data.point]
        plt.plot(times, values, label="sender {:d}".format(i))
    pretty_name = attrname.capitalize().replace("_", " ")
    plt.title(pretty_name)
    plt.xlabel("Time (s)")
    plt.ylabel(ylabel)
    plt.xlim([min(times), max(times)])
    plt.legend(loc='lower right')
    plt.savefig(figfilename, format="png", bbox_inches="tight")
    plt.close()

def read_log_file(f):
    data = simulationresults_pb2.SimulationsData()
    data.ParseFromString(f.read())
    return data

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("logfile", type=argparse.FileType('rb'),
    help="Log file to plot")
args = parser.parse_args()

logfile = args.logfile
data = read_log_file(logfile)
logfile.close()

PLOTS = [
    ("average_throughput_since_start", "Throughput"),
    ("average_delay_since_start", "Delay (ms)"),
]

for run_data in data.run_data:
    for attrname, ylabel in PLOTS:
        times = [point.seconds for point in run_data.point]
        num_senders = run_data.config.num_senders
        make_plot(attrname, ylabel, times, num_senders)