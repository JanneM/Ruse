/* do_task.h
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



#ifndef DO_TASK
#define DO_TASK

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

/* do a generic task for time seconds. If busy we try to keep the core 
 * occupied; if not, we sleep for the duration. Return a bogus value
 * to trick the compiler optimizer.
 */
int
do_task(unsigned int time, bool busy);


#endif
