import os

for fname in os.listdir(os.getcwd()):
    if not fname.endswith('.out'):
        continue
    if (os.stat(fname).st_size == 0):
        stats = fname[:-4].split('-')
        all_stats = ' '.join(stats)
        with open( fname, 'w' ) as f:
            f.write(all_stats + " NA NA NA NA NA\n")
