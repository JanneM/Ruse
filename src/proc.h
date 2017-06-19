/* proc.h - helper functions for the /proc filesystem
 *
 * 2017 Jan Moren
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>

#include "arr.h"

/* system page size, for calculating the memory use */
extern int syspagesize;

typedef struct {
    int pid;
    int parent;
    long int rss;
} procdata;


/* extract the current RSS (resident set size) and parent process if for
 * process pid.  If the pid does not exist, return -1
*/

bool
read_RSS(int pid, long int *rss, int *parent);

/* get all process pids on the system */

iarr *
get_all_pids();

/* Get data on all current processes on the system, with kernel processes filtered away */
int
get_all_procs(procdata *procs, iarr *plist);

/* Get total RSS for process tree rooted in pid */
long int
get_RSS(int pid);

