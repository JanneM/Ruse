#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char *argv[]) {

    cpu_set_t cpumask;
    int res = sched_getaffinity(0, sizeof(cpu_set_t), &cpumask);
    long totprocs = sysconf(_SC_NPROCESSORS_ONLN);
    int nprocs = CPU_COUNT(&cpumask);
    
    int jiffy = sysconf(_SC_CLK_TCK);
	
    printf("cores (total): %d (%ld)\n", nprocs, totprocs);
    printf("jiffy: %d\n", jiffy);
    

}


