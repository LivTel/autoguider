/* autoguider_server.c
** Autoguider server routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_server.c,v 1.2 2006-06-02 13:43:36 cjm Exp $
*/
/**
 * Server routines for the autoguider program.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "command_server.h"

#include "autoguider_general.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_server.c,v 1.2 2006-06-02 13:43:36 cjm Exp $";
/**
 * The server context to use for this server.
 * @see ../command_server/cdocs/command_server.html#Command_Server_Server_Context_T
 */
static Command_Server_Server_Context_T Server_Context = NULL;
/**
 * Command server port number.
 */
static unsigned short Port_Number = 1234;

/* internal functions */
static void Autoguider_Server_Connection_Callback(Command_Server_Handle_T connection_handle);
static int Send_Reply(Command_Server_Handle_T connection_handle,char *reply_message);
static int Send_Binary_Reply(Command_Server_Handle_T connection_handle,void *buffer_ptr,size_t buffer_length);
static int Send_Binary_Reply_Error(Command_Server_Handle_T connection_handle);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Autoguider server initialisation routine. Assumes CCD_Config_Load has previously been called
 * to load the configuration file.
 * It loads the unsigned short with key ""command.server.port_number" into the Port_Number variable
 * for use in Autoguider_Server_Start.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #Autoguider_Server_Start
 * @see #Port_Number
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_SERVER
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Unsigned_Short
 */
int Autoguider_Server_Initialise(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Initialise:started.");
#endif
	/* get port number from config */
	retval = CCD_Config_Get_Unsigned_Short("command.server.port_number",&Port_Number);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 200;
		sprintf(Autoguider_General_Error_String,"Failed to find port number in config file.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Initialise:finished.");
#endif
	return TRUE;
}

/**
 * Autoguider server start routine.
 * This routine starts the server. It does not return until the server is stopped using Autoguider_Server_Stop.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #Port_Number
 * @see #Autoguider_Server_Connection_Callback
 * @see #Server_Context
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_SERVER
 * @see ../command_server/cdocs/command_server.html#Command_Server_Start_Server
 */
int Autoguider_Server_Start(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Start:started.");
#endif
#if AUTOGUIDER_DEBUG > 2
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,
				      "Starting multi-threaded server on port %hu.",Port_Number);
#endif
	retval = Command_Server_Start_Server(&Port_Number,Autoguider_Server_Connection_Callback,&Server_Context);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 201;
		sprintf(Autoguider_General_Error_String,"Autoguider_Server_Start:"
			"Command_Server_Start_Server returned FALSE.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Start:finished.");
#endif
	return TRUE;
}

/**
 * Autoguider server stop routine.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #Server_Context
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_SERVER
 * @see ../command_server/cdocs/command_server.html#Command_Server_Close_Server
 */
int Autoguider_Server_Stop(void)
{
	int retval;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Stop:started.");
#endif
	retval = Command_Server_Close_Server(&Server_Context);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 202;
		sprintf(Autoguider_General_Error_String,"Autoguider_Server_Stop:"
			"Command_Server_Close_Server returned FALSE.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Stop:finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Server connection thread, invoked whenever a new command comes in.
 * @param connection_handle Connection handle for this thread.
 * @see #Send_Reply
 * @see #Send_Binary_Reply
 * @see #Send_Binary_Reply_Error
 * @see #Autoguider_Server_Stop
 * @see autoguider_command.html#Autoguider_Command_Status
 * @see autoguider_command.html#Autoguider_Command_Temperature
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_SERVER
 * @see ../command_server/cdocs/command_server.html#Command_Server_Read_Message
 */
static void Autoguider_Server_Connection_Callback(Command_Server_Handle_T connection_handle)
{
	void *buffer_ptr = NULL;
	size_t buffer_length = 0;
	char *reply_string = NULL;
	char *client_message = NULL;
	int retval;
	int seconds,i;

	/* get message from client */
	retval = Command_Server_Read_Message(connection_handle, &client_message);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 203;
		sprintf(Autoguider_General_Error_String,"Autoguider_Server_Connection_Callback:"
			"Failed to read message.");
		Autoguider_General_Error();
		return;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				      "received '%s'",client_message);
#endif
	/* do something with message */
	if(strncmp(client_message,"abort",5) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				       "abort detected.");
#endif
		retval = Autoguider_Command_Abort(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
		else
		{
			Autoguider_General_Error();
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Abort failed.");
			if(retval == FALSE)
				Autoguider_General_Error();
		}
	}
	else if(strncmp(client_message,"configload",10) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				       "configload detected.");
#endif
		retval = Autoguider_Command_Config_Load(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
		else
		{
			Autoguider_General_Error();
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Config_Load failed.");
			if(retval == FALSE)
				Autoguider_General_Error();
		}
	}
	else if(strncmp(client_message,"expose",6) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				       "expose detected.");
#endif
		retval = Autoguider_Command_Expose(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
		else
		{
			Autoguider_General_Error();
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Expose failed.");
			if(retval == FALSE)
				Autoguider_General_Error();
		}
	}
	else if(strncmp(client_message,"field",5) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				       "field detected.");
#endif
		retval = Autoguider_Command_Field(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
		else
		{
			Autoguider_General_Error();
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Field failed.");
			if(retval == FALSE)
				Autoguider_General_Error();
		}
	}
	else if(strncmp(client_message,"getfits",7) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				       "getfits detected.");
#endif
		retval = Autoguider_Command_Get_Fits(client_message,&buffer_ptr,&buffer_length);
		if(retval == TRUE)
		{
			retval = Send_Binary_Reply(connection_handle,buffer_ptr,buffer_length);
			if(buffer_ptr != NULL)
				free(buffer_ptr);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
		else
		{
			retval = Send_Binary_Reply_Error(connection_handle);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
	}
	else if(strncmp(client_message,"guide",5) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				       "guide detected.");
#endif
		retval = Autoguider_Command_Guide(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
		else
		{
			Autoguider_General_Error();
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Guide failed.");
			if(retval == FALSE)
				Autoguider_General_Error();
		}
	}
	else if(strcmp(client_message, "help") == 0)
	{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
			       "help detected.");
#endif
		Send_Reply(connection_handle, "help:\n"
			   "\tabort\n"
			   /*"\tbias\n"*/
			   /*"\tdark <ms>\n"*/
			   "\tconfigload\n"
			   "\texpose <ms>\n"
			   "\tfield [ms]\n"
			   "\tfield <dark|flat|object> <on|off>\n"
			   "\tgetfits [field|guide] [raw|reduced]\n"
			   "\tguide [on|off|window <sx> <sy> <ex> <ey>|exposure_length <ms>]\n"
			   "\tguide <dark|flat|object> <on|off>\n"
			   "\tguide <object> <index>\n"
			   "\thelp\n"
			   "\tobject get <list|count|index>\n"
			   /*"\tmultrun <length> <count> <object>\n"*/
			   "\tstatus temperature <get|status>\n"
			   "\tstatus <field|guide> <active|dark|flat|object>\n"
			   "\tstatus object <list|count>\n"
			   "\ttemperature [set <C>|cooler [on|off]]\n"
			   "\tshutdown\n");
	}
	else if(strncmp(client_message,"status",6) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
			       "status detected.");
#endif
		retval = Autoguider_Command_Status(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
		else
		{
			Autoguider_General_Error();
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Status failed.");
			if(retval == FALSE)
				Autoguider_General_Error();
		}
	}
	else if(strncmp(client_message,"temperature",11) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				       "temperature detected.");
#endif
		retval = Autoguider_Command_Temperature(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
				Autoguider_General_Error();
		}
		else
		{
			Autoguider_General_Error();
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Temperature failed.");
			if(retval == FALSE)
				Autoguider_General_Error();
		}
	}
	else if(strcmp(client_message, "shutdown") == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Autoguider_Server_Connection_Callback:"
				       "shutdown detected:about to stop.");
#endif
		retval = Send_Reply(connection_handle, "0 ok");
		if(retval == FALSE)
			Autoguider_General_Error();
		retval = Autoguider_Server_Stop();
		if(retval == FALSE)
			Autoguider_General_Error();
	}
	else
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,
					      "Autoguider_Server_Connection_Callback:"
					      "message unknown: '%s'\n", client_message);
