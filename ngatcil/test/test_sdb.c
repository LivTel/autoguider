/* test_sdb.c
** $Header: /home/cjm/cvs/autoguider/ngatcil/test/test_sdb.c,v 1.5 2014-02-03 10:35:26 cjm Exp $
*/
/**
 * Test SDB submission.
 * @author Chris Mottram
 * @version $Revision: 1.5 $
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
#include "log_udp.h"
#include "ngatcil_cil.h"
#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"
#include "ngatcil_ags_sdb.h"

/**
 * Number of datum changes to allow in one invocation of this program.
 */
#define TEST_SDB_DATUM_MAX_COUNT    (10)

/* internal structures */
/**
 * Structure collating information on Oid's to change and their new values.
 * <ul>
 * <li>Oid
 * <li>Value
 * </ul>
 */
struct Test_SDB_Datum_Struct
{
	int Oid;
	int Value;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: test_sdb.c,v 1.5 2014-02-03 10:35:26 cjm Exp $";
/**
 * SDB CIL hostname.
 * @see ../cdocs/ngatcil_ags_sdb.html#NGATCIL_AGS_SDB_MCC_DEFAULT
 */
static char SDB_CIL_Hostname[256] = NGATCIL_AGS_SDB_MCC_DEFAULT;
/**
 * SDB Cil port.
 * @see ../cdocs/ngatcil_ags_sdb.html#NGATCIL_AGS_SDB_CIL_PORT_DEFAULT
 */
static int SDB_CIL_Port = NGATCIL_AGS_SDB_CIL_PORT_DEFAULT;
/**
 * UDP CIL port to wait for AGS commands on.
 * @see ../cdocs/ngatcil_cil.html#NGATCIL_CIL_AGS_PORT_DEFAULT
 */
static int AGS_CIL_Port = NGATCIL_CIL_AGS_PORT_DEFAULT;
/**
 * File descriptor of AGS server socket, also used client-side for SDB submission.
 */
static int AGS_Socket_Fd = -1;
/**
 * List of modified datums to apply.
 * @see #Test_SDB_Datum_Struct
 * @see #TEST_SDB_DATUM_MAX_COUNT
 * @see #SDB_Datum_Count
 */
static struct Test_SDB_Datum_Struct SDB_Datum_List[TEST_SDB_DATUM_MAX_COUNT];
/**
 * Number of datums in the list.
 * @see #SDB_Datum_List
 * @see #TEST_SDB_DATUM_MAX_COUNT
 */
static int SDB_Datum_Count = 0;

/* internal functions */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);
static int AGS_CIL_Command_Server_Callback(int socket_id,void *message_buff,int message_length);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Main program.
 * @see #Parse_Arguments
 * @see #AGS_CIL_Port
 * @see #AGS_Socket_Fd
 * @see #SDB_CIL_Hostname
 * @see #SDB_CIL_Port
 */
int main(int argc, char* argv[])
{
	int retval,i;

	/* initialise SDB datums */
	if(!NGATCil_AGS_SDB_Initialise())
	{
		NGATCil_General_Error();
		return 5;
	}
	/* setup logging */
	NGATCil_General_Set_Log_Handler_Function(NGATCil_General_Log_Handler_Stdout);
	NGATCil_General_Set_Log_Filter_Function(NGATCil_General_Log_Filter_Level_Absolute);
	NGATCil_General_Set_Log_Filter_Level(LOG_VERBOSITY_VERY_VERBOSE);
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* start NGAT CIL AGS Command reply packet server */
	fprintf(stdout, "Starting AGS CIL Command reply UDP server on port %d.\n",AGS_CIL_Port);
	retval = NGATCil_UDP_Server_Start(AGS_CIL_Port,NGATCIL_CIL_BASE_PACKET_LENGTH+256,
					  &AGS_Socket_Fd,AGS_CIL_Command_Server_Callback);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return 4;
	}
	/* set SDB endpoint */
	if(!NGATCil_AGS_SDB_Remote_Host_Set(SDB_CIL_Hostname,SDB_CIL_Port))
		NGATCil_General_Error(); /* non-fatal  - default should be OK */
	/* send intialised datums to the SDB */
	if(!NGATCil_AGS_SDB_Status_Send(AGS_Socket_Fd))
	{
		NGATCil_General_Error();
		return 5;
	}
	/* apply requested datum changes */
	for(i=0;i<SDB_Datum_Count;i++)
	{
		if(!NGATCil_AGS_SDB_Value_Set(SDB_Datum_List[i].Oid,SDB_Datum_List[i].Value))
		{
			fprintf(stderr,"test_sdb:Setting datum[%d] %d  = %d failed.\n",i,SDB_Datum_List[i].Oid,
				SDB_Datum_List[i].Value);
			NGATCil_General_Error();
			return 6;
		}
	}/* end for */
	/* send any changed datums to the SDB */
	if(!NGATCil_AGS_SDB_Status_Send(AGS_Socket_Fd))
	{
		NGATCil_General_Error();
		return 5;
	}
	/* wait a bit for a reply */
	fprintf(stdout,"test_sdb:Waiting for SDB reply.\n");
	sleep(5);
	fprintf(stdout,"test_sdb:Finished waiting:stopping.\n");
	/* close AGS server socket Fd */
	retval = NGATCil_UDP_Close(AGS_Socket_Fd);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return 4;
	}
	fprintf(stdout, "test_sdb finished.\n");
	return 0;
}/* main */

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * AGS CIL Command server callback routine.
 */
