/* ngatcil_cil.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_cil.h,v 1.1 2006-06-06 15:58:03 cjm Exp $
*/
#ifndef NGATCIL_CIL_H
#define NGATCIL_CIL_H

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
*/
#endif
