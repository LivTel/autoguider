/* autoguider_cil.c
** Autoguider CIL server routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_cil.c,v 1.11 2008-12-09 17:25:55 cjm Exp $
*/
/**
 * Autoguider CIL Server routines for the autoguider program.
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
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ngatcil_ags_sdb.h"
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
static char rcsid[] = "$Id: autoguider_cil.c,v 1.11 2008-12-09 17:25:55 cjm Exp $";
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
/**
 * Boolean used to determine whether to send TCS guide packets.
 */
static int CIL_SDB_Send = TRUE;
/**
 * Number of continuous hearbeats received.
 */
static int CIL_CHB_Count = 0;

/* internal functions */
static int Autoguider_CIL_Server_Connection_Callback(int socket_id,void* message_buff,int message_length);
static int CIL_Command_TCS_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
				   size_t message_length);
static int CIL_Command_CHB_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
				   size_t message_length);
static int CIL_Command_MCP_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
				   size_t message_length);
static int CIL_Command_MCP_Activate_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
					    size_t message_length,int status);
static int CIL_Command_MCP_Safe_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
					size_t message_length);
static int CIL_UDP_Autoguider_On_Pixel_Reply_Send(float pixel_x,float pixel_y,int status,int sequence_number);
static int CIL_UDP_Autoguider_On_Brightest_Reply_Send(int status,int sequence_number);
static int CIL_UDP_Autoguider_On_Rank_Reply_Send(int rank,int status,int sequence_number);
static int CIL_UDP_Autoguider_Off_Reply_Send(int status,int sequence_number);
static int CIL_Command_Start_Session_Reply_Send(struct NGATCil_Ags_Packet_Struct cil_packet,int status);
static int CIL_Command_End_Session_Reply_Send(struct NGATCil_Ags_Packet_Struct cil_packet,int status);

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
 * @see #MCC_Hostname
 * @see #CIL_SDB_UDP_Port
 * @see #CIL_SDB_Send 
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 * @see ../ngatcil/cdocs/ngatcil_ags_sdb.html#NGATCil_AGS_SDB_Initialise
 */
int Autoguider_CIL_Server_Initialise(void)
{
	int retval;
	char *string_ptr = NULL;
	char mcc_hostname[256];
	int sdb_udp_port;

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
		Autoguider_General_Error_Number = 1136;
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
 	/* get cil MCC server hostname from config */
	retval = CCD_Config_Get_String("cil.mcc.hostname",&string_ptr);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1137;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
			"Failed to find CIL MCC server hostname (cil.mcc.hostname) in config file.");
		return FALSE;
	}
	if(strlen(string_ptr) > 255)
	{
		Autoguider_General_Error_Number = 1138;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
			"CIL MCC server hostname is too long (%d).",strlen(string_ptr));
		return FALSE;
	}
	strcpy(mcc_hostname,string_ptr);
	free(string_ptr);
	/* get cil SDB port number from config */
	retval = CCD_Config_Get_Integer("cil.sdb.port_number",&sdb_udp_port);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1139;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
		      "Failed to find CIL SDB port number (cil.sdb.port_number) in config file.");
		return FALSE;
	}
	/* get whether to send cil SDB packets from config */
	retval = CCD_Config_Get_Boolean("cil.sdb.packet.send",&CIL_SDB_Send);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1149;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
		      "Failed to find CIL SDB packet send (cil.sdb.packet.send) in config file.");
		return FALSE;
	}
	/* initialise AGS SDB timestamps */
#if AUTOGUIDER_DEBUG > 1
	 Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				"Autoguider_CIL_Server_Initialise:NGATCil_AGS_SDB_Initialise.");
#endif
	retval = NGATCil_AGS_SDB_Initialise();
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1141;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
		      "NGATCil_AGS_SDB_Initialise failed.");
		return FALSE;
	}
	retval = NGATCil_AGS_SDB_Remote_Host_Set(mcc_hostname,sdb_udp_port);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1142;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_Server_Initialise:"
		      "NGATCil_AGS_SDB_Remote_Host_Set(%s,%d) failed.",mcc_hostname,sdb_udp_port);
		return FALSE;
	}
	/* reset heartbeat count */
	CIL_CHB_Count = 0;
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
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_AGS_MAX_PACKET_LENGTH
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
		/* iAgsReceiveMessage in AgsMain.c suggests a buffer length of BASE + I_AGS_CIL_DATALEN (256) */
		retval = NGATCil_UDP_Server_Start(CIL_UDP_Port,NGATCIL_CIL_AGS_MAX_PACKET_LENGTH,&CIL_UDP_Socket_Fd,
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
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_Guide_Packet_Send(x=%.2f,y=%.2f,terminating=%#x,unreliable=%#x,"
				      "timecode=%.2f,status=%c):started.",x_pos,y_pos,terminating,unreliable,
				      timecode_secs,status_char);
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

/**
 * Set the Autoguider AG state.
 * @param state The state to use, from eAgsState_t (ngatcil_ags_sdb.h).
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_ags_sdb.html#eAgsState_t
 */
int Autoguider_CIL_SDB_Packet_State_Set(eAggState_t state)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_SDB_Packet_State_Set(%d):started.",state);
#endif
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_AGSTATE,state))
	{
		Autoguider_General_Error_Number = 1140;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_State_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_SDB_Packet_State_Set:finished.");
#endif
	return TRUE;
}

/**
 * Tell the SDB about the Autoguider exposure length.
 * @param ms The exposure length, in milliseconds.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_ags_sdb.html#eAgsState_t
 */
