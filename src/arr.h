/* arr.h - implement an array type for pid list 
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



#ifndef ARR_H
#define ARR_H
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

/* hacky implementation; should make it generic but for only two types it's
 * honestly less error-prone to just have one implementation per type.
 */

/* integer array */

typedef struct {
    unsigned int anr;   // number of allocated elements
    unsigned int len;	// current number of elements
    int *ilist;
} iarr;

/* Create a new iarr instance. nr is the initial size */
iarr*
iarr_create(unsigned int nr);


/* Insert value 'val'. true on success. */
bool
iarr_insert(iarr* arr, int val);

/* reset the element counter */
bool
iarr_reset(iarr *arr);


/* deallocate the iarr structure */
void
iarr_delete(iarr *arr);


/* double array */
typedef struct {
    unsigned int anr;   // number of allocated elements
    unsigned int len;	// current number of elements
    double *dlist;
} darr;

/* Create a new darr instance. nr is the initial size */
darr*
darr_create(unsigned int nr);


/* Insert value 'val'. true on success. */
bool
darr_insert(darr* arr, double val);

/* reset the element counter */
bool
darr_reset(darr *arr);


/* deallocate the darr structure */
void
darr_delete(darr *arr);


#endif
