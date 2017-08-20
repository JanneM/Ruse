/* output.c
 *
 * Output results.
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

#include "output.h"

void
print_steps(options *opts, size_t memory, int ts) {

    if (opts->steps) {
	fprintf(opts->fhandle, "%-9d %.1f\n", ts, ((double)memory)/1024.0);
    }
}

void
print_header(options *opts) {

    if (!opts->nohead && opts->steps) {
	fprintf(opts->fhandle, "time(s)   mem(MB)\n");
    }
}

void
print_summary(options *opts, size_t  memory, int ts) {
   
    if (!opts->nosum) {
	if (!opts->nohead && opts->steps) {
	    fprintf(opts->fhandle, "\n");
	}
	fprintf(opts->fhandle, "Time(s):  %d\n", ts);
	fprintf(opts->fhandle, "Mem(MB):  %.1f\n", ((double)memory)/1024.0);
    }
}


