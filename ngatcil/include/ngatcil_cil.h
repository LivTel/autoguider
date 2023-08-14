/* ngatcil_cil.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_cil.h,v 1.5 2006-08-29 14:12:06 cjm Exp $
*/
#ifndef NGATCIL_CIL_H
#define NGATCIL_CIL_H
#include <stdint.h>

/* hash defines */
/**
 * Length of Base CIL packet.
 * @see #NGATCil_Cil_Packet_Struct
 */
#define NGATCIL_CIL_BASE_PACKET_LENGTH  (28)
/**
 * Length of AGS CIL packet.
 * @see #NGATCil_Ags_Packet_Struct
 */
#define NGATCIL_CIL_AGS_PACKET_LENGTH   (44)
/**
 * Maximum Length of packet the AGS can receive. See I_AGS_CIL_DATALEN, iAgsReceiveMessage in AgsMain.c.
 */
#define NGATCIL_CIL_AGS_MAX_PACKET_LENGTH (256)
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
 * Default machine name to send CHB (continuous heart beat) replies to (scc).
 * See Cil.map.
 */
#define NGATCIL_CIL_CHB_HOSTNAME_DEFAULT   ("scc")
/**
 * Default port number to send CHB (continuous heart beat) replies on (13002).
 * See Cil.map.
 */
#define NGATCIL_CIL_CHB_PORT_DEFAULT (13002)
/**
 * Default machine name to send MCP (master control process) replies to (scc).
 * See Cil.map.
 */
#define NGATCIL_CIL_MCP_HOSTNAME_DEFAULT   ("scc")
/**
 * Default port number to send MCP (master control process) replies on (13001).
 * See Cil.map.
 */
#define NGATCIL_CIL_MCP_PORT_DEFAULT (13001)

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
 * Used in CIL packets, Class field, for acknowledge class packets.
 * Derived from Cil.h,eCilClass.
 */
#define E_CIL_ACK_CLASS           (3)
/**
 * Used in CIL packets, Class field, for acted upon class packets.
 * Derived from Cil.h,eCilClass.
 */
#define E_CIL_ACT_CLASS           (4)
/**
 * Used in CIL packets, Class field, for completed class packets.
 * Derived from Cil.h,eCilClass.
 */
#define E_CIL_COM_CLASS           (5)
/**
 * Used in CIL packets, Class field, for error class packets.
 * Derived from Cil.h,eCilClass.
 */
#define E_CIL_ERR_CLASS           (6)
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
 * Used in CIL packets from the TCS, Command field, to tell the AGS to start a session.
 * Derived from Ags.h
 */
#define E_AGS_START_SESSION       (10)
/**
 * Used in CIL packets from the TCS, Command field, to tell the AGS to end a session.
 * Derived from Ags.h
 */
#define E_AGS_END_SESSION         (11)
/**
 * Used in CHB hearbeat messages as the Service.
 * Value from TtlSystem.h.
 */
#define E_MCP_HEARTBEAT      255       /* heartbeat message */
/**
 * Used in MCP state change messages as the Service.
 * Value from TtlSystem.h.
 */
#define E_MCP_SHUTDOWN       254       /* shutdown message */
/**
 * Used in MCP state change messages as the Service.
 * Value from TtlSystem.h.
 */
#define E_MCP_SAFESTATE      253       /* safe-state message */
/**
 * Used in MCP state change messages as the Service.
 * Value from TtlSystem.h.
 */
#define E_MCP_ACTIVATE       252       /* activate message from safe-state */
/**
 * Used in CIL packets, Status field, to indicate all is well.
 * Derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 * According to the latest TtlSystem.h, it is STATUS_START(SYS), where STATUS_START is
 * #define STATUS_START(x) ((x) << 16) and SYS is 0x0001
 */
#define SYS_NOMINAL               (0x10000)
/**
 * According to folklore, TTL timestamps are not based on Unix time (1st Jan 1970) but on
 * 5th Jan 1980 00:00:00. Accordingly, this offset is applied, based on "date -d "Jan 5 00:00:00 +00 1980" "+%s""
 * returning 315878400.
 * But apparently not, according to test_cil_server TTL timestamps are based on Unix time (1st Jan 1970).
 */
/* define TTL_TIMESTAMP_OFFSET (315878400)*/
#define TTL_TIMESTAMP_OFFSET (0)

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