int Autoguider_CIL_SDB_Packet_Exp_Time_Set(int ms)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_SDB_Packet_Exp_Time_Set(%d):started.",ms);
#endif
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_INTTIME,ms))
	{
		Autoguider_General_Error_Number = 1145;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Exp_Time_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_SDB_Packet_Exp_Time_Set:finished.");
#endif
	return TRUE;
}

/**
 * Tell the SDB about the Autogudier guide centroid.
 * @param cx The centroid x position in pixels. Converted internally into millipixels to send to the SDB.
 * @param cy The centroid y position in pixels. Converted internally into millipixels to send to the SDB.
 * @param fwhm The centroid full width half maximum in pixels. 
 *        Converted internally into millipixels to send to the SDB.
 * @param mag The centroid magnitude.  
 *        Converted internally into millistarmags to send to the SDB.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 */
int Autoguider_CIL_SDB_Packet_Centroid_Set(float cx,float cy,float fwhm,float mag)
{
	int ivalue;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				 "Autoguider_CIL_SDB_Packet_Centroid_Set(cx=%.2f,cy=%.2f,fwhm=%.2f,mag%.2f):started.",
				      cx,cy,fwhm,mag);
#endif
	ivalue = (int)(cx*1000.0f);
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_CENTROIDX,ivalue))
	{
		Autoguider_General_Error_Number = 1146;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Centroid_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
	ivalue = (int)(cy*1000.0f);
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_CENTROIDY,ivalue))
	{
		Autoguider_General_Error_Number = 1147;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Centroid_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
	ivalue = (int)(fwhm*1000.0f);
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_FWHM,ivalue))
	{
		Autoguider_General_Error_Number = 1148;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Centroid_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
	ivalue = (int)(mag*1000.0f);
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_GUIDEMAG,ivalue))
	{
		Autoguider_General_Error_Number = 1150;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Centroid_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_SDB_Packet_Centroid_Set:finished.");
#endif
	return TRUE;
}

/**
 * Tell the SDB about the Autogudier guide window.
 * @param tlx The top left X position of the window in pixels. 
 *            Converted internally into millipixels to send to the SDB.
 * @param tly The top left Y position of the window in pixels. 
 *            Converted internally into millipixels to send to the SDB.
 * @param brx The bottom right X position of the window in pixels. 
 *            Converted internally into millipixels to send to the SDB.
 * @param bry The bottom right Y position of the window in pixels. 
 *            Converted internally into millipixels to send to the SDB.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 */
int Autoguider_CIL_SDB_Packet_Window_Set(int tlx,int tly,int brx,int bry)
{
	int ivalue;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				 "Autoguider_CIL_SDB_Packet_Window_Set(tlx=%d,tly=%d,brx=%d,bry=%d):started.",
				      tlx,tly,brx,bry);
#endif
	ivalue = (int)(tlx*1000.0);
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_WINDOW_TLX,ivalue))
	{
		Autoguider_General_Error_Number = 1104;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Window_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
	ivalue = (int)(tly*1000.0);
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_WINDOW_TLY,ivalue))
	{
		Autoguider_General_Error_Number = 1105;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Window_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
	ivalue = (int)(brx*1000.0);
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_WINDOW_BRX,ivalue))
	{
		Autoguider_General_Error_Number = 1114;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Window_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
	ivalue = (int)(bry*1000.0);
	if(!NGATCil_AGS_SDB_Value_Set(D_AGS_WINDOW_BRY,ivalue))
	{
		Autoguider_General_Error_Number = 1116;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Window_Set:"
			"NGATCil_AGS_SDB_Value_Set failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_SDB_Packet_Window_Set:finished.");
#endif
	return TRUE;
}

/**
 * Routine to submit status to the SDB using CIL.
 * The packet is only sent if CIL_SDB_Send is TRUE.
 * Assumes the CIL UDP server has previously been opened, to set CIL_UDP_Socket_Fd, 
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #CIL_UDP_Socket_Fd
 * @see #CIL_SDB_Send
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_agssdb.html#NGATCil_AGS_SDB_Status_Send
 */
int Autoguider_CIL_SDB_Packet_Send(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_SDB_Packet_Send:started.");
#endif
	if(CIL_SDB_Send)
	{
		/* submit the status to the SDB */
		retval = NGATCil_AGS_SDB_Status_Send(CIL_UDP_Socket_Fd);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 1151;
			sprintf(Autoguider_General_Error_String,
			"Autoguider_CIL_SDB_Packet_Send:NGATCil_AGS_SDB_Status_Send failed.");
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_SDB_Packet_Send:"
				      "Send is FALSE:Did not send packet.");
#endif
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"Autoguider_CIL_SDB_Packet_Send:finished.");
#endif
	return TRUE;
}

/**
 * Set whether the autoguider is configured to send SDB packets to the SDB.
 * @param on The value to use, either TRUE or FALSE.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #CIL_SDB_Send
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_CIL_SDB_Packet_Send_Set(int on)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(on))
	{
		Autoguider_General_Error_Number = 1144;
		sprintf(Autoguider_General_Error_String,"Autoguider_CIL_SDB_Packet_Send_Set:Illegal argument %d.",
			on);
		return FALSE;
	}
	CIL_SDB_Send = on;
	return TRUE;
}

/**
 * Get whether the autoguider is configured to send SDB packets to the SDB.
 * @see #CIL_SDB_Send
 */
