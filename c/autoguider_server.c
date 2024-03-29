/* autoguider_server.c
** Autoguider server routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_server.c,v 1.11 2009-01-30 18:01:33 cjm Exp $
*/
/**
 * Command Server routines for the autoguider program.
 * @author Chris Mottram
 * @version $Revision: 1.11 $
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log_udp.h"

#include "command_server.h"

#include "ccd_config.h"

#include "autoguider_command.h"
#include "autoguider_general.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_server.c,v 1.11 2009-01-30 18:01:33 cjm Exp $";
/**
 * The server context to use for this server.
 * @see ../command_server/cdocs/command_server.html#Command_Server_Server_Context_T
 */
static Command_Server_Server_Context_T Command_Server_Context = NULL;
/**
 * Command server port number.
 */
static unsigned short Command_Server_Port_Number = 1234;

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
 * It loads the unsigned short with key ""command.server.port_number" into the Command_Server_Port_Number variable
 * for use in Autoguider_Server_Start.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #Autoguider_Server_Start
 * @see #Command_Server_Port_Number
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Unsigned_Short
 */
int Autoguider_Server_Initialise(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("server","autoguider_server.c","Autoguider_Server_Initialise",
				      LOG_VERBOSITY_TERSE,"SERVER","started.");
#endif
	/* get port number from config */
	retval = CCD_Config_Get_Unsigned_Short("command.server.port_number",&Command_Server_Port_Number);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 200;
		sprintf(Autoguider_General_Error_String,"Failed to find port number in config file.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("server","autoguider_server.c","Autoguider_Server_Initialise",
				      LOG_VERBOSITY_TERSE,"SERVER","finished.");
#endif
	return TRUE;
}

/**
 * Autoguider server start routine.
 * This routine starts the server. It does not return until the server is stopped using Autoguider_Server_Stop.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #Command_Server_Port_Number
 * @see #Autoguider_Server_Connection_Callback
 * @see #Command_Server_Context
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see ../command_server/cdocs/command_server.html#Command_Server_Start_Server
 */
int Autoguider_Server_Start(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Start",
			       LOG_VERBOSITY_VERY_TERSE,"SERVER","started.");
#endif
#if AUTOGUIDER_DEBUG > 2
	Autoguider_General_Log_Format("server","autoguider_server.c","Autoguider_Server_Start",
				      LOG_VERBOSITY_VERY_TERSE,"SERVER",
				      "Starting multi-threaded server on port %hu.",Command_Server_Port_Number);
#endif
	retval = Command_Server_Start_Server(&Command_Server_Port_Number,Autoguider_Server_Connection_Callback,
					     &Command_Server_Context);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 201;
		sprintf(Autoguider_General_Error_String,"Autoguider_Server_Start:"
			"Command_Server_Start_Server returned FALSE.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Start",
			       LOG_VERBOSITY_VERY_TERSE,"SERVER","finished.");
#endif
	return TRUE;
}

/**
 * Autoguider server stop routine.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #Command_Server_Context
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see ../command_server/cdocs/command_server.html#Command_Server_Close_Server
 */
