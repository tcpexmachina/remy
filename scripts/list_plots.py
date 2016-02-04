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

for entry in os.listdir(directory):

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

    jsondict = json.load(argsfile)
    git_dict = jsondict.get("git", {})
    git_branch = git_dict.get("branch", "<unknown>")
    git_commit = git_dict.get("commit", "<unknown>")[:9]
    args_dict = jsondict.get("args", {})
    remyccs = args_dict.get("remycc", [])
    remyccs = [os.path.basename(r) for r in remyccs]
    npoints = args_dict.get("num_points", -1)

    print("{dirname:30} {branch:14} {commit:9} {npoints:>5d} {remyccs}".format(
        dirname=dirname, branch=git_branch, commit=git_commit, npoints=npoints,
        remyccs=" ".join(remyccs)))
