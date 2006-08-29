/* ngatcil_udp_raw.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_udp_raw.h,v 1.2 2006-08-29 14:12:06 cjm Exp $
*/
#ifndef NGATCIL_UDP_RAW_H
#define NGATCIL_UDP_RAW_H

extern int NGATCil_UDP_Open(char *hostname,int port_number,int *socket_id);
extern int NGATCil_UDP_Raw_To_Network_Byte_Order(void *message_buff,size_t message_buff_len);
extern int NGATCil_UDP_Raw_Send(int socket_id,void *message_buff,size_t message_buff_len);
extern int NGATCil_UDP_Raw_Send_To(int socket_fd,char *hostname,int port_number,
				   void *message_buff,size_t message_buff_len);
extern int NGATCil_UDP_Raw_Recv(int socket_id,void *message_buff,size_t message_buff_len);
extern int NGATCil_UDP_Close(int socket_id);
extern int NGATCil_UDP_Server_Start(int port_number,size_t message_length,int *socket_id,
			     int (*connection_handler)(int socket_id,void *message_buff,int message_length));

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/01 15:28:10  cjm
** Initial revision
**
*/
#endif
