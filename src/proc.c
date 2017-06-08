/* proc.c - helper functions for the /proc filesystem
 *
 * 2017 Jan Moren
 *
 */
#define _GNU_SOURCE
#include "proc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int syspagesize=0;

int
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
	return -1;
    }
    
    f = fopen(fname, "r");
    if (!f) {
	fprintf(stderr, "Failed to open '%s': %s\n", fname, strerror(errno));
	return -1;
    }

    if (getline(&line, &len, f) == -1) {
	fprintf(stderr, "Failed to read '%s': %s\n", fname, strerror(errno));
	return -1;
    }
    
    char *line_tmp = line;

    for(i=0; i<24; i++) {
	field = strsep(&line_tmp, " ");
	switch(i) {
	    case 3:	// parent
		res = sscanf(field, "%i", parent);
		continue;
	    case 23:	// rss
		res = sscanf(field, "%li", rss);
		continue;
	}
	if (res == EOF) {
	    fprintf(stderr, "Failed to scan '%s': %s\n", fname, strerror(errno));
	    return -1;
	}
    }

    if(line)
	free(line);
    
    *rss = (*rss)*syspagesize;
    return 0;
}





