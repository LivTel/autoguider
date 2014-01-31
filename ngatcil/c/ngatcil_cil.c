/* ngatcil_cil.c
** NGATCil General CIL packet tranmitting/receiving routines.
** $Header: /home/cjm/cvs/autoguider/ngatcil/c/ngatcil_cil.c,v 1.8 2014-01-31 17:30:25 cjm Exp $
*/
/**
 * NGAT Cil library transmission/receiving of CIL packets over UDP.
 * @author Chris Mottram
 * @version $Revision: 1.8 $
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
#include <time.h>
#include "log_udp.h"
#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"
#include "ngatcil_cil.h"
/* hash defines */

/* enums */

/* data types */

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ngatcil_cil.c,v 1.8 2014-01-31 17:30:25 cjm Exp $";
/**
 * CIL packet sequence number.
 */
static int Sequence_Number = 0;

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Create a CIL packet.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Packet_Create(int source_id,int dest_id,int class,int service,int seq_num,int command,
			      int status,int param1, int param2,struct NGATCil_Ags_Packet_Struct *packet)
{
	struct timespec current_time;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Packet_Create",LOG_VERBOSITY_VERBOSE,NULL,
			    "started.");
#endif
	if(packet == NULL)
	{
		NGATCil_General_Error_Number = 300;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Packet_Create:packet was NULL.");
		return FALSE;
	}
	packet->Cil_Base.Source_Id = htonl(source_id);
	packet->Cil_Base.Dest_Id = htonl(dest_id);
	packet->Cil_Base.Class = htonl(class);
	packet->Cil_Base.Service = htonl(service);
	packet->Cil_Base.Seq_Num = htonl(seq_num);
	clock_gettime(CLOCK_REALTIME,&current_time);
	packet->Cil_Base.Timestamp_Seconds = current_time.tv_sec-TTL_TIMESTAMP_OFFSET;
	packet->Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
	packet->Cil_Base.Timestamp_Seconds     = htonl(packet->Cil_Base.Timestamp_Seconds);
	packet->Cil_Base.Timestamp_Nanoseconds = htonl(packet->Cil_Base.Timestamp_Nanoseconds);
	packet->Command = htonl(command);
	packet->Status = htonl(status);
	packet->Param1 = htonl(param1);
	packet->Param2 = htonl(param2);
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Packet_Create",LOG_VERBOSITY_VERBOSE,NULL,
			    "finished.");
#endif
	return TRUE;
}

/**
 * Send a CIL packet. 
 * @param socket_id The socket file descriptor to use.
 * @param hostname The hostname of the host to send the packet to.
 * @param port_number The port number of the host to send the packet to.
 * @param packet The packet to send. The contents should have been put into network byte order
 *        (NGATCil_Cil_Packet_Create does this automatically).
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_udp_raw.html#NGATCil_UDP_Raw_Send_To
 */
int NGATCil_Cil_Packet_Send_To(int socket_id,char *hostname,int port_number,struct NGATCil_Ags_Packet_Struct packet)
{
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Packet_Send_To",LOG_VERBOSITY_VERBOSE,NULL,
			    "started.");
#endif
	retval = NGATCil_UDP_Raw_Send_To(socket_id,hostname,port_number,(void*)&packet,
				      sizeof(struct NGATCil_Ags_Packet_Struct));
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Packet_Send_To",LOG_VERBOSITY_VERBOSE,NULL,
			    "finished.");
#endif
	return retval;
}

/**
 * Receive a CIL packet. 
 * @param socket_id The socket file descriptor to use.
 * @param packet The address to put the received packet. The contents are translated by this routine from
 *        network byte order to host byte order.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_udp_raw.html#NGATCil_UDP_Raw_Recv
 */
int NGATCil_Cil_Packet_Recv(int socket_id,struct NGATCil_Ags_Packet_Struct *packet)
{
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Packet_Recv",LOG_VERBOSITY_VERBOSE,NULL,
			    "started.");