int Autoguider_Server_Stop(void)
{
	int retval;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("server","autoguider_server.c","Autoguider_Server_Stop",
				      LOG_VERBOSITY_VERY_TERSE,"SERVER","started.");
#endif
	retval = Command_Server_Close_Server(&Command_Server_Context);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 202;
		sprintf(Autoguider_General_Error_String,"Autoguider_Server_Stop:"
			"Command_Server_Close_Server returned FALSE.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("server","autoguider_server.c","Autoguider_Server_Stop",
				      LOG_VERBOSITY_VERY_TERSE,"SERVER","finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Server connection thread, invoked whenever a new command comes in.
 * A client message is read over the connection, and based on the start of the command one of the following command
 * routines in invoked to process the command.
 * <ul>
 * <li><b>abort</b> Autoguider_Command_Abort
 * <li><b>autoguide</b> Autoguider_Command_Autoguide
 * <li><b>agstate</b> Autoguider_Command_Agstate
 * <li><b>configload</b> Autoguider_Command_Config_Load
 * <li><b>expose</b> Autoguider_Command_Expose
 * <li><b>field</b> Autoguider_Command_Field
 * <li><b>getfits</b> Autoguider_Command_Get_Fits
 * <li><b>guide</b> Autoguider_Command_Guide
 * <li><b>log_level</b> Autoguider_Command_Log_Level
 * <li><b>object</b> Autoguider_Command_Object
 * <li><b>status</b> Autoguider_Command_Status
 * <li><b>temperature</b> Autoguider_Command_Temperature
 * </ul>
 * There are some commands that are handled internally in this routine:
 * <ul>
 * <li><b>help</b> Returns a help message describing the commandset.
 * <li><b>shutdown</b> Calls Autoguider_Server_Stop to stop the command server (and eventually the whole autoguider process).
 * </ul>
 * @param connection_handle Connection handle for this thread.
 * @see #Send_Reply
 * @see #Send_Binary_Reply
 * @see #Send_Binary_Reply_Error
 * @see #Autoguider_Server_Stop
 * @see autoguider_command.html#Autoguider_Command_Abort
 * @see autoguider_command.html#Autoguider_Command_Autoguide
 * @see autoguider_command.html#Autoguider_Command_Agstate
 * @see autoguider_command.html#Autoguider_Command_Config_Load
 * @see autoguider_command.html#Autoguider_Command_Expose
 * @see autoguider_command.html#Autoguider_Command_Field
 * @see autoguider_command.html#Autoguider_Command_Get_Fits
 * @see autoguider_command.html#Autoguider_Command_Guide
 * @see autoguider_command.html#Autoguider_Command_Log_Level
 * @see autoguider_command.html#Autoguider_Command_Object
 * @see autoguider_command.html#Autoguider_Command_Status
 * @see autoguider_command.html#Autoguider_Command_Temperature
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log_Format
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
		Autoguider_General_Error("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
					 LOG_VERBOSITY_VERY_TERSE,"SERVER");
		return;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				      LOG_VERBOSITY_VERY_TERSE,"SERVER","received '%s'",client_message);
#endif
	/* do something with message */
	if(strncmp(client_message,"abort",5) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","abort detected.");
#endif
		retval = Autoguider_Command_Abort(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Abort failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"autoguide",9) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","autoguide detected.");
#endif
		retval = Autoguider_Command_Autoguide(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Autoguide failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"agstate",7) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","agstate detected.");
#endif
		retval = Autoguider_Command_Agstate(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Agstate failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"configload",10) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","configload detected.");
#endif
		retval = Autoguider_Command_Config_Load(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Config_Load failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"expose",6) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","expose detected.");
#endif
		retval = Autoguider_Command_Expose(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Expose failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"field",5) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","field detected.");
#endif
		retval = Autoguider_Command_Field(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Field failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"getfits",7) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","getfits detected.");
#endif
		retval = Autoguider_Command_Get_Fits(client_message,&buffer_ptr,&buffer_length);
		if(retval == TRUE)
		{
			retval = Send_Binary_Reply(connection_handle,buffer_ptr,buffer_length);
			if(buffer_ptr != NULL)
				free(buffer_ptr);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			retval = Send_Binary_Reply_Error(connection_handle);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"guide",5) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER","guide detected.");
#endif
		retval = Autoguider_Command_Guide(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Guide failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"log_level",9) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER","log_level detected.");
#endif
		retval = Autoguider_Command_Log_Level(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Log_Level failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"object",6) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER","object detected.");
#endif
		retval = Autoguider_Command_Object(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Object failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strcmp(client_message, "help") == 0)
	{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
			       LOG_VERBOSITY_VERY_TERSE,"SERVER","help detected.");
#endif
		Send_Reply(connection_handle, "help:\n"
			   "\tabort\n"
			   "\tagstate <n>\n"
			   "\tautoguide on <brightest|pixel <x> <y>|rank <n>>\n"
			   "\tautoguide off\n"
			   "\tconfigload\n"
			   "\texpose <ms>\n"
			   "\tfield [<ms> [lock]]\n"
			   "\tfield <dark|flat|object> <on|off>\n"
			   "\tgetfits [field|guide|object] [raw|reduced]\n"
			   "\tguide [on|off]\n"
			   "\tguide window <sx> <sy> <ex> <ey>\n"
			   "\tguide window <cx> <cy>\n"
			   "\tguide exposure_length <ms> [lock]\n"
			   "\tguide <dark|flat|object|packet|window_track> <on|off>\n"
			   "\tguide object <index>\n"
			   "\thelp\n"
			   "\tlog_level <autoguider|ccd|command_server|object|ngatcil> <n>\n"
			   "\tobject <sigma|sigma_reject|ellipticity_limit|min_con_pix> <n>\n"
			   /*"\tmultrun <length> <count> <object>\n"*/
			   "\tstatus temperature <get|status>\n"
			   "\tstatus field <active|dark|flat|object>\n"
			   "\tstatus guide <active|dark|flat|object|packet>\n"
			   "\tstatus guide <cadence|timecode_scaling|exposure_length|window>\n"
			   "\tstatus guide <last_object|initial_position>\n"
			   "\tstatus object <list|count|median|mean|background_standard_deviation|threshold>\n"
			   "\tstatus object <sigma|sigma_reject|ellipticity_limit|min_con_pix>\n"
			   "\ttemperature [set <C>|cooler [on|off]]\n"
			   "\tshutdown\n");
	}
	else if(strncmp(client_message,"status",6) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
			       LOG_VERBOSITY_VERY_TERSE,"SERVER","status detected.");
#endif
		retval = Autoguider_Command_Status(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Status failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strncmp(client_message,"temperature",11) == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","temperature detected.");
#endif
		retval = Autoguider_Command_Temperature(client_message,&reply_string);
		if(retval == TRUE)
		{
			retval = Send_Reply(connection_handle,reply_string);
			if(reply_string != NULL)
				free(reply_string);
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
		else
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			retval = Send_Reply(connection_handle, "1 Autoguider_Command_Temperature failed.");
			if(retval == FALSE)
			{
				Autoguider_General_Error("server","autoguider_server.c",
							 "Autoguider_Server_Connection_Callback",
							 LOG_VERBOSITY_VERY_TERSE,"SERVER");
			}
		}
	}
	else if(strcmp(client_message, "shutdown") == 0)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
				       LOG_VERBOSITY_VERY_TERSE,"SERVER","shutdown detected:about to stop.");
