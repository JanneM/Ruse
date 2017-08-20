/* options.h
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

#ifndef OPTIONS_H
#define OPTIONS_H
#include <error.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    bool verbose;
    bool steps;
    unsigned int time;
    char *label;
    bool nofile;
    bool nohead;
    bool nosum;
    FILE *fhandle;
} options;



/* Get and parse user options. Return an options struct, and index to the first non-option argument in argv.
 */

options*
get_options(int *argc, char **argv[]);

#endif
