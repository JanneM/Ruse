/* thread.c - keep thread information 
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

#include "thread.h"

/* t_struct comparison function */
static int tstruct_cmp(const void *p1, const void *p2) {

    const t_struct *s1 = p1;
    const t_struct *s2 = p2;
    if (s1->pid < s2->pid)
	return -1;
    if (s1->pid > s2->pid)
	return 1; 
    return 0;
}

/* note: sorting in reverse order */
int
double_cmp (const void *a, const void *b)
{
  const double *da = (const double *) a;
  const double *db = (const double *) b;

  return (*da < *db) - (*da > *db);
}

/* tree traversal callback 
 * used for debuging only.
 */
static void
tstruct_action(const void *nodep, VISIT which, int depth)
{
    t_struct *datap;
    switch (which) {
        case preorder:
	case endorder:
	    break;
	case postorder:
	    datap = *(t_struct **) nodep;
	    printf("tree: %6d\t%6ld (intl)\n", (int)datap->pid, datap->utime);
	    break;
	case leaf:
	    datap = *(t_struct **) nodep;
	    printf("tree: %6d\t%6ld (leaf)\n", (int)datap->pid, datap->utime);
	    break;
    }
}

/* create a process tree, core lists and initialize */
pstruct * 
create_pstruct() {
    
    pstruct *pstr;
    FILE *f;
    char *line = NULL;
    size_t len = 0;

    if ((pstr = malloc(sizeof(pstruct)))==NULL) {
	error(0,errno, "create_pstruct: allocate pstruct");
	return NULL;
    }

    if ((f = fopen("/proc/uptime", "r"))==NULL) {
	error(0,errno, "create_pstruct: Failed to open '/proc/uptime'");
        return false;
    }
    if (getline(&line, &len, f) == -1) {
	fclose(f);
	error(0, errno, "Failed to read '/proc/uptime'");
	return NULL;
    }
    pstr->ptime = atof(line);
    pstr->dtime = -1.0;

    /* fill in hardware information */
    cpu_set_t cpumask;
    int res;
    if ((res = sched_getaffinity(0, sizeof(cpu_set_t), &cpumask))==-1) {
	error(0,errno, "create_pstruct: get affinity mask");
	return NULL;
    }
    pstr->hw_cores = sysconf(_SC_NPROCESSORS_ONLN);
    pstr->max_cores = CPU_COUNT(&cpumask);
    pstr->jiffy = sysconf(_SC_CLK_TCK);

#ifdef DEBUG
    printf("system cores: %ld\n", pstr->hw_cores);
    printf("   max cores: %d\n", pstr->max_cores);
    printf("     jiffies: %d\n", pstr->jiffy);
#endif
    pstr->proot = NULL; 
    if ((pstr->tstr = malloc(sizeof(t_struct)))==NULL) {
	error(0,errno, "create_pstruct: allocate t_struct");
	return NULL;
    }

    /* process usage for current interation, and total 
     * accumulated usage */
    pstr->proc_cur = darr_create(4);
    pstr->proc_acc = darr_create(4);

    pstr->iter = 0;

    return pstr;
}

/* set up for another iteration */
bool
do_thread_iter(pstruct *pstr) {

    FILE *f;
    char *line = NULL;
    size_t len = 0;
    double uptime;

    /* reset process list */
    darr_reset(pstr->proc_cur);

    if ((f = fopen("/proc/uptime", "r"))==NULL) {
	error(0,errno, "Failed to open '/proc/uptime'");
        return false;
    }
        
    if (getline(&line, &len, f) == -1) {
	fclose(f);
	error(0,errno, "Failed to read '/proc/uptime'");
	return false;
    }

    uptime = atof(line);
    pstr->dtime = uptime - pstr->ptime;
    pstr->ptime = uptime;

    return true;
}

/* add or update a thread/process in our collection.*/ 
bool 
add_thread(pstruct *pstr, pid_t pid, unsigned long utime, int core) {

    void *res;
    t_struct *tval;
    t_struct *resval;
    unsigned long udiff;

    if ((tval = calloc(sizeof(t_struct),1))==NULL) {
	error(0,errno, "failed creating tval:");
    }

    tval->pid = pid;
    tval->utime = 0;

    res = tsearch((void *)tval, &(pstr->proot), tstruct_cmp);

    /* should never actually happen - tsearch always returns 
     * a valid result */
    if (res == NULL) {
	return false;
    }
    resval = *(t_struct **) res;

#ifdef DEBUG
    printf("thread res# %d, %ld\n", resval->pid, resval->utime);
#endif

    /* old entry, so deallocate our temporary struct */
    if (resval != tval) {
        free(tval);
    }
    
    udiff = utime - resval->utime;
    resval->utime = utime;

    /* if we haven't just started, fill a list of time spent running 
     * since last iteration */
    if (pstr->dtime>0.0) {
        darr_insert(pstr->proc_cur, udiff/pstr->dtime);
    }
    
    return true;
}

/* get a sorted list and number of members */
bool
thread_summarize(pstruct *pstr) {
    
    pstr->iter++;

#ifdef DEBUG
    print_tree(pstr);
#endif    
    
    // sort process use
    qsort(pstr->proc_cur->dlist, pstr->proc_cur->len, sizeof(double), double_cmp);
    
    int pdiff;
    if ((pdiff = pstr->proc_cur->len - pstr->proc_acc->len)>0) {
        for (int i=0; i<pdiff; i++) {
            darr_insert(pstr->proc_acc, 0.0);
        }
    }

    for (int i=0; i<pstr->proc_cur->len; i++) {
        pstr->proc_acc->dlist[i] += pstr->proc_cur->dlist[i];
    }

    return true;
}

/* print tree of tracker processes. used for debugging. */
void
print_tree(pstruct *pstr) {
    
    printf("--- tree ---\n");
    twalk( (pstr->proot), tstruct_action );
}