#endif
		retval = Send_Reply(connection_handle, "1 failed message unknown");
		if(retval == FALSE)
			Autoguider_General_Error();
	}
	/* free message */
	free(client_message);
}

/**
 * Send a message back to the client.
 * @param connection_handle Globus_io connection handle for this thread.
 * @param reply_message The message to send.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_SERVER
 * @see ../command_server/cdocs/command_server.html#Command_Server_Write_Message
 */
static int Send_Reply(Command_Server_Handle_T connection_handle,char *reply_message)
{
	int retval;

	/* send something back to the client */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Send_Reply: about to send '%.80s'...",
				      reply_message);
#endif
	retval = Command_Server_Write_Message(connection_handle, reply_message);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 204;
		sprintf(Autoguider_General_Error_String,"Send_Reply:"
			"Writing message to connection failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Send_Reply: sent '%.80s'...",reply_message);
#endif
	return TRUE;
}

/**
 * Send a binary message back to the client.
 * @param connection_handle Globus_io connection handle for this thread.
 * @param buffer_ptr A pointer to the binary data to send.
 * @param buffer_length The number of bytes in the binary buffer.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_SERVER
 * @see ../command_server/cdocs/command_server.html#Command_Server_Write_Binary_Message
 */
static int Send_Binary_Reply(Command_Server_Handle_T connection_handle,void *buffer_ptr,size_t buffer_length)
{
	int retval;

#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Send_Binary_Reply: about to send %ld bytes.",
				      buffer_length);
#endif
	retval = Command_Server_Write_Binary_Message(connection_handle,buffer_ptr,buffer_length);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 206;
		sprintf(Autoguider_General_Error_String,"Send_Binary_Reply:"
			"Writing binary message of length %ld to connection failed.",buffer_length);
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Send_Binary_Reply: sent %ld bytes.",
				      buffer_length);
#endif
	return TRUE;
}

/**
 * Send a binary message back to the client, after something went wrong to stops ending a FITS image back.
 * This involves putting the error string into a buffer and sending that back as the binary data, the client
 * end should realise this is not a FITS image!
 * @param connection_handle Globus_io connection handle for this thread.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Error_To_String
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_SERVER
 * @see ../command_server/cdocs/command_server.html#Command_Server_Write_Binary_Message
 */
static int Send_Binary_Reply_Error(Command_Server_Handle_T connection_handle)
{
	char error_buff[1024];
	int retval;


	Autoguider_General_Error_To_String(error_buff);
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Send_Binary_Reply_Error:"
				      "about to send error '%s' : Length %ld bytes.",
				      error_buff,strlen(error_buff));
#endif
	retval = Command_Server_Write_Binary_Message(connection_handle,error_buff,strlen(error_buff));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 205;
		sprintf(Autoguider_General_Error_String,"Send_Binary_Reply_Error:"
			"Writing binary error message '%s' of length %ld to connection failed.",
			error_buff,strlen(error_buff));
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_SERVER,"Send_Binary_Reply_Error: sent %ld bytes.",
				      strlen(error_buff));
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
