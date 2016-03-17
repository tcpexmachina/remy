#!/usr/bin/python3
"""Runs sender-runner enough times to generate a plot, and plots the result.
This script requires Python 3."""

import sys
import os
import shutil
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

from remy_tool_runner import SenderRunnerRunner
import utils

use_color = True
DEFAULT_RESULTS_DIR = "results"
LAST_RESULTS_SYMLINK = "last"

REMYCCSPEC_REGEX = re.compile("^([\w/]+)\.\{(\d+)\:(\d+)(?:\:(\d+))?\}$")
LINK_PPT_TO_MBPS_CONVERSION = 10


class BaseRemyCCPerformancePlotGenerator:
    """Base class for generating and plotting data for a RemyCC.
    Subclasses should provide a constructor and a `get_statistics` method.

    `link_ppt_range` is an iterable of link speeds to plot.
    `data_dir`, optional, is a directory in which a file for each `generate()`
        call will be written. If omitted, data files will not be generated.
    `axes`, optional, is a `matplotlib.Axes` object to which plots will be added.
        If omitted, plots will not be generated.

    `link_ppt_prior` may be specified, in which case it should be a value
        returned by the `get_link_ppt_prior()` method of another generator. This
        is useful for the daisy-chaining link_ppt_prior state of multiple
        generators.
    """

    SENDER_REGEX = re.compile("^sender: \[tp=(-?\d+(?:\.\d+)?), del=(-?\d+(?:\.\d+)?)\]$", re.MULTILINE)
    NORM_SCORE_REGEX = re.compile("^normalized_score = (-?\d+(?:\.\d+)?)$", re.MULTILINE)
    LINK_PPT_PRIOR_REGEX = re.compile("^link_packets_per_ms\s+\{\n\s+low: (-?\d+(?:\.\d+)?)\n\s+high: (-?\d+(?:\.\d+)?)$", re.MULTILINE)

    def __init__(self, link_ppt_range, **kwargs):
        self.link_ppt_range = link_ppt_range
        self.data_dir = kwargs.pop("data_dir", None)
        self.axes = kwargs.pop("axes", None)
        self._link_ppt_priors = kwargs.pop("link_ppt_priors", [])
        self._progress_end_char = kwargs.pop("progress_end_char", '\r')
        if self._link_ppt_priors is None:
            self._link_ppt_priors = []

        if len(kwargs) > 0:
            raise TypeError("Unrecognized arguments: " + ", ".join(kwargs.keys()))

    def get_statistics(self, remyccfilename, link_ppt):
        """Must be implemented by subclasses. Should, for the given RemyCC and
        link speed, return a 3-tuple `(norm_score, sender_data,
        link_ppt_prior)`, where `norm_score` is the normalized score,
        `sender_data` is a list of `[throughput, delay] lists, and
        `link_ppt_prior` is a 2-tuple `(low, high)` being the link speed range
        on which the RemyCC was trained.
        """
        raise NotImplementedError("subclasses of BaseRemyCCPerformancePlotGenerator must implement get_statistics")

    def get_data_file(self, remyccfilename):
        if self.data_dir:
            data_filename = "data-{remycc}.csv".format(
                    remycc=os.path.basename(remyccfilename))
            data_file = open(os.path.join(self.data_dir, data_filename), "w")
            return data_file
        else:
            return None

    def generate(self, remyccfilename):
        data_file = self.get_data_file(remyccfilename)
        if data_file:
            data_csv = csv.writer(data_file)

        norm_scores = []
        npoints = len(self.link_ppt_range)

        for i, link_ppt in enumerate(link_ppt_range, start=1):
            print("\033[KGenerating score for if={:s}, link={:f} ({:d} of {:d})...".format(
                        remyccfilename, link_ppt, i, npoints),
                        file=sys.stderr, end=self._progress_end_char, flush=True)
            norm_score, sender_data, link_ppt_prior = self.get_statistics(remyccfilename, link_ppt)
            norm_scores.append(norm_score)
            sender_numbers = chain(*sender_data)
            if data_file:
                data_csv.writerow([link_ppt, norm_score] + list(sender_numbers))
            self._update_link_ppt_prior(link_ppt_prior)

        if data_file:
            data_file.close()

        if self.axes:
            print("\033[KPlotting for file {}...".format(remyccfilename), file=sys.stderr,
                    end=self._progress_end_char, flush=True)
            link_speeds = [LINK_PPT_TO_MBPS_CONVERSION*l for l in link_ppt_range]
            add_plot(self.axes, link_speeds, norm_scores, label=remyccfilename)

        print("\033[KDone file {}.".format(remyccfilename), file=sys.stderr)
        sys.stderr.flush()

    @classmethod
    def parse_senderrunner_output(cls, result):
        """Parses the output of sender-runner to extract the normalized score, and
        sender throughputs and delays. Returns a 3-tuple. The first element is the
        normalized score from the sender-runner script. The second element is a list
        of lists, one list for each sender, each inner list having two elements,
        [throughput, delay]. The third element is a list [low, high], being
        the link rate range under "prior assumptions"."""

        norm_matches = cls.NORM_SCORE_REGEX.findall(result)
        if len(norm_matches) != 1:
            print(result)
            raise RuntimeError("Found no or duplicate normalized scores in this output.")
        norm_score = float(norm_matches[0])

        sender_matches = cls.SENDER_REGEX.findall(result)
        sender_data = [map(float, x) for x in sender_matches] # [[throughput, delay], [throughput, delay], ...]
        if len(sender_data) == 0:
            print(result)
            warn("No senders found in this output.")

        link_ppt_prior_matches = cls.LINK_PPT_PRIOR_REGEX.findall(result)
        if len(link_ppt_prior_matches) != 1:
            print(result)
            raise RuntimeError("Found no or duplicate link packets per ms prior assumptions in this output.")
        link_ppt_prior = tuple(map(float, link_ppt_prior_matches[0]))

        # Divide norm_score the number of senders (sender-runner returns the sum)
        norm_score /= len(sender_data)

        return norm_score, sender_data, link_ppt_prior

    def _update_link_ppt_prior(self, link_ppt_prior):
        if link_ppt_prior in self._link_ppt_priors:
            return
        self._link_ppt_priors.append(link_ppt_prior)

    def get_link_ppt_priors(self):
        """If the prior optimizion settings for each file on which generate()
        has been called so far are the same, returns that setting. Otherwise,
        returns None."""
        return self._link_ppt_priors


