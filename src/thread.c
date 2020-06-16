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

    t_struct *s1 = (t_struct *)p1;
    t_struct *s2 = (t_struct *)p2;
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

/* tree traversal callback */
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
	    printf("%6d\t%6ld\n", (int)datap->pid, datap->utime);
	    break;
	case leaf:
	    datap = *(t_struct **) nodep;
	    printf("%6d\t%6ld\n", (int)datap->pid, datap->utime);
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
	perror("create_pstruct");
	return NULL;
    }

    f = fopen("/proc/uptime", "r");
    if (getline(&line, &len, f) == -1) {
	fclose(f);
	fprintf(stderr, "Failed to read '/proc/uptime': %s\n", strerror(errno));
	return NULL;
    }
    pstr->ptime = atof(line);
    pstr->dtime = -1.0;

    /* fill in hardware information */
    cpu_set_t cpumask;
    int res;
    if ((res = sched_getaffinity(0, sizeof(cpu_set_t), &cpumask))==-1) {
	perror("create_pstruct");
	return NULL;
    }
    pstr->hw_cores = sysconf(_SC_NPROCESSORS_ONLN);
    pstr->max_cores = CPU_COUNT(&cpumask);
    pstr->jiffy = sysconf(_SC_CLK_TCK);

#ifdef DEBUG
    printf("system cores: %ld\n", pstr->hw_cores);
    printf("   max cores: %ld\n", pstr->max_cores);
    printf("     jiffies: %d\n", pstr->jiffy);
#endif
    pstr->proot = NULL; 
    if ((pstr->tstr = malloc(sizeof(t_struct)))==NULL) {
	perror("create_pstruct");
	return NULL;
    }

    if ((pstr->cores = malloc(sizeof(double)*(pstr->hw_cores)))==NULL) {
	perror("create_pstruct");
	return NULL;
    }
    if ((pstr->cores_acc = malloc(sizeof(double)*(pstr->hw_cores)))==NULL) {
	perror("create_pstruct");
	return NULL;
    }
    /* initialize core lists */
    for (int i=0; i<pstr->hw_cores; i++) {
	pstr->cores[i] = -1.0;
	pstr->cores_acc[i] = 0.0;
    }
    pstr->using_cores = 0;
    pstr->peak_cores = 0;
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

    /* reset temporary core list */
    for (int i=0; i<pstr->hw_cores; i++) {
	pstr->cores[i]=-1.0;
    }
    f = fopen("/proc/uptime", "r");
    if (getline(&line, &len, f) == -1) {
	fclose(f);
	fprintf(stderr, "Failed to read '/proc/uptime': %s\n", strerror(errno));
	return false;
    }
    uptime = atof(line);
    pstr->dtime = uptime - pstr->ptime;
    pstr->ptime = uptime;
    return true;
}


bool 
add_thread(pstruct *pstr, int pid, unsigned long utime, int core) {

    void *res;
    t_struct *tval = pstr->tstr;
    t_struct *resval;
    unsigned long udiff;

    tval->pid = pid;
    tval->utime = 0;
    res = tsearch((void *)tval, &(pstr->proot), tstruct_cmp);
    if (res == NULL) {
	return false;
    }
    resval = *(t_struct **) res;
    /* new entry */
    if (resval == tval) {
	if ((pstr->tstr = malloc(sizeof(t_struct)))==NULL) {
	    perror("add_thread");
	    return false;
	}
    }

//    tdiff = uptime - max(starttime, resval->uptime); 
//    tdiff = uptime - resval->uptime;
    
    udiff = utime - resval->utime;
    resval->utime = utime;
    if (pstr->dtime>0.0) {
	if (pstr->cores[core]<0.0) {
	   pstr->cores[core] = udiff/pstr->dtime;
	} else {
	   pstr->cores[core] += udiff/pstr->dtime;
	}
    }
    return true;
}

// get a sorted list and number of members
bool
thread_summarize(pstruct *pstr) {
    
    pstr->iter++;
    int cmin=-1;
    int cmax=-1;
    int cores=0;


    for (int i=0; i<pstr->hw_cores; i++) {
	if (cmin == -1 && pstr->cores[i]>=0.0) {
	    cmin = i;
	}
	if (cmax == -1 && pstr->cores[i]<0.0) {
	    cmax = i;
	    break;
	}

    }
    // sort cores in place
    qsort(&(pstr->cores[cmin]), cmax-cmin, sizeof(double), double_cmp);
    
    for (int i=cmin; i<(cmax-cmin); i++) {
	pstr->cores_acc[cores] += pstr->cores[i];
	cores++;
    }
    pstr->using_cores = cores;
    if (cores>pstr->peak_cores) {
	pstr->peak_cores = cores;
    }
    return true;
}
