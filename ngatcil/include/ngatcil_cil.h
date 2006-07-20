/* ngatcil_cil.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_cil.h,v 1.4 2006-07-20 15:16:01 cjm Exp $
*/
#ifndef NGATCIL_CIL_H
#define NGATCIL_CIL_H

/* hash defines */
/**
 * Length of AGS CIL packet?
 */
#define NGATCIL_CIL_PACKET_LENGTH   (44)
/**
 * Default port number for CIL UDP port for TCS commands to the TCS.
 * See Cil.map.
 */
#define NGATCIL_CIL_TCS_PORT_DEFAULT (13021)
/**
 * Default port number for CIL UDP port for AGS commands.
 * See Cil.map.
 */
#define NGATCIL_CIL_AGS_PORT_DEFAULT (13024)
/**
 * Default machine name the AGS (autoguider) software is running on. Set to "acc".
 * See Cil.map.
 */
#define NGATCIL_CIL_ACC_DEFAULT      ("acc")
/**
 * Used in CIL packets, Class field, for an invalid class of packet.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_CIL_INV_CLASS           (0)
/**
 * Used in CIL packets, Class field, for command class packets.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_CIL_CMD_CLASS           (1)
/**
 * Used in CIL packets, Class field, for response (reply) class packets.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_CIL_RSP_CLASS           (2)
/**
 * Used in CIL packets, Service field, to denote the AutoGuider Service.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_AGS_CMD                 (0x4f0000)
/**
 * Used in CIL packets, Command field, to tell the AGS to Guide on the brightest object.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_AGS_GUIDE_ON_BRIGHTEST  (1)
/**
 * Used in CIL packets, Command field, to tell the AGS to Guide on the specified ranked object.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_AGS_GUIDE_ON_RANK       (2)
/**
 * Used in CIL packets, Command field, to tell the AGS to Guide an object in the specified magnitude range.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_AGS_GUIDE_ON_RANGE      (3)
/**
 * Used in CIL packets, Command field, to tell the AGS to Guide on the object nearest the specified pixel.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_AGS_GUIDE_ON_PIXEL      (4)
/**
 * Used in CIL packets, Command field, to tell the AGS to stop guiding.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define E_AGS_GUIDE_OFF           (5)
/**
 * Used in CIL packets, Status field, to indicate all is well.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 */
#define SYS_NOMINAL               (0x10000)

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

/**
 * AGS status codes.
 * Taken from /home/dev/src/cil/ngtcs/include/Ags.h, /home/dev/src/cil/ngtcs/include/TtlSystem.h
 */
enum NGATCIL_CIL_AGS_STATUS
{
	E_AGS_GEN_ERR = (0x004f << 16),/* Miscellaneous error. */
	E_AGS_UNEXPECTED_MSG,             /* Unexpected message class. */
	E_AGS_UNKNOWN_SOURCE,             /* Unexpected message source. */
	E_AGS_NO_HEARTBEATS,              /* Heartbeats have been missed. */
	E_AGS_INVREPLY,                   /* Invalid reply to an AGS command. */
	E_AGS_CMDQ_EMPTY,                 /* Command queue empty. */
	E_AGS_CONFIG_FILE_ERROR,          /* Error reading config file. */
	E_AGS_INVALID_DATUM,              /* Datum not in range of eAgsDataId_t. */
	E_AGS_INV_CMD,                    /* Invalid command. */
	E_AGS_BAD_CMD,                    /* Badly formed command. */
	E_AGS_CMDQ_ERR,                   /* Error with command queue to Agg. */
	E_AGS_INV_STATE,                  /* Invalid state requested. */
	E_AGS_UNKNOWN_OID,                /* Request to set unknown OID. */ 
	E_AGS_CMDQ_FULL,                  /* Command queue full. */
	E_AGS_CMD_TIMEOUT,                /* Command to Agg timed out. */
	E_AGS_CMD_NOT_PERMITTED,          /* Command to Agg not permitted. */
	E_AGS_LOOP_STOPPING,              /* Command rejected - already stopping. */
	E_AGS_LOOP_RUNNING,               /* Command rejected - already running. */
	E_AGS_LOOP_ERROR,                 /* Error starting guide loop. */
	E_AGS_BAD_FORMAT,                 /* Badly formatted command sent to Agg. */
	E_AGS_HW_ERR                     /* Error reported by camera hardware. */
};

