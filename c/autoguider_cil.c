/* autoguider_cil.c
** Autoguider CIL server routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_cil.c,v 1.5 2006-06-27 20:43:21 cjm Exp $
*/
/**
 * Autoguider CIL Server routines for the autoguider program.
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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ngatcil_general.h"
#include "ngatcil_cil.h"
#include "ngatcil_tcs_guide_packet.h" /* tcs guide packets */
#include "ngatcil_udp_raw.h"

#include "autoguider_command.h"
#include "autoguider_general.h"
#include "autoguider_guide.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_cil.c,v 1.5 2006-06-27 20:43:21 cjm Exp $";
/**
 * UDP CIL port to wait for TCS commands on.
 * @see ../cdocs/ngatcil_cil.html#NGATCIL_CIL_AGS_PORT_DEFAULT
 */
static int CIL_UDP_Port = NGATCIL_CIL_AGS_PORT_DEFAULT;
/**
 * UDP CIL Socket file descriptor of port to wait for TCS commands on.
 * @see #CIL_UDP_Port
 */
static int CIL_UDP_Socket_Fd = -1;
/**
 * Boolean, TRUE if we want to start the CIL UDP Server Port.
 */
static int CIL_UDP_Server_Start = TRUE;
/**
 * TCC hostname, the TCC should be running the TCS.
 * @see ../cdocs/ngatcil_tcs_guide_packet.html#NGATCIL_TCS_GUIDE_PACKET_TCC_DEFAULT
 */
static char TCC_Hostname[256] = NGATCIL_TCS_GUIDE_PACKET_TCC_DEFAULT;
/**
 * TCS UDP CIL Command port. Used for replies to AGS CIL commands.
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_TCS_PORT_DEFAULT
 */
static int CIL_TCS_UDP_Port = NGATCIL_CIL_TCS_PORT_DEFAULT;
/**
 * TCS UDP CIL Guide Packet port. Used to send RAW/ASCII guide packets to the TCS.
 * @see ../ngatcil/cdocs/ngatcil_tcs_guide_packet.html#NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT
 */
static int CIL_TCS_UDP_Guide_Port = NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT;
/**
 * UDP Socket file descriptor of the opened guide packet socket.
 */
static int CIL_TCS_Guide_Packet_Socket_Fd = -1;
/**
 * Boolean used to determine whether to send TCS guide packets.
 */
static int CIL_TCS_UDP_Guide_Packet_Send = TRUE;

