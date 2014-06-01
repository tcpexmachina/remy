#! /usr/bin/python

import sys
fh=sys.stdin
current_muxing = -1

from math import log

for line in fh.readlines():
  records = line.split()
  if (len(records) == 0):
    continue
  if (records[0] == "==="):
    if (current_muxing != -1):
      print current_muxing, " ", log( total_tpt / current_muxing ) - log( total_delay / current_muxing )
  elif (records[0] == "config:"):
    current_muxing = int(records[3].split("=")[1].split(",")[0])
    total_tpt = 0
    total_delay = 0
  elif (records[0] == "sender:"):
    tpt = float(records[1].split("=")[1].split(",")[0])
    delay = float(records[2].split("=")[1].split("]")[0])
    total_tpt = total_tpt + tpt
    total_delay = total_delay + delay