int Autoguider_CIL_SDB_Packet_Send_Get(void)
{
	return CIL_SDB_Send;
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
 * @see #CIL_Command_TCS_Process
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
	struct NGATCil_Ags_Packet_Struct ags_cil_packet;
	float pixel_x = 0.0f,pixel_y = 0.0f;
	int sequence_number = 0,status = 0,retval,rank;
	int *debug_message_buff_ptr = NULL;

#ifndef AUTOGUIDER_CIL_SILENCE
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_Server_Connection_Callback:started.");
#endif
#endif
	if(message_length < NGATCIL_CIL_BASE_PACKET_LENGTH)
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
			"received CIL packet of wrong length %d vs %d.",message_length,NGATCIL_CIL_BASE_PACKET_LENGTH);
		Autoguider_General_Error();
#endif
		return FALSE;
	}
	/* diddly won't work if NGATCIL_CIL_BASE_PACKET_LENGTH != sizeof(struct NGATCil_Cil_Packet_Struct) */
	memcpy(&cil_packet,message_buff,NGATCIL_CIL_BASE_PACKET_LENGTH);
	switch(cil_packet.Source_Id)
	{
		case E_CIL_CHB:
			if(!CIL_Command_CHB_Process(cil_packet,message_buff,message_length))
			{
				Autoguider_General_Error();
				return FALSE;
			}
			break;
		case E_CIL_MCP:
			if(!CIL_Command_MCP_Process(cil_packet,message_buff,message_length))
			{
				Autoguider_General_Error();
				return FALSE;
			}
			break;
		case E_CIL_TCS:
			if(!CIL_Command_TCS_Process(cil_packet,message_buff,message_length))
			{
				Autoguider_General_Error();
				return FALSE;
			}
			break;
		case E_CIL_SDB:
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				"Autoguider_CIL_Server_Connection_Callback:SDB reply received (not processed yet).");
#endif
			break;
		default:
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				"Autoguider_CIL_Server_Connection_Callback:Source_Id %d not supported yet.",
						      cil_packet.Source_Id);
#endif
			break;
	}
#if AUTOGUIDER_DEBUG > 1
	if(cil_packet.Source_Id != E_CIL_CHB)
	{
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "Autoguider_CIL_Server_Connection_Callback:finished.");
	}
#endif
	return TRUE;
}

/**
 * Process a CHB command received from the server.
 * @param cil_packet The parsed generic CIL header.
 * @param message_buff A pointer to the memory holding the CIL message.
 * @param message_length The length of message_buff in bytes.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #CIL_UDP_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_CHB_HOSTNAME_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_CHB_PORT_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Status_Reply_Packet_Struct
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Raw_To_Network_Byte_Order
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Raw_Send_To
 */
static int CIL_Command_CHB_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
				   size_t message_length)
{
	struct NGATCil_Status_Reply_Packet_Struct chb_reply_packet;
	struct timespec current_time;
	int sequence_number = 0,status = 0,retval;

	if(cil_packet.Service != E_MCP_HEARTBEAT)
	{
		Autoguider_General_Error_Number = 1154;
		sprintf(Autoguider_General_Error_String,"CIL_Command_CHB_Process:Service %d not heartbeat.",
			cil_packet.Service);
		return FALSE;
	}
	if(cil_packet.Class != E_CIL_CMD_CLASS)
	{
		Autoguider_General_Error_Number = 1155;
		sprintf(Autoguider_General_Error_String,"CIL_Command_CHB_Process:Class %d not command.",
			cil_packet.Class);
		return FALSE;
	}
	chb_reply_packet.Cil_Base.Source_Id = E_CIL_AGS;
	chb_reply_packet.Cil_Base.Dest_Id = E_CIL_CHB;
	chb_reply_packet.Cil_Base.Class = E_CIL_RSP_CLASS;
	chb_reply_packet.Cil_Base.Service = E_MCP_HEARTBEAT;
	chb_reply_packet.Cil_Base.Seq_Num = cil_packet.Seq_Num;
	clock_gettime(CLOCK_REALTIME,&current_time);
	chb_reply_packet.Cil_Base.Timestamp_Seconds = current_time.tv_sec;
	chb_reply_packet.Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
	chb_reply_packet.Status = SYS_OKAY_STATE;
	NGATCil_UDP_Raw_To_Network_Byte_Order((void*)&chb_reply_packet,sizeof(struct NGATCil_Status_Reply_Packet_Struct));
	if(!NGATCil_UDP_Raw_Send_To(CIL_UDP_Socket_Fd,NGATCIL_CIL_CHB_HOSTNAME_DEFAULT,NGATCIL_CIL_CHB_PORT_DEFAULT,
				    (void*)&chb_reply_packet,sizeof(struct NGATCil_Status_Reply_Packet_Struct)))
	{
		Autoguider_General_Error_Number = 1156;
		sprintf(Autoguider_General_Error_String,"CIL_Command_CHB_Process:NGATCil_UDP_Raw_Send_To failed.");
		return FALSE;
	}
	CIL_CHB_Count++;
#if AUTOGUIDER_DEBUG > 9
	if((CIL_CHB_Count % 1000) == 0)
	{
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
					      "CIL_Command_CHB_Process:Received/Processed %d heartbeats.",
					      CIL_CHB_Count);
	}
#endif

	return TRUE;
}