/* internal functions */
static int Autoguider_CIL_Server_Connection_Callback(int socket_id,void* message_buff,int message_length);
static int CIL_UDP_Autoguider_On_Reply_Send(float pixel_x,float pixel_y,int status,int sequence_number);
static int CIL_UDP_Autoguider_Off_Reply_Send(int status,int sequence_number);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Autoguider CIL UDP initialisation routine. Assumes CCD_Config_Load has previously been called
 * to load the configuration file.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #Autoguider_CIL_Server_Start
 * @see #CIL_UDP_Port
 * @see #CIL_UDP_Server_Start
 * @see #TCC_Hostname
 * @see #CIL_TCS_UDP_Port
 * @see #CIL_TCS_UDP_Guide_Port
 * @see #CIL_TCS_UDP_Guide_Packet_Send
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
int Autoguider_CIL_Server_Initialise(void)
{
	int retval;
	char *string_ptr = NULL;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Server_Initialise:started.");
#endif
	/* get cil server port number from config */
	retval = CCD_Config_Get_Integer("cil.server.port_number",&CIL_UDP_Port);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1100;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
			"Failed to find CIL server port number (cil.server.port_number) in config file.");
		return FALSE;
	}
	/* get whether to start the CIL UDP server */
	retval = CCD_Config_Get_Boolean("cil.server.start",&CIL_UDP_Server_Start);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1100;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
			"Failed to find CIL server start (cil.server.start) in config file.");
		return FALSE;
	}
 	/* get cil TCS server hostname from config */
	retval = CCD_Config_Get_String("cil.tcs.hostname",&string_ptr);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1111;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
			"Failed to find CIL TCS server hostname (cil.tcs.hostname) in config file.");
		return FALSE;
	}
	if(strlen(string_ptr) > 255)
	{
		Autoguider_General_Error_Number = 1112;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
			"CIL TCS server hostname is too long (%d).",strlen(string_ptr));
		return FALSE;
	}
	strcpy(TCC_Hostname,string_ptr);
	free(string_ptr);
	/* get cil TCS server port number from config */
	retval = CCD_Config_Get_Integer("cil.tcs.command_reply.port_number",&CIL_TCS_UDP_Port);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1113;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
		      "Failed to find CIL TCS server port number (cil.tcs.command_reply.port_number) in config file.");
		return FALSE;
	}
	/* get cil TCS guide port number from config */
	retval = CCD_Config_Get_Integer("cil.tcs.guide_packet.port_number",&CIL_TCS_UDP_Guide_Port);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1120;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
		      "Failed to find CIL TCS guide port number (cil.tcs.guide_packet.port_number) in config file.");
		return FALSE;
	}
	/* get whether to send cil TCS guide packets from config */
	retval = CCD_Config_Get_Boolean("cil.tcs.guide_packet.send",&CIL_TCS_UDP_Guide_Packet_Send);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1124;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
		      "Failed to find CIL TCS guide packet send (cil.tcs.guide_packet.send) in config file.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Server_Initialise:finished.");
#endif
	return TRUE;
}

/**
 * Autoguider CIL server start routine.
 * This routine starts the server. It returns immediately (the listening is done on a new thread).
 * Use Autoguider_Server_Stop to stop the started server.
 * The server is only started if CIL_UDP_Server_Start is TRUE.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #CIL_UDP_Port
 * @see #CIL_UDP_Server_Start
 * @see #CIL_UDP_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see #Autoguider_CIL_Server_Connection_Callback
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Server_Start
 */
int Autoguider_CIL_Server_Start(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Server_Start:started.");
#endif
	if(CIL_UDP_Server_Start)
	{
#if AUTOGUIDER_DEBUG > 2
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
					      "Autoguider_CIL_Server_Start:Starting server on port %d.",
					      CIL_UDP_Port);
#endif
		retval = NGATCil_UDP_Server_Start(CIL_UDP_Port,NGATCIL_CIL_PACKET_LENGTH,&CIL_UDP_Socket_Fd,
						  Autoguider_CIL_Server_Connection_Callback);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 1101;
			sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Start:"
				"GATCil_UDP_Server_Start returned FALSE.");
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 2
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				       "Autoguider_CIL_Server_Start:NOT starting CIL server.");
#endif
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Server_Start:finished.");
#endif
	return TRUE;
}

/**
 * Autoguider server stop routine. The server is NOT stopped if it was not started, see CIL_UDP_Server_Start.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #CIL_UDP_Socket_Fd
 * @see #CIL_UDP_Server_Start
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Close
 */
int Autoguider_CIL_Server_Stop(void)
{
	int retval;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Server_Stop:started.");
#endif
	if(CIL_UDP_Server_Start)
	{
		retval = NGATCil_UDP_Close(CIL_UDP_Socket_Fd);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 1102;
			sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Stop:"
				"NGATCil_UDP_Close returned FALSE.");
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 2
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				       "Autoguider_CIL_Server_Stop:NOT started so not stopping CIL server.");
#endif
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Server_Stop:finished.");
#endif
	return TRUE;
}

/**
 * Routine to open a socket file descriptor to send TCS RAW/ASCII (Not CIL!) UDP guide packets over.
 * Assumes Autoguider_CIL_Server_Initialise has been called to setup TCC_Hostname/CIL_TCS_UDP_Guide_Port.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #TCC_Hostname
 * @see #CIL_TCS_UDP_Guide_Port
 * @see #CIL_TCS_Guide_Packet_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Open
 */
