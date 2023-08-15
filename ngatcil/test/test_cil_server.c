/* test_cil_server.c
** $Header: /home/cjm/cvs/autoguider/ngatcil/test/test_cil_server.c,v 1.4 2014-01-31 17:31:00 cjm Exp $
*/
/**
 * Test CIL server to see what we receive.
 * @author Chris Mottram
 * @version $Revision: 1.4 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "log_udp.h"
#include "ngatcil_cil.h"
#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: test_cil_server.c,v 1.4 2014-01-31 17:31:00 cjm Exp $";
/**
 * File descriptor of the CIL Server UDP Command socket.
 */
static int Server_Socket_Fd = -1;
/**
 * UDP CIL port to wait for commands on.
 */
static int Server_CIL_Port = 0;
/**
 * When to stop the server.
 */
static int Quit = FALSE;

static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);
static int CIL_Command_Server_Callback(int socket_id,void *message_buff,int message_length);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Main program.
 * @see #Parse_Arguments
 * @see #Server_CIL_Port
 * @see #Server_Socket_Fd
 * @see #Quit
 */
int main(int argc, char* argv[])
{
	int retval;

	/* setup logging */
	NGATCil_General_Set_Log_Handler_Function(NGATCil_General_Log_Handler_Stdout);
	NGATCil_General_Set_Log_Filter_Function(NGATCil_General_Log_Filter_Level_Absolute);
	NGATCil_General_Set_Log_Filter_Level(LOG_VERBOSITY_VERY_VERBOSE);
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* start NGAT CIL AGS Command reply packet server */
	fprintf(stdout, "Starting CIL Command UDP server on port %d.\n",Server_CIL_Port);
	retval = NGATCil_UDP_Server_Start(Server_CIL_Port,1024,
					  &Server_Socket_Fd,CIL_Command_Server_Callback);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return 4;
	}
	while(Quit == FALSE)
	{
		sleep(1);
	}
	/* close server socket Fd */
	retval = NGATCil_UDP_Close(Server_Socket_Fd);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return 4;
	}
	fprintf(stdout, "test_cil_server finished.\n");
	return 0;
}/* main */

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * CIL Command server callback routine.
 */
static int CIL_Command_Server_Callback(int socket_id,void *message_buff,int message_length)
{
	struct NGATCil_Cil_Packet_Struct cil_packet;
	struct timespec current_time;
	int sequence_number,status,i;
	int *message_int_ptr = NULL;

	if(message_length < NGATCIL_CIL_BASE_PACKET_LENGTH)
	{
		fprintf(stderr,"AGS_CIL_Command_Server_Callback:"
			"received too short CIL packet of length %d vs %d.\n",message_length,
			NGATCIL_CIL_BASE_PACKET_LENGTH);
		return FALSE;
	}
	/* Won't work if NGATCIL_CIL_BASE_PACKET_LENGTH != sizeof(struct NGATCil_Cil_Packet_Struct) */
	memcpy(&cil_packet,message_buff,NGATCIL_CIL_BASE_PACKET_LENGTH);
	fprintf(stdout,"AGS_CIL_Command_Server_Callback:"
		"Source = %#x, Dest = %#x, Class = %#x, Service = %#x,SeqNum = %#x.\n",
		cil_packet.Source_Id,cil_packet.Dest_Id,cil_packet.Class,cil_packet.Service,cil_packet.Seq_Num);
	clock_gettime(CLOCK_REALTIME,&current_time);
	fprintf(stdout,"AGS_CIL_Command_Server_Callback:"
		"Timestamp secs = %#x, Timestamp NanoSecs = %#x, "
		"Unix secs = %#lx, Unix nanosecs = %#lx, Unix Secs-TTL_OFFSET = %#lx.\n",
		cil_packet.Timestamp_Seconds,cil_packet.Timestamp_Nanoseconds,
		current_time.tv_sec,current_time.tv_nsec,current_time.tv_sec-TTL_TIMESTAMP_OFFSET);
	/* print out message specific bit */
	message_int_ptr = (int *)message_buff;
	fprintf(stdout,"AGS_CIL_Command_Server_Callback:");
	for(i = (NGATCIL_CIL_BASE_PACKET_LENGTH/sizeof(int)); i < (message_length/sizeof(int)); i++)
	{
		fprintf(stdout,"Word %i = %#x ",i,message_int_ptr[i]);
		if((i % 4) == 0)
			fprintf(stdout,"\n");
	}
	fprintf(stdout,"\n");
	return TRUE;
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #SDB_CIL_Hostname
 * @see #SDB_CIL_Port
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level,agstate;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"-h")==0))
		{
			Help();
			exit(0);
		}
		else if((strcmp(argv[i],"-ngatcil_log_level")==0)||(strcmp(argv[i],"-ncl")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&log_level);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing log level %s failed.\n",argv[i+1]);
					return FALSE;
				}
				NGATCil_General_Set_Log_Filter_Level(log_level);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Log Level requires a level.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-port")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Server_CIL_Port);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing port number %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Port requires a number.\n");
				return FALSE;
			}
		}
		else
		{
			fprintf(stderr,"Parse_Arguments:argument '%s' not recognized.\n",argv[i]);
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Help routine.
 */
static void Help(void)
{
	fprintf(stdout,"Test SDB:Help.\n");
	fprintf(stdout,"test_cil_server\n");
	fprintf(stdout,"\t[-port <port_number>]\n");
	fprintf(stdout,"\t[-[ncl|ngatcil_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-h[elp]]\n");
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.3  2011/09/08 09:22:24  cjm
** Added #include <stdlib.h> for exit under newer kernels.
**
** Revision 1.2  2009/01/30 18:01:14  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.1  2006/08/29 14:15:44  cjm
** Initial revision
**
*/
