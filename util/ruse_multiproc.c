/* ruse_multiproc.c 
 *
 * Create multiple processes with specified memory allocation.
 *
 * Run this with Ruse as:
 * 
 *     ruse -t1 -s --stdout ./ruse_multiproc
 *
 * You should see output similar to:
 *
 * time(s)   mem(MB)
 * 0         0.7
 * 1         11.4
 * 2         13.5
 * 3         22.1
 * 4         22.1
 * 5         22.1
 * 6         11.4
 * 7         11.4
 * 
 * Time(s):  7
 * Mem(MB):  22.1
 * 
 *
 * Copyright 2017 Jan Moren
 *
 * This file is part of Ruse.
 *
 * Ruse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ruse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ruse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#define KB 1024
#define MB (KB*KB)

/* Allocate and touch a set amount of memory */
char *
memalloc(int mb) {
    char *mem = (char*)malloc(mb*MB);
    for(int i=0; i<mb*MB; i++) {
	mem[i] = 1;
    }
    return mem;
}

/* fork a new process that allocates memory, then sleeps */
pid_t
dofork(int stime){
    pid_t pid = fork();

    if (pid < 0)
    {
        error(EXIT_FAILURE, 0, "fork failed");
    }
    if (pid == 0)
    {
        /* We're the child */
	char *mem = memalloc(10);
	sleep(stime);
	free(mem);
	exit(0);
    }
    return pid;
}

int
main(int argc, char*argv[]) {

    int wstatus;

    /* create two 5-second processes, each allocating 10MB, with a
     * 2-second delay. 
     */

    dofork(5);
    sleep(2);
    dofork(5);

    wait(&wstatus);
    wait(&wstatus);
}
