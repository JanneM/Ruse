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
#include "options.h"

void 
show_help(const char *progname) {
    printf("Usage: %s [-o file] [--help] command [arg...]", progname);
    exit(EXIT_SUCCESS);
}


options*
get_options(int *argc, char **argv[]) {

    options *opts = malloc(sizeof(options));

    opts->verbose = false;
    opts->time    = 30;
    opts->nohead  = false;
    opts->label  = (char *)calloc(32, sizeof(char));

    
    int c;

    while (1) {
	int option_index = 0;
	static struct option long_options[] = {
	    {"verbose", no_argument,       0, 'v'},
	    {"help",    no_argument,       0, 'h'},
	    {"label",   required_argument, 0, 'l'},
	    {"time",    required_argument, 0, 't'},
	    {"no-header",no_argument,      0, 'n'},
	    {0,         0,                 0,  0 }
	};

	c = getopt_long(*argc, *argv, "+vhl:t:n",
		long_options, &option_index);
	if (c == -1)
	    break;
	
	switch (c) {

	    case 'v':
		opts->verbose = true;
		break;
	    
	    case 'h':
		show_help((**argv));
		exit(EXIT_SUCCESS);
		break;
	    
	    case 'l':
		strncpy(opts->label, optarg, 31);
		break;

	    case 't':
		opts->time = atoi(optarg);
		break;

	    case 'n':
		opts->nohead = true;
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
	strncpy(opts->label, (*argv)[0] , 31);
    }
    return opts;
}
