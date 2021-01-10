# Ruse


Ruse is a command-line utility that periodically measures the resource use of a process and its subprocesses, in the same way as the [Slurm] job scheduler. With Ruse you can find the actual memory, execution time and cores that you need for individual programs or MPI processes, presented in a way that is straightforward to understand and apply to your [Slurm] job submissions.

Ruse periodically samples the process and its subprocesses and keeps track of the CPU, time and maximum memory use. It also optionally records the sampled values over time. The purpose or Ruse is not to profile processes in detail, but to follow jobs that run for many minutes, hours or days, without any performance impact and without having to change the measured application in any way.

## Usage

To profile a command, run

```
ruse [FLAGS] command [ARG...]
```

Options include:

```
  -l, --label=LABEL      Prefix output with LABEL (default 'command')
      --stdout           Don't save to a file, but to stdout
      --no-header        Don't print a header line
      --no-summary       Don't print summary info
  -s, --steps            Print each sample step
  -p, --procs            Print process information (default)
      --no-procs         Don't print process information
  -t, --time=SECONDS     Sample every SECONDS (default 30)

      --rss              use RSS for memory estimation
      --pss              use PSS for memory estimation (default)

      --help             Print help
      --version          Display version
```

### Here are few examples. 

Let's begin with a couple of toy examples.

**Measure the "sleep" command:**

```
ruse sleep 150
```

Ruse samples the CPU and memory use every 30 seconds, beginning at 0, and writes a file `sleep-<pid>.ruse` with the contents:

```
Time:	       00:02:30
Mem:           0.6 MB
Cores:         4
Procs:         1
Total_procs:   1
Used_procs:    0
Proc(%): 
```

We see that "sleep" ran for 2 minutes and 30 seconds; it used about 0.6 MB memory; it had 4 cores available, but spawned only 1, and didn't actually use it. Not surprising, as "sleep" does nothing at all very efficiently.


**Measure "grep"**

Here we search for a string in random data, recreating a famous thought experiment in the process:

```
ruse -st1 grep "To be or not to be" </dev/random
```

We add the "-s" flag so it saves each intermediate measurement; and we set the time interval to 1 second with "-t1". We let this run for a few seconds, then interrupt it with ctrl-c:

```


   time         mem   processes  process usage
   sses  process usage
  (secs)        (MB)  tot  used  (sorted, %CPU)
      0         2.0     1     0 
      1         2.0     1     1    7
      2         2.0     1     1   10
      3         2.0     1     1   14
      4         2.0     1     1    8
...
     14         2.0     1     1    7
     15         2.0     1     1    8
     16         2.0     1     1    6
     17         2.0     1     1   11
     18         2.0     1     1   10

Time:          00:00:19
Memory:        2.0 MB
Cores:         6
Total_procs:   1
Used_procs:    1
Proc(%): 8.2   
```

"grep" uses 2.0MB memory. It is single-threaded, so we see it uses one process, and it uses around 8% CPU as it works. Presumably the rest of the time is spent waiting for random input from the /dev/random device.




Run ruse on make (making OpenCV), with a sampling frequency of 1 second, and logging all data:

```
ruse -t1 -s make -j12
```

Here we run 'make' in parallel; this calls a number of external binaries to run the different stages of compiling and linking many hundreds of source files. Ruse samples all their memory use, and gives us an output file that looks something like this:

```
time(s)   mem(MB)
0         2.2
1         649.9
2         988.5
3         1081.7
4         1392.9
...
280       626.7
281       627.1
282       76.5

Time(s):  282
Mem(MB):  3038.1
```

The entire build took 282 seconds, or almost five minutes, and used a maximum of 3038MB at one time.


## Build

Ruse uses the GNU Autotools. In most cases you can simply run "configure", then "make" and "make install". You will need the Linux software development tools (gcc, library headers and so on), but it needs no external dependencies.

Create a build directory. In that directory do 'configure', 'make' and 'make install:

```
$ mkdir build
$ cd build
$ ./configure
$ make && make install
```

Do './configure --help' to see the available options. In particular, you can use './configure --prefix=<some_path>' to install Ruse locally instead of system-wide.

