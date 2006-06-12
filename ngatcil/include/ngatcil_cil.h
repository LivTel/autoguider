/* ngatcil_cil.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_cil.h,v 1.2 2006-06-12 19:26:14 cjm Exp $
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

int NGATCil_Cil_Autoguide_Off_Send(int socket_id,int *sequence_number);
int NGATCil_Cil_Autoguide_Off_Parse(struct NGATCil_Cil_Packet_Struct packet,int *status,int *sequence_number);

int NGATCil_Cil_Autoguide_Off_Reply_Send(int socket_id,int status,int sequence_number);
int NGATCil_Cil_Autoguide_Off_Reply_Parse(struct NGATCil_Cil_Packet_Struct packet,int *status,int *sequence_number);

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/06 15:58:03  cjm
** Initial revision
**
*/
#endif
