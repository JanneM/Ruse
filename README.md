# Ruse

Ruse is a command-line utility that periodically measures the resource use of a process and its subprocesses. It is intended to help you find out how much resource to allocate to your jobs on a remote cluster or supercomputer. With Ruse you can find the actual memory, execution time and cores that you need for individual programs or MPI processes, presented in a way that is straightforward to understand and apply to your [Slurm] job submissions.

Ruse periodically samples the process and its subprocesses and keeps track of the CPU, time and maximum memory use. It also optionally records the sampled values over time. The purpose or Ruse is not to profile processes in detail, but to follow jobs that run for many minutes, hours or days, with no performance impact and without changing the measured application in any way.

## Usage

To profile a command, run

```
ruse [FLAGS] command [ARG...]
```

Options are:

```
  -l, --label=LABEL      Prefix output with LABEL (default 'command')
      --stdout           Don't save to a file, but to stdout
      --no-header        Don't print a header line
      --no-summary       Don't print summary info
  -s, --steps            Print each sample step
  -p, --procs            Print process information (default)
      --no-procs         Don't print process information
  -t, --time=SECONDS     Sample every SECONDS (default 30)

      --rss              use RSS for memory estimation (default)
      --pss              use PSS for memory estimation

      --help             Print help
      --version          Display version
```

### Examples of use

Let's begin with a toy example.

**Measure the "sleep" command:**

```
ruse sleep 150
```

Ruse samples the CPU and memory use every 30 seconds and writes a file `sleep-<pid>.ruse` with the contents:

```
Time:           00:02:30
Mem:            0.6 MB
Cores:          4
Procs:          1
Total_procs:    1
Active_procs:   0
Proc(%): 
```

We see that "sleep" ran for 2 minutes and 30 seconds; it used about 0.6 MB memory; it had 4 cores available, but spawned only 1 process, and no process was ever active. Not surprising, as "sleep" does nothing at all very efficiently.

**Measure a computational chemistry calculation**

Here we'll run an example calculation on a molecule using a computational chemistry package. This is an older application that starts subprograms in phases to do its calculations. We run the command as:

```
$ ruse -st1 g09<Azulene.inp >azu.out
```

This tells Ruse to record each time step with the "-s" option, and to use a one-second time step with "-t1". Our result file looks like this (a bit abbreviated):

```
   time         mem   processes  process usage
  (secs)        (MB)  tot   act  (sorted, %CPU)
      1       273.4     9     8   34   7   7   6   6   6   4   4
      2       284.7     9     8   87  87  87  84  83  83  83  82
      3       301.4     9     8   99  99  99  98  98  98  98  90
      4       345.4     9     8   67  53  53  53  52  52  51  50
      5        65.9     2     1   31
      6       289.8     9     8   98  98  98  98  97  97  91  83
      7        14.7     2     1   75
      8        15.1     2     1   58
      9       280.7     9     8   62  44  43  37  37  33  19  19
     10       287.4     9     8   99  99  99  99  95  94  70  69
     ...
     32       397.8     9     8  100 100 100 100 100 100  99  98
     33       448.5     9     8   98  98  98  98  98  98  96  93
     34       209.1     9     8   79  14  14  14  14   7   6   5
     35       291.5     9     8   72  51  51  50  48  48  47  45
     36       291.5     9     8  100  99  99  99  99  99  99  98
     37       331.2     9     8   99  98  98  98  98  98  98  97
     38       168.7     9     8   90   5   5   5   2   2   2   2

Time:           00:00:39
Memory:         448.5 MB
Cores:          8
Total_procs:    9
Active_procs:   8
Proc(%): 74.8  50.9  50.8  50.3  49.6  49.0  45.5  44.0  
```

We have 8 available hardware cores. We have at most 9 simultaneous processes in total, and at most 8 processes are active at any one time. This is a common pattern: A parent process reads input files and sets everything up; then other processes do the actual work while the parent waits for the job to finish.

Ruse lists the active processes at the end of each sample period, ordered by activity. This reflects the core usage of the job. At the end, Ruse displays the average of this active list. With 8 cores and at most 8 active processes we can see it uses one core a lot, and the other 7 cores around half the time.

The CPU use is *not* ordered by process. Each step displays the CPU use of the active processes or threads sorted from most active to the least. But the active processes may be completely different from one time step to the next. This doesn't tell us anything about specific processes, but it does tell us how efficiently we use the available cores overall.


## Build

Ruse uses the GNU Autotools. In most cases you can run "configure", then "make" and "make install" to build and install Ruse.  You will need the Linux software development tools (a C compiler such as GCC, library headers, the GNU Autotools and so on), but it needs no external dependencies.