/**
 * Process a MCP command received from the server.
 * @param cil_packet The parsed generic CIL header.
 * @param message_buff A pointer to the memory holding the CIL message.
 * @param message_length The length of message_buff in bytes.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #CIL_Command_MCP_Activate_Process
 * @see #CIL_Command_MCP_Safe_Process
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 */
static int CIL_Command_MCP_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
				   size_t message_length)
{
	struct NGATCil_Status_Reply_Packet_Struct chb_reply_packet;
	struct timespec current_time;
	int sequence_number = 0,status = 0,retval;

	if(cil_packet.Class != E_CIL_CMD_CLASS)
	{
		Autoguider_General_Error_Number = 1128;
		sprintf(Autoguider_General_Error_String,"CIL_Command_MCP_Process:Class %d not command.",
			cil_packet.Class);
		return FALSE;
	}
	switch(cil_packet.Service)
	{
		 case E_MCP_SHUTDOWN:
			 break;
		case E_MCP_ACTIVATE:
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				"CIL_Command_MCP_Process:Processing MCP Activate request.");
#endif
			if(!CIL_Command_MCP_Activate_Process(cil_packet,message_buff,message_length,SYS_OKAY_STATE))
				return FALSE;
			break;
		case E_MCP_SAFESTATE:
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				"CIL_Command_MCP_Process:Processing MCP Safe state request.");
#endif
			if(!CIL_Command_MCP_Safe_Process(cil_packet,message_buff,message_length))
				return FALSE;
			break;
		default:
			Autoguider_General_Error_Number = 1130;
			sprintf(Autoguider_General_Error_String,"CIL_Command_MCP_Process:Unknown Service %d.",
				cil_packet.Service);
			return FALSE;
			break;
	}
	return TRUE;
}

/**
 * Process an MCP activate command.
 * @param cil_packet The parsed generic CIL header.
 * @param message_buff A pointer to the memory holding the CIL message.
 * @param message_length The length of message_buff in bytes.
 * @param status The status to return, ususally SYS_OKAY_STATE.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #CIL_UDP_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_MCP_HOSTNAME_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_MCP_PORT_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Cil_Packet_Struct
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Status_Reply_Packet_Struct
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Raw_To_Network_Byte_Order
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Raw_Send_To
 */
static int CIL_Command_MCP_Activate_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
					    size_t message_length,int status)
{
	struct NGATCil_Cil_Packet_Struct act_reply_packet;
	struct NGATCil_Status_Reply_Packet_Struct com_reply_packet;
	struct timespec current_time;

	/* send Act reply */
	act_reply_packet.Source_Id = cil_packet.Dest_Id;
	act_reply_packet.Dest_Id = cil_packet.Source_Id;
	act_reply_packet.Class = E_CIL_ACT_CLASS;
	act_reply_packet.Service = cil_packet.Service;
	act_reply_packet.Seq_Num = cil_packet.Seq_Num;
	clock_gettime(CLOCK_REALTIME,&current_time);
	act_reply_packet.Timestamp_Seconds = current_time.tv_sec;
	act_reply_packet.Timestamp_Nanoseconds = current_time.tv_nsec;
	NGATCil_UDP_Raw_To_Network_Byte_Order((void*)&act_reply_packet,sizeof(struct NGATCil_Cil_Packet_Struct));
	/* act reply to mcp */
	if(!NGATCil_UDP_Raw_Send_To(CIL_UDP_Socket_Fd,NGATCIL_CIL_MCP_HOSTNAME_DEFAULT,NGATCIL_CIL_MCP_PORT_DEFAULT,
				    (void*)&act_reply_packet,sizeof(struct NGATCil_Cil_Packet_Struct)))
	{
		Autoguider_General_Error_Number = 1133;
		sprintf(Autoguider_General_Error_String,"CIL_Command_MCP_Activate_Process:NGATCil_UDP_Raw_Send_To failed.");
		return FALSE;
	}
	/* send COM reply */
	com_reply_packet.Cil_Base.Source_Id = cil_packet.Dest_Id;
	com_reply_packet.Cil_Base.Dest_Id = cil_packet.Source_Id;
	com_reply_packet.Cil_Base.Class = E_CIL_COM_CLASS;
	com_reply_packet.Cil_Base.Service = cil_packet.Service;
	com_reply_packet.Cil_Base.Seq_Num = cil_packet.Seq_Num;
	clock_gettime(CLOCK_REALTIME,&current_time);
	com_reply_packet.Cil_Base.Timestamp_Seconds = current_time.tv_sec;
	com_reply_packet.Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
	com_reply_packet.Status = status;
	NGATCil_UDP_Raw_To_Network_Byte_Order((void*)&com_reply_packet,
					      sizeof(struct NGATCil_Status_Reply_Packet_Struct));
	/* act reply to mcp */
	if(!NGATCil_UDP_Raw_Send_To(CIL_UDP_Socket_Fd,NGATCIL_CIL_MCP_HOSTNAME_DEFAULT,NGATCIL_CIL_MCP_PORT_DEFAULT,
				    (void*)&com_reply_packet,sizeof(struct NGATCil_Status_Reply_Packet_Struct)))
	{
		Autoguider_General_Error_Number = 1135;
		sprintf(Autoguider_General_Error_String,"CIL_Command_MCP_Activate_Process:"
			"NGATCil_UDP_Raw_Send_To failed.");
		return FALSE;
	}
	return TRUE;
}

