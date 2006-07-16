/* ngatcil_cil.c
** NGATCil General CIL packet tranmitting/receiving routines.
** $Header: /home/cjm/cvs/autoguider/ngatcil/c/ngatcil_cil.c,v 1.4 2006-07-16 20:12:46 cjm Exp $
*/
/**
 * NGAT Cil library transmission/receiving of CIL packets over UDP.
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
#include <string.h>
#include <time.h>

#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"
#include "ngatcil_cil.h"
/* hash defines */

/* enums */
/**
 * List of address codes for the telescope subsystems. Note that the
 * enumeration MUST start at zero and have the last element as 
 * E_CIL_EOL in order to work (c.f. eCilSetup() in Cil.h).
 * Derived from ngtcs Cil.h (/home/dev/src/cil/ngtcs/Cil.h) which claims to be v1.42 (2001/11/30).
 */
enum eCilNames
{
   E_CIL_BOL = 0,  /* Beginning of list (DO NOT USE FOR A TASK ID) */

   /*  MCS - MCP   */

   E_CIL_MCP,      /* Master Control Process */
   E_CIL_CHB,      /* Continuous Heartbeat (part of MCP package) */
   E_CIL_UI1,      /* Un-interruptable power-supply Interface task 1 */
   E_CIL_UI2,      /* Un-interruptable power-supply Interface task 2 */
   E_CIL_AI1,      /* Addressable power-switch Interface task 1 */
   E_CIL_AI2,      /* Addressable power-switch Interface task 2 */
   E_CIL_AI3,      /* Addressable power-switch Interface task 3 */
   E_CIL_MIT,      /* Modem Interface Task */

   /* MCS - rest   */

   E_CIL_MCB,      /* Master Command Broker */
   E_CIL_SDB,      /* Status Database */
   E_CIL_SFR,      /* Status database File Recovery task */
   E_CIL_SPT,      /* Services PLC Task */
   E_CIL_EPT,      /* Enclosure PLC Task */
   E_CIL_EPS,      /* Enclosure PLC Simulator */
   E_CIL_WMS,      /* Weather Monitoring System */
   E_CIL_AVS,      /* Audio-Visual System */

   /* TCS          */

   E_CIL_TCS,      /* Telescope Control System */
   E_CIL_RCS,      /* Robotic Control System */
   E_CIL_OCS,      /* Observatory Control System */
   E_CIL_AGS,      /* Autoguider system */
   E_CIL_AGP,      /* Autoguider guide packets */
   E_CIL_AGG,      /* Autoguider guide process */
   E_CIL_AGI,      /* Autoguider graphical interface */

   /* ECI - note that the ERx block must follow directly */

   E_CIL_EI0,      /* Engineering control Interface (session 0)             */
   E_CIL_EI1,      /* Engineering control Interface (session 1)             */
   E_CIL_EI2,      /* Engineering control Interface (session 2)             */
   E_CIL_EI3,      /* Engineering control Interface (session 3)             */
   E_CIL_EI4,      /* Engineering control Interface (session 4)             */

   /* ERT - note that these must directly follow the EIx block */

   E_CIL_ER0,      /* Engineering Reporting Task (session 0)                */
   E_CIL_ER1,      /* Engineering Reporting Task (session 1)                */
   E_CIL_ER2,      /* Engineering Reporting Task (session 2)                */
   E_CIL_ER3,      /* Engineering Reporting Task (session 3)                */
   E_CIL_ER4,      /* Engineering Reporting Task (session 4)                */

   /* Axis nodes   */

   E_CIL_AZC,      /* Azimuth ACN Comms-In                                  */
   E_CIL_AZN,      /* Azimuth ACN Control Node                              */
   E_CIL_AZS,      /* Azimuth ACN Comms-Out                                 */
   E_CIL_AZR,      /* Azimuth ACN SDB Reporting                             */
   E_CIL_AZT,      /* Azimuth ACN Test (formerly Simulator AZS)             */
   E_CIL_AZL,      /* Azimuth ACN Logger                                    */

   E_CIL_ELC,      /* Elevation ACN Comms-In                                */
   E_CIL_ELN,      /* Elevation ACN Control Node                            */
   E_CIL_ELS,      /* Elevation ACN Comms-Out                               */
   E_CIL_ELR,      /* Elevation ACN SDB Reporting                           */
   E_CIL_ELT,      /* Elevation ACN Test (formerly Simulator ELS)           */
   E_CIL_ELL,      /* Elevation ACN Logger                                  */