#endif
	if(packet == NULL)
	{
		NGATCil_General_Error_Number = 301;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Packet_Recv:packet was NULL.");
		return FALSE;
	}
	retval = NGATCil_UDP_Raw_Recv(socket_id,packet,sizeof(struct NGATCil_Ags_Packet_Struct));
	if(retval == FALSE)
		return FALSE;
	packet->Cil_Base.Source_Id = ntohl(packet->Cil_Base.Source_Id);
	packet->Cil_Base.Dest_Id = ntohl(packet->Cil_Base.Dest_Id);
	packet->Cil_Base.Class = ntohl(packet->Cil_Base.Class);
	packet->Cil_Base.Service = ntohl(packet->Cil_Base.Service);
	packet->Cil_Base.Seq_Num = ntohl(packet->Cil_Base.Seq_Num);
	packet->Cil_Base.Timestamp_Seconds     = ntohl(packet->Cil_Base.Timestamp_Seconds);
	packet->Cil_Base.Timestamp_Nanoseconds = ntohl(packet->Cil_Base.Timestamp_Nanoseconds);
	packet->Command = ntohl(packet->Command);
	packet->Status = ntohl(packet->Status);
	packet->Param1 = ntohl(packet->Param1);
	packet->Param2 = ntohl(packet->Param2);
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Packet_Recv",LOG_VERBOSITY_VERBOSE,NULL,
			    "finished.");
#endif
	return TRUE;
}

/**
 * Send an "Autoguide on pixel x y" CIL command packet on the specified socket.
 * For use for TCS (simulators).
 * @param socket_id The socket to send the packet over.
 * @param socket_id The socket to send the packet over.
 * @param hostname The hostname of the host to send the packet to.
 * @param pixel_x The X pixel position, 0..1023.
 * @param pixel_y The Y pixel position, 0..1023.
 * @param sequence_number The address of an integer to store the sequence number generated for this packet.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #NGATCil_Cil_Packet_Send_To
 * @see #SYS_NOMINAL
 * @see #E_AGS_GUIDE_ON_PIXEL
 * @see #E_AGS_CMD
 * @see #E_CIL_CMD_CLASS
 * @see #Sequence_Number
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_On_Pixel_Send(int socket_id,char *hostname,int port_number,float pixel_x,float pixel_y,
					int *sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Ags_Packet_Struct packet;
	int pixel_x_i,pixel_y_i,retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Pixel_Send",
				   LOG_VERBOSITY_VERBOSE,NULL,"started(%.2f,%.2f).",pixel_x,pixel_y);
#endif
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 302;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Send:Sequence number was NULL.");
		return FALSE;
	}
	if((pixel_x < 0.0f) || (pixel_x > 1023.0f))
	{
		NGATCil_General_Error_Number = 303;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Send:"
			"Pixel X %.2f out of range (0..1023).",pixel_x);
		return FALSE;
	}
	if((pixel_y < 0.0f) || (pixel_y > 1023.0f))
	{
		NGATCil_General_Error_Number = 304;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Send:"
			"Pixel Y %.2f out of range (0..1023).",pixel_y);
		return FALSE;
	}
	/* rewrite using NGATCil_Cil_Packet_Create to packet is in network byte order
	retval = NGATCil_Cil_Packet_Create(E_CIL_TCS,E_CIL_AGS,E_CIL_CMD_CLASS,E_AGS_CMD,Sequence_Number,E_AGS_GUIDE_ON_PIXEL,
			     SYS_NOMINAL,int param1, int param2,struct NGATCil_Ags_Packet_Struct *packet)
	*/
	packet.Cil_Base.Source_Id = E_CIL_TCS;
	packet.Cil_Base.Dest_Id = E_CIL_AGS;
	packet.Cil_Base.Class = E_CIL_CMD_CLASS;
	packet.Cil_Base.Service = E_AGS_CMD;
	/* increment sequence number - prevent overflow */
	Sequence_Number++;
	if(Sequence_Number > 200000000)
		Sequence_Number = 0;
	packet.Cil_Base.Seq_Num = Sequence_Number;
	(*sequence_number) = Sequence_Number;
	clock_gettime(CLOCK_REALTIME,&current_time);
	packet.Cil_Base.Timestamp_Seconds = current_time.tv_sec;
	packet.Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
	packet.Command = E_AGS_GUIDE_ON_PIXEL;
	packet.Status = SYS_NOMINAL;
	/* param 1 is X pixel position in millipixels */
	pixel_x_i = (int)(pixel_x * 1000.0f);
	packet.Param1 = pixel_x_i;
	/* param 2 is Y pixel position in millipixels */
	pixel_y_i = (int)(pixel_y * 1000.0f);
	packet.Param2 = pixel_y_i;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send_To(socket_id,hostname,port_number,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Pixel_Send",
				   LOG_VERBOSITY_VERBOSE,NULL,"sequence ID %d:finished.",(*sequence_number));
