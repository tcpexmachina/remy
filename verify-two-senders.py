import subprocess
import sys

import numpy as np

from multiprocessing import Pool

outdir = sys.argv[1]

def run_single_sender(linkspeed, rtt):
    call_str = "./src/find-cycles -i dnafiles/bigbertha2x.dna.5 -n 1 -l %f -r %f > tmp.out" % (linkspeed, rtt)
    subprocess.call(call_str, shell=True)
    
def build_cmd_and_call(args):
    offset, linkspeed, rtt = args
    call_str = "./src/find-cycles -i dnafiles/bigbertha2x.dna.5 -n 2 -l %f -r %f -o %f > %s/%f-%f-%u " % (linkspeed, rtt, offset, outdir, offset, linkspeed, rtt)
    subprocess.call(call_str, shell=True)

run_single_sender(1.0, 150)

with open('tmp.out', 'r') as result:
    offset, start, cycle_len, delay, tp, utility = [r.strip() for r in result.readlines()][0].split()

end = float(start) + float(cycle_len)

pool = Pool()
offsets = np.linspace(0, end, 1000)
args = [(o, 1.0, 150) for o in offsets]
pool.map(build_cmd_and_call, args)
