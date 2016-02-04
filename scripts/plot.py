#!/usr/bin/python3
"""Runs rat-runner enough times to generate a plot, and plots the result.
This script requires Python 3."""

import sys
import os
import argparse
import subprocess
import re
import time
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import csv
import json
from math import log2
from warnings import warn
from itertools import chain
from socket import gethostname

use_color = True
DEFAULT_RESULTS_DIR = "results"

HLINE1 = "-" * 80 + "\n"
HLINE2 = "=" * 80 + "\n"
ROOTDIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RATRUNNERCMD = os.path.join(ROOTDIR, "src", "rat-runner")
SENDER_REGEX = re.compile("^sender: \[tp=(-?\d+(?:\.\d+)?), del=(-?\d+(?:\.\d+)?)\]$", re.MULTILINE)
NORM_SCORE_REGEX = re.compile("^normalized_score = (-?\d+(?:\.\d+)?)$", re.MULTILINE)
LINK_PPT_PRIOR_REGEX = re.compile("^link_packets_per_ms\s+\{\n\s+low: (-?\d+(?:\.\d+)?)\n\s+high: (-?\d+(?:\.\d+)?)$", re.MULTILINE)
REMYCCSPEC_REGEX = re.compile("^([\w/]+)\.\{(\d+)\:(\d+)(?:\:(\d+))?\}$")
NORM_SCORE_GROUP = 1
LINK_PPT_TO_MBPS_CONVERSION = 10

def print_command(command):
    message = "$ " + " ".join(command)
    if use_color:
        message = "\033[1;36m" + message + "\033[0m"
    print(message)

def run_command(command, show=True, writefile=None, includestderr=True):
    """Runs a command returns its output.
    Raises subprocess.CalledProcessError if the command returned a non-zero exit code.
    If `show` is True, also writes the output to stdout.
    If `writefile` is True, also writes the output to the file object `writefile`.
    If `includestderr` is True, stderr from the called process is also captured."""
    kwargs = {}
    if includestderr:
        kwargs['stderr'] = subprocess.STDOUT

    output = subprocess.check_output(command, **kwargs)
    output = output.decode()

    if show:
        print_command(command)
        sys.stdout.write(output)
        sys.stdout.flush()

    if writefile:
        writefile.writelines([
            HLINE2,
            "This was the console output for the command:\n",
            "    " + " ".join(command) + "\n",
            HLINE2,
            "\n"
        ])
        writefile.write(output)

    return output

def run_ratrunner(remyccfilename, parameters, console_file=None):
    """Runs rat-runner with the given parameters and returns the result.
    `remyccfilename` is the name of the RemyCC to test.
    `parameters` is a dict of parameters.
    If `console_file` is specified, it must be a file object, and the output will be written to it."""
    defaults = dict(nsenders=2, link_ppt=1.0, delay=100.0, mean_on=5000.0, mean_off=5000.0, buffer_size="inf")
    unrecognized_parameters = [k for k in parameters if k not in defaults]
    if unrecognized_parameters:
        warn("Unrecognized parameters: {}".format(unrecognized_parameters))
    defaults.update(parameters)
    parameters = defaults

    command = [
        RATRUNNERCMD,
        "if={:s}".format(remyccfilename),
        "nsrc={:d}".format(parameters["nsenders"]),
        "link={:f}".format(parameters["link_ppt"]),
        "rtt={:f}".format(parameters["delay"]),
        "on={:f}".format(parameters["mean_on"]),
        "off={:f}".format(parameters["mean_off"]),
        "buf={:s}".format(parameters["buffer_size"]),
    ]

    return run_command(command, show=False, writefile=console_file, includestderr=True)

def parse_ratrunner_output(result):
    """Parses the output of rat-runner to extract the normalized score, and
    sender throughputs and delays. Returns a 3-tuple. The first element is the
    normalized score from the rat-runnner script. The second element is a list
    of lists, one list for each sender, each inner list having two elements,
    [throughput, delay]. The third element is a list [low, high], being
    the link rate range under "prior assumptions"."""

    norm_matches = NORM_SCORE_REGEX.findall(result)
    if len(norm_matches) != 1:
        print(result)
        raise RuntimeError("Found no or duplicate normalized scores in this output.")
    norm_score = float(norm_matches[0])

    sender_matches = SENDER_REGEX.findall(result)
    sender_data = [map(float, x) for x in sender_matches] # [[throughput, delay], [throughput, delay], ...]
    if len(sender_data) == 0:
        print(result)
        warn("No senders found in this output.")

    link_ppt_prior_matches = LINK_PPT_PRIOR_REGEX.findall(result)
    if len(link_ppt_prior_matches) != 1:
        print(result)
        raise RuntimeError("Found no or duplicate link packets per ms prior assumptions in this output.")
    link_ppt_prior = tuple(map(float, link_ppt_prior_matches[0]))

    # Divide norm_score the number of senders (rat-runner returns the sum)
    norm_score /= len(sender_data)

    return norm_score, sender_data, link_ppt_prior

