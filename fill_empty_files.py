import os

for fname in os.listdir(os.getcwd()):
    if (os.stat(fname).st_size == 0):
        stats = fname.split('-')
        time = stats[0]
        with open( fname, 'w' ) as f:
            f.write(time + " -100\n")