int Autoguider_CIL_Guide_Packet_Open(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Guide_Packet_Open:started.");
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Guide_Packet_Open:Opening %s:%d.",
				      TCC_Hostname,CIL_TCS_UDP_Guide_Port);
#endif
	retval = NGATCil_UDP_Open(TCC_Hostname,CIL_TCS_UDP_Guide_Port,&CIL_TCS_Guide_Packet_Socket_Fd);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1121;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Guide_Packet_Open:NGATCil_UDP_Open failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Guide_Packet_Open:finished.");
#endif
	return TRUE;
}

/**
 * Routine to send a TCS guide packet to the TCS guide packet port as a raw/ASCII UDP packet.
 * Assumes Autoguider_CIL_Guide_Packet_Open has been called to setup CIL_TCS_Guide_Packet_Socket_Fd.
 * @param x_pos The X position of the AG centroid, in pixels from the <b>edge of the CCD</b>, 
 *        <b>NOT</b> the guide window.
 * @param y_pos The Y position of the AG centroid, in pixels from the <b>edge of the CCD</b>, 
 *        <b>NOT</b> the guide window.
 * @param terminating Boolean. If TRUE the autoguider is stopping guiding.
 * @param unreliable Boolean. If TRUE the centroid is unreliable.
 * @param timecode_secs The number of seconds the TCS should wait for until the next guide packet will be sent
 *        (the TCS actually waits for twice this time). This is a float in the range 0..9999 seconds.
 *        If unreliable is TRUE this value will be sent negative.
 *        If terminating is TRUE the value 0.0 will be sent instead.
 * @param status_char The status byte, should be a character in the range '0'..'7' ('0' == most confident) or 
 *       NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED or NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW.
 *       More info from Steve Foale at TTL:
 *       <ul>
 *       <li>0 means confident
 *       <li>Bit 0 set means FWHM approaching limit
 *       <li>Bit 1 set means brightness approaching limit
 *       <li>Bit 2 set means critical error
 *       </ul>
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #CIL_TCS_Guide_Packet_Socket_Fd
 * @see #CIL_TCS_UDP_Guide_Packet_Send
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_tcs_guide_packet.html#NGATCil_TCS_Guide_Packet_Send
 * @see ../ngatcil/cdocs/ngatcil_tcs_guide_packet.html#NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED
 * @see ../ngatcil/cdocs/ngatcil_tcs_guide_packet.html#NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW
 */
int Autoguider_CIL_Guide_Packet_Send(float x_pos,float y_pos,int terminating,int unreliable,float timecode_secs,
				     char status_char)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Guide_Packet_Send:started.");
#endif
	if(CIL_TCS_UDP_Guide_Packet_Send)
	{
		/* send the guide packet
		** we are relying on this routine to check the arguments - it does. */
		retval = NGATCil_TCS_Guide_Packet_Send(CIL_TCS_Guide_Packet_Socket_Fd,x_pos,y_pos,terminating,
						       unreliable,timecode_secs,status_char);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 1122;
			sprintf(Autoguider_General_Error_String,
			"Autoguider_CIL_Guide_Packet_Send:NGATCil_TCS_Guide_Packet_Send failed.");
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Guide_Packet_Send:"
				      "Send is FALSE:Did not send packet:"
				    "x_pos=%f,y_pos=%f,terminating=%d,unreliable=%d,timecode_secs=%f,status_char=%c.",
				      x_pos,y_pos,terminating,unreliable,timecode_secs,status_char);
#endif
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Guide_Packet_Send:finished.");
#endif
	return TRUE;
}

/**
 * Routine to close the opened socket file descriptor CIL_TCS_Guide_Packet_Socket_Fd.
 * Assumes Autoguider_CIL_Guide_Packet_Open has been called to setup CIL_TCS_Guide_Packet_Socket_Fd.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #CIL_TCS_Guide_Packet_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Close
 */
