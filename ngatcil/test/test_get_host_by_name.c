/* test_get_host_by_name.c
** $Header: /home/cjm/cvs/autoguider/ngatcil/test/test_get_host_by_name.c,v 1.1 2023-08-08 08:57:35 cjm Exp $
*/
/**
 * Test gethostbyname and replacements.
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
/**
 * Define BSD Source to get BSD prototypes, including gethostbyname_r.
 */
#define _BSD_SOURCE (1)

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h> /* htons etc */
#include <sys/types.h>
#include <sys/socket.h>

#define TRUE 1
#define FALSE 0

int NGATCil_General_Error_Number;
char NGATCil_General_Error_String[256];
static int Get_Host_By_Name(const char *name,in_addr_t *host_addr_zero);
static int NGATCil_General_Log_Format(char *sub_system,char *source_filename,
				      char *function,int level,char *category,char *format,...);

int main(int argc, char *argv[])
{
	char buff[256];
	char *hostname = NULL;
	in_addr_t saddr;
	struct hostent *host_entry;

	if(argc != 2)
	{
		fprintf(stderr,"test_get_host_by_name <hostname>.\n");
		return 1;
	}
	hostname = argv[1];
	fprintf(stdout,"hostname = %s.\n",hostname);
	saddr = inet_addr(hostname);
	if(saddr == INADDR_NONE)
	{
		host_entry = gethostbyname(hostname);
		if(host_entry == NULL)
		{
			fprintf(stderr,"gethostbyname(%s) returned NULL.\n",hostname);
			return 1;
		}
		strcpy(buff,inet_ntoa(*(struct in_addr *)(host_entry->h_addr_list[0])));
		saddr = inet_addr(buff);
		/*saddr = (in_addr_t)(host_entry->h_addr_list[0]);*/
		fprintf(stdout,"saddr(gethostbyname) = %u.\n",saddr);
		fprintf(stdout,"saddr(gethostbyname) = %u.%u.%u.%u.\n",(saddr&0xff),((saddr>>8)&0xff),
			((saddr>>16)&0xff),((saddr>>24)&0xff));
		if(!Get_Host_By_Name(hostname,&saddr))
		{
			fprintf(stderr,"Get_Host_By_Name failed: Error %d : %s.\n",NGATCil_General_Error_Number,
				NGATCil_General_Error_String);
			return 2;
		}
		fprintf(stdout,"saddr(Get_Host_By_Name) = %u.\n",saddr);
		fprintf(stdout,"saddr(Get_Host_By_Name) = %u.%u.%u.%u.\n",(saddr&0xff),((saddr>>8)&0xff),
			((saddr>>16)&0xff),((saddr>>24)&0xff));
	}
	else
		fprintf(stdout,"Input hostname was numeric.\n");
	fprintf(stdout,"saddr = %u.\n",saddr);
	fprintf(stdout,"saddr = %u.%u.%u.%u.\n",(saddr&0xff),((saddr>>8)&0xff),((saddr>>16)&0xff),((saddr>>24)&0xff));
	return 0;
}

/**
 * Internal routine to get a host address from it's name. This is traditionally handled by a call
 * to gethostbyname. Unfortunately that routine is not re-entrant because the pointer it returns
 * is to a block of reusable memory in glibc, so a second call to gethostbyname from another thread
 * in the process can lead to the pointers returned from the first call being freed leading to SIGSEGV.
 * This routine wraps gethostbyname_r, the re-entrant version of that routine.
 * @param name The hostname to translate. This should be allocated, zero-terminated and non-null.
 * @param host_addr_zero The address of a in_addr_t (actually an unsigned 32 bit int in Linux).
 *       On return filled with a null-terminated, network byte ordered copy of the first hostent host address
 *       list entry returned by gethostbyname_r. NULL can be returned on failure.
 * @return The routine returns TRUE on success and FALSE on failure.
 */