static int AGS_CIL_Command_Server_Callback(int socket_id,void *message_buff,int message_length)
{
	struct NGATCil_Cil_Packet_Struct cil_packet;
	int sequence_number,status,i;
	int *message_int_ptr = NULL;

	if(message_length < NGATCIL_CIL_BASE_PACKET_LENGTH)
	{
		fprintf(stderr,"AGS_CIL_Command_Server_Callback:"
			"received CIL packet of wrong length %d vs %d.\n",message_length,
			NGATCIL_CIL_BASE_PACKET_LENGTH);
		return FALSE;
	}
	/* Won't work if NGATCIL_CIL_BASE_PACKET_LENGTH != sizeof(struct NGATCil_Cil_Packet_Struct) */
	memcpy(&cil_packet,message_buff,NGATCIL_CIL_BASE_PACKET_LENGTH);
	fprintf(stdout,"AGS_CIL_Command_Server_Callback:"
		"Source = %#x, Dest = %#x, Class = %#x, Service = %#x,SeqNum = %#x.\n",
		cil_packet.Source_Id,cil_packet.Dest_Id,cil_packet.Class,cil_packet.Service,cil_packet.Seq_Num);
	message_int_ptr = (int *)message_buff;
	fprintf(stdout,"AGS_CIL_Command_Server_Callback:");
	for(i = (NGATCIL_CIL_BASE_PACKET_LENGTH/sizeof(int)); i < (message_length/sizeof(int)); i++)
	{
		fprintf(stdout,"Word %i = %#x",i,message_int_ptr[i]);
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
 * @see #SDB_Datum_List
 * @see #SDB_Datum_Count
 * @see #TEST_SDB_DATUM_MAX_COUNT
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level,agstate;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-agstate")==0))
		{
			if((i+1)<argc)
			{
				if(strcmp(argv[i+1],"OFF") == 0)
					agstate = E_AGS_OFF;
				else if(strcmp(argv[i+1],"ON_BRIGHTEST") == 0)
					agstate = E_AGS_ON_BRIGHTEST;
				else if(strcmp(argv[i+1],"IDLE") == 0)
					agstate = E_AGS_IDLE;
				else
				{
					fprintf(stderr,"Parse_Arguments:agstate %s not known.\n",argv[i+1]);
					return FALSE;
				}
				if(SDB_Datum_Count >= TEST_SDB_DATUM_MAX_COUNT)
				{
					fprintf(stderr,"Parse_Arguments:SDB Datum list run out of room.\n");
					return FALSE;
				}
				SDB_Datum_List[SDB_Datum_Count].Oid = D_AGS_AGSTATE;
				SDB_Datum_List[SDB_Datum_Count].Value = agstate;
				SDB_Datum_Count++;
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:-agstate requires a state [OFF|ON_BRIGHTEST|IDLE].\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"-h")==0))
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
		else if((strcmp(argv[i],"-sdb_hostname")==0)||(strcmp(argv[i],"-sdb_host")==0))
		{
			if((i+1)<argc)
			{
				strncpy(SDB_CIL_Hostname,argv[i+1],256);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:SDB Host requires a hostname.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-sdb_port")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&SDB_CIL_Port);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing port number %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:SDB Port requires a number.\n");
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
	fprintf(stdout,"test_sdb\n");
	fprintf(stdout,"\t[-agstate <OFF|ON_BRIGHTEST|IDLE>]\n");
	fprintf(stdout,"\t[-h[elp]]\n");
	fprintf(stdout,"\t[-sdb_host[name] <hostname>]\n");
	fprintf(stdout,"\t[-sdb_port <port_number>]\n");
	fprintf(stdout,"\t[-[ncl|ngatcil_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-start_server]\n");
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.4  2014/01/31 17:31:59  cjm
** Comment tidy-up.
**
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
