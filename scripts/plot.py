#!/usr/bin/python3
"Runs rat-runner enough times to generate a plot, and plots the result."

import os
import argparse
import subprocess
import re
import time
import numpy as np
from warnings import warn
from itertools import chain

use_color = True

HLINE1 = "-" * 80
HLINE2 = "=" * 80
ROOTDIR = os.path.dirname(os.path.dirname(__file__))
RATRUNNERCMD = os.path.join(ROOTDIR, "src", "rat-runner")
SENDER_REGEX = re.compile("^sender: \[tp=(-?\d+(?:\.\d+)?), del=(-?\d+(?:\.\d+)?)\]$", re.MULTILINE)
NORM_SCORE_REGEX = re.compile("^normalized_score = (-?\d+(?:\.\d+)?)$", re.MULTILINE)
NORM_SCORE_GROUP = 1

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
            "This was the console output for the command:",
            "    " + " ".join(command),
            HLINE2,
            ""
        ])
        writefile.write(output)

    return output

def run_ratrunner(remyccfilename, parameters, console_file=None):
    """Runs rat-runner with the given parameters and returns the result.
    `remyccfilename` is the name of the RemyCC to test.
    `parameters` is a dict of parameters."""
    defaults = dict(nsenders=2, link_ppt=1.0, delay=100.0, mean_on=5000.0, mean_off=5000.0)
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
    ]

    return run_command(command, show=False, writefile=console_file, includestderr=True)

def parse_ratrunner_output(result):
    """Returns the normalized score, throughputs and delays from the rat-runnner script."""

    norm_matches = NORM_SCORE_REGEX.findall(result)
    if len(norm_matches) != 1:
        print(result)
        raise RuntimeError("Found no or duplicate normalized scores in this output.".format(NORM_SCORE_REGEX.pattern))
    norm_score = float(norm_matches[0])

    sender_matches = SENDER_REGEX.findall(result)
    sender_data = [map(float, x) for x in sender_matches] # [[throughput, delay], [throughput, delay], ...]
    if len(sender_data) == 0:
        print(result)
        warn("No senders found in this output.")

    return norm_score, sender_data

def compute_normalized_score(remyccfilename, parameters, console_dir=None):
    kwargs = {}
    if console_dir:
        filename = "ratrunner-{remycc}-{nsenders:d}-{link_ppt:f}-{delay:f}-{mean_on:f}-{mean_off:f}.out".format(
                remycc=remyccfilename, **parameters)
        filename = os.path.join(console_dir, filename)
        kwargs["console_file"] = open(filename)

    output = run_ratrunner(remyccfilename, parameters, **kwargs)

    if "console_file" in kwargs:
        kwargs["console_file"].close()

    norm_score = parse_ratrunner_output(output)
    return norm_score

def generate_data_and_plot(remyccfilename, link_ppt_range, parameters, console_dir=None, results_dir=None, plots_dir=None):
    if results_dir:
        results_filename = "results-{remycc}-{nsenders:d}-{delay:f}-{mean_on:f}-{mean_off:f}.csv".format(
                remycc=remyccfilename, **parameters)
        results_file = open(os.path.join(results_dir, results_filename))
        results_csv = csv.writer(results_file)

    for link_ppt in link_ppt_range:
        parameters["link_ppt"] = link_ppt
        norm_score, sender_data = compute_normalized_score(remyccfilename, parameters, console_dir)
        sender_numbers = chain(*sender_data)
        if results_dir:
            results_csv.writerow([link_ppt, norm_score] + list(sender_numbers))

    results_csv.close()

def log_arguments(argsfile, args):
    argsfile.write("Started at " + time.asctime() + "\n")
    argsfile.write("Git commit: " + subprocess.check_output(['git', 'rev-parse', 'HEAD']) + "\n")

    argsfile.write("\nArguments:\n")
    for key, value in vars(args):
        argsfile.write("{:>20} = {}".format(key, value))
    argsfile.close()

# Script starts here

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("remycc", nargs="+", type=str,
    help="RemyCC file(s) to run")
parser.add_argument("-n", "--num-points", type=int, default=1000,
    help="Number of points to plot")
parser.add_argument("-s", "--nsenders", type=int, default=2,
    help="Number of senders")
parser.add_argument("-l", "--link-ppt", type=float, default=[1.0, 1000.0], nargs=2, metavar="PPMS",
    help="Link packets per millisecond, range to test, first argument is low, second is high")
parser.add_argument("-d", "--delay", type=float, default=150.0,
    help="Delay (milliseconds)")
parser.add_argument("-q", "--mean-on", type=float, default=1000.0,
    help="Mean on duration (milliseconds)")
parser.add_argument("-w", "--mean-off", type=float, default=1000.0,
    help="Mean off duration (milliseconds)")
parser.add_argument("--dry-run", action="store_true", default=False,
    help="Print commands, don't run them.")
parser.add_argument("-r", "--results-dir", type=str, default="results",
    help="Directory to place output files in, will be created if it doesn't exist. (default: plot-output-<timestamp>)")
parser.add_argument("--no-console-output-files", action="store_false", default=True, dest="console_output_files",
    help="Don't generate console output files")
args = parser.parse_args()

results_dirname = args.results_dir
output_dirname = os.path.join(results_dirname, "results-" + time.strftime("%Y%m%d-%H%M%S"))
console_dirname = os.path.join(output_dirname, "outputs")
results_dirname = os.path.join(output_dirname, "results")
plots_dirname = os.path.join(output_dirname, "plots")

args_file = open(os.path.join(output_dirname, "args.txt"))
log_arguments(args_file, args)
args_file.close()

link_ppt_range = np.logspace(np.log10(args.link_ppt[0]), np.log10(args.link_ppt[1]), args.num_points)
parameter_keys = ["nsenders", "delay", "mean_on", "mean_off"]
parameters = {key: getattr(args, key) for key in parameter_keys}

for remyccfile in args.remycc:
    generate_data_and_plot(remyccfile, link_ppt_range, parameters, console_dirname, results_dirname, plots_dirname)
