#!/bin/bash

cat $(ls *.out | sort -k1,1g -t '-') > all_offset_results