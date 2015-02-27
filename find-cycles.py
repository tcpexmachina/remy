import subprocess

from multiprocessing import Pool

rewma_range = [float(i) / 100 for i in range(0, 500, 4)]
buffer_range = range(0, 200, 1)

def build_cmd_and_call(args):
    rewma, buff = args
    call_str = "./src/find-cycles rewma=%f buff=%u if=dnafiles/absolute-intersend.dna.8" % (rewma, buff)
    subprocess.call(call_str, shell=True)

pool = Pool()
pairs = [(r, b) for r in rewma_range for b in buffer_range]
for p in pairs:
    print p
#pool.map(build_cmd_and_call, pairs)
