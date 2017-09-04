/* proc.h - helper functions for the /proc filesystem
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


#ifndef PROC_H
#define PROC_H
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>
#include "arr.h"


/* system page size, for calculating the memory use */
extern int syspagesize;

typedef struct {
    int pid;
    int parent;
    size_t rss;
} procdata;


/* extract the current RSS (resident set size) and parent process for
 * process pid.  If the pid does not exist, return -1
 */

bool
read_RSS(int pid, size_t *rss, int *parent);

/* get all process pids on the system */

iarr *
get_all_pids();

/* Get data on all current processes on the system 
 */

int
get_all_procs(procdata *procs, iarr *plist);

/* Get total RSS for process tree rooted in pid */

size_t
get_RSS(int pid);

#endif