   E_CIL_CSC,      /* Cassegrain ACN Comms-In                               */
   E_CIL_CSN,      /* Cassegrain ACN Control Node                           */
   E_CIL_CSS,      /* Cassegrain ACN Comms-Out                              */
   E_CIL_CSR,      /* Cassegrain ACN SDB Reporting                          */
   E_CIL_CST,      /* Cassegrain ACN Test (formerly Simulator CSS)          */
   E_CIL_CSL,      /* Cassegrain ACN Logger                                 */

   E_CIL_OMC,      /* Auxiliary (Optical) Mechanism Comms-In                */
   E_CIL_OMN,      /* Auxiliary (Optical) Mechanism Control Node            */
   E_CIL_OMS,      /* Auxiliary (Optical) Mechanism Comms-Out               */
   E_CIL_OMR,      /* Auxiliary (Optical) Mechanism SDB Reporting           */
   E_CIL_OMT,      /* Auxiliary (Optical) Mechanism Test (formerly AMS)     */
   E_CIL_OML,      /* Auxiliary (Optical) Mechanism Logger                  */

   E_CIL_MSC,      /* Primary Mirror Support Comms-In                       */
   E_CIL_MSN,      /* Primary Mirror Support Control Node                   */
   E_CIL_MSS,      /* Primary Mirror Support Comms-Out                      */
   E_CIL_MSR,      /* Primary Mirror Support SDB Reporting                  */
   E_CIL_MST,      /* Primary Mirror Support Test                           */
   E_CIL_MSL,      /* Primary Mirror Support Logger                         */

   /* Test units   */

   E_CIL_TU0,      /* Test Unit 0 (for testing/debugging only) */
   E_CIL_TU1,      /* Test Unit 1 (for testing/debugging only) */
   E_CIL_TU2,      /* Test Unit 2 (for testing/debugging only) */
   E_CIL_TU3,      /* Test Unit 3 (for testing/debugging only) */
   E_CIL_TU4,      /* Test Unit 4 (for testing/debugging only) */
   E_CIL_TU5,      /* Test Unit 5 (for testing/debugging only) */
   E_CIL_TU6,      /* Test Unit 6 (for testing/debugging only) */
   E_CIL_TU7,      /* Test Unit 7 (for testing/debugging only) */
   E_CIL_TU8,      /* Test Unit 8 (for testing/debugging only) */
   E_CIL_TU9,      /* Test Unit 9 (for testing/debugging only) */
   E_CIL_TES,      /* Test Echo Server (for testing only) */
   E_CIL_TMB,      /* Test Message Broker (for testing only) */
   E_CIL_IPT,      /* IUCAA Prototype TCS */
   E_CIL_TST,      /* Test Scripting Tool (for testing only) */

   /* Misc         */

   E_CIL_LOG,      /* System logging (syslogd) */

   /* DAT          */

   E_CIL_DA0,       /* Data Analysis Tool (session 0) */
   E_CIL_DA1,       /* Data Analysis Tool (session 1) */
   E_CIL_DA2,       /* Data Analysis Tool (session 2) */
   E_CIL_DA3,       /* Data Analysis Tool (session 3) */
   E_CIL_DA4,       /* Data Analysis Tool (session 4) */
   E_CIL_DA5,       /* Data Analysis Tool (session 5) */
   E_CIL_DA6,       /* Data Analysis Tool (session 6) */
   E_CIL_DA7,       /* Data Analysis Tool (session 7) */
   E_CIL_DA8,       /* Data Analysis Tool (session 8) */
   E_CIL_DA9,       /* Data Analysis Tool (session 9) */

   /* CMT          */

   E_CIL_CM0,       /* Computer Monitoring Task (session 0) */
   E_CIL_CM1,       /* Computer Monitoring Task (session 1) */
   E_CIL_CM2,       /* Computer Monitoring Task (session 2) */
   E_CIL_CM3,       /* Computer Monitoring Task (session 3) */
   E_CIL_CM4,       /* Computer Monitoring Task (session 4) */
   E_CIL_CM5,       /* Computer Monitoring Task (session 5) */
   E_CIL_CM6,       /* Computer Monitoring Task (session 6) */
   E_CIL_CM7,       /* Computer Monitoring Task (session 7) */
   E_CIL_CM8,       /* Computer Monitoring Task (session 8) */
   E_CIL_CM9,       /* Computer Monitoring Task (session 9) */

