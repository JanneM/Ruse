#!/usr/bin env python3

import subprocess
import os
import sys
import time

ITERTIME = 1
iter=0      # waiting iterations

jiffy = float(os.sysconf(os.sysconf_names['SC_CLK_TCK']))

cmdline = sys.argv[1:]
print(cmdline)

proc = subprocess.Popen(cmdline)
tzero = time.clock_gettime(time.CLOCK_REALTIME)
print(proc.pid)

procdir = os.path.join("/proc", str(proc.pid))

while (proc.poll() == None): 
    iter += 1
    print(iter)
    time.sleep(iter*ITERTIME - 
            (time.clock_gettime(time.CLOCK_REALTIME) - tzero))

    tdir = os.path.join(procdir, "task")
    threads = os.listdir(tdir)
    for t in threads:
        with open(os.path.join(tdir, t, "stat")) as f:
                line = f.readline().split()
                utime = float(line[13])/jiffy
                starttime = float(line[21])/jiffy
                core = line[38]

                print("{} - utime: {:.2f} \t start: {:.2f} \t core: {:d}"
                        .format(t, utime, starttime, int(core)))

