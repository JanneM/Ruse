/* arr.c - implement an array type for pid list 
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

#include "arr.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

/* Create a new iarr instance. elems is the initial size. NULL on failure. */
iarr*
iarr_create(unsigned int elems) {

    iarr *arr;
    
    if (elems<1) {
	 elems = 1;
    }

    if ((arr = malloc(sizeof(iarr)))==NULL) {
	perror("iarr_create");
	return NULL;
    }
    arr->len = 0;
    arr->anr = elems;

    if ((arr->ilist = malloc(arr->anr*sizeof(int)))==NULL) {
	perror("iarr_create");
	free(arr);
	return NULL;
    }
    
    return arr;
}


/* Insert value 'val'. true on success. */
bool
iarr_insert(iarr* arr, int val) {

    if (arr->len == arr->anr) {
	arr->anr = (int)(arr->anr*1.5);
	int *tmp;
	if ((tmp = realloc(arr->ilist, arr->anr*sizeof(int))) == NULL) {
	    perror("iarr_insert");
	    return false;
	}
	arr->ilist = tmp;
    }

    arr->ilist[arr->len] = val;
    arr->len++;
    return true;
}


/* deallocate the iarr structure */
void
iarr_delete(iarr *arr) {
   
    free(arr->ilist);
    free(arr);
}

