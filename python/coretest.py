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
ncores = os.cpu_count()
nprocs = os.environ.get('OMP_NUM_THREADS', os.cpu_count())  

print(nprocs)

proc = subprocess.Popen(cmdline)
tzero = time.clock_gettime(time.CLOCK_REALTIME)
print(proc.pid)

procdir = os.path.join("/proc", str(proc.pid))

# utime for a thread
tutlist={}
corelist=[0.0]*nprocs
corearr = [0.0]*ncores

with open('/proc/uptime', 'r') as f:
    uptime = float(f.readline().split()[0])

while (proc.poll() == None): 
    iter += 1
    print(iter)
    time.sleep(iter*ITERTIME - 
            (time.clock_gettime(time.CLOCK_REALTIME) - tzero))
    
    prev_uptime = uptime
    with open('/proc/uptime', 'r') as f:
        uptime = float(f.readline().split()[0])
    tdir = os.path.join(procdir, "task")
    threads = os.listdir(tdir)
    tnum = len(threads)
    for i in range(0,6):
        corelist[i] = 0.0
    
    for t in threads:
        with open(os.path.join(tdir, t, "stat")) as f:
                line = f.readline().split()
                utime = float(line[13])/jiffy
                starttime = float(line[21])/jiffy
                core = int(line[38])
                 
                uprev = tutlist.setdefault(t, 0.0)

                tprev = max(starttime, prev_uptime)
                tdiff = uptime-tprev
                udiff = utime - uprev
                tutlist[t] = utime
                print("{}: {}\t utime: {:.2f}\t  prop: {:.2f}\t start: {:.2f}\t core: {:d}"
                        .format(t, uptime, utime, udiff/tdiff, starttime, int(core)))


                corelist[core] += udiff/tdiff
    for v in corelist:
        print("{:.4f}\t".format(v), end="")

    print()


                