/**
 * Process state definitions.
 * The various processes of the telescope system maintain a concept of
 * their own "state". This is used internally, but is also reported to
 * other parts of the system. In order to provide a common form to
 * specify these states, the following enumerated list is provided.
 * Because these states are part of the interprocess communications,
 * the list should only have additional states added to the end of it,
 * and not inserted in the middle.
 * Note also that these are process states and not other states (e.g.
 * mechanisms). They are to be used to determine if a process is
 * operating, rather than how a system is functioning, for the
 * purposes of MCP control of software recovery.
 * The SYS_OFF_STATE and SYS_TIMEOUT_STATE are not set by the process, 
 * but may be set by the MCP. These are inferred states. 
 * The SYS_INVALID_STATE is never explicitly set. It is to prevent an 
 * uninitialised state variable from having a valid state on creation.
 * All states must be explicitly set, and this guards against unset cases.
 *  TtlSystem.h
 */
enum eSysProcState_e
{
   SYS_INVALID_STATE = 0,       /* No state has been set (uninitialised). */
   SYS_OKAY_STATE,     /* The process is operating normally.              */
   SYS_INIT_STATE,     /* The process is performing its initial start-up  */
                       /* or awaiting initialisation commands to take the */
                       /* process to SYS_OKAY_STATE (if appropriate).     */
   SYS_STANDBY_STATE,  /* This state is used for a process or the system  */
                       /* when software is running, but not yet ready for */
                       /* full operation. To move from this state to OKAY */
                       /* will require some action to be taken. For       */
                       /* example, axis software will attain this state   */
                       /* until the axis is homed.                        */
   SYS_WARN_STATE,     /* A problem has occurred, but no automatic        */
                       /* intervention is required from the MCP. The      */
                       /* telescope may still be used in this state, but  */
                       /* there is a possibility that operational         */
                       /* performance may be degraded.                    */
   SYS_FAILED_STATE,   /* A problem has occurred with the process that    */
                       /* requires intervention by the MCP.               */
   SYS_SAFE_STATE,     /* The process has ceased normal operation (maybe  */
                       /* only temporarily) and is either about to        */
                       /* terminate itself or be able to be terminated by */
                       /* the MCP/system without risk of hardware damage  */
                       /* or serious data loss.                           */
   SYS_OFF_STATE,      /* The process is not running. THIS IS AN INFERRED */
                       /* STATE which is set by the MCP only.             */
   SYS_TIMEOUT_STATE,  /* The process is not responding to the MCP heart- */
                       /* beat messages. THIS IS AN INFERRED STATE which  */
                       /* is set by the MCP only.                         */
   SYS_SUSPEND_STATE   /* This is the usual state of a process or the     */         
                       /* system when it is awaiting the clearing of an   */
                       /* external condition which prevents normal        */
                       /* operation. The process is performing monitoring */
                       /* and reporting duties, but it will not accept    */
                       /* operating instructions (e.g. motion commands).  */
};

/* data types */
/**
 * Structure defining packet contents of a base CIL command/reply packet.
 * All the integers should be sent over UDP in network byte order (htonl).
 * Packet contents are derived from "Liverpool 2.0m Telescope, Communications Interface Library User Guide,
 * Version 0.16, 21st May 2001".
 * <dl>
 * <dt>Source_Id</dt> <dd></dd>
 * <dt>DestId</dt> <dd></dd>
 * <dt>Class</dt> <dd></dd>
 * <dt>Service</dt> <dd></dd>
 * <dt>Seq_Num</dt> <dd>Unsigned int</dd>
 * <dt>Timestamp_Seconds</dt> <dd></dd>
 * <dt>Timestamp_Nanoseconds</dt> <dd></dd>
 * </dl>
 */
struct NGATCil_Cil_Packet_Struct
{
	int32_t Source_Id;
	int32_t Dest_Id;
	int32_t Class;
	int32_t Service;
	uint32_t Seq_Num;
	int32_t Timestamp_Seconds;
	int32_t Timestamp_Nanoseconds;
};

/**
 * Structure defining packet contents of a AGS CIL command/reply packet.
 * All the integers should be sent over UDP in network byte order (htonl).
 * Packet contents are derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005".
 * NB Assumes sizeof(int) == 32, should perhaps replace with Int32_t.
 * <dl>
 * <dt>Cil_Base</dt> <dd>NGATCil_Cil_Packet_Struct, base Cil details.</dd>
 * <dt>Command</dt> <dd></dd>
 * <dt>Status</dt> <dd></dd>
 * <dt>Param1</dt> <dd></dd>
 * <dt>Param2</dt> <dd></dd>
 * </dl>
 * @see #NGATCil_Cil_Packet_Struct
 */