/**
 * Process an MCP safe command.
 * @param cil_packet The parsed generic CIL header.
 * @param message_buff A pointer to the memory holding the CIL message.
 * @param message_length The length of message_buff in bytes.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #CIL_UDP_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_MCP_HOSTNAME_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_MCP_PORT_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Cil_Packet_Struct
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Status_Reply_Packet_Struct
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Raw_To_Network_Byte_Order
 * @see ../ngatcil/cdocs/ngatcil_udp_raw.html#NGATCil_UDP_Raw_Send_To
 */
static int CIL_Command_MCP_Safe_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
					    size_t message_length)
{
	struct NGATCil_Cil_Packet_Struct act_reply_packet;
	struct NGATCil_Status_Reply_Packet_Struct com_reply_packet;
	struct timespec current_time;

	/* send Act reply */
	act_reply_packet.Source_Id = cil_packet.Dest_Id;
	act_reply_packet.Dest_Id = cil_packet.Source_Id;
	act_reply_packet.Class = E_CIL_ACT_CLASS;
	act_reply_packet.Service = cil_packet.Service;
	act_reply_packet.Seq_Num = cil_packet.Seq_Num;
	clock_gettime(CLOCK_REALTIME,&current_time);
	act_reply_packet.Timestamp_Seconds = current_time.tv_sec;
	act_reply_packet.Timestamp_Nanoseconds = current_time.tv_nsec;
	NGATCil_UDP_Raw_To_Network_Byte_Order((void*)&act_reply_packet,sizeof(struct NGATCil_Cil_Packet_Struct));
	/* act (acted upon) reply to mcp */
	if(!NGATCil_UDP_Raw_Send_To(CIL_UDP_Socket_Fd,NGATCIL_CIL_MCP_HOSTNAME_DEFAULT,NGATCIL_CIL_MCP_PORT_DEFAULT,
				    (void*)&act_reply_packet,sizeof(struct NGATCil_Cil_Packet_Struct)))
	{
		Autoguider_General_Error_Number = 1143;
		sprintf(Autoguider_General_Error_String,
			"CIL_Command_MCP_Safe_Process:NGATCil_UDP_Raw_Send_To failed.");
		return FALSE;
	}
	/* set status to safe */
	/* send COM (completed) reply */
	com_reply_packet.Cil_Base.Source_Id = cil_packet.Dest_Id;
	com_reply_packet.Cil_Base.Dest_Id = cil_packet.Source_Id;
	com_reply_packet.Cil_Base.Class = E_CIL_COM_CLASS;
	com_reply_packet.Cil_Base.Service = cil_packet.Service;
	com_reply_packet.Cil_Base.Seq_Num = cil_packet.Seq_Num;
	clock_gettime(CLOCK_REALTIME,&current_time);
	com_reply_packet.Cil_Base.Timestamp_Seconds = current_time.tv_sec;
	com_reply_packet.Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
	com_reply_packet.Status = SYS_SAFE_STATE;
	NGATCil_UDP_Raw_To_Network_Byte_Order((void*)&com_reply_packet,
					      sizeof(struct NGATCil_Status_Reply_Packet_Struct));
	/* completed reply to mcp */
	if(!NGATCil_UDP_Raw_Send_To(CIL_UDP_Socket_Fd,NGATCIL_CIL_MCP_HOSTNAME_DEFAULT,NGATCIL_CIL_MCP_PORT_DEFAULT,
				    (void*)&com_reply_packet,sizeof(struct NGATCil_Status_Reply_Packet_Struct)))
	{
		Autoguider_General_Error_Number = 1157;
		sprintf(Autoguider_General_Error_String,"CIL_Command_MCP_Safe_Process:"
			"NGATCil_UDP_Raw_Send_To failed.");
		return FALSE;
	}
	return TRUE;
}

/**
 * Process a TCS command received from the server.
 * @param cil_packet The parsed generic CIL header.
 * @param message_buff A pointer to the memory holding the CIL message.
 * @param message_length The length of message_buff in bytes.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #CIL_UDP_Autoguider_On_Pixel_Reply_Send
 * @see #CIL_UDP_Autoguider_On_Brightest_Reply_Send
 * @see #CIL_UDP_Autoguider_On_Rank_Reply_Send
 * @see #CIL_UDP_Autoguider_Off_Reply_Send
 * @see #CIL_Command_Start_Session_Reply_Send
 * @see #CIL_Command_End_Session_Reply_Send
 * @see autoguider_command.html#Autoguider_Command_Autoguide_On
 * @see autoguider_command.html#COMMAND_AG_ON_TYPE
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see autoguider_guide.html#Autoguider_Guide_On
 * @see autoguider_guide.html#Autoguider_Guide_Off
 */