Create a build directory. In that directory do 'configure', 'make' and 'make install:

```
$ mkdir build
$ cd build
$ ../configure
$ make && make install
```

Do `configure --help` to see the available options.

* You can use `configure --prefix=<some_path>` to install Ruse into the given path instead of system-wide.
* `--enable-pss` will make PSS the default method for measuring memory (see below before you do that).
* `--with-extras` will build and install a couple of small utilities useful for testing Ruse.

If you cloned the git repository and need to recreate the build files, you can run `./autogen.sh` in the top directory to do so. Then you can do `configure`, `make` and `make install` as above.


## Options

*  -l LABEL, --label=LABEL  
  
  Ruse outputs a file named `<name of program>-<PID>.ruse` by default. If you don't want to use the name of the program, you can set a different name here.


* --stdout           
  
  Print the output on screen instead of saving into a file. Good for debugging and testing.


* --no-header        

  Don't print the header lines at the top of the file when printing each time step. This is good for when you want to read the output with a different program.


* --no-summary       

  Don't print the summary info at the end. This is useful when you just want to read the stepwise data with another program.

  
* -s, --steps  

  Print each sample step. Each line has the form:

  ``` Time(seconds)  Memory(MB)  total-processes  active-processes  sorted list of CPU use```

  Each value is separated by white space. The sorted list has as many elements as the number of active processes. If process information is turned off, this will print only time and memory.


* -p, --procs            

  Print process information. This finds the number of available cores; the total number of processes, the number of active processes and the CPU usage, in percent. 

  * Total processes is the number of child processes and threads the application has at the time of taking the sample. 

  * Active processes are child processes and threads that have a non-zero CPU use during the previous sample period.
  
  * The process list is a list of active processes' CPU usage, in percent, sorted from highest to lowest. 

  If the number of cores his higher than the max number of active processes, the excess cores go unused. This generally means you have too many cores allocated.

  If the number of cores is lower, then some of those active processes are sharing a core between them. For some jobs that are IO bound this is fine, and an efficient use of resources. In other cases the job could benefit from more cores.

  Ruse measures core usage at the end of each sample step only. When a process stops, Ruse will miss the CPU time used by the process from the last sample step to when it stopped. If a process starts, then stops within one sample window, Ruse will miss the process and all of the CPU time used. The core usage measurement is only approximate, and will tend to undercount CPU use for jobs that run a lot of subprograms.


* --no-procs         

  Don't print process information. We still calculate it in the background; it is very efficient and there is little to gain by actively disabling it.


* -t SECONDS, --time=SECONDS

  Sample process and memory use every SECONDS. In general, a shorter interval may let you catch some transient events, or to measure a short-running application. But it comes at the potential cost of higher overhead and of much longer result files. The default is 10 seconds. For most applications there is little reason to change this value.


* --rss              use RSS for memory estimation 
  --pss              use PSS for memory estimation 

  RSS and PSS  are two ways to measure the amount of memory used by a process. "RSS" (Resident Set Size) counts the amount of physical memory used by a process. It doesn't take shared memory into account, making the estimation pessimistic. PSS (Proportional Set Size) accounts for shared memory areas. But the estimation is slow; for a large process (hundreds of GB) calculating it can take a lot of time.

  Ruse defaults to RSS due to the performance impact. In most cases the actual difference is reasonably small. The estimation is meant to be a lower bound for job allocation, so being pessimistic is not really a problem.

  You can enable PSS with the `--pss` option. You can also default to PSS at build time by passing `--enable-pss` to the configure invocation. Do note that this can have a significant performance impact; avoid using short sample time periods if you do this.


* --help, --version

  Display a short help text with the options, and show the version of Ruse.


## FAQ

#### Why is the sampling rate so low? 10 seconds is quite long.

Ruse is not meant to profile quick programs on your local computer. It's meant to help allocate resources on compute clusters for jobs that often takes hours, days or weeks to finish. 

When you run a job on a cluster you typically have to specify the resources — the number of nodes and cores, the amount of memory and the amount of time — you will need ahead of time. Ruse measures these parameters while running a job so you know how much you really use of these resources and can ask for a sensible amount.

Ruse uses 10 seconds as a compromise. It's long enough that Ruse itself doesn't consume any significant amount of CPU time; and short enough that we won't miss any significant memory or process events for most jobs.

The lower sample limit is 1 second. With a faster rate, Ruse itself would start to take a non-trivial amount of CPU resources. If you need finer granularity than that, we suggest it is time for you to use a real application profiler.


#### Doesn't Slurm already tell you how much resources you use?

