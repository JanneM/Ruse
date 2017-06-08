/* proc.h - helper functions for the /proc filesystem
 *
 * 2017 Jan Moren
 *
 */

/* system page size, for calculating the memory use */
extern int syspagesize;

/* extract the current RSS (ressident set size) and parent process if for
 * process pid.  If the pid does not exist, return -1
*/

int
read_RSS(int pid, long int *rss, int *parent);



