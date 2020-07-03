/* thread.h - keep thread information 
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


#ifndef THREAD_H
#define THREAD_H
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <search.h>
#include <string.h>

typedef struct {
    pid_t pid;
    unsigned long utime;
} t_struct;

typedef struct {
    void *proot;
    t_struct *tstr;

    double *cores;		    // array of temporary cores
    double *cores_cur;		    // array of current cores
    double *cores_acc;		    // array of accumulated cores
    long int hw_cores;		    // number of available cores in hardware
    unsigned int max_cores;	    // number of allocated cores
    unsigned int using_cores;	    // number of cores used this iteration
    unsigned int peak_cores;	    // peak number of cores
    unsigned int iter;		    // iterations
   
    int jiffy;
    double ptime;
    double dtime;
} pstruct;


// Create a process tree and a core list
pstruct * 
create_pstruct();


// reset core list for the next iteration
bool 
do_thread_iter(pstruct *pstr);

// query/add a process to the tree, and update core list
bool
add_thread(pstruct *pstr, int pid, unsigned long utime, int core);

// get a sorted list and number of members
bool
thread_summarize(pstruct *pstr);

void
print_tree(pstruct*pstr);
#endif
