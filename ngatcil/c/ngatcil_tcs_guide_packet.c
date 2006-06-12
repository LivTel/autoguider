/* ngatcil_tcs_guide_packet.c
** NGATCil TCS guide packet tranmitting/receiving routines.
** $Header: /home/cjm/cvs/autoguider/ngatcil/c/ngatcil_tcs_guide_packet.c,v 1.3 2006-06-12 19:25:53 cjm Exp $
*/
/**
 * NGAT Cil library transmission/receiving of TCS guide packets over UDP.
 * @author Chris Mottram
 * @version $Revision: 1.3 $
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

/* hash defines */

/* data types */

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ngatcil_tcs_guide_packet.c,v 1.3 2006-06-12 19:25:53 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Try to setup a UDP socket talking to the default TCC machine and guide packet port.
 * @param socket_id The address of an integer to store the created socket file descriptor.
 *       This parameter is not checked for nullness, it is assumed the underlying NGATCil_UDP_Open
 *       routine checks this.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_udp_raw.html#NGATCil_UDP_Open
 * @see #NGATCIL_TCS_GUIDE_PACKET_TCC_DEFAULT
 * @see #NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET
 */
int NGATCil_TCS_Guide_Packet_Open_Default(int *socket_id)
{
	int retval;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Open_Default:started.");
#endif
	retval = NGATCil_UDP_Open(NGATCIL_TCS_GUIDE_PACKET_TCC_DEFAULT,NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT,
				  socket_id);
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,
			    "NGATCil_TCS_Guide_Packet_Open_Default:finished.");
#endif
	return retval;
}

/**
 * Send a TCS guide packet over the specified socket as a UDP packet.
 * The packet contents are derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 * @param socket_id The socket descriptor to send the packet over.
 * @param x_pos The X position of the AG centroid, in pixels from the <b>edge of the CCD</b>, 
 *        <b>NOT</b> the guide window.
 * @param y_pos The Y position of the AG centroid, in pixels from the <b>edge of the CCD</b>, 
 *        <b>NOT</b> the guide window.
 * @param timecode_terminating Boolean. If TRUE the timecode in the sent guide packet will contain the
 *        terminating timecode.
 * @param timecode_unreliable Boolean. If TRUE the timecode in the sent guide packet will be negative,
 *        which tells the TCS the centroid is unreliable.
 * @param timecode_secs The number of seconds the TCS should wait for until the next guide packet will be sent
 *        (the TCS actually waits for twice this time). This is a float in the range 0..9999 seconds.
 *        If timecode_unreliable is TRUE this value will be sent negative.
 *        If timecode_terminating is TRUE the value 0.0 will be sent instead.
 * @param status_char The status byte, should be a character in the range '0'..'7' ('0' == most confident) or 
 *       NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED or NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED
 * @see #NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW
 * @see #NGATCil_TCS_Guide_Packet_To_String
 * @see ngatcil_udp_raw.html#NGATCil_UDP_Raw_Send
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET
 */
int NGATCil_TCS_Guide_Packet_Send(int socket_id,float x_pos,float y_pos,
					 int timecode_terminating,int timecode_unreliable,
					 float timecode_secs,char status_char)
{
	char packet_buff[35];
	char x_pos_buff[9];
	char y_pos_buff[9];
	char timecode_buff[9];
	char checksum_buff[5];
	int retval,i,checksum;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:started.");
#endif
	/* check parameters */
	if(!NGATCIL_GENERAL_IS_BOOLEAN(timecode_terminating))
	{
		NGATCil_General_Error_Number = 200;
		sprintf(NGATCil_General_Error_String,
			"NGATCil_TCS_Guide_Packet_Send:Illegal value for timecode terminating (%d).",
			timecode_terminating);
		return FALSE;
	}
	if(!NGATCIL_GENERAL_IS_BOOLEAN(timecode_unreliable))
	{
		NGATCil_General_Error_Number = 201;
		sprintf(NGATCil_General_Error_String,
			"NGATCil_TCS_Guide_Packet_Send:Illegal value for timecode unreliable (%d).",
			timecode_unreliable);
		return FALSE;
	}
	if((x_pos < -9999.99f) || (x_pos > 9999.99f))
	{
		NGATCil_General_Error_Number = 202;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Send:x_pos out of range (%.2f).",x_pos);
		return FALSE;
	}
	if((y_pos < -9999.99f) || (y_pos > 9999.99f))
	{
		NGATCil_General_Error_Number = 203;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Send:y_pos out of range (%.2f).",y_pos);
		return FALSE;
	}
	if((timecode_secs < 0.01f) || (timecode_secs > 9999.99f))
	{
		NGATCil_General_Error_Number = 204;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Send:"
			"timecode_secs out of range (%.2f).",timecode_secs);
		return FALSE;
	}
	if(((status_char < '0') || (status_char > '7')) && (status_char != NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED) && 
	   (status_char != NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW))
	{
		NGATCil_General_Error_Number = 205;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Send:"
			"Illegal status char %c.",status_char);
		return FALSE;
	}
	/* setup buffers */
	strcpy(packet_buff,"");
	/* x pos */
	sprintf(x_pos_buff+1,"%07.2f",fabs(x_pos));
	if(x_pos >= 0.0f)
		x_pos_buff[0] = '0';
	else
		x_pos_buff[0] = '-';
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:"
				   "x_pos_buff = %s.",x_pos_buff);
