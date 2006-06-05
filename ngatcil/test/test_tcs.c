/* test_tcs.c
** $Header: /home/cjm/cvs/autoguider/ngatcil/test/test_tcs.c,v 1.1 2006-06-01 15:28:14 cjm Exp $
*/
/**
 * Test server that pretends to be an TCS, and receives guide packets sent by an autoguider.
 * connecting process. The command line is as follows:
 * <pre>
 * test_tcs 
 * </pre>
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

#include <stdio.h>
#include <string.h>

#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"
#include "ngatcil_tcs_guide_packet.h"

#include "command_server.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: test_tcs.c,v 1.1 2006-06-01 15:28:14 cjm Exp $";
/**
 * Command server (telnet) port.
 */
static unsigned short Command_Server_Port = 1234;
/**
 * The server context to use for this server.
 */
static Command_Server_Server_Context_T Server_Context = NULL;
/**
 * TCS UDP guide packet port.
 * @see #NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT
 */
static int TCS_UDP_Port = NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT;
/**
 * File descriptor of TCS Guide packet UDP port socket.
 */
static int TCS_UDP_Socket_Fd = -1;

/* internal routines */
static void Test_TCS_Server_Connection_Callback(Command_Server_Handle_T connection_handle);
static void Send_Reply(Command_Server_Handle_T connection_handle,char *reply_message);
static int Test_TCS_Guide_UDP_Server_Callback(int socket_id,void *message_buff,int message_length);
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Main program.
 * @see #Parse_Arguments
 * @see #Help
 */
int main(int argc, char* argv[])
{
	int retval;

	if(!Parse_Arguments(argc,argv))
		return 1;

	if(Command_Server_Port == 0)
	{
		Help();
		return 3;
	}
	/* setup logging */
	Command_Server_Set_Log_Handler_Function(Command_Server_Log_Handler_Stdout);
	Command_Server_Set_Log_Filter_Function(Command_Server_Log_Filter_Level_Bitwise);
	Command_Server_Set_Log_Filter_Level(COMMAND_SERVER_LOG_BIT_GENERAL);

	NGATCil_General_Set_Log_Handler_Function(NGATCil_General_Log_Handler_Stdout);
	NGATCil_General_Set_Log_Filter_Function(NGATCil_General_Log_Filter_Level_Bitwise);
	NGATCil_General_Set_Log_Filter_Level(NGATCIL_GENERAL_LOG_BIT_GENERAL|NGATCIL_GENERAL_LOG_BIT_UDP_RAW|
					     NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET);
	/* start NGAT Cil handling */
	fprintf(stdout, "Starting TCS guide packet UDP server on port %hu.\n",TCS_UDP_Port);
	retval = NGATCil_UDP_Server_Start(TCS_UDP_Port,NGATCIL_TCS_GUIDE_PACKET_LENGTH,&TCS_UDP_Socket_Fd,
					  Test_TCS_Guide_UDP_Server_Callback);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return 4;
	}
	/* start command server */
	fprintf(stdout, "Starting command server on port %hu.\n",Command_Server_Port);
	retval = Command_Server_Start_Server(&Command_Server_Port,Test_TCS_Server_Connection_Callback,&Server_Context);
	if(retval == FALSE)
	{
		Command_Server_Error();
		return 4;
	}
	fprintf(stdout, "Command Server finished\n");
	fprintf(stdout, "Closing TCS Guide UDP socket.\n");
	retval = NGATCil_UDP_Close(TCS_UDP_Socket_Fd);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return 4;
	}
	fprintf(stdout, "test_tcs finished.\n");
	return 0;
}/* main */

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Function invoked in each thread.
 * @param connection_handle Connection handle for this thread.
 * @see #Abort
 * @see #Send_Reply
 */
