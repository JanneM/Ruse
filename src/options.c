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

#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <libgen.h>
#include "options.h"

void 
show_help(const char *progname) {
    printf("Usage: %s [FLAGS] [--help] command [ARG...]\n", progname);
    printf("\
Measures the time and memory taken for a command process and \n\
all its subprocesses. \n\
\n\
  -l, --label=LABEL      Prefix output with LABEL (default \n\
                         'command')\n\
      --stdout           Don't save to a file, but to stdout\n\
      --no-header        Don't print a header line\n\
      --no-summary       Don't print summary info\n\
  -s, --steps            Print each sample step        \n\
  -t, --time=SECONDS     Sample every SECONDS (default 30)\n\
\n\
      --help             Print help\n\
  -v, --verbose          Verbose output\n\
      --version          Display version\n\
\n");
    exit(EXIT_SUCCESS);
}


options*
get_options(int *argc, char **argv[]) {

    options *opts = malloc(sizeof(options));

    opts->verbose = false;
    opts->steps   = false;
    opts->time    = 30;
    opts->nohead  = false;
    opts->nofile  = false;
    opts->nosum   = false;
    opts->label  = (char *)calloc(32, sizeof(char));

    
    int c;

    while (1) {
	int option_index = 0;
	static struct option long_options[] = {
	    {"verbose", no_argument,       0, 'v'},
	    {"version", no_argument,       0,  1 },
	    {"help",    no_argument,       0,  2 },
	    {"label",   required_argument, 0, 'l'},
	    {"step",    no_argument,       0, 's'},
	    {"time",    required_argument, 0, 't'},
	    {"stdout",  no_argument,       0,  3 },
	    {"no-header",no_argument,      0,  4 },
	    {"no-summary",no_argument,     0,  5 },
	    {0,         0,                 0,  0 }
	};

	c = getopt_long(*argc, *argv, "+vhl:t:s",
		long_options, &option_index);
	if (c == -1)
	    break;
	
	switch (c) {

	    case 'v':
		opts->verbose = true;
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
	    case '?':
    default:
		show_help((**argv));
		exit(EXIT_FAILURE);
		break;
	}

    }
    if (optind >= *argc) {
	fprintf(stderr, "missing a program to run\n");
	show_help((**argv));
	exit(EXIT_FAILURE);
    }

    *argc = *argc - optind;
    *argv = (*argv)+optind; 
    if (strlen(opts->label) == 0) {
	char *bname =strdup((*argv)[0]);
	strncpy(opts->label, basename(bname) , 31);
	free(bname);
    }
    return opts;
}
