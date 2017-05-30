/* memuse - show memory usage and wall-clock time for a process
 *
 * 2017 Jan Moren
 *
 */

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#define KB 1024
#define MB (KB*KB)

void help(const char *cname)
{
    printf("Usage: %s [-o file] [--help] command [arg...]", cname);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    struct rusage r;
    time_t t1,t2;

    pid_t pid = fork();
    if (pid < 0)
    {
	fprintf(stderr, "fork(): %s\n", strerror(errno));
	return EXIT_FAILURE;
    }
    if (pid == 0)
    {
	execvp(argv[1], &argv[1]);
	fprintf(stderr, "%s: %s \n", argv[1], strerror(errno));
	exit(EXIT_FAILURE);
    }
    else
    {	
	int s=0;
	time(&t1);
	int err = waitpid(pid, &s, 0);
	time(&t2);
	if (err == -1) {
	    fprintf(stderr, "Failed to get child process\n");
	    exit(EXIT_FAILURE);
	}
	getrusage(RUSAGE_CHILDREN, &r);
	printf("Mem(MB): %li\nTime(s): %li\n", (long int)ceil(1.0*r.ru_maxrss/KB), (t2-t1));
    }
    return EXIT_SUCCESS;
}