#endif
	return TRUE;
}

/**
 * Parse an "Autoguide on pixel x y" CIL command packet.
 * For use by the AGS.
 * @param packet The received packet to parse. This should have been translated to host byte order using
 *         NGATCil_Cil_Packet_Recv.
 * @param pixel_x The address of a float to store the X pixel position requested.
 * @param pixel_y The address of a float to store the Y pixel position requested.
 * @param sequence_number The address of an integer to store the sequence_number.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #E_CIL_CMD_CLASS
 * @see #E_AGS_CMD
 * @see #E_AGS_GUIDE_ON_PIXEL
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_On_Pixel_Parse(struct NGATCil_Ags_Packet_Struct packet,float *pixel_x,float *pixel_y,
					 int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Pixel_Parse",
			    LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(pixel_x == NULL)
	{
		NGATCil_General_Error_Number = 305;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Parse:pixel_x was NULL.");
		return FALSE;
	}
	if(pixel_y == NULL)
	{
		NGATCil_General_Error_Number = 306;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Parse:pixel_y was NULL.");
		return FALSE;
	}
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 307;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Parse:Sequence number was NULL.");
		return FALSE;
	}
	if(packet.Cil_Base.Class != E_CIL_CMD_CLASS)
	{
		NGATCil_General_Error_Number = 308;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Parse:"
			"Wrong class number (%d vs %d).",packet.Cil_Base.Class,E_CIL_CMD_CLASS);
		return FALSE;
	}
	if(packet.Cil_Base.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 309;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Parse:"
			"Wrong service number (%d vs %d).",packet.Cil_Base.Service,E_AGS_CMD);
		return FALSE;
	}
	if(packet.Command != E_AGS_GUIDE_ON_PIXEL)
	{
		NGATCil_General_Error_Number = 310;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Parse:"
			"Wrong command number (%d vs %d).",packet.Command,E_AGS_GUIDE_ON_PIXEL);
		return FALSE;
	}
	(*pixel_x) = ((float)(packet.Param1))/1000.0f;
	(*pixel_y) = ((float)(packet.Param2))/1000.0f;
	(*sequence_number) = packet.Cil_Base.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Pixel_Parse",
				   LOG_VERBOSITY_VERBOSE,NULL,"finished(%.2f,%.2f,%d).",
				   (*pixel_x),(*pixel_y),(*sequence_number));
#endif
	return TRUE;
}

/**
 * Send an "Autoguide on pixel x y" CIL reply packet on the specified socket.
 * For use by the AGS.
 * @param socket_id The socket to send the packet over.
 * @param hostname The hostname of the host to send the packet to.
 * @param port_number The port number of the host to send the packet to.
 * @param pixel_x The X pixel position, 0..1023.
 * @param pixel_y The Y pixel position, 0..1023.
 * @param status The status to send for this packet. 
 * @param sequence_number The sequence number to use for this packet. Should be retrieved from the
 *        "Autoguide on pixel x y" CIL command packet that caused this response to be sent.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #SYS_NOMINAL
 * @see #E_AGS_GUIDE_ON_PIXEL
 * @see #E_AGS_CMD
 * @see #E_CIL_RSP_CLASS
 * @see #Sequence_Number
 * @see #NGATCil_Cil_Packet_Send_To
 * @see #NGATCil_Cil_Packet_Create
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_On_Pixel_Reply_Send(int socket_id,char *hostname,int port_number,float pixel_x,float pixel_y,
					      int status,int sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Ags_Packet_Struct packet;
	int pixel_x_i,pixel_y_i,retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Pixel_Reply_Send",
				   LOG_VERBOSITY_VERBOSE,NULL,"started(%.2f,%.2f,%#x,%d).",
				   pixel_x,pixel_y,status,sequence_number);
#endif
	if((pixel_x < 0.0f) || (pixel_x > 1023.0f))
	{
		NGATCil_General_Error_Number = 311;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Send:"
			"Pixel X %.2f out of range (0..1023).",pixel_x);
		return FALSE;
	}
	if((pixel_y < 0.0f) || (pixel_y > 1023.0f))
	{
		NGATCil_General_Error_Number = 312;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Send:"
			"Pixel Y %.2f out of range (0..1023).",pixel_y);
		return FALSE;
	}
	/* param 1 is X pixel position in millipixels */
	pixel_x_i = (int)(pixel_x * 1000.0f);
	packet.Param1 = pixel_x_i;
	/* param 2 is Y pixel position in millipixels */
	pixel_y_i = (int)(pixel_y * 1000.0f);
	packet.Param2 = pixel_y_i;
	/* use NGATCil_Cil_Packet_Create to packet is in network byte order*/
	retval = NGATCil_Cil_Packet_Create(E_CIL_AGS,E_CIL_TCS,E_CIL_RSP_CLASS,E_AGS_CMD,sequence_number,
					   E_AGS_GUIDE_ON_PIXEL,
					   status,pixel_x_i,pixel_y_i,&packet);
	if(retval == FALSE)
		return FALSE;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send_To(socket_id,hostname,port_number,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Pixel_Reply_Send",
			    LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Parse an "Autoguide on pixel x y" CIL reply packet.
 * For use by the TCS (simulator).
 * @param packet The received packet to parse. This should have been translated to host byte order using
 *         NGATCil_Cil_Packet_Recv.
 * @param pixel_x The address of a float to store the X pixel position requested.
 * @param pixel_y The address of a float to store the Y pixel position requested.
 * @param status The address of an integer to store the status.
 * @param sequence_number The address of an integer to store the sequence_number.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #E_CIL_RSP_CLASS
 * @see #E_AGS_CMD
 * @see #E_AGS_GUIDE_ON_PIXEL
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse(struct NGATCil_Ags_Packet_Struct packet,float *pixel_x,float *pixel_y,
					       int *status,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse",
			    LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(pixel_x == NULL)
	{
		NGATCil_General_Error_Number = 313;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:pixel_x was NULL.");
		return FALSE;
	}
	if(pixel_y == NULL)
	{
		NGATCil_General_Error_Number = 314;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:pixel_y was NULL.");
		return FALSE;
	}
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 315;
		sprintf(NGATCil_General_Error_String,
			"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:Sequence number was NULL.");
		return FALSE;
	}
	if(status == NULL)
	{
		NGATCil_General_Error_Number = 316;
		sprintf(NGATCil_General_Error_String,
			"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:status was NULL.");
		return FALSE;
	}
	if(packet.Cil_Base.Class != E_CIL_RSP_CLASS)
	{
		NGATCil_General_Error_Number = 317;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:"
			"Wrong class number (%d vs %d).",packet.Cil_Base.Class,E_CIL_RSP_CLASS);
		return FALSE;
	}
	if(packet.Cil_Base.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 318;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:"
			"Wrong service number (%d vs %d).",packet.Cil_Base.Service,E_AGS_CMD);
		return FALSE;
	}
	if(packet.Command != E_AGS_GUIDE_ON_PIXEL)
	{
		NGATCil_General_Error_Number = 319;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:"
			"Wrong command number (%d vs %d).",packet.Command,E_AGS_GUIDE_ON_PIXEL);
		return FALSE;
	}
	(*status) = packet.Status;
	(*pixel_x) = ((float)(packet.Param1))/1000.0f;
	(*pixel_y) = ((float)(packet.Param2))/1000.0f;
	(*sequence_number) = packet.Cil_Base.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse",
				   LOG_VERBOSITY_VERBOSE,NULL,"finished(%#x,%.2f,%.2f,%d).",
				   (*status),(*pixel_x),(*pixel_y),(*sequence_number));
