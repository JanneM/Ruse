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

Ruse samples the CPU and memory use every 30 seconds and writes a file `sleep-<pid>.ruse` with the contents:

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

**Matrix multiplication**

Here a small test program that does a matrix multiplication using OpenBLAS:

```python3
#!/usr/bin/env python3
import numpy as np

s = 4000
a = np.array(np.random.random_sample((s, s)))
b = np.array(np.random.random_sample((s, s )))

c=np.matmul(a,b)
```

We run this with Ruse:

```
$ ruse --stdout -st1 ./mat.py
   time         mem   processes  process usage
  (secs)        (MB)  tot  used  (sorted, %CPU)
      0         2.9     1     0 
      1       181.8     1     1   82
      2       397.3     6     6   78  60  57  57  56  52
      3       397.3     6     6  100 100  99  99  99  92
      4       397.3     6     6   99  99  99  95  93  92
      5       397.3     6     6   98  97  96  83  78  63
      6       397.3     6     6  100 100 100 100  96  91
      7       397.3     6     6  100  99  99  99  97  90
      8       397.3     6     6  100  99  99  99  96  91
      9       397.3     6     6  100 100  99  99  98  95
     10       397.3     6     6   99  99  97  86  72  65
     11       397.3     6     6   99  99  99  98  97  97
     12       397.3     6     6  100  99  99  99  99  97

Time:          00:00:13
Memory:        397.3 MB
Cores:         6
Total_procs:   6
Used_procs:    6
Proc(%): 88.9  80.8  80.2  78.0  75.4  71.2  
```

We use "--stdout" to print the result on screen instead of into a file. "-s" displays each sample step, and "-t 1" sets the sample step time to 1 second. 

The running display shows us the number of seconds since start; the current used memory; total allocated processes; the number of processes that actually did something the past time step; and the percentage of CPU use for all used processes, sorted by activity.

At the end we get a summary with the total time; maximum used memory; available cores; peak total, then peak used processes; and the average CPU use, again sorted by amount of activity.

**Measure a computational chemistry calculation**

Here we'll run an example calculation on a molecule using a computational chemistry package. We run the command as:

```
$ ruse -st1 g09<Azurgrgrg.inp >azo.out
```

This tells Ruse to record each time step with the "-s" option, and to use a one-second time step with "-t1". Our result file looks like this (a bit abbreviated):

```
ddffsdf
```

We have 8 available cores; 9 processes in total; and at most 8 processes are actually used at any one time. This is a common pattern: The parent process reads input files and sets everything up; then a set of worker threads do the actual calculations while the parent process is idle.

This application runs caluclations in phases, with some phases using all available cores, and others using only a single core at a time. This is also a common pattern, and one that limits the effective speed you can get from using multiple cores. Memory use, too, varies substantially during different phases of calculation.

In the end, we can see that the most efficient process was quite efficient at over 90% CPU use, while the other processes are less efficient in practice, though this is expected. 

Please remember that the CPU use is *not* ordered by process. The first process in the list is unlikely to be the same process from time step to time step. It is sinmple the most efficient process, whichever it may be. The summary at the end shows you how efficiently this job is able to use one core, a second core and so on. It does not show how efficient any single process has been. 

This is an average across the entire running time; as we had phases with only a single core running, the averages for the second to last processes reflect that, even though they were very efficient when they did run. But what we care about here is how efficiently we use the resources overall, and how fast our job will finish, and this is a good measurement of that.




**Measure "grep"**

Here we search for a string in random data, recreating a famous thought experiment in the process:

```
ruse -st1 grep "To be or not to be" </dev/random
```

We add the "-s" flag so it saves each intermediate measurement; and we set the time interval to 1 second with "-t1". We let this run for a few seconds, then interrupt it with ctrl-c:

```
   time         mem   processes  process usage
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

Ruse uses the GNU Autotools. In most cases you can run "./autogen.sh" to create the configuration files, then run "configure", then "make" and "make install".  You will need the Linux software development tools (gcc, library headers, the GNU Autotools and so on), but it needs no external dependencies.

Create a build directory. In that directory do 'configure', 'make' and 'make install:

```
$ ./autogen.sh
$ mkdir build
$ cd build
$ ./configure
$ make && make install
```

Do './configure --help' to see the available options. In particular, you can use './configure --prefix=<some_path>' to install Ruse locally instead of system-wide.

If you want to recreate the build files, you can run "autogen.sh" to do so. Then you can do 'configure', 'make' and 'make install as above.

## Options

*  -l LABEL, --label=LABEL  
  
  Ruse outputs a file named "\<name of program\>-\<PID\>.ruse" by default. If you don't want the name of the binary, you can set a different name here.

* --stdout           
  
  Print the output on screen instead of saving into a file. Good for debugging and testing.

* --no-header        

  Don't print the header lines at the top of the file when printing each time step. This is good for when you want to read the output with a different program.

* --no-summary       

  Don't print the summary info at the end. This is good when you just want to read the stepwise data with another program.

  
* -s, --steps  

  Print each sample step. Each line has the form:

    Time(seconds)  Memory(MB)  total-processes  active-processes  sorted list of CPU use

  Each value is separated by white space. The sorted list has as many elements as the number of active processes. If process information is turned off, this will print only time and memory.


* -p, --procs            

  Print process information. This finds the number of available cores; the total number of processes, the number of active processes and the CPU usage, in percent. 

  * Total processes is the number of child processes and threads the application has at the timple of takign the sample. 

  * Active processes are child processes and threads that have a non-zero CPU use during the previous sample period.
  
  * The process list is a list of active processes' CPU usage, in percent, sorted from highest to lowest. 

  If the number of cores his higher than the number of active processes, the excess cores go unused. This generally means you have too many cores allocated.

  If the number of cores is lower, then some of those active processes are sharing a core between them. This may well be fine, and an efficient use of resources. In other cases it can mean the job could benefit from more cores.


* --no-procs         

  Don't print process information. We still calculate it in the background; it is very efficient and there is little to gain by actively disabling it.


* -t SECONDS, --time=SECONDS

  Sample process and memory use every SECONDS. In general, a shorter interval may let you catch some transient events, or measure a short-running application. But it comes at the potential cost of higher overhead and of much longer result files. If you are only collecting the summary data there is little reason to change this value.

* --rss              use RSS for memory estimation 
  --pss              use PSS for memory estimation 

  RSS and PSS  are two ways to measure the amount of memory used by a process. "RSS" (Resident Set Size) counts the amount of physical memory actually used (not just allocated) by a process. This is a simple and fast measurement, but it doesn't take into account memory shared among multiple processes. RSS tends to be pessimistic as a result.

  PSS (Proportional Set Size) takes shared memory areas into account, making it more accurate. But to estimate it you have to go through each allocated memory block record and sum up the values. This is slow; for a large process that uses hundreds of GB this can take on the order of seconds.

  Ruse defaults to RSS due to the performance impact. This is a pessimistic estimation, but in most cases the actual difference is fairly small. The memory estimation is meant to be a lower bound for job allocation, so being pessimistic is not really a problem. For clusters where [Slurm] uses RSS this is the best estimate. 

You can enable PSS with the "--pss" option. You can default to PSS at build time by passing "--enable-pss" to the configure invocation. Again, note that this can have a significant performance impact; avoid using short time periods if you do this.



* --help, --version

  Display a short help text with the options, and show the version of Ruse.




#### Interpret process usage

You will see a fair mumber of different patterns depending on the nature of the application. 

* Even pattern: 90.0 88.4 86.5 86.2
  
  This is typical for a CPU-bound highly parallel application. Each process is fully occupied doing calculation.

* One busy, others not: 90.0 12.1 8.9 7.3

  This can arise when you have a single-threaded application that periodically uses a well-optimized parallel numerical library. A Python or Matlab program that uses matrix operations will show this pattern for instance.

* Kind of iffy: 61.3 8.4 4.2 4.1 

  





## FAQ

#### Why is the sampling rate so low? 30 seconds is an eternity.

Ruse is not meant to profile quick programs on your local computer. It's meant for compute clusters, where an individual job typically takes hours, days or weeks to finish. 

When you run a job on a cluster you typically have to specify the resources — the number of nodes and cores, the amount of memory and the amount of time — you will need ahead of time. You neeed to know the peak memory usage, the number of cores you can use and the total time so you can ask for a sensible amount of these resources.

Ruse uses 30 seconds by default because [Slurm] — the job scheduler we use — samples the resources used by jobs at that rate. In practice, few applications will allocate and deallocate memory or threads so fast that we miss anything significant with a 30-second sample rate. 

Our lower limit is 1 second. If you need finer granularity than that, we suggest it is time for you to break out a real profiler.

#### Doesn't Slurm already tell you how much resources you use?

Yes, [Slurm] can be configured to show you the memory used. But the data is not simple to interpret unless you understand how [Slurm] works. This goes especially if your job has multiple subjobs, or uses MPI. Also, [Slurm] will not report the memory use of individual commands (unless you run them as subjobs) and will not give you a memory profile over time. 

[Slurm] will not give you any information on the number of processes or their CPU usage. With modern high-core systems this is becoming more and more important to get right.

Ruse attempts to report the data in a way that is easy to interpret and use to create a reasonable job submission. We also aim for Ruse to be lightweight enough that you can use it on production jobs without any performance penalty.

#### How does Ruse measure memory use, exactly?

By default, Ruse measures the RSS (Resident Set Size) each sample step. This is a simple and fast measurement that counts the amount of memory that is allocated and in use for the application. 

However, in practice this is often an over-estimation. The Linux kernel is smart; if multiple processes all want to access the same data, Linux will only store it once, then let the processes share it. A very common case is shared libraries. 

Almost all processes in a Linux system will link to a library called "libc", for instance. This library is about 2MB in size, and a Linux system easily has hundreds of processes running at once. If they all had their own copy in memory, you would use several hundred megabytes just for this one library. Instead, we have only a single (read-only) copy, and all processes on the system just use that. 

RSS counts those 2MB of libc fully for each and every application. PSS (Proprtional Set Size) is an alternative that takes this into account. Linux keeps track of the number of processes that have allocated a given memory area, and can calculate the proportion of that memory "belonging to" any given process. If five processes all use one memory area, they would each be responsible for 1/5 of the total.

The drawback is that to get the PSS for a process, you (as in, we or the kernel) have to go through each and every allocated memory area, look up how many processes have allocated this, then calculate the proportion owed by this process. A large application has thousands of separate memory areas, and in extreme cases estimating this calue can take seconds.

Also, in practice this often doesn't matter much. For scientific applications, especially those that are memory hungry, shared data such as hared libraries is only a small fraction of the memory needed. The vast bulk is input data or intermediate data structures that are unique to the running process. That memory allocation doesn't differ between RSS and PSS.

We default to using RSS for these reasons. It is far more efficient; it will tend to be pessimistic, so you won't go wrong using it as basis for your memory allocations; and in practice the difference is usually not large enough to worry about. 

#### What's the deal with the process measurement?

We are measuring the amount of CPU used for each process (including child processs and threads). A each step we collate these, and sort in descending order. This is added to a running total. At the end, the values are divided by the number of time steps to give you an average CPU use for each simultaneous process in the job.

There are two things to keep in mind here:

* We don't care about the identity of these processes. The first process CPU value at each time step is the value for whatever process happened to use the most CPU during that time step. The final total is the average of the most active process at each time step, whichever process that happened to be each time. 

* We don't track *core* usage at all. Due to issues such as core migration it is not possible to get it reliably correct with a sampling approach such as this. You would need to use more intrusive profiling methods for that.

This still tells you most of what you need to know. The percentage tells you how much CPU was used over time, and the number of simultaneous processes tell you the upper limit of the number of cores you could conceivably need.

Let's you had a total that looks like this:

```
Cores:         4
Total_procs:   4
Used_procs:    4
Proc(%): 90.0 65.0 40.0 33.3 
```

Your used processes equel the number of cores, and they are in fairly high use. You are making use of the cores you have allocated. It also looks like the third and fourth process is not contributing a huge amount, so increasing the number of cores is not that likely to help you very much.

If you have something like this:

```
Cores:         4
Total_procs:   2
Used_procs:    1
Proc(%): 75.0 
```

You have more cores than used processes. This tell you your application is not multithreaded. It can use only one core at a time, and allocating more is a waste of reasources. If it is supposed to use multiple cores, it's time to check the documentation or search online for how to enable that.

Finally, with something like this:

```
Cores:         16
Total_procs:   16
Used_procs:    16
Proc(%): 80.0 5.0 4.7 4.5 4.4 4.4 4.3 3.9 3.3 2.1 2.1 2.0 1.8 1.8 1.8 1.7  
```

One or a few processes are well-used, and the rest are used only very little. This tells you that your job can use multiple processes, but that these many - 16 - is probably too much. It's spending almost all its time on single-threaded processing, and only a small part of time actually running in parallel. You most likely want to reduce the number of cores to 8 or even less. You wuill lose very little time, and will free up resdources to, for instance, run another job like it in parallel.

#### Any future plans for Ruse?

We now have the ability to measure the process CPU use and to use RSS and PSS for memory estimation. This fulfils the plans I have had for Ruse up to this point, and I currently have no specific plans.

A couple of possible future directions that could be interesting:

* Find a non-intrusive, low-resource way to track core usage. I don't see a way to do that now.

* Rewrite it in Rust :) No, seriously, I would have benefited from using Rust for this, and if I had started Ruse this year I probably would have. 

I am open to any suggestions and patches!

[Slurm]: https://slurm.schedmd.com/