def compute_normalized_score(remyccfilename, parameters, console_dir=None):
    """Runs rat-runner on the given RemyCC `remyccfilename` and with the given
    parameters, and returns the normalized score and sender throughputs and delays.
    `parameters` is a dict of parameters.
    """
    kwargs = {}
    if console_dir:
        filename = "ratrunner-{remycc}-{link_ppt:f}.out".format(
                remycc=os.path.basename(remyccfilename), **parameters)
        filename = os.path.join(console_dir, filename)
        kwargs["console_file"] = open(filename, "w")

    output = run_ratrunner(remyccfilename, parameters, **kwargs)

    if "console_file" in kwargs:
        kwargs["console_file"].close()

    return parse_ratrunner_output(output)

def add_plot(axes, link_speeds, norm_scores, **kwargs):
    """Adds a plot for the given link-packets-per-ms `link_ppts` and normalized
    scores `norm_scores` to the `axes`."""
    return plt.semilogx(link_speeds, norm_scores, axes=axes, **kwargs)

def generate_data_and_plot(remyccfilename, link_ppt_range, parameters, console_dir=None, data_dir=None, axes=None):
    """For a given RemyCC `remyccfilename`, runs rat-runner once on each link
    speed in `link_ppt_range` (each being specified in packets per millisecond),
    with other parameters as specified in the dict `parameters`. Returns the
    link rates specified under "prior assumptions".

    If `console_dir` is specified, the outputs of each rat-runner run are stored in
        a file (each) in that directory.
    If `data_dir` is specified, data from the run is written to a (single) file in
        that directory.
    If `axes` is specified, a plot will be added to those axes.
    """
    if data_dir:
        data_filename = "data-{remycc}.csv".format(
                remycc=os.path.basename(remyccfilename))
        data_file = open(os.path.join(data_dir, data_filename), "w")
        data_csv = csv.writer(data_file)

    norm_scores = []
    npoints = len(link_ppt_range)

    for i, link_ppt in enumerate(link_ppt_range, start=1):
        parameters["link_ppt"] = link_ppt
        print("\033[KGenerating score for if={:s}, link={:f} ({:d} of {:d})...".format(
                    remyccfilename, link_ppt, i, npoints),
                    file=sys.stderr, end='\r', flush=True)
        norm_score, sender_data, link_ppt_prior = compute_normalized_score(remyccfilename, parameters, console_dir)
        norm_scores.append(norm_score)
        sender_numbers = chain(*sender_data)
        if data_dir:
            data_csv.writerow([link_ppt, norm_score] + list(sender_numbers))

    if axes:
        print("\033[KPlotting for file {}...".format(remyccfilename), file=sys.stderr, end='\r', flush=True)
        link_speeds = [LINK_PPT_TO_MBPS_CONVERSION*l for l in link_ppt_range]
        add_plot(axes, link_speeds, norm_scores, label=remyccfilename)

    data_file.close()

    print("\033[KDone file {}.".format(remyccfilename), file=sys.stderr)
    sys.stderr.flush()

    return link_ppt_prior

def plot_from_original_file(datafilename, axes):
    """Plots data from the file `datafile` to the axes `axes`."""
    link_speeds = []
    norm_scores = []
    try:
        datafile = open(datafilename)
        for line in datafile:
            row = line.split() # at whitespace, treat consecutive spaces as one
            row = [float(x) for x in row]
            link_speeds.append(row[0])
            norm_score = log2(row[1]/row[0]) - log2(row[2]/150)
            norm_scores.append(norm_score)
        datafile.close()
        add_plot(axes, link_speeds, norm_scores, label=datafilename)
    except (IOError, ValueError) as e:
        print("Error plotting from {}: {}".format(datafilename, e), file=sys.stderr)

def log_arguments(argsfile, args):
    jsondict = {
        "start-time": time.asctime(),
        "machine-name": gethostname(),
        "git": {
            "commit": subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode().strip(),
            "branch": subprocess.check_output(['git', 'symbolic-ref', '--short', '--quiet', 'HEAD']).decode().strip(),
        },
        "args": vars(args)
    }
    json.dump(jsondict, argsfile, indent=2, sort_keys=True)

def make_results_dir(dirname):
    if dirname is None:
        dirname = os.path.join(DEFAULT_RESULTS_DIR, "results" + time.strftime("%Y%m%d-%H%M%S"))
    if os.path.islink("last"):
        os.unlink("last")
    os.symlink(dirname, "last")
    if not os.path.exists(dirname):
        os.makedirs(dirname, exist_ok=True)
    return dirname