#endif
	/* y pos */
	sprintf(y_pos_buff+1,"%07.2f",fabs(y_pos));
	if(y_pos >= 0.0f)
		y_pos_buff[0] = '0';
	else
		y_pos_buff[0] = '-';
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:"
				   "y_pos_buff = %s.",y_pos_buff);
#endif
	/* timecode */
	if(timecode_terminating)
	{
		strcpy(timecode_buff,"00000.00");
	}
	else
	{
			sprintf(timecode_buff+1,"%07.2f",timecode_secs);
			if(timecode_unreliable)
				timecode_buff[0] = '-';
			else
				timecode_buff[0] = '0';
	}
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:"
				   "timecode_buff = %s.",timecode_buff);
#endif
	/* packet (up to checksum) */
	retval = sprintf(packet_buff,"%s %s %s %c ",x_pos_buff,y_pos_buff,timecode_buff,status_char);
	if(retval != 29)
	{
		NGATCil_General_Error_Number = 206;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Send:"
			"Malformed packet_buff (%s,%d).",packet_buff,retval);
		return FALSE;
	}
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:"
				   "packet_buff (without checksum) = '%s'.",packet_buff);
#endif
	/* compute checksum */
	checksum = 0;
	for(i=0;i<29;i++) /* 29 (0..28) bytes up to checksum (8+1+8+1+8+1+1+1) */
	{
		checksum += (int)(packet_buff[i]);
	}
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:"
				   "checksum = %d.",checksum);
#endif
	sprintf(checksum_buff,"%04d",checksum);
#if NGATCIL_DEBUG > 9
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:"
				   "checksum_buff = '%s'.",checksum_buff);
#endif
	strcat(packet_buff,checksum_buff);
	strcat(packet_buff,"\r");
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:"
				   "packet_buff (with checksum) = '%s' (length %d).",
				   NGATCil_TCS_Guide_Packet_To_String(packet_buff,strlen(packet_buff)),
				   strlen(packet_buff));
#endif
	/* send packet  -  this is 29 bytes, plus 4 (+1 (cr)) bytes checksum (no \0) = 34 bytes. */
	retval = NGATCil_UDP_Raw_Send(socket_id,packet_buff,34);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Send:finished.");
#endif
	return TRUE;
}

/**
 * Get a TCS guide packet from the socket and parse it's contents. The parsing is done by calling 
 * NGATCil_TCS_Guide_Packet_Parse.
 * @param socket_id The socket to get the guide packet from.
 * @param x_pos The address of a float to store the X centroid position from the guide packet.
 * @param y_pos The address of a float to store the Y centroid position from the guide packet.
 * @param timecode_terminating The address of an integer to store a boolean. If the timecode string in the
 *        guide packet is 0.0f, this boolean is TRUE, which means the autoguider has sent a guide termination packet.
 * @param timecode_unreliable The address of an integer to store a boolean. If the timecode string in the
 *        guide packet is less than 0.0, this boolean is TRUE, which means the autoguider thinks the centroid is
 *        unreliable. The absolute value of the timecode string is then stored in the timecode_secs paramater.
 * @param timecode_secs The address of a float to store the number of seconds until the autoguider expects to send
 *        the next guide packet.
 * @param status_char The address of a character to store the reliability status of the centroid. Can be a character
 *        from 0-7 denoting the confidence of the packet (0 is very confident), or 
 *        NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED denoting no guide star was found or 
 *        NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW denoting the guide star is near the edge of the guide window.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED
 * @see #NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW
 * @see #NGATCil_TCS_Guide_Packet_Parse
 * @see #NGATCil_TCS_Guide_Packet_To_String
 * @see ngatcil_udp_raw.html#NGATCil_UDP_Raw_Recv
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET
 */
