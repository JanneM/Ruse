# Ruse


A command-line utility to periodically measure the resource use of a process and its subprocesses, in the same way as the [Slurm] job scheduler. It measures the memory used as RSS or PSS and the CPU usage par process and thread. 

You can find the actual memory, execution time and cores that you need for individual programs or MPI processes, presented in a way that is straightforward to understand and apply to your [Slurm] job submissions.

Ruse periodically samples the process and its subprocesses and keeps track of the CPU, time and maximum memory use. It also optionally records the sampled values over time. The purpose or Ruse is not to profile short processes in detail, but to follow jobs that run for many minutes, hours or days. 

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

For example:

```
ruse sleep 150
```

Ruse samples the CPU and memory use every 30 seconds, beginning at 0, and writes a file `sleep-<pid>.ruse` with the contents:

```
Time:    00:02:30
Mem:     0.6 MB
Cores:   4
Procs:   1
Proc(%): 0.0
```

We see that "sleep" ran for 2:30; it used about 0.6 MB memory; it had 4 cores available, but at most used 1; and that process used 0.0% of CPU. Not surprising, as "sleep" basically does nothing at all.


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

We can plot the data graphically using "gnuplot":

```
$ gnuplot
...
gnuplot> set key autotitle columnhead
gnuplot> set xlabel "time (s)"
gnuplot> set ylabel "Mem (MB)"
gnuplot> plot 'make-<pid>.ruse' using 1:2 lw 2 with lines
```

We first tell gnuplot to treat the first line in the file as column labels, set the x and y axis labels, then plot columns 1 and 2 with thick lines: [OpenCV build over time](doc/opencv_make.png)

## Build

You will need the Linux software development tools (gcc, library headers and so on) as well as the GNU Autotools installed.

First run autogen.sh to create the configuration files. Then do 'configure', 'make' and 'make install:

```
$ ./autogen.sh
$ ./configure
$ make && make install
```

Do './configure --help' to see the available options. In particular, you can use './configure --prefix=<some_path>' to install Ruse locally instead of system-wide.

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
