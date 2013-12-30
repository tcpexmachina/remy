#!/bin/bash

pls ./batch-runner nsrc=2 link=0.1 axes=0,1,2,3 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=0,1,2 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=0,1,3 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=0,2,3 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=1,2,3 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=0,1 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=0,3 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=2,3 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=0,2 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=1,3 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=1,2 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=0 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=1 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=2 of=signal_test_outfiles/test01
pls ./batch-runner nsrc=2 link=0.1 axes=3 of=signal_test_outfiles/test01