class SenderRunnerFilesMixin:
    """Provides functionality relating to sender-runner output files. Subclass
    constructors must provide a `console_dir` attribute to objects of the class.
    This may be None; if so, this `get_console_filename` returns None."""

    def get_console_filename(self, remyccfilename, link_ppt):
        if self.console_dir is None:
            return None

        filename = "senderrunner-{remycc}-{link_ppt:f}.out".format(
                remycc=os.path.basename(remyccfilename), link_ppt=link_ppt)
        filename = os.path.join(self.console_dir, filename)
        return filename


class SenderRunnerRemyCCPerformancePlotGenerator(SenderRunnerFilesMixin, BaseRemyCCPerformancePlotGenerator):
    """Generates data and plots by invoking sender-runner to generate a score for
    every point. In addition to the arguments taken by BaseRemyCCPerformancePlotGenerator:

    `parameters` is a dictionary of parameters to pass to sender-runner.
    `console_dir`, optional, is the directory to which sender-runner outputs will be written,
        one file per data point.
    """

    def __init__(self, link_ppt_range, parameters, **kwargs):
        senderrunnercmd = kwargs.pop("senderrunnercmd")
        if senderrunnercmd is not None:
            parameters["command"] = senderrunnercmd
        self.senderrunner = SenderRunnerRunner(**parameters)
        self.console_dir = kwargs.pop("console_dir", None)
        super(SenderRunnerRemyCCPerformancePlotGenerator, self).__init__(link_ppt_range, **kwargs)

    def get_statistics(self, remyccfilename, link_ppt):
        """Runs sender-runner on the given RemyCC `remyccfilename` and with the given
        parameters, and returns the normalized score and sender throughputs and delays.
        """
        outfile = self.get_console_filename(remyccfilename, link_ppt)
        output = self.senderrunner.run(remyccfilename, {'link_ppt': link_ppt}, outfile=outfile)
        return self.parse_senderrunner_output(output)


