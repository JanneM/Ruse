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
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <search.h>
#include <string.h>
#include "arr.h"

/* tree node structure */
typedef struct {
    pid_t pid;                      // process PID
    unsigned long utime;            // time at last update
} t_struct;

typedef struct {
    void *proot;                    // process tree root    
    t_struct *tstr;                 // tree node structure

    long int hw_cores;		    // number of available cores in hardware
    unsigned int max_cores;	    // number of allocated cores

    darr *proc_cur;                 // current process use
    darr *proc_acc;                 // accumulated process use     
    unsigned int iter;		    // iterations
   
    int jiffy;                      // jiffies. Not currently used
    double ptime;                   // current time since start
    double dtime;                   // time since last iteration
} pstruct;


/* Create a process tree and a process list */
pstruct * 
create_pstruct();


/* reset process list for the next iteration */
bool 
do_thread_iter(pstruct *pstr);

/* query/add a process to the tree, and poopulate process list */
bool
add_thread(pstruct *pstr, int pid, unsigned long utime, int core);

/* get a sorted process list, update accumulated process time */
bool
thread_summarize(pstruct *pstr);

/* print out the current tree. For debugging. */
void
print_tree(pstruct *pstr);
#endif
