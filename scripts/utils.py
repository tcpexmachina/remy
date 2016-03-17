# This file needs to be compatible with both Python 2 and Python 3.

import os
import time
from socket import gethostname
import subprocess
import json

def log_arguments(argsfile, args):
    if os.path.isdir(argsfile):
        dir_given = True
        argsfile = open(os.path.join(argsfile, "args.json"), "w")
    else:
        dir_given = False

    jsondict = {
        "start-time": time.asctime(),
        "machine-name": gethostname(),
        "args": vars(args),
        "git": {},
    }
    try:
        jsondict["git"]["describe"] = subprocess.check_output(['git', 'describe', '--always', '--dirty']).decode().strip()
    except subprocess.CalledProcessError:
        pass
    json.dump(jsondict, argsfile, indent=2, sort_keys=True)

    if dir_given:
        argsfile.close()

def make_output_dir(dirname, default_parent, default_child, symlink):
    if dirname is None:
        dirname = os.path.join(default_parent, default_child)
    if os.path.islink(symlink):
        os.unlink(symlink)
    os.symlink(dirname, symlink)
    if not os.path.exists(dirname):
        os.makedirs(dirname)
    return dirname
