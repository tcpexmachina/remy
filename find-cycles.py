import subprocess
import sys

from multiprocessing import Pool

outdir = sys.argv[1]

rewma_range = [float(i) / 1000 for i in range(0, 2000, 10)]
buffer_range = range(0, 5000, 2)

def build_cmd_and_call(args):
    rewma, buff = args
    call_str = "./src/find-cycles -e %f -b %u -i dnafiles/bigbertha2.dna.5 -l 1 -r 150 -n 1 > %s/%f-%u" % (rewma, buff, outdir, rewma, buff)
    subprocess.call(call_str, shell=True)

pool = Pool()
pairs = [(r, b) for r in rewma_range for b in buffer_range]
pool.map(build_cmd_and_call, pairs)