   /* CCT          */

   E_CIL_CC0,       /* Computer Control Task (session 0) */
   E_CIL_CC1,       /* Computer Control Task (session 1) */
   E_CIL_CC2,       /* Computer Control Task (session 2) */
   E_CIL_CC3,       /* Computer Control Task (session 3) */
   E_CIL_CC4,       /* Computer Control Task (session 4) */
   E_CIL_CC5,       /* Computer Control Task (session 5) */
   E_CIL_CC6,       /* Computer Control Task (session 6) */
   E_CIL_CC7,       /* Computer Control Task (session 7) */
   E_CIL_CC8,       /* Computer Control Task (session 8) */
   E_CIL_CC9,       /* Computer Control Task (session 9) */

   E_CIL_EOL       /* End of list marker (DO NOT USE FOR A TASK ID) */
};

/* data types */

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ngatcil_cil.c,v 1.4 2006-07-16 20:12:46 cjm Exp $";
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Packet_Create(int source_id,int dest_id,int class,int service,int seq_num,int command,
			      int status,int param1, int param2,struct NGATCil_Cil_Packet_Struct *packet)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Packet_Create:started.");
#endif
	if(packet == NULL)
	{
		NGATCil_General_Error_Number = 300;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Packet_Create:packet was NULL.");
		return FALSE;
	}
	packet->Source_Id = htonl(source_id);
	packet->Dest_Id = htonl(dest_id);
	packet->Class = htonl(class);
	packet->Service = htonl(service);
	packet->Seq_Num = htonl(seq_num);
	packet->Command = htonl(command);
	packet->Status = htonl(status);
	packet->Param1 = htonl(param1);
	packet->Param2 = htonl(param2);
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Packet_Create:finished.");
#endif
	return TRUE;
}

/**
 * Send a CIL packet. 
 * @param socket_id The socket file descriptor to use.
 * @param packet The packet to send. The contents should have been put into network byte order
 *        (NGATCil_Cil_Packet_Create does this automatically).
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_udp_raw.html#NGATCil_UDP_Raw_Send
 */
