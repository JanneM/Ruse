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
print_time(FILE *f, int ts) {
    int d, h, m, s;
    d = ts/(24*60*60);
    ts %= (24*60*60);
    h = ts/(60*60);
    ts %= (60*60);
    m = ts/60;
    s = ts % 60;
    fprintf(f, "Time:          ");
    if (d>0) {
        fprintf(f, "%d-", d);
    }
    fprintf(f, "%02d:%02d:%02d\n", h, m, s);
}
void
print_mem(FILE *f, size_t mem) {
    
    int d = 1024;
    if (mem>(d*d*d)) {
        fprintf(f, "Memory:        %.1f TB\n", (double)mem/(d*d*d));
    } else 
    if (mem>(d*d)) {
        fprintf(f, "Memory:        %.1f GB\n", (double)mem/(d*d));
    } else { 
        fprintf(f, "Memory:        %.1f MB\n", (double)mem/(d));
    }
}

/* output one iteration data */
void
print_steps(options *opts, size_t memory, pstruct *pstr, int ts) {

    if (opts->steps) {
	fprintf(opts->fhandle, "%7d %11.1f", ts, ((double)memory)/1024.0);
        if (opts->procs) {
                fprintf(opts->fhandle, "%6d %5d ", pstr->nproc, pstr->proc_cur->len); 
                for (int i=0; i < pstr->proc_cur->len; i++) {
                    fprintf(opts->fhandle, "%4.0f", pstr->proc_cur->dlist[i]);
                }
        }
        fprintf(opts->fhandle, "\n");
    }

    fflush(opts->fhandle);
}

/* print header info */
void
print_header(options *opts) 
{

    if (!opts->nohead && opts->steps) { 
	fprintf(opts->fhandle, "   time         mem   ");
        if (opts->procs) {
	    fprintf(opts->fhandle, "processes  process usage");
        }
        fprintf(opts->fhandle, "\n");
	fprintf(opts->fhandle, "  (secs)        (MB)  ");
	if (opts->procs) {
	    fprintf(opts->fhandle, "tot  used  (sorted, %%CPU)");
	}
        fprintf(opts->fhandle, "\n");
        fflush(opts->fhandle);
    }
}

/* print the final summary */
void
print_summary(options *opts, size_t memory, pstruct *pstr, int ts) {
   
    if (!opts->nosum) {
	if (!opts->nohead && opts->steps) {
	    fprintf(opts->fhandle, "\n");
	}
        print_time(opts->fhandle, ts);
        print_mem(opts->fhandle, memory); 
//	fprintf(opts->fhandle, "Mem:   %-9.1f\n", ((double)memory)/1024.0);
        if (opts->procs) {

            char pad[5] = "";
            if (pstr->max_cores>9) {
                strncpy(pad, " ", 2);
            }
            if (pstr->max_cores>99) {
                strncpy(pad, "  ", 3);
            } 
            if (pstr->max_cores>999) {
                strncpy(pad, "   ", 4);
            }

	    fprintf(opts->fhandle, "Cores:      %s%4d\n", pad, pstr->max_cores);
	    fprintf(opts->fhandle, "Total_procs:%s%4d\n", pad, pstr->max_proc);
	    fprintf(opts->fhandle, "Used_procs: %s%4d\n", pad, pstr->proc_acc->len);
            fprintf(opts->fhandle, "Proc(%%): ");
            for (int i=0; i < pstr->proc_acc->len; i++) {
                fprintf(opts->fhandle, "%-6.1f", pstr->proc_acc->dlist[i]/pstr->iter);
            }
            fprintf(opts->fhandle, "\n");
        }
        fflush(opts->fhandle);
    }
}