#endif
	return TRUE;
}

/**
 * Parse an "autoguide on brightest" CIL command packet.
 * For use by the AGS.
 * @param packet The received packet to parse. This should have been translated to host byte order using
 *         NGATCil_Cil_Packet_Recv.
 * @param sequence_number The address of an integer to store the sequence_number.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #E_CIL_CMD_CLASS
 * @see #E_AGS_CMD
 * @see #E_AGS_GUIDE_ON_BRIGHTEST
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_On_Brightest_Parse(struct NGATCil_Ags_Packet_Struct packet,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Brightest_Parse",
			    LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 331;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Brightest_Parse:"
			"Sequence number was NULL.");
		return FALSE;
	}
	if(packet.Cil_Base.Class != E_CIL_CMD_CLASS)
	{
		NGATCil_General_Error_Number = 332;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Brightest_Parse:"
			"Wrong class number (%d vs %d).",packet.Cil_Base.Class,E_CIL_CMD_CLASS);
		return FALSE;
	}
	if(packet.Cil_Base.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 333;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Brightest_Parse:"
			"Wrong service number (%d vs %d).",packet.Cil_Base.Service,E_AGS_CMD);
		return FALSE;
	}
	if(packet.Command != E_AGS_GUIDE_ON_BRIGHTEST)
	{
		NGATCil_General_Error_Number = 334;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Brightest_Parse:"
			"Wrong command number (%d vs %d).",packet.Command,E_AGS_GUIDE_ON_BRIGHTEST);
		return FALSE;
	}
	(*sequence_number) = packet.Cil_Base.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Brightest_Parse",
				   LOG_VERBOSITY_VERBOSE,NULL,"finished(%d).",(*sequence_number));
#endif
	return TRUE;
}

/**
 * Send an "autoguide on brightest" CIL reply packet on the specified socket.
 * For use by the AGS.
 * @param hostname The hostname of the host to send the packet to.
 * @param port_number The port number of the host to send the packet to.
 * @param socket_id The socket to send the packet over.
 * @param status The status to send for this packet. 
 * @param sequence_number The sequence number to use for this packet. Should be retrieved from the
 *        "autoguide on brightest" CIL command packet that caused this response to be sent.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #SYS_NOMINAL
 * @see #E_AGS_GUIDE_ON_BRIGHTEST
 * @see #E_AGS_CMD
 * @see #E_CIL_RSP_CLASS
 * @see #Sequence_Number
 * @see #NGATCil_Cil_Packet_Send_To
 * @see #NGATCil_Cil_Packet_Create
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_On_Brightest_Reply_Send(int socket_id,char *hostname,int port_number,int status,
						  int sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Ags_Packet_Struct packet;
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Brightest_Reply_Send",
				   LOG_VERBOSITY_VERBOSE,NULL,"started(%#x,%d).",status,sequence_number);
#endif
	retval = NGATCil_Cil_Packet_Create(E_CIL_AGS,E_CIL_TCS,E_CIL_RSP_CLASS,E_AGS_CMD,sequence_number,
					   E_AGS_GUIDE_ON_BRIGHTEST,status,0,0,&packet);
	if(retval == FALSE)
		return FALSE;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send_To(socket_id,hostname,port_number,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Brightest_Reply_Send",
			    LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Parse an "autoguide on rank <n>" CIL command packet.
 * For use by the AGS.
 * @param packet The received packet to parse. This should have been translated to host byte order using
 *         NGATCil_Cil_Packet_Recv or UDP_Raw_Server_Thread.
 * @param rank The address of an integer to store  the parsed rank.
 * @param sequence_number The address of an integer to store the sequence_number.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #E_CIL_CMD_CLASS
 * @see #E_AGS_CMD
 * @see #E_AGS_GUIDE_ON_RANK
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_On_Rank_Parse(struct NGATCil_Ags_Packet_Struct packet,int *rank,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Rank_Parse",
			    LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 335;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Rank_Parse:"
			"Sequence number was NULL.");
		return FALSE;
	}
	if(rank == NULL)
	{
		NGATCil_General_Error_Number = 336;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Rank_Parse:Rank was NULL.");
		return FALSE;
	}
	if(packet.Cil_Base.Class != E_CIL_CMD_CLASS)
	{
		NGATCil_General_Error_Number = 337;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Rank_Parse:"
			"Wrong class number (%d vs %d).",packet.Cil_Base.Class,E_CIL_CMD_CLASS);
		return FALSE;
	}
	if(packet.Cil_Base.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 338;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Rank_Parse:"
			"Wrong service number (%d vs %d).",packet.Cil_Base.Service,E_AGS_CMD);
		return FALSE;
	}
	if(packet.Command != E_AGS_GUIDE_ON_RANK)
	{
		NGATCil_General_Error_Number = 339;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Rank_Parse:"
			"Wrong command number (%d vs %d).",packet.Command,E_AGS_GUIDE_ON_RANK);
		return FALSE;
	}
	(*sequence_number) = packet.Cil_Base.Seq_Num;
	(*rank) = packet.Param1;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Rank_Parse",
				   LOG_VERBOSITY_VERBOSE,NULL,
				   "finished(%d,%d).",(*rank),(*sequence_number));
#endif
	return TRUE;
}

/**
 * Send an "autoguide on rank" CIL reply packet on the specified socket.
 * For use by the AGS.
 * @param socket_id The socket to send the packet over.
 * @param hostname The hostname of the host to send the packet to.
 * @param port_number The port number of the host to send the packet to.
 * @param rank The rank requested to autoguide on.
 * @param status The status to send for this packet. 
 * @param sequence_number The sequence number to use for this packet. Should be retrieved from the
 *        "autoguide on rank" CIL command packet that caused this response to be sent.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #SYS_NOMINAL
 * @see #E_AGS_GUIDE_ON_RANK
 * @see #E_AGS_CMD
 * @see #E_CIL_RSP_CLASS
 * @see #Sequence_Number
 * @see #NGATCil_Cil_Packet_Send_To
 * @see #NGATCil_Cil_Packet_Create
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_On_Rank_Reply_Send(int socket_id,char *hostname,int port_number,int rank,int status,
					     int sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Ags_Packet_Struct packet;
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Rank_Reply_Send",
				   LOG_VERBOSITY_VERBOSE,NULL,"started(%d,%#x,%d).",rank,status,sequence_number);
#endif
	retval = NGATCil_Cil_Packet_Create(E_CIL_AGS,E_CIL_TCS,E_CIL_RSP_CLASS,E_AGS_CMD,sequence_number,
					   E_AGS_GUIDE_ON_RANK,status,rank,0,&packet);
	if(retval == FALSE)
		return FALSE;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send_To(socket_id,hostname,port_number,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_On_Rank_Reply_Send",
			    LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Send an "Autoguide off" CIL command packet on the specified socket.
 * For use for TCS (simulators).
 * @param socket_id The socket to send the packet over.
 * @param hostname The hostname of the host to send the packet to.
 * @param port_number The port number of the host to send the packet to.
 * @param sequence_number The address of an integer to store the sequence number generated for this packet.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #SYS_NOMINAL
 * @see #E_AGS_GUIDE_OFF
 * @see #E_AGS_CMD
 * @see #E_CIL_CMD_CLASS
 * @see #Sequence_Number
 * @see #NGATCil_Cil_Packet_Send_To
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_Off_Send(int socket_id,char *hostname,int port_number,int *sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Ags_Packet_Struct packet;
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_Off_Send",
			    LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 320;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Send:Sequence number was NULL.");
		return FALSE;
	}
	packet.Cil_Base.Source_Id = E_CIL_TCS;
	packet.Cil_Base.Dest_Id = E_CIL_AGS;
	packet.Cil_Base.Class = E_CIL_CMD_CLASS;
	packet.Cil_Base.Service = E_AGS_CMD;
	/* increment sequence number - prevent overflow */
	Sequence_Number++;
	if(Sequence_Number > 200000000)
		Sequence_Number = 0;
	packet.Cil_Base.Seq_Num = Sequence_Number;
	(*sequence_number) = Sequence_Number;
	clock_gettime(CLOCK_REALTIME,&current_time);
	packet.Cil_Base.Timestamp_Seconds = current_time.tv_sec;
	packet.Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
	packet.Command = E_AGS_GUIDE_OFF;
	packet.Status = SYS_NOMINAL;
	packet.Param1 = 0;
	packet.Param2 = 0;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send_To(socket_id,hostname,port_number,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_Off_Send",
				   LOG_VERBOSITY_VERBOSE,NULL,"finished(%d).",(*sequence_number));
