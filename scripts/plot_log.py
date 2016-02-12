#!/usr/bin/python2
"""Reads and plots the data in a simulation log protocol buffer file."""

import argparse
try:
    import simulationresults_pb2
except ImportError:
    print("Run 'make' in the directory one level above this one before using this script.")
    exit(1)
import matplotlib.pyplot as plt

def datagetter(attrname):
    def datagetterfunc(points, sender_data_id):
        return [getattr(point.sender_data[sender_data_id], attrname) for point in points]
    return datagetterfunc

def make_plot(attrname, times, num_senders, unit=None, valuesfunc=None):
    figfilename = "{}.png".format(attrname)
    print("Generating {}...".format(figfilename))
    plt.figure()
    if valuesfunc is None:
        valuesfunc = datagetter(attrname)
    for i in range(num_senders):
        values = valuesfunc(run_data.point, i)
        plt.plot(times, values, label="sender {:d}".format(i))
    pretty_name = attrname.capitalize().replace("_", " ")
    plt.title(pretty_name)
    plt.xlabel("Time (s)")
    plt.ylabel(pretty_name + "({})".format(unit) if unit else "")
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
    ("average_throughput", None),
    ("average_delay", "ms"),
    ("sending_duration", "ms"),
    ("packets_received", None),
    ("total_delay", "ms"),
]

def diffgetter(numerator, denominator):
    def diffgetterfunc(points, sender_data_id):
        numerator_values = datagetter(numerator)(points, sender_data_id)
        denominator_values = datagetter(denominator)(points, sender_data_id)
        data = [0.0]
        for i in xrange(1, len(points)):
            n = numerator_values[i] - numerator_values[i-1]
            d = denominator_values[i] - denominator_values[i-1]
            if n == 0 and d == 0:
                data.append(data[-1])
            elif d == 0:
                data.append(0.0)
            else:
                data.append(n/d)
        return data
    return diffgetterfunc

for run_data in data.run_data:
    for attrname, unit in PLOTS:
        times = [point.seconds for point in run_data.point]
        num_senders = run_data.config.num_senders
        make_plot(attrname, times, num_senders, unit=unit)

    make_plot("throughput", times, num_senders, valuesfunc=diffgetter("packets_received", "sending_duration"))
    make_plot("delay", times, num_senders, valuesfunc=diffgetter("total_delay", "packets_received"))