int NGATCil_Cil_Packet_Send(int socket_id,struct NGATCil_Cil_Packet_Struct packet)
{
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Packet_Send:started.");
#endif
	retval = NGATCil_UDP_Raw_Send(socket_id,(void*)&packet,sizeof(struct NGATCil_Cil_Packet_Struct));
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Packet_Send:finished.");
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
int NGATCil_Cil_Packet_Recv(int socket_id,struct NGATCil_Cil_Packet_Struct *packet)
{
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Packet_Recv:started.");
#endif
	if(packet == NULL)
	{
		NGATCil_General_Error_Number = 301;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Packet_Recv:packet was NULL.");
		return FALSE;
	}
	retval = NGATCil_UDP_Raw_Recv(socket_id,packet,sizeof(struct NGATCil_Cil_Packet_Struct));
	if(retval == FALSE)
		return FALSE;
	packet->Source_Id = ntohl(packet->Source_Id);
	packet->Dest_Id = ntohl(packet->Dest_Id);
	packet->Class = ntohl(packet->Class);
	packet->Service = ntohl(packet->Service);
	packet->Seq_Num = ntohl(packet->Seq_Num);
	packet->Command = ntohl(packet->Command);
	packet->Status = ntohl(packet->Status);
	packet->Param1 = ntohl(packet->Param1);
	packet->Param2 = ntohl(packet->Param2);
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Packet_Recv:finished.");
#endif
	return TRUE;
}

/**
 * Send an "Autoguide on pixel x y" CIL command packet on the specified socket.
 * For use for TCS (simulators).
 * @param socket_id The socket to send the packet over.
 * @param pixel_x The X pixel position, 0..1023.
 * @param pixel_y The Y pixel position, 0..1023.
 * @param sequence_number The address of an integer to store the sequence number generated for this packet.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #NGATCil_Cil_Packet_Send
 * @see #SYS_NOMINAL
 * @see #E_AGS_GUIDE_ON_PIXEL
 * @see #E_AGS_CMD
 * @see #E_CIL_CMD_CLASS
 * @see #Sequence_Number
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_On_Pixel_Send(int socket_id,float pixel_x,float pixel_y,int *sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Cil_Packet_Struct packet;
	int pixel_x_i,pixel_y_i,retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Pixel_Send(%.2f,%.2f):started.",pixel_x,pixel_y);
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
	/* diddly rewrite using NGATCil_Cil_Packet_Create to packet is in network byte order
	retval = NGATCil_Cil_Packet_Create(E_CIL_TCS,E_CIL_AGS,E_CIL_CMD_CLASS,E_AGS_CMD,Sequence_Number,E_AGS_GUIDE_ON_PIXEL,
			     SYS_NOMINAL,int param1, int param2,struct NGATCil_Cil_Packet_Struct *packet)
	*/
	packet.Source_Id = E_CIL_TCS;
	packet.Dest_Id = E_CIL_AGS;
	packet.Class = E_CIL_CMD_CLASS;
	packet.Service = E_AGS_CMD;
	/* increment sequence number - prevent overflow */
	Sequence_Number++;
	if(Sequence_Number > 200000000)
		Sequence_Number = 0;
	packet.Seq_Num = Sequence_Number;
	(*sequence_number) = Sequence_Number;
	clock_gettime(CLOCK_REALTIME,&current_time);
	packet.Timestamp_Seconds = current_time.tv_sec;
	packet.Timestamp_Nanoseconds = current_time.tv_nsec;
	packet.Command = E_AGS_GUIDE_ON_PIXEL;
	packet.Status = SYS_NOMINAL;
	/* param 1 is X pixel position in millipixels */
	pixel_x_i = (int)(pixel_x * 1000.0f);
	packet.Param1 = pixel_x_i;
	/* param 2 is Y pixel position in millipixels */
	pixel_y_i = (int)(pixel_y * 1000.0f);
	packet.Param2 = pixel_y_i;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send(socket_id,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Pixel_Send:sequence ID %d:finished.",(*sequence_number));
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_On_Pixel_Parse(struct NGATCil_Cil_Packet_Struct packet,float *pixel_x,float *pixel_y,
					 int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_On_Pixel_Parse:started.");
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
	if(packet.Class != E_CIL_CMD_CLASS)
	{
		NGATCil_General_Error_Number = 308;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Parse:"
			"Wrong class number (%d vs %d).",packet.Class,E_CIL_CMD_CLASS);
		return FALSE;
	}
	if(packet.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 309;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Parse:"
			"Wrong service number (%d vs %d).",packet.Service,E_AGS_CMD);
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
	(*sequence_number) = packet.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Pixel_Parse:(%.2f,%.2f,%d):finished.",
				   (*pixel_x),(*pixel_y),(*sequence_number));
#endif
	return TRUE;
}