static int Get_Host_By_Name(const char *name,in_addr_t *host_addr_zero)
{
	char buff[32];
	struct hostent hostbuf,*hp = NULL;
	struct in_addr inaddr;
	size_t hstbuflen;
	char *tmphstbuf = NULL;
	int retval;
	int herr;

	NGATCil_General_Error_Number = 0;
	if(name == NULL)
	{
		NGATCil_General_Error_Number = 126;
		sprintf(NGATCil_General_Error_String,"Get_Host_By_Name:name was NULL.");
		return FALSE;
	}
	if(host_addr_zero == NULL)
	{
		NGATCil_General_Error_Number = 127;
		sprintf(NGATCil_General_Error_String,"Get_Host_By_Name:host_addr_zero was NULL.");
		return FALSE;
	}
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","Get_Host_By_Name",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "Get_Host_By_Name(%s) Started.",name);
#endif /* NGATCIL_DEBUG */
	hstbuflen = 1024;
	/* Allocate buffer, remember to free it to avoid memory leakage.  */
	tmphstbuf = malloc(hstbuflen);
	if(tmphstbuf == NULL)
	{
		NGATCil_General_Error_Number = 128;
		sprintf(NGATCil_General_Error_String,"Get_Host_By_Name:memory allocation of tmphstbuf failed(%d).",
			hstbuflen);
		return FALSE;

	}
	while((retval = gethostbyname_r(name,&hostbuf,tmphstbuf,hstbuflen,&hp,&herr)) == ERANGE)
	{
		/* Enlarge the buffer.  */
		hstbuflen *= 2;
		tmphstbuf = realloc(tmphstbuf, hstbuflen);
		/* check realloc succeeds */
		if(tmphstbuf == NULL)
		{
			NGATCil_General_Error_Number = 129;
			sprintf(NGATCil_General_Error_String,
				"Get_Host_By_Name:memory reallocation of tmphstbuf failed(%d).",hstbuflen);
			return FALSE;
		}
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","Get_Host_By_Name",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "Get_Host_By_Name:gethostbyname_r returned ERANGE:Increasing buffer size to %d.",
				   hstbuflen);
#endif /* NGATCIL_DEBUG */
	}/* while */
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","Get_Host_By_Name",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "Get_Host_By_Name:gethostbyname_r loop exited with retval %d and hp %p.",retval,hp);
#endif /* NGATCIL_DEBUG */
	if(retval != 0)
	{
		if(tmphstbuf != NULL)
			free(tmphstbuf);
		NGATCil_General_Error_Number = 130;
		sprintf(NGATCil_General_Error_String,"Get_Host_By_Name:gethostbyname_r failed to find host %s (%d).",
			name,herr);
		return FALSE;
	}
	if(hp == NULL)
	{
		if(tmphstbuf != NULL)
			free(tmphstbuf);
		NGATCil_General_Error_Number = 131;
		sprintf(NGATCil_General_Error_String,
			"Get_Host_By_Name:gethostbyname_r returned NULL return pointer for hostname %s (%d).",
			name,herr);
		return FALSE;
	}
	/* copy result */
	/*(*host_addr_zero) = (in_addr_t)(hp->h_addr_list[0]);*/
	/*strcpy(buff,inet_ntoa(*(struct in_addr *)(hp->h_addr_list[0])));
	  (*host_addr_zero) = inet_addr(buff);*/
	inaddr = *(struct in_addr *)(hp->h_addr_list[0]);
	(*host_addr_zero) = inaddr.s_addr;
	/* free buffer*/
	if(tmphstbuf != NULL)
		free(tmphstbuf);
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","Get_Host_By_Name",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "Get_Host_By_Name(%s) Finished and returned %u.%u.%u.%u (network byte ordered).",
				   name,((*host_addr_zero)&0xff),(((*host_addr_zero)>>8)&0xff),
				   (((*host_addr_zero)>>16)&0xff),(((*host_addr_zero)>>24)&0xff));
#endif /* NGATCIL_DEBUG */
	return TRUE;
}

static int NGATCil_General_Log_Format(char *sub_system,char *source_filename,
				      char *function,int level,char *category,char *format,...)
{
	va_list ap;
	char buff[1024];

/* format the arguments */
	va_start(ap,format);
	vsprintf(buff,format,ap);
	va_end(ap);
	fprintf(stdout,"%s : %s : %s : %s : %s\n",sub_system,source_filename,function,category,buff);
}
