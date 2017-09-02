/* do_task.c
 *
 * generic busy-work task for test programs
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

#include "do_task.h"

double time_diff_milli(struct timespec *toc, struct timespec *tic) {
    return (1e3*(toc->tv_sec-tic->tv_sec)+
            (toc->tv_nsec-tic->tv_nsec)/1e6);
}

int
do_task(unsigned int time, bool busy){

    if (busy) {
	int t=0;
	struct timespec tstart, tnow;
	
	clock_gettime(CLOCK_REALTIME, &tstart);

	do {
	    for (int i=0; i<10000000; i++) {
		t=t+exp((float)i/1000000.0);
		if (t>10) {
		    t=0;
		}
	    }
	    clock_gettime(CLOCK_REALTIME, &tnow);
	    t = time_diff_milli(&tnow, &tstart);
	} while (time*1000>t);
	return t;
    } else {
	sleep(time);
	return 0;
    }
}
