#!/usr/bin/python3
"Runs rat-runner enough times to generate a plot, and plots the result."

import os.path
import argparse
import subprocess
import re
import numpy as np

use_color = True

ROOTDIR = os.path.dirname(os.path.dirname(__file__))
RATRUNNERCMD = os.path.join(ROOTDIR, "src", "rat-runner")
NORM_SCORE_REGEX = re.compile("^\s*normalized_score = (-?\d+(\.\d+)?)\s*$", re.MULTILINE)
NORM_SCORE_GROUP = 1

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("remycc", nargs="+", type=str,
    help="RemyCC file(s) to run")
parser.add_argument("-n", "--num-points", type=int, default=100,
    help="Number of points to plot")
parser.add_argument("-s", "--nsenders", type=int, default=2,
    help="Number of senders")
parser.add_argument("-l", "--link-ppt", type=float, default=[-1.0, 1.0], nargs=2, metavar="PPMS",
    help="Link packets per millisecond, range to test, first argument is low, second is high")
parser.add_argument("-d", "--delay", type=float, default=100.0,
    help="Delay (milliseconds)")
parser.add_argument("-q", "--mean-on", type=float, default=5000.0,
    help="Mean on duration (milliseconds)")
parser.add_argument("-w", "--mean-off", type=float, default=5000.0,
    help="Mean off duration (milliseconds)")
parser.add_argument("--dry-run", action="store_true", default=False,
    help="Print commands, don't run them.")
parser.add_argument("-o", "--output-file", type=argparse.FileType('w'), default=open('output.txt', 'w'),
    help="Output file to write results to")
args = parser.parse_args()

def print_command(command):
    message = "$ " + " ".join(command)
    if use_color:
        message = "\033[1;36m" + message + "\033[0m"
    print(message)

def get_output_from_command(command, show=True):
    """Runs a command, returns its output and optionally
    Raises subprocess.CalledProcessError if the command returned a non-zero exit code."""
    print_command(command)
    output = subprocess.check_output(command, stderr=subprocess.STDOUT)
    output = output.decode()
    if show:
        sys.stdout.write(output)
        sys.stdout.flush()
    return output

def run_ratrunner(remyccfilename, nsenders=2, link_ppt=1.0, delay=100.0, mean_on=5000.0, mean_off=5000.0):
    """Runs rat-runner with the given parameters and returns the result."""
    command = [
        RATRUNNERCMD,
        "if={:s}".format(remyccfilename),
        "nsrc={:d}".format(nsenders),
        "link={:f}".format(link_ppt),
        "rtt={:f}".format(delay),
        "on={:f}".format(mean_on),
        "off={:f}".format(mean_off),
    ]
    return get_output_from_command(command, show=False)

def parse_ratrunner_output(result):
    """Returns the normalized score from the rat-runnner script."""
    m = NORM_SCORE_REGEX.search(result)
    if not m:
        print(result)
        raise RuntimeError("Could not find a match for {} in this output.".format(NORM_SCORE_REGEX.pattern))

    norm_score = m.group(NORM_SCORE_GROUP)
    return float(norm_score)

link_ppt_range = np.logspace(args.link_ppt[0], args.link_ppt[1], args.num_points)

for remyccfile in args.remycc:
    for link_ppt in link_ppt_range:
        output = run_ratrunner(remyccfile, args.nsenders, link_ppt, args.delay, args.mean_on, args.mean_off)
        norm_score = parse_ratrunner_output(output)
        line = "{:f},{:f}\n".format(link_ppt, norm_score)
        print(line)
        args.output_file.write(line)
