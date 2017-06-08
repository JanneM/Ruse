/* arr.h - implement an array type for pid list 
 *
 * 2017 Jan Moren
 *
 */

#include <stdbool.h>

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


/* deallocate the iarr structure */
void
iarr_delete(iarr *arr);