/* data types */
/**
 * Structure defining packet contents of a CIL command/reply packet.
 * All the integers should be sent over UDP in network byte order (htonl).
 * Packet contents are derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 * NB Assumes sizeof(int) == 32, should perhaps replace with Int32_t.
 * <dl>
 * <dt>Source_Id</dt> <dd></dd>
 * <dt>DestId</dt> <dd></dd>
 * <dt>Class</dt> <dd></dd>
 * <dt>Service</dt> <dd></dd>
 * <dt>Seq_Num</dt> <dd></dd>
 * <dt>Timestamp_Seconds</dt> <dd></dd>
 * <dt>Timestamp_Nanoseconds</dt> <dd></dd>
 * <dt>Command</dt> <dd></dd>
 * <dt>Status</dt> <dd></dd>
 * <dt>Param1</dt> <dd></dd>
 * <dt>Param2</dt> <dd></dd>
 * </dl>
 */
struct NGATCil_Cil_Packet_Struct
{
	int Source_Id;
	int Dest_Id;
	int Class;
	int Service;
	int Seq_Num;
	int Timestamp_Seconds;
	int Timestamp_Nanoseconds;
	int Command;
	int Status;
	int Param1;
	int Param2;
};

int NGATCil_Cil_Packet_Create(int source_id,int dest_id,int class,int service,int seq_num,int command,
			      int status,int param1, int param2,struct NGATCil_Cil_Packet_Struct *packet);
int NGATCil_Cil_Packet_Send(int socket_id,struct NGATCil_Cil_Packet_Struct packet);
int NGATCil_Cil_Packet_Recv(int socket_id,struct NGATCil_Cil_Packet_Struct *packet);

int NGATCil_Cil_Autoguide_On_Pixel_Send(int socket_id,float pixel_x,float pixel_y,int *sequence_number);
int NGATCil_Cil_Autoguide_On_Pixel_Parse(struct NGATCil_Cil_Packet_Struct packet,float *pixel_x,float *pixel_y,
					 int *sequence_number);

int NGATCil_Cil_Autoguide_On_Pixel_Reply_Send(int socket_id,float pixel_x,float pixel_y,int status,
					      int sequence_number);
int NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse(struct NGATCil_Cil_Packet_Struct packet,float *pixel_x,float *pixel_y,
					       int *status,int *sequence_number);

int NGATCil_Cil_Autoguide_On_Brightest_Parse(struct NGATCil_Cil_Packet_Struct packet,int *sequence_number);
int NGATCil_Cil_Autoguide_On_Brightest_Reply_Send(int socket_id,int status,int sequence_number);

int NGATCil_Cil_Autoguide_On_Rank_Parse(struct NGATCil_Cil_Packet_Struct packet,int *rank,int *sequence_number);
int NGATCil_Cil_Autoguide_On_Rank_Reply_Send(int socket_id,int rank,int status,int sequence_number);

int NGATCil_Cil_Autoguide_Off_Send(int socket_id,int *sequence_number);
int NGATCil_Cil_Autoguide_Off_Parse(struct NGATCil_Cil_Packet_Struct packet,int *status,int *sequence_number);

int NGATCil_Cil_Autoguide_Off_Reply_Send(int socket_id,int status,int sequence_number);
int NGATCil_Cil_Autoguide_Off_Reply_Parse(struct NGATCil_Cil_Packet_Struct packet,int *status,int *sequence_number);

/*
** $Log: not supported by cvs2svn $
** Revision 1.3  2006/06/29 17:04:24  cjm
** Added Rank/Brightest AG code.
**
** Revision 1.2  2006/06/12 19:26:14  cjm
** Added more #defines/enums.
**
** Revision 1.1  2006/06/06 15:58:03  cjm
** Initial revision
**
*/
#endif