struct NGATCil_Ags_Packet_Struct
{
	struct NGATCil_Cil_Packet_Struct Cil_Base;
	int32_t Command;
	int32_t Status;
	int32_t Param1;
	int32_t Param2;
};

/**
 * Structure defining packet contents of a AGS CIL reply packet, where the user data is just a status word.
 * Used for CHB reply packets. Used for TCS START_SESSION/END_SESSION reply packets.
 * Used for MCP COM reply packets.
 * All the integers should be sent over UDP in network byte order (htonl).
 * Packet contents are derived from AGS source code.
 * NB Assumes sizeof(int) == 32, should perhaps replace with Int32_t.
 * <dl>
 * <dt>Cil_Base</dt> <dd>NGATCil_Cil_Packet_Struct, base Cil details.</dd>
 * <dt>Status</dt> <dd></dd>
 * </dl>
 * @see #NGATCil_Cil_Packet_Struct
 */
struct NGATCil_Status_Reply_Packet_Struct
{
	struct NGATCil_Cil_Packet_Struct Cil_Base;
	int32_t Status;
};

/**
 * Structure defining packet contents of a AGS CIL TCS generic reply packet.
 * Used for START_SESSION/END_SESSION TCS replies.
 * All the integers should be sent over UDP in network byte order (htonl).
 * Packet contents are derived from AGS source code.
 * NB Assumes sizeof(int) == 32, should perhaps replace with Int32_t.
 * <dl>
 * <dt>Cil_Base</dt> <dd>NGATCil_Cil_Packet_Struct, base Cil details.</dd>
 * <dt>Status</dt> <dd></dd>
 * </dl>
 * @see #NGATCil_Cil_Packet_Struct
 */
struct NGATCil_Tcs_Reply_Packet_Struct
{
	struct NGATCil_Cil_Packet_Struct Cil_Base;
	int32_t Status;
};

int NGATCil_Cil_Packet_Create(int source_id,int dest_id,int class,int service,int seq_num,int command,
			      int status,int param1, int param2,struct NGATCil_Ags_Packet_Struct *packet);
int NGATCil_Cil_Packet_Send_To(int socket_id,char *hostname,int port_number,struct NGATCil_Ags_Packet_Struct packet);
int NGATCil_Cil_Packet_Recv(int socket_id,struct NGATCil_Ags_Packet_Struct *packet);

int NGATCil_Cil_Autoguide_On_Pixel_Send(int socket_id,char *hostname,int port_number,float pixel_x,float pixel_y,
					int *sequence_number);
int NGATCil_Cil_Autoguide_On_Pixel_Parse(struct NGATCil_Ags_Packet_Struct packet,float *pixel_x,float *pixel_y,
					 int *sequence_number);

int NGATCil_Cil_Autoguide_On_Pixel_Reply_Send(int socket_id,char *hostname,int port_number,float pixel_x,float pixel_y,
					      int status,int sequence_number);
int NGATCil_Cil_Autoguide_On_Pixel_Reply_Parse(struct NGATCil_Ags_Packet_Struct packet,float *pixel_x,float *pixel_y,
					       int *status,int *sequence_number);

int NGATCil_Cil_Autoguide_On_Brightest_Parse(struct NGATCil_Ags_Packet_Struct packet,int *sequence_number);
int NGATCil_Cil_Autoguide_On_Brightest_Reply_Send(int socket_id,char *hostname,int port_number,int status,
						  int sequence_number);

int NGATCil_Cil_Autoguide_On_Rank_Parse(struct NGATCil_Ags_Packet_Struct packet,int *rank,int *sequence_number);
int NGATCil_Cil_Autoguide_On_Rank_Reply_Send(int socket_id,char *hostname,int port_number,int rank,int status,
					     int sequence_number);

int NGATCil_Cil_Autoguide_Off_Send(int socket_id,char *hostname,int port_number,int *sequence_number);
int NGATCil_Cil_Autoguide_Off_Parse(struct NGATCil_Ags_Packet_Struct packet,int *status,int *sequence_number);

int NGATCil_Cil_Autoguide_Off_Reply_Send(int socket_id,char *hostname,int port_number,int status,int sequence_number);
int NGATCil_Cil_Autoguide_Off_Reply_Parse(struct NGATCil_Ags_Packet_Struct packet,int *status,int *sequence_number);

/*
** $Log: not supported by cvs2svn $
** Revision 1.4  2006/07/20 15:16:01  cjm
** Added eCilNames.
**
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
