/* proc.c - helper functions for the /proc filesystem
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

#include "proc.h"

int syspagesize=0;

bool
read_parent(int pid, int *parent) {

    int res;
    char *line = NULL;
    size_t len=0;
    char *field;
    char *fname;
    FILE *f;

    res = asprintf(&fname, "/proc/%i/stat", pid);
    if (res == -1) {
	fprintf(stderr, "Failed to convert proc file name\n");
	return false;
    }
    
    f = fopen(fname, "r");
    
    // pids may disappear. This is not an error
    if (!f) {
	return false;
    }

    if (getline(&line, &len, f) == -1) {
	fclose(f);
	fprintf(stderr, "Failed to read '%s': %s\n", fname, strerror(errno));
	return false;
    }
    
    char *line_tmp = line;

    field = strsep(&line_tmp, " ");
    field = strsep(&line_tmp, " ");
    field = strsep(&line_tmp, " ");
    field = strsep(&line_tmp, " ");
    *parent = atol(field);
    
    // ignore kernel processes
    if (*parent == 2) {
	fclose(f);
	return false;
    }
    if(line)
	free(line);

    fclose(f);    
    return true;
}

bool
read_stat(int pid, size_t *rss, pstruct *pstr) {

    int i;
    int res;
    char *line = NULL;
    size_t len=0;
    char *field;
    char *fname;
    FILE *f;
    unsigned long utime;
 //   unsigned long long starttime;
    int core;

    res = asprintf(&fname, "/proc/%i/stat", pid);
    if (res == -1) {
	fprintf(stderr, "Failed to convert proc file name\n");
	return false;
    }
    
    f = fopen(fname, "r");
    
    // pids may disappear. This is not an error
    if (!f) {
	return false;
    }

    if (getline(&line, &len, f) == -1) {
	fclose(f);
	fprintf(stderr, "Failed to read '%s': %s\n", fname, strerror(errno));
	return false;
    }
    
    char *line_tmp = line;

    for(i=0; i<39; i++) {
	field = strsep(&line_tmp, " ");
	switch(i) {
	    case 13:	// execute time
		utime = atol(field);
		continue;
/*	    case 21:	// process start time
		starttime = atoll(field);
		continue;
*/	    case 23:	// rss
		*rss = atol(field);
		continue;
	    case 38:	// core
		core = atoi(field);
		continue;
	}
    }

    if(line)
	free(line);

    fclose(f);    
    *rss  *= syspagesize;
    return true;
}

/* get all pids on the system */
iarr *
get_all_pids() {
    
    DIR *df;
    struct dirent *dir;
    size_t res;
    iarr *plist;
    if ((df = opendir("/proc")) == NULL){
	perror("get_all_pids:");
	// Should we possibly not fail here?
	exit(EXIT_FAILURE);
    }

    if ((plist = iarr_create(4)) == NULL) {
	closedir(df);
	error(EXIT_FAILURE, 0, "failed to create PID list container");
    }

    while ((dir = readdir(df)) != NULL) {
	
	if (dir->d_type != DT_DIR) {
	    continue;
	}
	
	/* any entry in /proc that begins with a digit is _probably_ a pid.
	 * If it's not, looking up the data will fail and we'll just skip it,
	 * so no harm done. */
	if ((dir->d_name[0]<'0') || (dir->d_name[0]>'9')) {
	    continue;
	}
	res = atol(dir->d_name);
	
	if (iarr_insert(plist, (int)res) == false) {
	    error(EXIT_FAILURE, 0, "failed to insert into PID list container");
        }
    }
    closedir(df);
    return plist;
}


int
get_all_procs(procdata *procs, iarr *plist) {
  
    int elems;
    int pidc, procc;
    int pid;
	
    elems = plist->len;

    // look up, fill in data for all procs
    
    pidc = 0;
    procc = 0;
    for (pidc = 0; pidc<elems; pidc++) {
	pid = plist->ilist[pidc];
	if ((read_parent(pid, &(procs[procc].parent))) == true) {
	    procs[procc].pid = pid;
	    procc++;
	}
    }
    return procc;
}

size_t
get_RSS_r(int pid, procdata *p, int l, pstruct *pstr) {
    
    size_t rss=0;
    size_t get_rss;

    for (int i=0; i<l; i++) {
	if (p[i].parent == pid) {
#ifdef DEBUG
    printf("%d ", p[i].pid);
#endif
	    read_stat(p[i].pid, &get_rss, pstr);
	    rss += get_rss + get_RSS_r(p[i].pid, p, l, pstr);
	}
    }
    return rss;
}


size_t
get_RSS(int pid, pstruct *pstr) {

    int elems;
    iarr *plist;
    procdata *procs;
    size_t rss = 0;
    size_t get_rss;


    if ((plist = get_all_pids()) == NULL) {
	exit(EXIT_FAILURE); // should be caught in get_all_pids
    }
    
    if (plist->len < 1) {
	error(EXIT_FAILURE, 0, "failed to get process pid list.\n");
    }

    if ((procs = calloc(plist->len, sizeof(procdata))) == NULL) {
	perror("get_all_procs");
	exit(EXIT_FAILURE);
    }

    elems = get_all_procs(procs, plist);
    for (int i=0; i<elems; i++) {
	if (procs[i].pid == pid) {
#ifdef DEBUG
    printf("procs: %d ", pid);
#endif
	    read_stat(pid, &get_rss, pstr);
	    rss = get_rss + get_RSS_r(pid, procs, elems, pstr);
	    break;
	}
    }
#ifdef DEBUG
    printf("\n");
#endif
    return rss;
}