static int CIL_Command_TCS_Process(struct NGATCil_Cil_Packet_Struct cil_packet,void *message_buff,
				   size_t message_length)
{
	struct NGATCil_Ags_Packet_Struct ags_cil_packet;
	float pixel_x = 0.0f,pixel_y = 0.0f;
	int sequence_number = 0,status = 0,retval,rank;
	int *debug_message_buff_ptr = NULL;

	if((cil_packet.Class != E_CIL_CMD_CLASS))	       
	{
		Autoguider_General_Error_Number = 1152;
		sprintf(Autoguider_General_Error_String,"CIL_Command_TCS_Process:Class %d not command.",
			cil_packet.Class);
		return FALSE;
	}
	if(message_length != NGATCIL_CIL_AGS_PACKET_LENGTH)
	{
		Autoguider_General_Error_Number = 1153;
		sprintf(Autoguider_General_Error_String,"CIL_Command_TCS_Process:"
			"received CIL AGS command packet of wrong length %d vs %d.",message_length,
			NGATCIL_CIL_AGS_PACKET_LENGTH);
		return FALSE;
	}
	/* diddly won't work if NGATCIL_CIL_AGS_PACKET_LENGTH != sizeof(struct NGATCil_Ags_Packet_Struct) */
	memcpy(&ags_cil_packet,message_buff,NGATCIL_CIL_AGS_PACKET_LENGTH);
	switch(ags_cil_packet.Command)
	{
		case E_AGS_GUIDE_ON_BRIGHTEST:
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				  "CIL_Command_TCS_Process:Command is autoguider on brightest.");
#endif
			if(!NGATCil_Cil_Autoguide_On_Brightest_Parse(ags_cil_packet,&sequence_number))
			{
				Autoguider_General_Error_Number = 1126;
				sprintf(Autoguider_General_Error_String,"CIL_Command_TCS_Process:"
					"Failed to parse autoguider on brightest command.");
				CIL_UDP_Autoguider_On_Brightest_Reply_Send(E_AGS_BAD_CMD,sequence_number);
				return FALSE;
			}
			/* field, select object nearest pixel, start guide thread */
			retval = Autoguider_Command_Autoguide_On(COMMAND_AG_ON_TYPE_BRIGHTEST,0.0f,0.0f,0);
			if(retval == TRUE)
			{
				if(!CIL_UDP_Autoguider_On_Brightest_Reply_Send(SYS_NOMINAL,sequence_number))
				{
					Autoguider_General_Error_Number = 1127;
					sprintf(Autoguider_General_Error_String,
						"CIL_Command_TCS_Process:"
						"Failed to send autoguider on brightest reply.");
					return FALSE;
				}
			}
			else
			{
				CIL_UDP_Autoguider_On_Brightest_Reply_Send(E_AGS_LOOP_ERROR,sequence_number);
				return FALSE;
			}
			break;
		case E_AGS_GUIDE_ON_PIXEL:
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				    "CIL_Command_TCS_Process:Command is autoguider on pixel.");
#endif
			if(!NGATCil_Cil_Autoguide_On_Pixel_Parse(ags_cil_packet,&pixel_x,&pixel_y,&sequence_number))
			{
				Autoguider_General_Error_Number = 1106;
				sprintf(Autoguider_General_Error_String,"CIL_Command_TCS_Process:"
					"Failed to parse autoguider on pixel command.");
				CIL_UDP_Autoguider_On_Pixel_Reply_Send(pixel_x,pixel_y,E_AGS_BAD_CMD,sequence_number);
				return FALSE;
			}
			/* field, select object nearest pixel, start guide thread */
			retval = Autoguider_Command_Autoguide_On(COMMAND_AG_ON_TYPE_PIXEL,pixel_x,pixel_y,0);
			if(retval == TRUE)
			{
				if(!CIL_UDP_Autoguider_On_Pixel_Reply_Send(pixel_x,pixel_y,SYS_NOMINAL,sequence_number))
				{
					Autoguider_General_Error_Number = 1107;
					sprintf(Autoguider_General_Error_String,
						"CIL_Command_TCS_Process:Failed to send autoguider on pixel reply.");
					return FALSE;
				}
			}
			else
			{
				CIL_UDP_Autoguider_On_Pixel_Reply_Send(pixel_x,pixel_y,E_AGS_LOOP_ERROR,
								       sequence_number);
				return FALSE;
			}
			break;
		case E_AGS_GUIDE_ON_RANK:
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				    "CIL_Command_TCS_Process:Command is autoguider on rank.");
#endif
			if(!NGATCil_Cil_Autoguide_On_Rank_Parse(ags_cil_packet,&rank,&sequence_number))
			{
				Autoguider_General_Error_Number = 1131;
				sprintf(Autoguider_General_Error_String,"CIL_Command_TCS_Process:"
					"Failed to parse autoguider on rank command.");
				CIL_UDP_Autoguider_On_Rank_Reply_Send(rank,E_AGS_BAD_CMD,sequence_number);
				return FALSE;
			}
			/* field, select object nearest pixel, start guide thread */
			retval = Autoguider_Command_Autoguide_On(COMMAND_AG_ON_TYPE_RANK,0.0f,0.0f,rank);
			if(retval == TRUE)
			{
				if(!CIL_UDP_Autoguider_On_Rank_Reply_Send(rank,SYS_NOMINAL,sequence_number))
				{
					Autoguider_General_Error_Number = 1132;
					sprintf(Autoguider_General_Error_String,
						"CIL_Command_TCS_Process:Failed to send autoguider on rank reply.");
					return FALSE;
				}
			}
			else
			{
				CIL_UDP_Autoguider_On_Rank_Reply_Send(rank,E_AGS_LOOP_ERROR,sequence_number);
				return FALSE;
			}
			break;
		case E_AGS_GUIDE_OFF:
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
					"CIL_Command_TCS_Process:Command is autoguider off.");
#endif
			if(!NGATCil_Cil_Autoguide_Off_Parse(ags_cil_packet,&status,&sequence_number))
			{
				Autoguider_General_Error_Number = 1108;
				sprintf(Autoguider_General_Error_String,"CIL_Command_TCS_Process:"
						"Failed to parse autoguider off command.");
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
						"CIL_Command_TCS_Process:Failed to send autoguider off reply.");
					return FALSE;
				}
			}
			else
			{
				CIL_UDP_Autoguider_Off_Reply_Send(E_AGS_LOOP_STOPPING,sequence_number);
			}
			break;
		case E_AGS_START_SESSION:
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
					"CIL_Command_TCS_Process:Command is start session.");