int NGATCil_TCS_Guide_Packet_Recv(int socket_id,float *x_pos,float *y_pos,
					 int *timecode_terminating,int *timecode_unreliable,
					 float *timecode_secs,char *status_char)
{
	char packet_buff[35];
	int retval,computed_checksum,checksum;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Recv:started.");
#endif
	/* check parameters */
	if(x_pos == NULL)
	{
		NGATCil_General_Error_Number = 207;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Recv:x_pos was NULL.");
		return FALSE;
	}
	if(y_pos == NULL)
	{
		NGATCil_General_Error_Number = 208;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Recv:y_pos was NULL.");
		return FALSE;
	}
	if(timecode_terminating == NULL)
	{
		NGATCil_General_Error_Number = 209;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Recv:timecode_terminating was NULL.");
		return FALSE;
	}
	if(timecode_unreliable == NULL)
	{
		NGATCil_General_Error_Number = 210;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Recv:timecode_unreliable was NULL.");
		return FALSE;
	}
	if(timecode_secs == NULL)
	{
		NGATCil_General_Error_Number = 211;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Recv:timecode_secs was NULL.");
		return FALSE;
	}
	if(status_char == NULL)
	{
		NGATCil_General_Error_Number = 212;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Recv:status_char was NULL.");
		return FALSE;
	}
	/* get packet */
	retval = NGATCil_UDP_Raw_Recv(socket_id,packet_buff,34);
	if(retval == FALSE)
		return FALSE;
	/* terminate packet_buff (as a string) properly */
	packet_buff[34] = '\0';
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,
				   "NGATCil_TCS_Guide_Packet_Recv:received '%s'.",
				   NGATCil_TCS_Guide_Packet_To_String(packet_buff,strlen(packet_buff)));
#endif
	/* parse packet */
	retval = NGATCil_TCS_Guide_Packet_Parse(packet_buff,35,x_pos,y_pos,timecode_terminating,
						timecode_unreliable,timecode_secs,status_char);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Recv:finished.");
#endif
	return TRUE;
}

/**
 * Parse a TCS guide packet .
 * The packet parsing is derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 * @param packet_buff  A pointer to memory containing the read packet to parse.
 * @param packet_buff_length An integer containing the length of the packet. This must be at least 35 bytes,
 *      A TCS guide packet should be 34 bytes long, the extra byte is to add a NULL termintor (not transmitted
 *      over the socket).
 * @param x_pos The address of a float to store the X centroid position from the guide packet.
 * @param y_pos The address of a float to store the Y centroid position from the guide packet.
 * @param timecode_terminating The address of an integer to store a boolean. If the timecode string in the
 *        guide packet is 0.0f, this boolean is TRUE, which means the autoguider has sent a guide termination packet.
 * @param timecode_unreliable The address of an integer to store a boolean. If the timecode string in the
 *        guide packet is less than 0.0, this boolean is TRUE, which means the autoguider thinks the centroid is
 *        unreliable. The absolute value of the timecode string is then stored in the timecode_secs paramater.
 * @param timecode_secs The address of a float to store the number of seconds until the autoguider expects to send
 *        the next guide packet.
 * @param status_char The address of a character to store the reliability status of the centroid. Can be a character
 *        from 0-7 denoting the confidence of the packet (0 is very confident), or 
 *        NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED denoting no guide star was found or 
 *        NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW denoting the guide star is near the edge of the guide window.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED
 * @see #NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW
 * @see #NGATCil_TCS_Guide_Packet_To_String
 * @see ngatcil_udp_raw.html#NGATCil_UDP_Raw_Recv
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET
 */