class OutputsDirectoryRemyCCPerformancePlotGenerator(SenderRunnerFilesMixin, BaseRemyCCPerformancePlotGenerator):
    """Generates data and plots by parsing outputs from an existing directory.
    In addition to the arguments taken by BaseRemyCCPerformancePlotGenerator:

    `console_dir` is the directory in which existing outputs are found. The
    relevant outputs files must all exist with the correct names. If any don't,
    `generate()` will print a warning and skip the point.
    """

    def __init__(self, link_ppt_range, console_dir, **kwargs):
        self.console_dir = console_dir
        super(OutputsDirectoryRemyCCPerformancePlotGenerator, self).__init__(link_ppt_range, **kwargs)

    def get_statistics(self, remyccfilename, link_ppt):
        filename = self.get_console_filename(remyccfilename, link_ppt)
        f = open(filename, "r")
        contents = f.read()
        f.close()
        return self.parse_senderrunner_output(contents)


def add_plot(axes, link_speeds, norm_scores, **kwargs):
    """Adds a plot for the given link-packets-per-ms `link_ppts` and normalized
    scores `norm_scores` to the `axes`."""
    return plt.semilogx(link_speeds, norm_scores, axes=axes, **kwargs)

def process_replot_argument(replot_dir, results_dir):
    """Reads the args.json file in a results directory, copies it to an
    appropriate location in the current results directory and returns the link
    speed range and a list of RemyCC files."""
    argsfilename = os.path.join(replot_dir, "args.json")
    argsfile = open(argsfilename)
    jsondict = json.load(argsfile)
    argsfile.close()
    args = jsondict["args"]
    remyccs = args["remycc"]
    link_ppt_range = np.logspace(np.log10(args["link_ppt"][0]), np.log10(args["link_ppt"][1]), args["num_points"])
    console_dir = os.path.join(replot_dir, "outputs")

    replots_dirname = os.path.join(results_dir, "replots", os.path.basename(replot_dir))
    os.makedirs(replots_dirname, exist_ok=True)
    target_filename = os.path.join(replots_dirname, "args.json")
    shutil.copy(argsfilename, target_filename)

    return remyccs, link_ppt_range, console_dir

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
            norm_score -= log2(0.75) # reverse the reversal of the equal-share normalization
            norm_scores.append(norm_score)
        datafile.close()
        add_plot(axes, link_speeds, norm_scores, label=datafilename)
    except (IOError, ValueError) as e:
        print("Error plotting from {}: {}".format(datafilename, e), file=sys.stderr)

def generate_remyccs_list(specs):
    """Returns a list of RemyCC files, for example:
        ["myremycc.5"] -> ["myremycc.5"]
        ["myremycc.[3:3:9]"] -> ["myremycc.3", "myremycc.6", "myremycc.9"]
    """
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

def make_results_dir(dirname):
    return utils.make_output_dir(dirname, DEFAULT_RESULTS_DIR, "results" + time.strftime("%Y%m%d-%H%M%S"), LAST_RESULTS_SYMLINK)


# Script starts here

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("remycc", nargs="*", type=str,
    help="RemyCC file(s) to run, can also use e.g. name.[5:5:30] to do name.5, name.10, ..., name.30")
parser.add_argument("--sender", type=str, default="",
    help="Indicate that we are running poisson senders. ")
parser.add_argument("-R", "--replot", type=str, action="append", default=[],
    help="Replot results in this directory from output files (can be specified multiple times)")
parser.add_argument("-n", "--num-points", type=int, default=1000,
    help="Number of points to plot")
parser.add_argument("-l", "--link-ppt", type=float, default=[0.1, 100.0], nargs=2, metavar="PPMS",
    help="Link packets per millisecond, range to test, first argument is low, second is high")
parser.add_argument("-O", "--results-dir", type=str, default=None,
    help="Directory to place output files in.")
parser.add_argument("--no-console-output-files", action="store_false", default=True, dest="console_output_files",
    help="Don't generate console output files")
parser.add_argument("--originals", type=str, default="originals",
    help="Directory in which to look for original data files to add to plot.")