#endif
			/* write INIT to SDB. */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_INIT))
				Autoguider_General_Error(); /* no need to fail */
			if(!Autoguider_CIL_SDB_Packet_Send())
				Autoguider_General_Error(); /* no need to fail */
			/* write IDLE (ready) to SDB. */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
				Autoguider_General_Error(); /* no need to fail */
			if(!Autoguider_CIL_SDB_Packet_Send())
				Autoguider_General_Error(); /* no need to fail */
			/* send reply OK */
			if(!CIL_Command_Start_Session_Reply_Send(ags_cil_packet,SYS_NOMINAL))
				Autoguider_General_Error();
			break;
		case E_AGS_END_SESSION:
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
					"CIL_Command_TCS_Process:Command is end session.");
#endif
			/* send reply OK */
			if(!CIL_Command_End_Session_Reply_Send(ags_cil_packet,SYS_NOMINAL))
				Autoguider_General_Error();
			break;

		default:
			Autoguider_General_Error_Number = 1110;
			sprintf(Autoguider_General_Error_String,"CIL_Command_TCS_Process:"
				"received CIL packet from TCS with unknown command %d.",ags_cil_packet.Command);
			return FALSE;
			break;
	}/* end switch on command */
	return TRUE;
}

/**
 * Send a reply to an "autoguider on brightest" command sent to the AGS CIL port,
 * by sending a CIL UDP reply packet to the TCS UDP CIL command port.
 * @param status Whether the autoguider has started autoguiding, either SYS_NOMINAL or an error code.
 * @param sequence_number The command sequence number - should be the same as parsed from the command request.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #TCC_Hostname
 * @see #CIL_TCS_UDP_Port
 * @see #CIL_UDP_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Open
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Close
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_On_Brightest_Reply_Send
 */
static int CIL_UDP_Autoguider_On_Brightest_Reply_Send(int status,int sequence_number)
{
	int retval;

#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_On_Brightest_Reply_Send:started.");
#endif
	retval = NGATCil_Cil_Autoguide_On_Brightest_Reply_Send(CIL_UDP_Socket_Fd,TCC_Hostname,CIL_TCS_UDP_Port,status,
							       sequence_number);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1129;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_On_Pixel_Reply_Send:"
			"Failed to send autoguide on pixel reply packet.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_On_Brightest_Reply_Send:finished.");
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
 * @see #CIL_UDP_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Open
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Close
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_On_Pixel_Reply_Send
 */
static int CIL_UDP_Autoguider_On_Pixel_Reply_Send(float pixel_x,float pixel_y,int status,int sequence_number)
{
	int retval;

#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_On_Pixel_Reply_Send:started.");
#endif
	retval = NGATCil_Cil_Autoguide_On_Pixel_Reply_Send(CIL_UDP_Socket_Fd,TCC_Hostname,CIL_TCS_UDP_Port,
							   pixel_x,pixel_y,status,sequence_number);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1115;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_On_Pixel_Reply_Send:"
			"Failed to send autoguide on pixel reply packet.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_On_Pixel_Reply_Send:finished.");
#endif
	return TRUE;
}

/**
 * Send a reply to an "autoguider on rank" command sent to the AGS CIL port,
 * by sending a CIL UDP reply packet to the TCS UDP CIL command port.
 * @param rank The rank of star to guide on.
 * @param status Whether the autoguider has started autoguiding, either SYS_NOMINAL or an error code.
 * @param sequence_number The command sequence number - should be the same as parsed from the command request.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #TCC_Hostname
 * @see #CIL_TCS_UDP_Port
 * @see #CIL_UDP_Socket_Fd
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Open
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_UDP_Close
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_On_Rank_Reply_Send
 */
static int CIL_UDP_Autoguider_On_Rank_Reply_Send(int rank,int status,int sequence_number)
{
	int retval;

#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_On_Rank_Reply_Send:started.");
#endif
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "CIL_UDP_Autoguider_On_Rank_Reply_Send:Opening UDP connection to %s:%d.",
				      TCC_Hostname,CIL_TCS_UDP_Port);
#endif
	retval = NGATCil_Cil_Autoguide_On_Rank_Reply_Send(CIL_UDP_Socket_Fd,TCC_Hostname,CIL_TCS_UDP_Port,rank,status,
							  sequence_number);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1134;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_On_Rank_Reply_Send:"
			"Failed to send autoguide on rank reply packet.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_On_Rank_Reply_Send:finished.");
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
 * @see #CIL_UDP_Socket_Fd
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
	int retval;

#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_Off_Reply_Send:started.");
#endif
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_CIL,
				      "CIL_UDP_Autoguider_Off_Reply_Send:Opening UDP connection to %s:%d.",
				      TCC_Hostname,CIL_TCS_UDP_Port);
#endif
	retval = NGATCil_Cil_Autoguide_Off_Reply_Send(CIL_UDP_Socket_Fd,TCC_Hostname,CIL_TCS_UDP_Port,status,
						      sequence_number);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1118;
		sprintf(Autoguider_General_Error_String,"CIL_UDP_Autoguider_Off_Reply_Send:"
			"Failed to send autoguide off reply packet.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_CIL,"CIL_UDP_Autoguider_Off_Reply_Send:finished.");
#endif
	return TRUE;
}