/**
 * Send an "Autoguide on pixel x y" CIL reply packet on the specified socket.
 * For use by the AGS.
 * @param socket_id The socket to send the packet over.
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
 * @see #NGATCil_Cil_Packet_Send
 & @see #NGATCil_Cil_Packet_Create
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_On_Pixel_Reply_Send(int socket_id,float pixel_x,float pixel_y,int status,
					      int sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Cil_Packet_Struct packet;
	int pixel_x_i,pixel_y_i,retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Pixel_Reply_Send(%.2f,%.2f,%#x,%d):started.",
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
	retval = NGATCil_Cil_Packet_Send(socket_id,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Send:finished.");
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse(struct NGATCil_Cil_Packet_Struct packet,float *pixel_x,float *pixel_y,
					       int *status,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:started.");
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
	if(packet.Class != E_CIL_RSP_CLASS)
	{
		NGATCil_General_Error_Number = 317;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:"
			"Wrong class number (%d vs %d).",packet.Class,E_CIL_RSP_CLASS);
		return FALSE;
	}
	if(packet.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 318;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:"
			"Wrong service number (%d vs %d).",packet.Service,E_AGS_CMD);
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
	(*sequence_number) = packet.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse:(%#x,%.2f,%.2f,%d):finished.",
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_On_Brightest_Parse(struct NGATCil_Cil_Packet_Struct packet,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_On_Brightest_Parse:started.");
#endif
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 331;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Brightest_Parse:"
			"Sequence number was NULL.");
		return FALSE;
	}
	if(packet.Class != E_CIL_CMD_CLASS)
	{
		NGATCil_General_Error_Number = 332;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Brightest_Parse:"
			"Wrong class number (%d vs %d).",packet.Class,E_CIL_CMD_CLASS);
		return FALSE;
	}
	if(packet.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 333;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Brightest_Parse:"
			"Wrong service number (%d vs %d).",packet.Service,E_AGS_CMD);
		return FALSE;
	}
	if(packet.Command != E_AGS_GUIDE_ON_BRIGHTEST)
	{
		NGATCil_General_Error_Number = 334;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Brightest_Parse:"
			"Wrong command number (%d vs %d).",packet.Command,E_AGS_GUIDE_ON_BRIGHTEST);
		return FALSE;
	}
	(*sequence_number) = packet.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Brightest_Parse:(%.2f,%.2f,%d):finished.",
				   (*sequence_number));
#endif
	return TRUE;
}

/**
 * Send an "autoguide on brightest" CIL reply packet on the specified socket.
 * For use by the AGS.
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
 * @see #NGATCil_Cil_Packet_Send
 * @see #NGATCil_Cil_Packet_Create
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_On_Brightest_Reply_Send(int socket_id,int status,int sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Cil_Packet_Struct packet;
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Brightest_Reply_Send(%#x,%d):started.",
				   status,sequence_number);
#endif
	retval = NGATCil_Cil_Packet_Create(E_CIL_AGS,E_CIL_TCS,E_CIL_RSP_CLASS,E_AGS_CMD,sequence_number,
					   E_AGS_GUIDE_ON_BRIGHTEST,status,0,0,&packet);
	if(retval == FALSE)
		return FALSE;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send(socket_id,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_On_Brightest_Reply_Send:finished.");
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_On_Rank_Parse(struct NGATCil_Cil_Packet_Struct packet,int *rank,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_On_Rank_Parse:started.");
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
	if(packet.Class != E_CIL_CMD_CLASS)
	{
		NGATCil_General_Error_Number = 337;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Rank_Parse:"
			"Wrong class number (%d vs %d).",packet.Class,E_CIL_CMD_CLASS);
		return FALSE;
	}
	if(packet.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 338;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Rank_Parse:"
			"Wrong service number (%d vs %d).",packet.Service,E_AGS_CMD);
		return FALSE;
	}
	if(packet.Command != E_AGS_GUIDE_ON_RANK)
	{
		NGATCil_General_Error_Number = 339;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_On_Rank_Parse:"
			"Wrong command number (%d vs %d).",packet.Command,E_AGS_GUIDE_ON_RANK);
		return FALSE;
	}
	(*sequence_number) = packet.Seq_Num;
	(*rank) = packet.Param1;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Rank_Parse:(%.2f,%.2f,%d,%d):finished.",
				   (*rank),(*sequence_number));
#endif
	return TRUE;
}

/**
 * Send an "autoguide on rank" CIL reply packet on the specified socket.
 * For use by the AGS.
 * @param socket_id The socket to send the packet over.
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
 * @see #NGATCil_Cil_Packet_Send
 * @see #NGATCil_Cil_Packet_Create
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_On_Rank_Reply_Send(int socket_id,int rank,int status,int sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Cil_Packet_Struct packet;
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_On_Rank_Reply_Send(%d,%#x,%d):started.",
				   rank,status,sequence_number);
#endif
	retval = NGATCil_Cil_Packet_Create(E_CIL_AGS,E_CIL_TCS,E_CIL_RSP_CLASS,E_AGS_CMD,sequence_number,
					   E_AGS_GUIDE_ON_RANK,status,rank,0,&packet);
	if(retval == FALSE)
		return FALSE;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send(socket_id,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_On_Brightest_Reply_Send:finished.");
#endif
	return TRUE;
}

/**
 * Send an "Autoguide off" CIL command packet on the specified socket.
 * For use for TCS (simulators).
 * @param socket_id The socket to send the packet over.
 * @param sequence_number The address of an integer to store the sequence number generated for this packet.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #SYS_NOMINAL
 * @see #E_AGS_GUIDE_OFF
 * @see #E_AGS_CMD
 * @see #E_CIL_CMD_CLASS
 * @see #Sequence_Number
 * @see #NGATCil_Cil_Packet_Send
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_Off_Send(int socket_id,int *sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Cil_Packet_Struct packet;
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_Off_Send:started.");
#endif
	if(sequence_number == NULL)
	{
		NGATCil_General_Error_Number = 320;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Send:Sequence number was NULL.");
		return FALSE;
	}
	packet.Source_Id = E_CIL_TCS;
	packet.Dest_Id = E_CIL_AGS;
	packet.Class = E_CIL_CMD_CLASS;
	packet.Service = E_AGS_CMD;
	/* increment sequence number - prevent overflow */
	Sequence_Number++;
	if(Sequence_Number > 200000000)
		Sequence_Number = 0;
	packet.Seq_Num = Sequence_Number;
	(*sequence_number) = Sequence_Number;
	clock_gettime(CLOCK_REALTIME,&current_time);
	packet.Timestamp_Seconds = current_time.tv_sec;
	packet.Timestamp_Nanoseconds = current_time.tv_nsec;
	packet.Command = E_AGS_GUIDE_OFF;
	packet.Status = SYS_NOMINAL;
	packet.Param1 = 0;
	packet.Param2 = 0;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send(socket_id,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_Off_Send:(%d):finished.",
				   (*sequence_number));
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_Off_Parse(struct NGATCil_Cil_Packet_Struct packet,int *status,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_Off_Parse:started.");
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
	if(packet.Class != E_CIL_CMD_CLASS)
	{
		NGATCil_General_Error_Number = 323;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Parse:"
			"Wrong class number (%d vs %d).",packet.Class,E_CIL_CMD_CLASS);
		return FALSE;
	}
	if(packet.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 324;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Parse:"
			"Wrong service number (%d vs %d).",packet.Service,E_AGS_CMD);
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
	(*sequence_number) = packet.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_Off_Parse:(%#x,%d):finished.",
				   (*status),(*sequence_number));
