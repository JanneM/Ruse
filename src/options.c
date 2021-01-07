/* options.c
 *
 * Parse user options.
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
#define _GNU_SOURCE

#include "options.h"

void 
show_help(const char *progname) {
    printf("Usage: %s [FLAGS] [--help] command [ARG...]\n", progname);
    printf("\
Measures the time and memory taken for a command and all its subprocesses.\n\
Also shows how much CPU time the processes take. \n\
\n\
To measure a command 'command', put 'ruse' in front\n\
\n\
  ruse command ....\n\
\n\
  -l, --label=LABEL      Prefix output with LABEL (default 'command')\n\
      --stdout           Don't save to a file, but to stdout\n\
      --no-header        Don't print a header line\n\
      --no-summary       Don't print summary info\n\
  -s, --steps            Print each sample step\n\
  -p, --procs            Print process information (default)\n\
      --no-procs         Don't print process information\n\
  -t, --time=SECONDS     Sample every SECONDS (default 30)\n\
\n");
#ifdef ENABLE_PSS
    printf("\
      --rss              use RSS for memory estimation\n\
      --pss              use PSS for memory estimation (default)\n");
#else
    printf("\
      --rss              use RSS for memory estimation (default)\n\
      --pss              use PSS for memory estimation\n");
#endif
    printf("\n\
      --help             Print help\n\
      --version          Display version\n\
\n");
    exit(EXIT_SUCCESS);
}


options*
get_options(int *argc, char **argv[]) {

    options *opts = malloc(sizeof(options));

#ifdef ENABLE_PSS
    opts->pss     = true;
    


#else
    opts->pss     = false;
#endif

    opts->verbose = false;
    opts->steps   = false;
    opts->procs   = true;
    opts->time    = 30;
    opts->nohead  = false;
    opts->nofile  = false;
    opts->nosum   = false;
    opts->label  = (char *)calloc(32, sizeof(char));

    
    int c;

    while (1) {
	int option_index = 0;
	static struct option long_options[] = {
//	    {"verbose",     no_argument,       0, 'v'},
	    {"version",     no_argument,       0,  1 },
	    {"help",        no_argument,       0,  2 },
	    {"label",       required_argument, 0, 'l'},
	    {"step",        no_argument,       0, 's'},
	    {"procs",       no_argument,       0, 'p'},
	    {"time",        required_argument, 0, 't'},
	    {"stdout",      no_argument,       0,  3 },
	    {"no-header",   no_argument,       0,  4 },
	    {"no-summary",  no_argument,       0,  5 },
	    {"no-procs",    no_argument,       0,  6 },
	    {"rss",         no_argument,       0,  7 },
	    {"pss",         no_argument,       0,  8 },
	    {0,             0,                 0,  0 }
	};

	c = getopt_long(*argc, *argv, "+hl:t:sp",
		long_options, &option_index);
	if (c == -1)
	    break;
	
	switch (c) {
	    
	    case 1:
		printf("%s %s\n", *argv[0], VERSION);
		exit(EXIT_SUCCESS);
		break;
	    case 2:
		show_help((**argv));
		exit(EXIT_SUCCESS);
		break;
	    
	    case 'l':
		strncpy(opts->label, optarg, 31);
		break;

	    case 's':
		opts->steps = true;
		break;
	    case 't':
		opts->time = atoi(optarg);
		if (opts->time<1) {
		    error(0, 0, "time must be a positive integer\n");
		    show_help((**argv));
		    exit(EXIT_FAILURE);
		}
		break;
	    case 3:
		opts->nofile = true;
		break;

	    case 4:
		opts->nohead = true;
		break;

	    case 5:
		opts->nosum = true;
		break;
	    case 'p':
		opts->procs = true;
		break;
	    case 6:
		opts->procs = false;
		break;
	    case 7:
		opts->pss = false;
		break;
	    case 8:
		opts->pss = true;
		break;
	    case '?':
    default:
		show_help((**argv));
		exit(EXIT_FAILURE);
		break;
	}

    }
    if (optind >= *argc) {
	error(0, 0, "missing a program to profile\n");
	show_help((**argv));
	exit(EXIT_FAILURE);
    }

    /* do we support rapid PSS information (kernel 4.18+) */
    char *fname;
    int res;
    FILE *f;
    pid_t pid = getpid();
    if ((res = asprintf(&fname, "/proc/%ld/smaps_rollup", (long)pid)) == -1) {
        error(EXIT_FAILURE, 0, "failed to create file name string.\n");
    }

    if ((f = fopen(fname, "r"))==NULL) {
        error(0, 0, "\n** Warning **\n\
/proc/*/smaps_rollup not supported on this system.\n\
Rapid PSS estimation not possible. Falling back on RSS.\n\
To remove this message, use \"--rss\" or rebuild without PSS.\n\n");
        opts->pss = false;
    }

    // point argv and argc to profiled program forwards
    *argc = *argc - optind;
    *argv = (*argv)+optind; 
    
    // create default label from profiled binary name
    if (strlen(opts->label) == 0) {
	char *bname =strdup((*argv)[0]);
	strncpy(opts->label, basename(bname) , 31);
	free(bname);
    }

    // Set output file handle
    if (opts->nofile) {
	opts->fhandle = stdout;
    } else {
	char *fname;
	pid_t pid = getpid();
	int res = asprintf(&fname, "%s-%ld.ruse", opts->label, (long)pid);
	if (res == -1) {
	    error(EXIT_FAILURE, 0, "failed to create output file name.\n");
	}

	if ((opts->fhandle = fopen(fname, "w"))==NULL) {
	    error(EXIT_FAILURE, errno, "failed to open output file.");
        }
    }
    return opts;
}
