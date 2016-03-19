#!/usr/bin/python3
"""Lists all the results in the results directory."""

import os
import argparse
import json

DEFAULT_RESULTS_DIR = "results"

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("directory", type=str, default=DEFAULT_RESULTS_DIR, nargs="?",
    help="Directory to look in (default '{}')".format(DEFAULT_RESULTS_DIR))
parser.add_argument("-v", "--verbosity", type=int, default=0,
    help="Verbosity, 0=normal, 1=print errors, 2=print skipped")
args = parser.parse_args()

directory = args.directory
entries = os.listdir(directory)
entries.sort()

for entry in entries:

    dirname = os.path.join(directory, entry)

    if not os.path.isdir(dirname):
        if args.verbosity >= 2:
            print("{} - not a directory".format(dirname))
        continue

    argsfilename = os.path.join(dirname, "args.json")
    try:
        argsfile = open(argsfilename, "r")
    except IOError as e:
        if args.verbosity >= 1:
            print("{} - could not open args.json: {}".format(dirname, e))
        continue

    try:
        jsondict = json.load(argsfile)
    except ValueError as e:
        if args.verbosity >= 1:
            print("{} - could not parse args.json: {}".format(dirname, e))
        continue
    argsfile.close()
    git_dict = jsondict.get("git", {})
    git_branch = git_dict.get("branch", "<unknown>")
    git_commit = git_dict.get("commit", "<unknown>")[:9]
    args_dict = jsondict.get("args", {})
    remyccs = args_dict.get("remycc", [])
    remyccs = " ".join([os.path.basename(r) for r in remyccs])
    if remyccs:
        npoints = args_dict.get("num_points", -1)
    else:
        npoints = "-"

    replots = args_dict.get("replot", [])
    if replots:
        replots_str = (remyccs and ", replots:" or "replots:")
        for replot in replots:
            replot_basename = os.path.basename(replot)
            try:
                replot_argsfile = open(os.path.join(dirname, "replots", replot_basename, "args.json"))
                replot_args = json.load(replot_argsfile)["args"]
                replot_argsfile.close()
                replot_remyccs = " ".join([os.path.basename(r) for r in replot_args["remycc"]])
                replot_npoints = replot_args["num_points"]
            except (IOError, KeyError):
                replots_str += " {basename}<error>".format(basename=replot_basename)
            else:
                if replot_remyccs:
                    replots_str += " {basename}[{remyccs}/{npoints}]".format(basename=replot_basename,
                            remyccs=replot_remyccs, npoints=replot_npoints)
                else:
                    replots_str += " {basename}[]".format(basename=replot_basename)
    else:
        replots_str = ""

    plotsdirname = os.path.join(dirname, "plots")
    noplots = "[no plots] " if os.path.isdir(plotsdirname) and len(os.listdir(plotsdirname)) == 0 else ""

    print("{dirname:30} {branch:14} {commit:9} {npoints:>5} {noplots}{remyccs}{replots}".format(
        dirname=dirname, branch=git_branch, commit=git_commit, npoints=npoints,
        remyccs=remyccs, noplots=noplots, replots=replots_str))