parser.add_argument("--sender-runner", type=str, default=None,
    help="sender-runner executable location, defaults to ../src/sender-runner")
parser.add_argument("--newlines", action="store_const", dest="progress_end_char", const='\n', default='\r',
    help="Print newlines (\\n) instead of carriage returns (\\r) when reporting progress")
senderrunner_group = parser.add_argument_group("sender-runner arguments")
senderrunner_group.add_argument("-s", "--nsenders", type=int, default=2,
    help="Number of senders")
senderrunner_group.add_argument("-d", "--delay", type=float, default=150.0,
    help="Delay (milliseconds)")
senderrunner_group.add_argument("-q", "--mean-on", type=float, default=1000.0,
    help="Mean on duration (milliseconds)")
senderrunner_group.add_argument("-w", "--mean-off", type=float, default=1000.0,
    help="Mean off duration (milliseconds)")
senderrunner_group.add_argument("-b", "--buffer-size", type=str, default="inf",
    help="Buffer size, a number or 'inf' for infinite buffers")
args = parser.parse_args()

# Sanity-check arguments, warn user say they can stop things early
if not os.path.isdir(args.originals):
    warn("The path {} is not a directory.".format(args.originals))
for replot_dir in args.replot:
    if not os.path.isdir(replot_dir):
        warn("The path {} is not a directory.".format(replot_dir))
if len(args.remycc) == 0 and len(args.replot) == 0:
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
utils.log_arguments(results_dirname, args)

# Generate parameters
link_ppt_range = np.logspace(np.log10(args.link_ppt[0]), np.log10(args.link_ppt[1]), args.num_points)
parameter_keys = ["sender", "nsenders", "delay", "mean_on", "mean_off", "buffer_size"]
parameters = {key: getattr(args, key) for key in parameter_keys}
remyccfiles = generate_remyccs_list(args.remycc)

ax = plt.axes()

# Generate data and plots (the main part)
generator = SenderRunnerRemyCCPerformancePlotGenerator(link_ppt_range, parameters,
        console_dir=console_dirname, data_dir=data_dirname, axes=ax, senderrunnercmd=args.sender_runner,
        progress_end_char=args.progress_end_char)
for remyccfile in remyccfiles:
    generator.generate(remyccfile)
link_ppt_priors = generator.get_link_ppt_priors()

# Generate replots
for replot_dir in args.replot:
    remyccs, link_ppt_range, outputs_dir = process_replot_argument(replot_dir, results_dirname)
    generator = OutputsDirectoryRemyCCPerformancePlotGenerator(link_ppt_range, outputs_dir,
            link_ppt_priors=link_ppt_priors, data_dir=data_dirname, axes=ax,
            progress_end_char=args.progress_end_char)
    for remycc in remyccs:
        generator.generate(remycc)
    link_ppt_priors = generator.get_link_ppt_priors()

# Generate original plots
if os.path.isdir(args.originals):
    for filename in os.listdir(args.originals):
        path = os.path.join(args.originals, filename)
        if not os.path.isfile(path):
            warn("Skipping {}: not a file".format(path))
        print("Plotting file {}...".format(path), file=sys.stderr)
        plot_from_original_file(path, ax)

# If all RemyCCs had the same training range, plot it on the graph.
if len(link_ppt_priors) == 1:
    link_ppt_low, link_ppt_high = link_ppt_priors[0]
    plt.axvspan(LINK_PPT_TO_MBPS_CONVERSION*link_ppt_low, LINK_PPT_TO_MBPS_CONVERSION*link_ppt_high,
            linewidth=0.0, facecolor="0.2", alpha=0.2)
elif len(link_ppt_priors) > 1:
    print("Multiple link_ppt_priors found, not highlighting on plot.")

# Make plot pretty and save
plot_filename = "link_ppt"
ax.set_xlabel("link speed (Mbps)")
ax.set_ylabel("normalized score")
ax.legend(loc='lower center', bbox_to_anchor=(0.5, 1))
plt.savefig(os.path.join(plots_dirname, "{:s}.png".format(plot_filename)), format="png", bbox_inches="tight")
plt.savefig(os.path.join(plots_dirname, "{:s}.pdf".format(plot_filename)), format="pdf", bbox_inches="tight")