If you want to recreate the build files, you can run "autogen.sh" to do so. Then you can do 'configure', 'make' and 'make install as above.

## Features

### Memory estimation

There are (at least) two ways to measure the amount of memory used by a process: "RSS" (Resident Set Size) and "PSS" (Proportional Set Size). RSS counts the amount of physical memory actually used (not just allocated) by a process. This is a simple and fast measurement, but pessimistic, as it doesn't take into account memory shared by multiple processes. PSS takes shared memory areas into account, but to estimate it you have to go through each allocated memory block and sum up the values each time. This is slow; for a large process — hundreds of GB — this can take on the order of seconds.

Because of the performance impact, Ruse defaults to RSS. This is a pessimistic estimation, but in most cases the actual difference is fairly small. The memory estimation is meant to be a lower bound for job allocation, so being pessimistic is not a bad thing. Also, for installations where [Slurm] uses RSS this is the best estimate. 

You can use PSS by using the "--pss" option. You can also default to PSS at build time by passing "--enable-pss" to the configure invocation. Note again that this can have a significant performance impact; avoid using short time periods if you do this.

### Process CPU usage

Ruse records the total number of processes and threads at each time step. It also records the amount of CPU each process used since the last step. Active processes are the ones with a non-zero amount of activity.

At each time, the active process CPU use is sorted, optionally displayed, and added to the total. At the end, Ruse dismpays the peak number of total simultaneous processes; total active processes, and the average CPU use.

Note that we do not care *which* process did what in the display. The most active process may be different from time step to time step. We only care about the pattern of activity.

#### Interpret process usage

You will see a fair mumber of different patterns depending on the nature of the application. 

* Even pattern: 90.0 88.4 86.5 86.2
  
  This is typical for a CPU-bound highly parallel application. Each process is fully occupied doing calculation.

* One busy, others not: 90.0 12.1 8.9 7.3

  This can arise when you have a single-threaded application that periodically uses a well-optimized parallel numerical library. A Python or Matlab program that uses matrix operations will show this pattern for instance.

* Kind of iffy: 61.3 8.4 4.2 4.1 

  





## FAQ

#### Why is the sampling rate so low? 30 seconds is an eternity, and even 1 second misses a lot of events.

Ruse is not meant to profile quick programs on your local computer. It's meant for compute clusters, where an individual job typically takes hours, days or weeks to finish. 

And to run a job on a cluster you typically have to specify the resources — the number of nodes and cores, the amount of memory and the time — you will need ahead of time. You want to know the peak memory usage and time so you can ask for a sensible amount of these resources.

Ruse uses 30 seconds by default because [Slurm] — the job scheduler we use — samples the resources used by jobs at that rate. We also limit it to no more than one sample per second to make sure Ruse itself does not significantly slow down the job.

#### Doesn't Slurm already tell you how much memory you use?

Yes, [Slurm] can be configured to do that. But the data can be confusing or difficult to read, and you need to know a fair bit about how [Slurm] works to interpret the values correctly. This goes especially if your job has multiple subjobs, or uses MPI. Also, [Slurm] will not report the memory use of individual commands (unless you run them as subjobs) and will not give you a memory profile over time. 

Ruse attempts to report the data in a way that is easy to interpret and use to create a job submission. 

#### How does Ruse measure memory use, exactly?

Every sample interval, Ruse checks the process and all its subprocesses for the current "Resident Set Size" (RSS), and sums those values for the final result. It keeps track of the maximum over time and reports that at the end of the run. [Slurm] measures job memory use in the same way.

This does mean it can count some memory multiple times. if your binary runs two processes that, say, both link to a shared library, then the memory used by that library code will be counted twice. There is another measure called "PSS" that does not count memory twice; shared memory is split among all processes that uses it. But this 
is an unworkable measure in multi-user systems, as the actions of other users (starting or stopping processes that use that same shared memory) will affect the amount of memory your process is considered to use.

#### Any future plans for Ruse?

Yes. I hope to add an estimation of CPU and core use over time as well. There's no specific timeline for that, though.


[Slurm]: https://slurm.schedmd.com/