int NGATCil_TCS_Guide_Packet_Parse(void *packet_buff,int packet_buff_length,float *x_pos,float *y_pos,
				   int *timecode_terminating,int *timecode_unreliable,
				   float *timecode_secs,char *status_char)
{
	char *packet_buff_string = NULL;
	int retval,computed_checksum,checksum,i;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Parse:started.");
#endif
	/* check parameters */
	if(packet_buff == NULL)
	{
		NGATCil_General_Error_Number = 215;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:packet_buff was NULL.");
		return FALSE;
	}
	if(x_pos == NULL)
	{
		NGATCil_General_Error_Number = 216;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:x_pos was NULL.");
		return FALSE;
	}
	if(y_pos == NULL)
	{
		NGATCil_General_Error_Number = 217;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:y_pos was NULL.");
		return FALSE;
	}
	if(timecode_terminating == NULL)
	{
		NGATCil_General_Error_Number = 218;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:timecode_terminating was NULL.");
		return FALSE;
	}
	if(timecode_unreliable == NULL)
	{
		NGATCil_General_Error_Number = 219;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:timecode_unreliable was NULL.");
		return FALSE;
	}
	if(timecode_secs == NULL)
	{
		NGATCil_General_Error_Number = 220;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:timecode_secs was NULL.");
		return FALSE;
	}
	if(status_char == NULL)
	{
		NGATCil_General_Error_Number = 221;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:status_char was NULL.");
		return FALSE;
	}
	if(packet_buff_length < 35)
	{
		NGATCil_General_Error_Number = 222;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:"
			"packet_buff_length was too small(%d).",packet_buff_length);
		return FALSE;
	}
	/* terminate packet_buff (as a string) properly */
	packet_buff_string = (char *)packet_buff;
	packet_buff_string[34] = '\0';
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,
				   "NGATCil_TCS_Guide_Packet_Parse:received '%s'.",
				   NGATCil_TCS_Guide_Packet_To_String(packet_buff_string,strlen(packet_buff_string)));
#endif
	retval = sscanf(packet_buff_string,"%f %f %f %c %d\r",x_pos,y_pos,timecode_secs,status_char,&checksum);
	if(retval != 5)
	{
		NGATCil_General_Error_Number = 213;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:"
			"Failed to parse packet_buff_string '%s'(%d).",
			NGATCil_TCS_Guide_Packet_To_String(packet_buff_string,strlen(packet_buff_string)),retval);
		return FALSE;
	}
	/* sort out timecode flags */
	if((*timecode_secs) > 0.0f)
	{
		(*timecode_terminating) = FALSE;
		(*timecode_unreliable) = FALSE;
	}
	else if((*timecode_secs) < 0.0f)
	{
		(*timecode_terminating) = FALSE;
		(*timecode_unreliable) = TRUE;
		(*timecode_secs) = fabs((*timecode_secs));
	}
	else
	{
		(*timecode_terminating) = TRUE;
		(*timecode_unreliable) = FALSE;
	}
	/* check the checksum */
	computed_checksum = 0;
	for(i=0;i<29;i++) /* 29 (0..28) bytes up to checksum (8+1+8+1+8+1+1+1) */
	{
		computed_checksum += (int)(packet_buff_string[i]);
	}
	if(computed_checksum != checksum)
	{
		NGATCil_General_Error_Number = 214;
		sprintf(NGATCil_General_Error_String,"NGATCil_TCS_Guide_Packet_Parse:"
			"Checksum mismatch (%d vs %d) on packet '%s'.",computed_checksum,checksum,
			NGATCil_TCS_Guide_Packet_To_String(packet_buff_string,strlen(packet_buff_string)));
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET,"NGATCil_TCS_Guide_Packet_Parse:finished.");
#endif
	return TRUE;
}

/**
 * Strip out <CR> from guide packets to make them print better. Expects the guide packet to have previously been
 * NULL terminated.
 */
char *NGATCil_TCS_Guide_Packet_To_String(void *packet_buff,int packet_buff_length)
{
	char *packet_buff_string = NULL;
	static char print_packet_buff[40];
	int i;

	packet_buff_string = (char *)packet_buff;
	if(strlen(packet_buff_string) > 35)
	{
		sprintf(print_packet_buff,"Buffer wrong length:%d,%d.",strlen(packet_buff_string),packet_buff_length);
		return print_packet_buff;
	}
	strcpy(print_packet_buff,packet_buff_string);
	for(i=0;i<packet_buff_length;i++)
	{
		if(print_packet_buff[i] == '\r')
		{
			print_packet_buff[i] = ' ';
		}
	}
	/*strcat(print_packet_buff,"<cr>");*/
	return print_packet_buff;
}
/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2006/06/05 18:55:08  cjm
** Fixed checksum.
** Added NGATCil_TCS_Guide_Packet_To_String.
**
** Revision 1.1  2006/06/01 15:28:06  cjm
** Initial revision
**
*/