static void Test_TCS_Server_Connection_Callback(Command_Server_Handle_T connection_handle)
{
	char *client_message = NULL;
	int retval;
	int seconds,i;

	/* get message from client */
	retval = Command_Server_Read_Message(connection_handle, &client_message);
	if(retval == FALSE)
	{
		Command_Server_Error();
		return;
	}
	printf("test_tcs: received '%s'\n", client_message);
	/* do something with message */
	if(strcmp(client_message, "help") == 0)
	{
		printf("server: help detected.\n");
		Send_Reply(connection_handle, "help:\n"
			   "\thelp\n"
			   "\tshutdown\n");
	}
	else if(strcmp(client_message, "shutdown") == 0)
	{
		printf("server: shutdown detected:about to stop.\n");
		Send_Reply(connection_handle, "ok");
		retval = Command_Server_Close_Server(&Server_Context);
		if(retval == FALSE)
			Command_Server_Error();
	}
	else if(strncmp(client_message, "autoguider on", strlen("autoguider on")) == 0)
	{
		printf("server: autoguider on detected.\n");
		printf("server: not implemented yet.\n");
		Send_Reply(connection_handle, "not implemented yet.");
	}
	else if(strncmp(client_message, "autoguider off", strlen("autoguider off")) == 0)
	{
		printf("server: autoguider off detected.\n");
		printf("server: not implemented yet.\n");
		Send_Reply(connection_handle, "not implemented yet.");
	}
	else
	{
		printf("test_tcs: message unknown: '%s'\n", client_message);
		Send_Reply(connection_handle, "failed message unknown");
	}
	/* free message */
	free(client_message);
}

/**
 * Send a message back to the client.
 * @param connection_handle Connection handle for this thread.
 * @param reply_message The message to send.
 */
static void Send_Reply(Command_Server_Handle_T connection_handle,char *reply_message)
{
	int retval;

	/* send something back to the client */
	printf("server: about to send '%s'\n", reply_message);
	retval = Command_Server_Write_Message(connection_handle, reply_message);
	if(retval == FALSE)
	{
		Command_Server_Error();
		return;
	}
	printf("server: sent '%s'\n", reply_message);
}

/**
 * UDP Guide packet socket callback.
 */
static int Test_TCS_Guide_UDP_Server_Callback(int socket_id,void *message_buff,int message_length)
{
	char packet_buff[NGATCIL_TCS_GUIDE_PACKET_LENGTH+1];
	float x_pos,y_pos,timecode_secs;
	char status_char;
	int retval,timecode_terminating,timecode_unreliable;

	fprintf(stdout,"Test_TCS_Guide_UDP_Server_Callback(socket_id=%d,buff=%p,message_length=%d).\n",
		socket_id,message_buff,message_length);
	if(message_length != NGATCIL_TCS_GUIDE_PACKET_LENGTH)
	{
		fprintf(stdout,"Test_TCS_Guide_UDP_Server_Callback received odd guide packet of length %d.\n",
			message_length);
		return FALSE;
	}
	/* the message_buff is the same size as the packet's, e.g. 34 bytes.
	** We need the packet in a 35 byte buffer for \0 handling, before we pass the packet to the parser. */
	memcpy(packet_buff,message_buff,message_length);
	retval = NGATCil_TCS_Guide_Packet_Parse(packet_buff,NGATCIL_TCS_GUIDE_PACKET_LENGTH+1,&x_pos,&y_pos,
						&timecode_terminating,&timecode_unreliable,&timecode_secs,
						&status_char);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return FALSE;
	}
	fprintf(stdout,"Guide Packet:x=%.2f,y=%.2f,status=%c:",x_pos,y_pos,status_char);
	if(timecode_terminating)
	{
		fprintf(stdout,"The autoguider is terminating.\n");
	}
	else if(timecode_unreliable)
	{
		fprintf(stdout,"The packet was unreliable:retry in %.2f seconds.\n",timecode_secs);
	}
	else
	{
		fprintf(stdout,"Next packet in %.2f seconds.\n",timecode_secs);
	}
	return TRUE;
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-command_server_log_level")==0)||(strcmp(argv[i],"-csl")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&log_level);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing log level %s failed.\n",argv[i+1]);
					return FALSE;
				}
				Command_Server_Set_Log_Filter_Level(log_level);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Log Level requires a level.\n");
				return FALSE;
			}
		}
		if((strcmp(argv[i],"-ngatcil_log_level")==0)||(strcmp(argv[i],"-ncl")==0))
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
		else if((strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"-h")==0))
		{
			Help();
			exit(0);
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
	fprintf(stdout,"Test TCS:Help.\n");
	fprintf(stdout,"test_tcs\n");
	fprintf(stdout,"\t[-[csl|command_server_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-[ncl|ngatcil_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-h[elp]]\n");
}
/*
** $Log: not supported by cvs2svn $
*/