int Autoguider_CIL_Guide_Packet_Close(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Guide_Packet_Close:started.");
#endif
	retval = NGATCil_UDP_Close(CIL_TCS_Guide_Packet_Socket_Fd);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1123;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Guide_Packet_Close:NGATCil_UDP_Close failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_Guide_Packet_Close:finished.");
#endif
	return TRUE;
}

/**
 * Set whether the autoguider is configured to send TCS guide packets to the TCS.
 * @param on The value to use, either TRUE or FALSE.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #CIL_TCS_UDP_Guide_Packet_Send
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_CIL_Guide_Packet_Send_Set(int on)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(on))
	{
		Autoguider_General_Error_Number = 1125;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Guide_Packet_Send_Set:Illegal argument %d.",
			on);
		return FALSE;
	}
	CIL_TCS_UDP_Guide_Packet_Send = on;
	return TRUE;
}

/**
 * Get whether the autoguider is configured to send TCS guide packets to the TCS.
 * @see #CIL_TCS_UDP_Guide_Packet_Send
 */
int Autoguider_CIL_Guide_Packet_Send_Get(void)
{
	return CIL_TCS_UDP_Guide_Packet_Send;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * This callback is called when a CIL command packet is recieved from the CIL server.
 * @param socket_id The socket file descriptor.
 * @param message_buff A pointer to the memory holding the CIL message.
 * @param message_length The length of message_buff in bytes.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #CIL_UDP_Autoguider_On_Reply_Send
 * @see #CIL_UDP_Autoguider_Off_Reply_Send
 * @see autoguider_command.html#Autoguider_Command_Autoguide_On
 * @see autoguider_command.html#COMMAND_AG_ON_TYPE
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see autoguider_guide.html#Autoguider_Guide_On
 * @see autoguider_guide.html#Autoguider_Guide_Off
 * @see ../ngatcil/cdocs/ngatcil_cil.html#E_CIL_CMD_CLASS
 * @see ../ngatcil/cdocs/ngatcil_cil.html#E_AGS_CMD
 * @see ../ngatcil/cdocs/ngatcil_cil.html#E_AGS_BAD_CMD
 * @see ../ngatcil/cdocs/ngatcil_cil.html#SYS_NOMINAL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#E_AGS_LOOP_ERROR
 * @see ../ngatcil/cdocs/ngatcil_cil.html#E_AGS_LOOP_STOPPING
 */
static int Autoguider_CIL_Server_Connection_Callback(int socket_id,void* message_buff,int message_length)
{
	struct NGATCil_Cil_Packet_Struct cil_packet;
	float pixel_x = 0.0f,pixel_y = 0.0f;
	int sequence_number = 0,status = 0,retval;
	int *debug_message_buff_ptr = NULL;

#ifndef AUTOGUIDER_CIL_SILENCE
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_Server_Connection_Callback:started.");
#endif
#endif
	if(message_length != NGATCIL_CIL_PACKET_LENGTH)
	{
#ifndef AUTOGUIDER_CIL_SILENCE
#if AUTOGUIDER_DEBUG > 1
		debug_message_buff_ptr = (int*)message_buff;
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_Server_Connection_Callback:Unexpected packet of length %d:"
					      "Source = %#x,Dest = %#x,Class = %#x,Service = %#x,SeqNUm = %#x.",
					      message_length,debug_message_buff_ptr[0],debug_message_buff_ptr[1],
					      debug_message_buff_ptr[2],debug_message_buff_ptr[3],
					      debug_message_buff_ptr[4]);
#endif
		Autoguider_General_Error_Number = 1103;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Connection_Callback:"
			"received CIL packet of wrong length %d vs %d.",message_length,NGATCIL_CIL_PACKET_LENGTH);
		Autoguider_General_Error();
#endif
		return FALSE;
	}
	/* diddly won't work if NGATCIL_CIL_PACKET_LENGTH != sizeof(struct NGATCil_Cil_Packet_Struct) */
	memcpy(&cil_packet,message_buff,message_length);
	if(cil_packet.Class != E_CIL_CMD_CLASS)
	{
		Autoguider_General_Error_Number = 1104;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Connection_Callback:"
			"received CIL packet for wrong class %d vs %d.",cil_packet.Class,E_CIL_CMD_CLASS);
		Autoguider_General_Error();
		return FALSE;
	}
	if(cil_packet.Service != E_AGS_CMD)
	{
		Autoguider_General_Error_Number = 1105;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Connection_Callback:"
			"received CIL packet for wrong service %d vs %d.",cil_packet.Service,E_AGS_CMD);
		Autoguider_General_Error();
		return FALSE;
	}
	switch(cil_packet.Command)
	{
		case E_AGS_GUIDE_ON_PIXEL:
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_Server_Connection_Callback:Command is autoguider on pixel.");
#endif
			if(!NGATCil_Cil_Autoguide_On_Pixel_Parse(cil_packet,&pixel_x,&pixel_y,&sequence_number))
			{
				Autoguider_General_Error_Number = 1106;
				sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Connection_Callback:"
					"Failed to parse autoguider on pixel command.");
				Autoguider_General_Error();
				CIL_UDP_Autoguider_On_Reply_Send(pixel_x,pixel_y,E_AGS_BAD_CMD,sequence_number);
				return FALSE;
			}
			/* field, select object nearest pixel, start guide thread */
			retval = Autoguider_Command_Autoguide_On(COMMAND_AG_ON_TYPE_PIXEL,pixel_x,pixel_y,0);
			if(retval == TRUE)
			{
				if(!CIL_UDP_Autoguider_On_Reply_Send(pixel_x,pixel_y,SYS_NOMINAL,sequence_number))
				{
					Autoguider_General_Error_Number = 1107;
					sprintf(Autoguider_General_Error_String,
						"Autoguider_CIL_Server_Connection_Callback:"
						"Failed to send autoguider on pixel reply.");
					Autoguider_General_Error();
					return FALSE;
				}
			}
			else
			{
				Autoguider_General_Error();
				CIL_UDP_Autoguider_On_Reply_Send(pixel_x,pixel_y,E_AGS_LOOP_ERROR,sequence_number);
				return FALSE;
			}
			break;
		case E_AGS_GUIDE_OFF:
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_Server_Connection_Callback:Command is autoguider off.");
#endif
			if(!NGATCil_Cil_Autoguide_Off_Parse(cil_packet,&status,&sequence_number))
			{
				Autoguider_General_Error_Number = 1108;
				sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Connection_Callback:"
					"Failed to parse autoguider off command.");
				Autoguider_General_Error();
				return FALSE;
			}
			/* stop guide thread */
			retval = Autoguider_Guide_Off();
			if(retval == TRUE)
			{
				if(!CIL_UDP_Autoguider_Off_Reply_Send(SYS_NOMINAL,sequence_number))
				{
					Autoguider_General_Error_Number = 1109;
					sprintf(Autoguider_General_Error_String,
						"Autoguider_CIL_Server_Connection_Callback:"
						"Failed to send autoguider off reply.");
					return FALSE;
				}
			}
			else
			{
				Autoguider_General_Error();
				CIL_UDP_Autoguider_Off_Reply_Send(E_AGS_LOOP_STOPPING,sequence_number);
			}
			break;
		default:
			Autoguider_General_Error_Number = 1110;
			sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Connection_Callback:"
				"received CIL packet with unknown command %d.",cil_packet.Command);
			return FALSE;
			break;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_Server_Connection_Callback:finished.");
