/* ruse_omp.c 
 *
 * OpenMP threads with specified memory allocation.
 *
 * Run this with Ruse as:
 * 
 *     ruse -t1 -s --stdout ./ruse_omp
 *
 * You should see output similar to:
 *
 * time(s)   mem(MB)
 * 0         1.5
 * 1         11.4
 * 2         13.4
 * 3         21.4
 * 4         21.4
 * 5         21.4
 * 6         11.6
 * 7         11.6
 * 
 * Time(s):  7
 * Mem(MB):  21.4
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

#include "options_omp.h"
#include "do_task.h"

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

int
main(int argc, char*argv[]) {

    options *opts = get_options(&argc, &argv);

    /* Run a number of iterations of alternating OpenMP parallel threads 
     * and single-process execution.
     */

    for (int j=0; j<opts->iter; j++) {

	#pragma omp parallel for
	for (int i=0; i<opts->procs; i++) {
	    char *mem = memalloc(10);
	    do_task(opts->time, opts->busy);
	    free(mem);    
	}
	if (opts->single>0) {	
	    do_task(opts->single, opts->busy);
	}
    }
}
