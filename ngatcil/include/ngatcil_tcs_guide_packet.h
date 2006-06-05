/* ngatcil_tcs_guide_packet.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_tcs_guide_packet.h,v 1.2 2006-06-05 18:56:41 cjm Exp $
*/
#ifndef NGATCIL_TCS_GUIDE_PACKET_H
#define NGATCIL_TCS_GUIDE_PACKET_H

/**
 * Status value to send in a guide packet. This means the autoguider failed to find a guide star.
 */
#define NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED    ('F')
/**
 * Status value to send in a guide packet. This means the guide star is too close to the edge of the guide window.
 */
#define NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW    ('W')

/**
 * Default machine name to send guide packets to (tcc). Should be mapped to 192.168.1.10 in /etc/hosts.
 */
#define NGATCIL_TCS_GUIDE_PACKET_TCC_DEFAULT      ("tcc")
/**
 * Default port number to send guide packets to (13025). 
 */
#define NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT     (13025)
/**
 * Default length of guide packets. 
 */
#define NGATCIL_TCS_GUIDE_PACKET_LENGTH           (34)


extern int NGATCil_TCS_Guide_Packet_Open_Default(int *socket_id);
extern int NGATCil_TCS_Guide_Packet_Send(int socket_id,float x_pos,float y_pos,
					 int timecode_terminating,int timecode_unreliable,
					 float timecode_secs,char status_char);
extern int NGATCil_TCS_Guide_Packet_Recv(int socket_id,float *x_pos,float *y_pos,
					 int *timecode_terminating,int *timecode_unreliable,
					 float *timecode_secs,char *status_char);
extern int NGATCil_TCS_Guide_Packet_Parse(void *packet_buff,int packet_buff_length,float *x_pos,float *y_pos,
					  int *timecode_terminating,int *timecode_unreliable,
					  float *timecode_secs,char *status_char);
extern char *NGATCil_TCS_Guide_Packet_To_String(void *packet_buff,int packet_buff_length);
/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/01 15:28:10  cjm
** Initial revision
**
*/
#endif
