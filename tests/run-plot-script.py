#!/usr/bin/python

import subprocess
import csv
import os
import shutil
import unittest

class TestPlotScript(unittest.TestCase):

    RESULTS_DIR = "plot-results"
    REMYCC_NAME = os.path.join(os.environ["srcdir"], "RemyCC-2014-100x.dna")
    PLOT_SCRIPT = os.path.join(os.environ["srcdir"], "../scripts/plot.py")
    ORIGINALS_DIR = os.path.join(os.environ["srcdir"], "../scripts/originals")
    SENDERRUNNER = os.path.abspath("../src/sender-runner")

    def test_scripts(self):

        # Run as command line, not as Python import, to check command line interface
        subprocess.check_output([self.PLOT_SCRIPT, self.REMYCC_NAME, "-n=10", "-O", self.RESULTS_DIR,
                "--originals", self.ORIGINALS_DIR, "--sender-runner", self.SENDERRUNNER, "--newlines"])

        # Check data matches what was expected
        EXPECTED_DATA = [
            [0.1           , -3.652, 1.259, 15.58, 1.258, 16.08],
            [0.215443469003, -1.098, 1.078, 2.305, 1.074, 2.307],
            [0.464158883361, -0.493, 0.952, 1.339, 0.948, 1.337],
            [1.0           , -0.444, 0.808, 1.097, 0.805, 1.099],
            [2.15443469003 , -0.559, 0.876, 1.283, 0.865, 1.283],
            [4.64158883361 , -0.586, 0.812, 1.219, 0.811, 1.218],
            [10.0          , -0.533, 0.715, 1.038, 0.719, 1.038],
            [21.5443469003 , -0.672, 0.640, 1.021, 0.640, 1.020],
            [46.4158883361 , -1.119, 0.463, 1.000, 0.457, 1.000],
            [100.0         , -2.233, 0.212, 1.000, 0.212, 1.000],
        ]

        result_file = open(os.path.join(self.RESULTS_DIR, "data", "data-" + os.path.basename(self.REMYCC_NAME) + ".csv"))
        result_csv = csv.reader(result_file)

        for expected_row, actual_row_str in zip(EXPECTED_DATA, result_csv):
            actual_row = [float(x) for x in actual_row_str]

            expected_link_ppt = expected_row.pop(0)
            actual_link_ppt = actual_row.pop(0)
            self.assertAlmostEqual(expected_link_ppt, actual_link_ppt)

            expected_norm_score = expected_row.pop(0)
            actual_norm_score = actual_row.pop(0)
            self.assertAlmostEqual(expected_norm_score, actual_norm_score, delta=0.5)

            for expected, actual in zip(expected_row, actual_row):
                self.assertLess(abs((actual - expected)/expected), 0.2, msg="{} is not within 20% of {}".format(actual, expected))
                self.assertEqual(actual > 0, expected > 0)

        # clean up
        os.unlink("last")
        shutil.rmtree(self.RESULTS_DIR)

if __name__ == '__main__':
    unittest.main()
