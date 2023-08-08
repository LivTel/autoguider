/* test_signedness.c
** $Header: /home/cjm/cvs/autoguider/ngatcil/test/test_signedness.c,v 1.1 2023-08-08 08:57:35 cjm Exp $
*/
/**
 * Test signed/unsigned int conversions
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h> /* htons etc */
#include <sys/types.h>
#include <sys/socket.h>

/* internal structures */
union signunion
{
	uint32_t ui;
	int i;
	char c[4];
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: test_signedness.c,v 1.1 2023-08-08 08:57:35 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
int main(int argc, char* argv[])
{
	union signunion data;

	data.ui = (unsigned int)1;
	fprintf(stdout,"data.ui = %d, %u (%d%d%d%d).\n",data.ui,data.ui,data.c[0],data.c[1],data.c[2],data.c[3]);
	fprintf(stdout,"data.i = %d, %u (%d%d%d%d).\n",data.i,data.i,data.c[0],data.c[1],data.c[2],data.c[3]);
	data.i = 1;
	fprintf(stdout,"data.ui = %d, %u (%d%d%d%d).\n",data.ui,data.ui,data.c[0],data.c[1],data.c[2],data.c[3]);
	fprintf(stdout,"data.i = %d, %u (%d%d%d%d).\n",data.i,data.i,data.c[0],data.c[1],data.c[2],data.c[3]);

	data.ui = (unsigned int)1;
	data.ui = htonl(data.ui);
	fprintf(stdout,"htonl data.ui = %d, %u (%d%d%d%d).\n",data.ui,data.ui,data.c[0],data.c[1],data.c[2],data.c[3]);
	fprintf(stdout,"htonl data.i = %d, %u (%d%d%d%d).\n",data.i,data.i,data.c[0],data.c[1],data.c[2],data.c[3]);
	data.i = 1;
	data.i = htonl(data.i);
	fprintf(stdout,"htonl data.ui = %d, %u (%d%d%d%d).\n",data.ui,data.ui,data.c[0],data.c[1],data.c[2],data.c[3]);
	fprintf(stdout,"htonl data.i = %d, %u (%d%d%d%d).\n",data.i,data.i,data.c[0],data.c[1],data.c[2],data.c[3]);

	data.i = 1;
	data.i = ntohl(data.i);
	fprintf(stdout,"ntohl data.ui = %d, %u (%d%d%d%d).\n",data.ui,data.ui,data.c[0],data.c[1],data.c[2],data.c[3]);
	fprintf(stdout,"ntohl data.i = %d, %u (%d%d%d%d).\n",data.i,data.i,data.c[0],data.c[1],data.c[2],data.c[3]);
	return 0;
}

/*
** $Log: not supported by cvs2svn $
*/