#endif
	return TRUE;
}

/**
 * Parse an "Autoguide off" CIL command packet.
 * For use by the AGS.
 * @param packet The received packet to parse. This should have been translated to host byte order using
 *         NGATCil_Cil_Packet_Recv.
 * @param status The address of an integer to store the status.
 * @param sequence_number The address of an integer to store the sequence_number.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #E_CIL_CMD_CLASS
 * @see #E_AGS_CMD
 * @see #E_AGS_GUIDE_OFF
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_Off_Parse(struct NGATCil_Ags_Packet_Struct packet,int *status,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_Off_Parse",
			    LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(status == NULL)
	{
		NGATCil_General_Error_Number = 321;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Parse:status was NULL.");
		return FALSE;
	}
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 322;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Parse:Sequence number was NULL.");
		return FALSE;
	}
	if(packet.Cil_Base.Class != E_CIL_CMD_CLASS)
	{
		NGATCil_General_Error_Number = 323;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Parse:"
			"Wrong class number (%d vs %d).",packet.Cil_Base.Class,E_CIL_CMD_CLASS);
		return FALSE;
	}
	if(packet.Cil_Base.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 324;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Parse:"
			"Wrong service number (%d vs %d).",packet.Cil_Base.Service,E_AGS_CMD);
		return FALSE;
	}
	if(packet.Command != E_AGS_GUIDE_OFF)
	{
		NGATCil_General_Error_Number = 325;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Parse:"
			"Wrong command number (%d vs %d).",packet.Command,E_AGS_GUIDE_OFF);
		return FALSE;
	}
	(*status) = packet.Status;
	(*sequence_number) = packet.Cil_Base.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_Off_Parse",
				   LOG_VERBOSITY_VERBOSE,NULL,"finished(%#x,%d).",(*status),(*sequence_number));
#endif
	return TRUE;
}

/**
 * Send an "Autoguide off" CIL reply packet on the specified socket.
 * For use by the AGS.
 * @param socket_id The socket to send the packet over.
 * @param hostname The hostname of the host to send the packet to.
 * @param port_number The port number of the host to send the packet to.
 * @param status The status to send for this packet. 
 * @param sequence_number The sequence number to use for this packet. Should be retrieved from the
 *        "Autoguide off" CIL command packet that caused this response to be sent.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #SYS_NOMINAL
 * @see #E_AGS_GUIDE_OFF
 * @see #E_AGS_CMD
 * @see #E_CIL_RSP_CLASS
 * @see #Sequence_Number
 * @see #NGATCil_Cil_Packet_Send_To
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_Off_Reply_Send(int socket_id,char *hostname,int port_number,int status,int sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Ags_Packet_Struct packet;
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_Off_Reply_Send",
				   LOG_VERBOSITY_VERBOSE,NULL,"started(%#x,%d).",status,sequence_number);
#endif
	retval = NGATCil_Cil_Packet_Create(E_CIL_AGS,E_CIL_TCS,E_CIL_RSP_CLASS,E_AGS_CMD,sequence_number,
					   E_AGS_GUIDE_OFF,status,0,0,&packet);
	if(retval == FALSE)
		return FALSE;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send_To(socket_id,hostname,port_number,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_Off_Reply_Send",
			    LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Parse an "Autoguide off" CIL reply packet.
 * For use by the TCS (simulator).
 * @param packet The received packet to parse. This should have been translated to host byte order using
 *         NGATCil_Cil_Packet_Recv.
 * @param status The address of an integer to store the status.
 * @param sequence_number The address of an integer to store the sequence_number.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #E_CIL_RSP_CLASS
 * @see #E_AGS_CMD
 * @see #E_AGS_GUIDE_OFF
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_Cil_Autoguide_Off_Reply_Parse(struct NGATCil_Ags_Packet_Struct packet,int *status,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_Off_Reply_Parse",
			    LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 326;
		sprintf(NGATCil_General_Error_String,
			"NGATCil_Cil_Autoguide_Off_Reply_Parse:Sequence number was NULL.");
		return FALSE;
	}
	if(status == NULL)
	{
		NGATCil_General_Error_Number = 327;
		sprintf(NGATCil_General_Error_String,
			"NGATCil_Cil_Autoguide_Off_Reply_Parse:status was NULL.");
		return FALSE;
	}
	if(packet.Cil_Base.Class != E_CIL_RSP_CLASS)
	{
		NGATCil_General_Error_Number = 328;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Reply_Parse:"
			"Wrong class number (%d vs %d).",packet.Cil_Base.Class,E_CIL_RSP_CLASS);
		return FALSE;
	}
	if(packet.Cil_Base.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 329;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Reply_Parse:"
			"Wrong service number (%d vs %d).",packet.Cil_Base.Service,E_AGS_CMD);
		return FALSE;
	}
	if(packet.Command != E_AGS_GUIDE_OFF)
	{
		NGATCil_General_Error_Number = 330;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Reply_Parse:"
			"Wrong command number (%d vs %d).",packet.Command,E_AGS_GUIDE_OFF);
		return FALSE;
	}
	(*status) = packet.Status;
	(*sequence_number) = packet.Cil_Base.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_cil.c","NGATCil_Cil_Autoguide_Off_Reply_Parse",
				   LOG_VERBOSITY_VERBOSE,NULL,"finished(%#x,%d).",(*status),(*sequence_number));
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/*
** $Log: not supported by cvs2svn $
** Revision 1.7  2009/01/30 18:00:52  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.6  2006/08/29 14:07:57  cjm
** Rewritten using new Cil_Base element in packet structure.
**
** Revision 1.5  2006/07/20 15:15:27  cjm
** Removed eCilNames (and put in header).
**
** Revision 1.4  2006/07/16 20:12:46  cjm
** rewrite using NGATCil_Cil_Packet_Create to ensure packet is in network byte order.
**
** Revision 1.3  2006/06/29 17:02:19  cjm
** Added Rank/Brightest AG code.
**
** Revision 1.2  2006/06/07 11:11:24  cjm
** Added logging.
**
** Revision 1.1  2006/06/06 15:58:00  cjm
** Initial revision
**
*/