#endif
	return TRUE;
}

/**
 * Send an "Autoguide off" CIL reply packet on the specified socket.
 * For use by the AGS.
 * @param socket_id The socket to send the packet over.
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
 * @see #NGATCil_Cil_Packet_Send
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_Off_Reply_Send(int socket_id,int status,int sequence_number)
{
	struct timespec current_time;
	struct NGATCil_Cil_Packet_Struct packet;
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_Off_Reply_Send(%#x,%d):started.",status,sequence_number);
#endif
	retval = NGATCil_Cil_Packet_Create(E_CIL_AGS,E_CIL_TCS,E_CIL_RSP_CLASS,E_AGS_CMD,sequence_number,
					   E_AGS_GUIDE_OFF,status,0,0,&packet);
	if(retval == FALSE)
		return FALSE;
	/* send packet */
	retval = NGATCil_Cil_Packet_Send(socket_id,packet);
	if(retval == FALSE)
		return FALSE;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_Off_Reply_Send:finished.");
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_CIL
 */
int NGATCil_Cil_Autoguide_Off_Reply_Parse(struct NGATCil_Cil_Packet_Struct packet,int *status,int *sequence_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_CIL,"NGATCil_Cil_Autoguide_Off_Reply_Parse:started.");
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
	if(packet.Class != E_CIL_RSP_CLASS)
	{
		NGATCil_General_Error_Number = 328;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Reply_Parse:"
			"Wrong class number (%d vs %d).",packet.Class,E_CIL_RSP_CLASS);
		return FALSE;
	}
	if(packet.Service != E_AGS_CMD)
	{
		NGATCil_General_Error_Number = 329;
		sprintf(NGATCil_General_Error_String,"NGATCil_Cil_Autoguide_Off_Reply_Parse:"
			"Wrong service number (%d vs %d).",packet.Service,E_AGS_CMD);
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
	(*sequence_number) = packet.Seq_Num;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_CIL,
				   "NGATCil_Cil_Autoguide_Off_Reply_Parse:(%#x,%d):finished.",
				   (*status),(*sequence_number));
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/*
** $Log: not supported by cvs2svn $
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