/**
 * Process a Start Session command received from the TCS.
 * @param cil_packet The parsed AGS to TCS CIL command.
 * @param status What status to return to the TCS, should be SYS_NOMINAL.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #CIL_UDP_Socket_Fd
 * @see #TCC_Hostname
 * @see #CIL_TCS_UDP_Port
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_TCS_HOSTNAME_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_TCS_PORT_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Ags_Packet_Struct
 */
static int CIL_Command_Start_Session_Reply_Send(struct NGATCil_Ags_Packet_Struct cil_packet,int status)
{
	struct NGATCil_Ags_Packet_Struct tcs_reply_packet;
	struct timespec current_time;
	int retval;

	tcs_reply_packet.Cil_Base.Source_Id = E_CIL_AGS;
	tcs_reply_packet.Cil_Base.Dest_Id = E_CIL_TCS;
	tcs_reply_packet.Cil_Base.Class = E_CIL_RSP_CLASS;
	tcs_reply_packet.Cil_Base.Service = cil_packet.Cil_Base.Service;
	tcs_reply_packet.Cil_Base.Seq_Num = cil_packet.Cil_Base.Seq_Num;
	clock_gettime(CLOCK_REALTIME,&current_time);
	tcs_reply_packet.Cil_Base.Timestamp_Seconds = current_time.tv_sec;
	tcs_reply_packet.Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
	tcs_reply_packet.Command = cil_packet.Command;
	tcs_reply_packet.Status = status;
	tcs_reply_packet.Param1 = cil_packet.Param1;
	tcs_reply_packet.Param2 = cil_packet.Param2;
	NGATCil_UDP_Raw_To_Network_Byte_Order((void*)&tcs_reply_packet,sizeof(struct NGATCil_Ags_Packet_Struct));
	if(!NGATCil_UDP_Raw_Send_To(CIL_UDP_Socket_Fd,TCC_Hostname,CIL_TCS_UDP_Port,
				    (void*)&tcs_reply_packet,sizeof(struct NGATCil_Ags_Packet_Struct)))
	{
		Autoguider_General_Error_Number = 1117;
		sprintf(Autoguider_General_Error_String,
			"CIL_Command_Start_Session_Reply_Send:NGATCil_UDP_Raw_Send_To failed.");
		return FALSE;
	}
	return TRUE;
}

/**
 * Process a End Session command received from the TCS.
 * @param cil_packet The parsed generic CIL header.
 * @param status What status to return to the TCS, should be SYS_NOMINAL.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs.
 * @see #CIL_UDP_Socket_Fd
 * @see #TCC_Hostname
 * @see #CIL_TCS_UDP_Port
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_CIL
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_TCS_HOSTNAME_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCIL_CIL_TCS_PORT_DEFAULT
 * @see ../ngatcil/cdocs/ngatcil_cil.html#NGATCil_Ags_Packet_Struct
 */
static int CIL_Command_End_Session_Reply_Send(struct NGATCil_Ags_Packet_Struct cil_packet,int status)
{
	struct NGATCil_Ags_Packet_Struct tcs_reply_packet;
	struct timespec current_time;
	int retval;

	tcs_reply_packet.Cil_Base.Source_Id = E_CIL_AGS;
	tcs_reply_packet.Cil_Base.Dest_Id = E_CIL_TCS;
	tcs_reply_packet.Cil_Base.Class = E_CIL_RSP_CLASS;
	tcs_reply_packet.Cil_Base.Service = cil_packet.Cil_Base.Service;
	tcs_reply_packet.Cil_Base.Seq_Num = cil_packet.Cil_Base.Seq_Num;
	clock_gettime(CLOCK_REALTIME,&current_time);
	tcs_reply_packet.Cil_Base.Timestamp_Seconds = current_time.tv_sec;
	tcs_reply_packet.Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
	tcs_reply_packet.Command = cil_packet.Command;
	tcs_reply_packet.Status = status;
	tcs_reply_packet.Param1 = cil_packet.Param1;
	tcs_reply_packet.Param2 = cil_packet.Param2;
	NGATCil_UDP_Raw_To_Network_Byte_Order((void*)&tcs_reply_packet,
					      sizeof(struct NGATCil_Ags_Packet_Struct));
	if(!NGATCil_UDP_Raw_Send_To(CIL_UDP_Socket_Fd,TCC_Hostname,CIL_TCS_UDP_Port,
				    (void*)&tcs_reply_packet,sizeof(struct NGATCil_Ags_Packet_Struct)))
	{
		Autoguider_General_Error_Number = 1119;
		sprintf(Autoguider_General_Error_String,
			"CIL_Command_End_Session_Reply_Send:NGATCil_UDP_Raw_Send_To failed.");
		return FALSE;
	}
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.10  2006/12/19 17:50:20  cjm
** Init state flicker code.
**
** Revision 1.9  2006/08/29 13:55:42  cjm
** Swapped socket FD used to send CIL UDP packets to server socket FD - as receiving end checks source port number.
** Added Session start/end handling.
** Added MCP command processing.
** Added CHB processing.
** Added SDB handling.
**
** Revision 1.8  2006/07/20 15:11:48  cjm
** Added some CIL SDB submission software.
**
** Revision 1.7  2006/07/14 14:01:48  cjm
** Fixed duplicate error code.
**
** Revision 1.6  2006/06/29 17:04:34  cjm
** Added CIL handling of rank/brightest.
**
** Revision 1.5  2006/06/27 20:43:21  cjm
** Added AUTOGUIDER_CIL_SILENCE.
** Added Autoguider_Command_Autoguide_On call.
**
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
