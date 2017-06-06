/* ruse - show memory usage and wall-clock time for a process
 *
 * 2017 Jan Moren
 *
 */

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "proc.h"

#define KB 1024
#define MAX(x,y) ((x) > (y) ? (x): (y))
#define CLOCKID CLOCK_REALTIME
#define SIG SIGUSR1

/* Our defined timer */
timer_t timerid;

/* get the signal ID out from the callback */
volatile sig_atomic_t sigtype;

void 
show_help(const char *progname)
{
    printf("Usage: %s [-o file] [--help] command [arg...]", progname);
    exit(EXIT_SUCCESS);
}

/* Signal handler. Accepts a timer signal with the right timer ID or a SIGCHLD
 * event.
*/
void
sighandler(int signal, siginfo_t *siginfo, void *uc) {
    
    if (signal==SIG &&
        *(timer_t*)(siginfo->si_value.sival_ptr) == timerid) {
	    sigtype = signal;
	
    } else if (signal==SIGCHLD) {
	sigtype=signal;

    } else {
	sigtype=0;
    }
}

int 
main(int argc, char *argv[])
{
    time_t t1,t2;
    struct sigaction sa_time, sa_chld;
    struct sigevent sev;
    struct itimerspec its; 
    long int mem;
    long int rss = 0;
    int parent = 0;
    sigset_t mask;
    sigset_t old_mask;

    syspagesize = getpagesize()/KB;

    /* block signals from the child until we're ready 
     * to receive them
    */
    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &old_mask);

    /* Time the process*/
    time(&t1);

    pid_t pid = fork();
    if (pid < 0)
    {
	perror("fork failed");
	return EXIT_FAILURE;
    }
    if (pid == 0)
    {
	/* We're the child */
	execvp(argv[1], &argv[1]);
	perror("execvp() failed");
	exit(EXIT_FAILURE);
    }

    /* We're the parent */

    /* Get child exit signals, but not stop or cont */
    sa_chld.sa_flags = SA_SIGINFO|SA_NOCLDSTOP;
    sa_chld.sa_sigaction = sighandler;
    sigemptyset(&sa_chld.sa_mask);
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
	perror("sigaction");
	exit(EXIT_FAILURE);
    }

    /* catch SIG from timer */
    sa_time.sa_flags = SA_SIGINFO;
    sa_time.sa_sigaction = sighandler;
    sigemptyset(&sa_time.sa_mask);
    if (sigaction(SIG, &sa_time, NULL) == -1) {
	perror("sigaction");
	exit(EXIT_FAILURE);
    }

    /* set up a timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCKID, &sev, &timerid) == -1) {
	perror("timer_create");
	exit(EXIT_FAILURE);
    }

    /* Start the timer */
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    if (timer_settime(timerid, 0, &its, NULL) == -1){
	perror("timer_settime");
	exit(EXIT_FAILURE);
    }

    mem = 0;

    while(1) {
	sigsuspend(&old_mask);
	printf("type: %s\n", strsignal(sigtype));

	if (sigtype == SIG) {
	    if (read_RSS(pid, &rss, &parent) == -1) {
		fprintf(stderr, "read_RSS failure\n");
		exit(EXIT_FAILURE);
	    }

	    printf("mem: %li\n", rss);
	    mem = MAX(mem, rss); 

	} else if (sigtype == SIGCHLD) {
	    break;	
	}
    }

    time(&t2);
    int status;
    waitpid(pid, &status, 0);
    printf("Mem(MB): %li\nTime(s): %li\n", mem, (t2-t1));

    return EXIT_SUCCESS;
}
