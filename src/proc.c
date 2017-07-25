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

typedef struct {
    int pid;
    int parent;
    long int rss;
} procstr;

bool
read_RSS(int pid, long int *rss, int *parent) {

    int i;
    int res;
    char *line = NULL;
    size_t len=1023;
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
	fprintf(stderr, "Failed to read '%s': %s\n", fname, strerror(errno));
	return false;
    }
    
    char *line_tmp = line;

    for(i=0; i<24; i++) {
	field = strsep(&line_tmp, " ");
	switch(i) {
	    case 3:	// parent
		*parent = atol(field);
		
		// ignore kernel processes
		if (*parent == 2) {
		    fclose(f);
		    return false;
		}
		continue;
	    case 23:	// rss
		*rss = atol(field);
		continue;
	}
    }

    if(line)
	free(line);

    fclose(f);    
    *rss  *= syspagesize;
    return true;
}

iarr *
get_all_pids() {
    
    DIR *df;
    struct dirent *dir;
    long int res;
    iarr *plist;

    if ((df = opendir("/proc")) == NULL){
	perror("get_all_pids:");
	exit(EXIT_FAILURE);
    }
    
    if ((plist = iarr_create(4)) == NULL) {
	exit(EXIT_FAILURE);
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
	    exit(EXIT_FAILURE);
        }
    }
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
	if ((read_RSS(pid, &(procs[procc].rss), &(procs[procc].parent))) == true) {
	    procs[procc].pid = pid;
	    procc++;
	}
    }
    return procc;
}

long int
get_RSS_r(int pid, procdata *p, int l) {
    
    long int rss=0;
    for (int i=0; i<l; i++) {
	if (p[i].parent == pid) {
	    rss += p[i].rss + get_RSS_r(p[i].pid, p, l);
	}
    }
    return rss;
}


long int
get_RSS(int pid) {

    int elems;
    iarr *plist;
    procdata *procs;
    long int rss = 0;

    if ((plist = get_all_pids()) == NULL) {
	exit(EXIT_FAILURE);
    }

    if (plist->len < 1) {
	fprintf(stderr, "failed to get process pid list.\n");
	exit(EXIT_FAILURE);
    }

    if ((procs = calloc(plist->len, sizeof(procdata))) == NULL) {
	perror("get_all_procs");
	exit(EXIT_FAILURE);
    }

    elems = get_all_procs(procs, plist);
    
    for (int i=0; i<elems; i++) {
	if (procs[i].pid == pid) {
	    rss = procs[i].rss + get_RSS_r(pid, procs, elems);
	    break;
	}
    }
    return rss;
}