#endif
		retval = Send_Reply(connection_handle, "0 ok");
		if(retval == FALSE)
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
		retval = Autoguider_Server_Stop();
		if(retval == FALSE)
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log_Format("server","autoguider_server.c","Autoguider_Server_Connection_Callback",
					      LOG_VERBOSITY_VERY_TERSE,"SERVER","message unknown: '%s'\n",
					      client_message);
#endif
		retval = Send_Reply(connection_handle, "1 failed message unknown");
		if(retval == FALSE)
		{
			Autoguider_General_Error("server","autoguider_server.c",
						 "Autoguider_Server_Connection_Callback",
						 LOG_VERBOSITY_VERY_TERSE,"SERVER");
		}
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
 * @see ../command_server/cdocs/command_server.html#Command_Server_Write_Message
 */
static int Send_Reply(Command_Server_Handle_T connection_handle,char *reply_message)
{
	int retval;

	/* send something back to the client */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("server","autoguider_server.c","Send_Reply",LOG_VERBOSITY_TERSE,"SERVER",
				      "about to send '%.80s'...",reply_message);
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
	Autoguider_General_Log_Format("server","autoguider_server.c","Send_Reply",LOG_VERBOSITY_TERSE,"SERVER",
				      "sent '%.80s'...",reply_message);
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
 * @see ../command_server/cdocs/command_server.html#Command_Server_Write_Binary_Message
 */
static int Send_Binary_Reply(Command_Server_Handle_T connection_handle,void *buffer_ptr,size_t buffer_length)
{
	int retval;

#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("server","autoguider_server.c","Send_Binary_Reply",
				      LOG_VERBOSITY_INTERMEDIATE,"SERVER",
				      "about to send %ld bytes.",buffer_length);
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
	Autoguider_General_Log_Format("server","autoguider_server.c","Send_Binary_Reply",
				      LOG_VERBOSITY_INTERMEDIATE,"SERVER","sent %ld bytes.",buffer_length);
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
 * @see ../command_server/cdocs/command_server.html#Command_Server_Write_Binary_Message
 */
static int Send_Binary_Reply_Error(Command_Server_Handle_T connection_handle)
{
	char error_buff[1024];
	int retval;


	Autoguider_General_Error_To_String("server","autoguider_server.c","Send_Binary_Reply_Error",
				      LOG_VERBOSITY_INTERMEDIATE,"SERVER",error_buff);
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("server","autoguider_server.c","Send_Binary_Reply_Error",
				      LOG_VERBOSITY_INTERMEDIATE,"SERVER",
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
	Autoguider_General_Log_Format("server","autoguider_server.c","Send_Binary_Reply_Error",
				      LOG_VERBOSITY_INTERMEDIATE,"SERVER","sent %ld bytes.",strlen(error_buff));
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.10  2007/01/19 14:26:34  cjm
** Added guide window_track <on|off> command for guide window tracking
** control.
**
** Revision 1.9  2006/08/29 13:55:42  cjm
** Added agstate command call.
**
** Revision 1.8  2006/07/16 20:13:54  cjm
** Fixed help.
**
** Revision 1.7  2006/06/29 17:04:34  cjm
** Added help message.
**
** Revision 1.6  2006/06/21 14:10:04  cjm
** Updated help.
**
** Revision 1.5  2006/06/20 18:42:38  cjm
** Fixed help message.
**
** Revision 1.4  2006/06/20 13:05:21  cjm
** Added autoguide command.
**
** Revision 1.3  2006/06/12 19:22:13  cjm
** Added log_level handling.
** Some variables names changes.
**
** Revision 1.2  2006/06/02 13:43:36  cjm
** Fixed logging to stop logging retply messages which may be longer than
** the internal loggers buffers - only log first 80 chars of message.
**
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
