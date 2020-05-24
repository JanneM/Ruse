#!/usr/bin env python3

import subprocess
import os
import sys
import time

ITERTIME = 1
iter=0      # waiting iterations

cmdline = sys.argv[1:]
print(cmdline)

proc = subprocess.Popen(cmdline)
tzero = time.clock_gettime(time.CLOCK_REALTIME)
print(proc.pid)

while (proc.poll() == None): 
    iter += 1
    print(iter)
    time.sleep(iter*ITERTIME - 
            (time.clock_gettime(time.CLOCK_REALTIME) - tzero))