[Slurm] can be configured to show you the memory used by a job. But the data is not simple to interpret unless you understand how [Slurm] works. This goes especially if your job has multiple subjobs, or uses MPI. 

[Slurm] will not report the memory use of individual commands, and can't give you a memory profile over time. [Slurm] will also not give you any information on the number on process CPU usage. With modern high-core systems core allocation is becoming important to get right.

Ruse attempts to report the data in a way that is easy to interpret and use to create a reasonable job submission. We also aim for Ruse to be lightweight enough that you can use it on production jobs without any performance penalty.


#### How does Ruse measure the memory use?

By default, Ruse measures the RSS (Resident Set Size) each sample step. This is a simple and fast measurement that counts the amount of memory that is allocated and in use for the application. 

This is often an over-estimation. If multiple processes all want access to the same data, the data will be loaded into memory only once, and the processes share the memory area. Shared libraries is a very common case. RSS will count the full size of that memory area for each process.

PSS (Proprtional Set Size) is an alternative that takes shared memory into account. Linux keeps track of the number of processes that share a given memory area, and can calculate the proportion of that memory "belonging to" any given process. You get the PSS for a process by going through its allocated memory areas and sum up the proportion of each memory area belonging to this process. A large application has thousands of separate memory areas, and estimating this value can take up to a second or more in extreme cases.

In practice the difference often doesn't matter. In many cases RSS is only 10-15% higher than PSS. For scientific applications, especially those that are memory hungry, shared data is only a small fraction of the memory needed. The vast bulk of allocted memory is used for input or working data structures that are unique to the running process. That memory isn't shared and doesn't differ between RSS and PSS.

We default to using RSS with Ruse. It is far more efficient; it will tend to be pessimistic, so you won't go wrong using it as basis for your memory allocations; and in most cases the difference is usually not large enough to worry about. 


#### What's the deal with the process measurement?

At each time step, we measure the amount of CPU used for each active process, including child processes and threads. We sort the CPU usage in descending order. This is added to a running total. At the end, the totals are divided by the total running time to give you an average per-active process CPU use.

There are two things to keep in mind:

* We don't care about the identity of the processes. The first process value at each time step is the value for whatever process happened to use the most CPU during that time step. The first process final average is the average of the most active process at each time step, whichever processes that happened to be each time. 

* We don't track *core* usage against processes. Due to issues such as core migration it's not possible to directly get core usage reliably correct with a sampling approach. You would need to use more intrusive profiling methods for that.

Active process activity is a stand-in for core activity. In practice the process use will closely correspond to core usage so this tells you most of what you need to know. The percentages tell you how much CPU was used over time, and the number of simultaneous active processes tell you the upper limit of the number of cores you could conceivably need.

Let's say you had a total that looks like this:

```
Cores:          4
Total_procs:    4
Active_procs:   4
Proc(%): 90.0  65.0  40.0  33.3 
```

Your used processes equal the number of cores, and they are in use. You are making use of the cores you have allocated. It also looks like the third and fourth process are not contributing a huge amount, so increasing the number of cores may not improve your execution time that much.

If you have something like this:

```
Cores:          4
Total_procs:    2
Active_procs:   1
Proc(%): 75.0 
```

You have multiple cores but only one active process. This tells you your application is not multithreaded. It can use only one core at a time, and allocating more is a waste of resources. If it's supposed to to be able to use multiple cores, it's time to check the documentation or search online for how to enable that.

Finally, with something like this:

```
Cores:          16
Total_procs:    16
Active_procs:   16
Proc(%): 80.0  45.0  4.7  4.5  4.4  4.4  4.3  3.9  3.3  2.1  2.1  2.0  1.8  1.8  1.8  1.7  
```

One or a few processes are well-used, and the rest are used only very little. This tells you that your job can use multiple processes, but that this amount - 16 - is probably too much. It's spending almost all its time on single-threaded processing, and only a small part of time actually running in parallel. You most likely want to reduce the number of cores to 8 or even less. You will lose very little time, and will free up resources to, for instance, run another job like it in parallel.


#### Future plans for Ruse

We now have the ability to measure the process CPU use and to use RSS and PSS for memory estimation. This matches what I envisioned for Ruse, and I currently have no specific future plans.

A couple of possible directions that could be interesting:

* Find a non-intrusive, low-resource way to track core usage per process. I don't see a way to do that now.

* Improved reporting. It might be nice to produce JSON output, and perhaps a utility to plot the data graphically.

* Rewrite it in Rust :) No, seriously, I would have benefited from using Rust for this, and if I had started Ruse this year I probably would have. 

I am open to any suggestions, and patches are welcome.


[Slurm]: https://slurm.schedmd.com/