#endif
	return TRUE;
}

/**
 * Send a reply to an "autoguider on pixel" command sent to the AGS CIL port,
 * by sending a CIL UDP reply packet to the TCS UDP CIL command port.
 * @param pixel_x The X pixel to guide on in pixels.
 * @param pixel_y The Y pixel to guide on in pixels.
 * @param status Whether the autoguider has started autoguiding, either SYS_NOMINAL or an error code.
 * @param sequence_number The command sequence number - should be the same as parsed from the command request.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #TCC_Hostname
 * @see #CIL_TCS_UDP_Port
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Open
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Close
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_On_Pixel_Reply_Send
 */
static int CIL_UDP_Autoguider_On_Reply_Send(float pixel_x,float pixel_y,int status,int sequence_number)
{
	int retval,socket_fd;

#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_On_Reply_Send:started.");
#endif
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "CIL_UDP_Autoguider_On_Reply_Send:Opening UDP connection to %s:%d.",
				      TCC_Hostname,CIL_TCS_UDP_Port);
#endif
	retval = NGATCil_UDP_Open(TCC_Hostname,CIL_TCS_UDP_Port,&socket_fd);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1114;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_On_Reply_Send:"
			"Failed to open UDP CIL port (%s:%d).",TCC_Hostname,CIL_TCS_UDP_Port);
		return FALSE;
	}
	retval = NGATCil_Cil_Autoguide_On_Pixel_Reply_Send(socket_fd,pixel_x,pixel_y,status,sequence_number);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1115;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_On_Reply_Send:"
			"Failed to send autoguide on pixel reply packet.");
		NGATCil_UDP_Close(socket_fd);
		return FALSE;
	}
	retval = NGATCil_UDP_Close(socket_fd);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1116;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_On_Reply_Send:"
			"Failed to close UDP CIL port (%s:%d).",
			TCC_Hostname,CIL_TCS_UDP_Port);
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_On_Reply_Send:finished.");
#endif
	return TRUE;
}