def generate_remyccs_list(specs):
    result = []
    for spec in specs:
        match = REMYCCSPEC_REGEX.match(spec)
        if not match:
            result.append(spec)
        else:
            name = match.group(1)
            start = int(match.group(2))
            if match.group(4) is None:
                stop = int(match.group(3))
                step = 1
            else:
                stop = int(match.group(4))
                step = int(match.group(3))
            result.extend("{name}.{index:d}".format(name=name, index=index) for index in range(start, stop+1, step))
    return result




# Script starts here

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("remycc", nargs="*", type=str,
    help="RemyCC file(s) to run, can also use e.g. name.[5:5:30] to do name.5, name.10, ..., name.30")
parser.add_argument("-n", "--num-points", type=int, default=1000,
    help="Number of points to plot")
parser.add_argument("-s", "--nsenders", type=int, default=2,
    help="Number of senders")
parser.add_argument("-l", "--link-ppt", type=float, default=[0.1, 100.0], nargs=2, metavar="PPMS",
    help="Link packets per millisecond, range to test, first argument is low, second is high")
parser.add_argument("-d", "--delay", type=float, default=150.0,
    help="Delay (milliseconds)")
parser.add_argument("-q", "--mean-on", type=float, default=1000.0,
    help="Mean on duration (milliseconds)")
parser.add_argument("-w", "--mean-off", type=float, default=1000.0,
    help="Mean off duration (milliseconds)")
parser.add_argument("-b", "--buffer", type=str, default="inf",
    help="Buffer size, a number or 'inf' for infinite buffers")
parser.add_argument("--dry-run", action="store_true", default=False,
    help="Print commands, don't run them.")
parser.add_argument("-r", "--results-dir", type=str, default=None,
    help="Directory to place output files in.")
parser.add_argument("--no-console-output-files", action="store_false", default=True, dest="console_output_files",
    help="Don't generate console output files")
parser.add_argument("--originals", type=str, default="originals",
    help="Directory in which to look for original data files to add to plot.")
args = parser.parse_args()

# Sanity-check arguments, warn user say they can stop things early
if not os.path.isdir(args.originals):
    warn("The path {} is not a directory.".format(args.originals))
if len(args.remycc) == 0:
    warn("No RemyCC files specified, plotting only originals.")

# Make directories
results_dirname = make_results_dir(args.results_dir)
console_dirname = os.path.join(results_dirname, "outputs")
data_dirname = os.path.join(results_dirname, "data")
plots_dirname = os.path.join(results_dirname, "plots")

os.makedirs(console_dirname, exist_ok=True)
os.makedirs(data_dirname, exist_ok=True)
os.makedirs(plots_dirname, exist_ok=True)

# Log arguments
args_file = open(os.path.join(results_dirname, "args.json"), "w")
log_arguments(args_file, args)
args_file.close()

# Generate parameters
link_ppt_range = np.logspace(np.log10(args.link_ppt[0]), np.log10(args.link_ppt[1]), args.num_points)
parameter_keys = ["nsenders", "delay", "mean_on", "mean_off"]
parameters = {key: getattr(args, key) for key in parameter_keys}

remyccfiles = generate_remyccs_list(args.remycc)

ax = plt.axes()

# Generate data and plots (the main part)
link_ppt_priors = []
for remyccfile in remyccfiles:
    link_ppt_prior = generate_data_and_plot(remyccfile, link_ppt_range, parameters, console_dirname, data_dirname, ax)
    link_ppt_priors.append(link_ppt_prior)

# Add the remaining plots
if os.path.isdir(args.originals):
    for filename in os.listdir(args.originals):
        path = os.path.join(args.originals, filename)
        if not os.path.isfile(path):
            warn("Skipping {}: not a file".format(path))
        print("Plotting file {}...".format(path), file=sys.stderr)
        plot_from_original_file(path, ax)

# If there were RemyCCs involved, check they all had the same training range.
# If they did, highlight the range on the graph.
if len(link_ppt_priors) > 0:
    if not all([l == link_ppt_priors[0] for l in link_ppt_priors]):
        print(set(link_ppt_priors))
        warn("Not all RemyCCs had the same training range.")
    else:
        link_ppt_low, link_ppt_high = link_ppt_priors[0]
        plt.axvspan(LINK_PPT_TO_MBPS_CONVERSION*link_ppt_low, LINK_PPT_TO_MBPS_CONVERSION*link_ppt_high,
                linewidth=0.0, facecolor="0.2", alpha=0.2)

# Make plot pretty and save
plot_filename = "link_ppt"
ax.set_xlabel("link speed (Mbps)")
ax.set_ylabel("normalized score")
box = ax.get_position()
ax.legend(loc='lower center', bbox_to_anchor=(0.5, 1))
plt.savefig(os.path.join(plots_dirname, "{:s}.png".format(plot_filename)), format="png", bbox_inches="tight")
plt.savefig(os.path.join(plots_dirname, "{:s}.pdf".format(plot_filename)), format="pdf", bbox_inches="tight")
