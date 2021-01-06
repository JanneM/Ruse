/* proc.c - read /proc filesystem info
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
#include <errno.h>
int syspagesize=0;

/* extract the parent process for
 * process pid.  If the pid does not exist, return -1
*/
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
	error(0,0, "Failed to convert proc file name\n");
	return false;
    }
    
    f = fopen(fname, "r");
    // pids may disappear. This is not an error
    if (!f) {
	return false;
    }

    if (getline(&line, &len, f) == -1) {
	fclose(f);
	error(0, errno, "Failed to read '%s'", fname);
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

/* read current used memory as PSS */
bool
read_pss_mem(int pid, size_t *mem) {

    int i;
    int res;
    char line[128];
    char *fname;
    FILE *f;

    if ((res = asprintf(&fname, "/proc/%i/smaps_rollup", pid)) == -1) {
	error(0,0, "Failed to convert smaps_rollup path\n");
	return false;
    }
    
    f = fopen(fname, "r");
    free(fname); 
    // pids may disappear. This is not an error
    if (!f) {
	return false;
    }
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "Pss:", 4) != 0) {
            continue;
        }

        for (i = 4; i < sizeof(line); i++) {

            if (!isdigit(line[i])) {
                continue;
            }
            if (sscanf(&line[i],"%lu", mem) != 1) {
                error(0, 0, "failed to read PSS field.");
            }
            break;
        }
    }
    fclose(f);
    return true;
}

/* read current used memory as RSS */
bool
read_rss_mem(int pid, size_t *mem) {

    int i;
    int res;
    char *line = NULL;
    size_t len=0;
    char *field;
    char *fname;
    FILE *f;

    res = asprintf(&fname, "/proc/%i/stat", pid);
    if (res == -1) {
	error(0,0, "Failed to convert proc file name\n");
	return false;
    }
    
    f = fopen(fname, "r");
    free(fname); 
    // pids may disappear. This is not an error
    if (!f) {
	return false;
    }

    if (getline(&line, &len, f) == -1) {
	fclose(f);
	error(0, errno, "Failed to read '%s'", fname);
	return false;
    }
    
    char *line_tmp = line;

    for(i=0; i<24; i++) {
	field = strsep(&line_tmp, " ");
    }
    *mem = atol(field);

    if(line)
	free(line);

    fclose(f);    
    *mem  *= syspagesize;
    return true;
}

/* read current actually used memory */
inline bool
read_mem(int pid, size_t *mem, bool use_pss) {

    if (use_pss) {
        return read_pss_mem(pid, mem);
    } else {
        return read_rss_mem(pid, mem);
    }
}


/* read thread/process usage */
bool
read_threads(int pid, pstruct *pstr) {
    int i;
    int res;
    unsigned long tnum;
    
    char *line = NULL;
    size_t len=0;
    char *field;
    char *dname;
    char *fname;
    DIR *df;
    struct dirent *dir;
    FILE *f;
    int core = -1;
    unsigned long utime = 0;

    res = asprintf(&dname, "/proc/%i/task", pid);
    if (res == -1) {
	error(0,0, "Failed to create task dir name\n");
	return false;
    }
    df = opendir(dname);
    free(dname);

    if (df == NULL) {
	// processes may suddenly disappear. This is not a failure.
	return true;
    }

    while ((dir = readdir(df)) != NULL) {
	
	if (dir->d_type != DT_DIR) {
	    continue;
	}
	
        errno = 0;
	tnum = strtoul(dir->d_name, NULL, 10);
        // '.' and '..' convert to "valid" 0 values
        if (errno !=0 || tnum == 0) {
            continue;
        }

        res = asprintf(&fname, "/proc/%i/task/%li/stat", pid, tnum);
        if (res == -1) {
	    error(0,0, "Failed to convert proc file name\n");
	    return false;
        }
    
        f = fopen(fname, "r");
        if (fname != NULL) {
            free(fname);
            fname = NULL;
        }
    
        // pids may disappear. This is not an error.
        if (f == NULL) {
	    continue;
        }

        if (getline(&line, &len, f) == -1) {
            fclose(f);
            free(line);
            error(0, errno, "Failed to read stat from '%ld'", tnum);
            return false;
        }
        
        char *line_tmp = line;
        core=-1;
        for(i=0; i<39; i++) {
            field = strsep(&line_tmp, " ");
            switch(i) {
//                case 2:
//                    strncpy(state, field, 2);
//                    continue;
                case 13:	// execute time
                    utime = atol(field);
                    continue;
                case 38:	// core
                    core = atoi(field);
                    continue;
            }
        }

        free(line);
        line = NULL;
        len = 0;

        fclose(f);
        add_thread(pstr, tnum, utime, core); 

    } // readdir

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
	error(0,errno, "get_all_pids:");
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

/* get all procs on the system */
int
get_all_procs(procdata *procs, iarr *plist) {
  
    int elems;
    int pidc, procc;
    int pid;
	
    elems = plist->len;
    
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

/* Get total RSS and process usage for process tree rooted in pid */
size_t
get_process_data_r(int pid, procdata *p, int l, pstruct *pstr, bool use_pss) {
    
    size_t mem=0;
    size_t proc_mem;

    for (int i=0; i<l; i++) {
	if (p[i].parent == pid) {
#ifdef DEBUG
    printf("%d ", p[i].pid);
#endif
	    read_mem(p[i].pid, &proc_mem, use_pss);
            read_threads(p[i].pid, pstr);
	    mem += proc_mem + get_process_data_r(p[i].pid, p, l, pstr, use_pss);
	}
    }
    return mem;
}


/* Get total RSS and process usage for process tree rooted in pid */
size_t
get_process_data(int pid, pstruct *pstr, bool use_pss) {

    int elems;
    iarr *plist;
    procdata *procs;
    size_t mem = 0;
    size_t proc_mem;


    if ((plist = get_all_pids()) == NULL) {
	exit(EXIT_FAILURE); // should be caught in get_all_pids
    }
    
    if (plist->len < 1) {
	error(EXIT_FAILURE, 0, "failed to get process pid list.\n");
    }

    if ((procs = calloc(plist->len, sizeof(procdata))) == NULL) {
	error(0,errno, "get_all_procs");
	exit(EXIT_FAILURE);
    }

    elems = get_all_procs(procs, plist);

    if (do_thread_iter(pstr) == false) {
        exit(EXIT_FAILURE);
    }

    for (int i=0; i<elems; i++) {
	if (procs[i].pid == pid) {
#ifdef DEBUG
    printf("procs: %d ", pid); fflush(stdout);
#endif
	    read_mem(pid, &proc_mem, use_pss);

            read_threads(pid, pstr);
	    mem = proc_mem + get_process_data_r(pid, procs, elems, pstr, use_pss);
	    break;
	}
    }
#ifdef DEBUG
    printf("\n");
#endif
    thread_summarize(pstr);
    return mem;
}