/**
 * Send a reply to an "autoguider off" command sent to the AGS CIL port,
 * by sending a CIL UDP reply packet to the TCS UDP CIL command port.
 * @param status Reply status.
 * @param sequence_number Sequence number.
 * @see #TCC_Hostname
 * @see #CIL_TCS_UDP_Port
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Open
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Close
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_Off_Reply_Send
 */
static int CIL_UDP_Autoguider_Off_Reply_Send(int status,int sequence_number)
{
	int retval,socket_fd;

#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_Off_Reply_Send:started.");
#endif
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "CIL_UDP_Autoguider_Off_Reply_Send:Opening UDP connection to %s:%d.",
				      TCC_Hostname,CIL_TCS_UDP_Port);
#endif
	retval = NGATCil_UDP_Open(TCC_Hostname,CIL_TCS_UDP_Port,&socket_fd);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1117;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_Off_Reply_Send:"
			"Failed to open UDP CIL port (%s:%d).",TCC_Hostname,CIL_TCS_UDP_Port);
		NGATCil_General_Error();
		return FALSE;
	}
	retval = NGATCil_Cil_Autoguide_Off_Reply_Send(socket_fd,status,sequence_number);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1118;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_Off_Reply_Send:"
			"Failed to send autoguide off reply packet.");
		NGATCil_UDP_Close(socket_fd);
		return FALSE;
	}
	retval = NGATCil_UDP_Close(socket_fd);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1119;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_Off_Reply_Send:"
			"Failed to close UDP CIL port (%s:%d).",TCC_Hostname,CIL_TCS_UDP_Port);
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_Off_Reply_Send:finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.4  2006/06/21 14:07:01  cjm
** Added information on status char.
**
** Revision 1.3  2006/06/20 18:42:38  cjm
** Added capability to turn CIL UDP server on or off.
**
** Revision 1.2  2006/06/20 13:05:21  cjm
** Added CIL_TCS_UDP_Guide_Packet_Send/Autoguider_CIL_Guide_Packet_Send_Set / Autoguider_CIL_Guide_Packet_Send_Get to allow configuration of whether to send TCS guide packets.
**
** Revision 1.1  2006/06/12 19:21:07  cjm
** Initial revision
**
*/
