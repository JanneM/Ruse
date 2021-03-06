/* options_multiproc.c
 *
 * Parse user options for ruse_multiproc.c
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

#include "options_multiproc.h"

void 
show_help(const char *progname) {
    printf("Usage: %s [FLAGS] [--help]\n\n", progname);
    printf("\
Run a test program that forks <procs> subprocesses, staggered \n\
by 2 seconds, and each running for <time> seconds.\n\
\n\
  -p, --procs=PROCS      Fork into PROCS subprocesses.\n\
  -t, --time=SECONDS     Running time (5 seconds)\n\
  -m, --mem=MB           Allocated memory (10Mb)\n\
\n\
      --busy             keep cores busy (default)\n\
      --idle             keep cores idle\n\
\n\
      --help             Print help\n\
\n");
    exit(EXIT_SUCCESS);
}


options*
get_options(int *argc, char **argv[]) {

    options *opts = malloc(sizeof(options));
    opts->procs   = 2;
    opts->busy	  = true;
    opts->time    = 5;
    opts->mem     = 10;

    int c;

    while (1) {
	int option_index = 0;
	static struct option long_options[] = {
	    {"help",    no_argument,       0,  1 },
	    {"procs",    required_argument,   0, 'p'},
	    {"busy",    no_argument,       0, '2'},
	    {"idle",    no_argument,       0, '3'},
	    {"time",    required_argument, 0, 't'},
	    {"mem",    required_argument, 0, 'm'},
	    {0,         0,                 0,  0 }
	};

	c = getopt_long(*argc, *argv, "+ht:p:m:",
		long_options, &option_index);
	if (c == -1)
	    break;
	    
	switch (c) {
	    
	    case 1:
		show_help((**argv));
		exit(EXIT_SUCCESS);
		break;
	    case 2:
		opts->busy=true;
		break;
	    case 3:
		opts->busy=false;
		break;
	    case 't':
		opts->time = atoi(optarg);
		if (opts->time<1) {
		    fprintf(stderr, "\nError: time must be a positive integer\n\n");
		    show_help((**argv));
		    exit(EXIT_FAILURE);
		}
		break;
	    case 'p':
		opts->procs = atoi(optarg);
		if (opts->procs<1) {
		    fprintf(stderr, "\nError: procs must be a positive integer\n\n");
		    show_help((**argv));
		    exit(EXIT_FAILURE);
		}
		break;
	    case 'm':
		opts->mem = atoi(optarg);
		if (opts->mem<1) {
		    fprintf(stderr, "\nError: mem must be a positive integer\n\n");
		    show_help((**argv));
		    exit(EXIT_FAILURE);
		}
		break;
	    case '?':
	    default:
		show_help((**argv));
		exit(EXIT_FAILURE);
		break;
	}

    }

    return opts;
}
