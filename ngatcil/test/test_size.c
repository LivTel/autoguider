/* test_size.c
** $Id$
*/
/**
 * Test size of integers (32 bit vs 64 bit compilation).
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
#include <stdio.h>
#include <stdint.h>

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
int main(int argc, char* argv[])
{
	long mylong;
	int myint;
	int32_t my32int;
	uint32_t my32uint;
	
	fprintf(stdout,"size of int   = %ld.\n",sizeof(int));
	fprintf(stdout,"size of long  = %ld.\n",sizeof(long));
	fprintf(stdout,"size of short = %ld.\n",sizeof(short int));
	fprintf(stdout,"size of int32_t = %ld.\n",sizeof(int32_t));
	fprintf(stdout,"size of uint32_t = %ld.\n",sizeof(uint32_t));
	return 0;
}
