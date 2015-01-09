import subprocess

var_range = [float(i) / 100 for i in range(0, 1000, 10)]

slow_rewma = 1.0
for sewma in var_range:
    for rewma in var_range:
        for rttr in var_range:
            call_str = "./src/find-cycles sewma=%f rewma=%f rttr=%f slow=%f if=../remy_vis/outfiles/bigbertha2x.dna.5" % (sewma, rewma, rttr, slow_rewma)
            print call_str
            subprocess.call(call_str, shell=True)
