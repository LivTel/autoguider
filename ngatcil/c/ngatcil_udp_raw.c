/* ngatcil_udp_raw.c
** NGATCil UDP raw transmission routines
** $Header: /home/cjm/cvs/autoguider/ngatcil/c/ngatcil_udp_raw.c,v 1.11 2012-03-05 11:20:50 cjm Exp $
*/
/**
 * NGAT Cil library raw UDP packet transmission.
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
/**
 * Define BSD Source to get BSD prototypes, including gethostbyname_r.
 */
#define _BSD_SOURCE (1)

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h> /* htons etc */
#include <sys/types.h>
#include <sys/socket.h>
#include "log_udp.h"
#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"

/* hash defines */

/* data types */
/**
 * Server context structure holding data to be passed into threads started by NGATCil_UDP_Server_Start.
 * <dl>
 * <dt>Socket_Id</dt> <dd>The server socket to recv on.</dd>
 * <dt>Message_Length</dt> <dd>The length of message to receive.</dd>
 * <dt>Connection_Handler</dt> <dd>The connection handler to call for each packet received.</dd>
 * </dl>
 * @see #NGATCil_UDP_Server_Start
 */
struct UDP_Raw_Server_Context_Struct
{
	int Socket_Id;
	size_t Message_Length;
	int (*Connection_Handler)(int socket_id,void *message_buff,int message_length);
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ngatcil_udp_raw.c,v 1.11 2012-03-05 11:20:50 cjm Exp $";

/* internal function declaration */
static void *UDP_Raw_Server_Thread(void *);
static int Get_Host_By_Name(const char *name,in_addr_t *host_addr_zero);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Routine to open a UDP socket and connect the default endpoint to a specified host/port.
 * @param hostname The hostname the socket will talk to, either numeric or via /etc/hosts.
 * @param port_number The port number to send to in host (normal) byte order.
 * @param socket_id The address of an integer to store the created socket file descriptor.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #Get_Host_By_Name
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_UDP_Open(char *hostname,int port_number,int *socket_id)
{
	int socket_errno,retval;
	unsigned short int network_port_number;
	in_addr_t saddr;
	struct hostent *host_entry;
	struct sockaddr_in server;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Open",
				   LOG_VERBOSITY_VERBOSE,NULL,"NGATCil_UDP_Open(%s,%d):started.",
				   hostname,port_number);
#endif
	if(socket_id == NULL)
	{
		NGATCil_General_Error_Number = 102;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Open:socket_is was NULL.");
		return FALSE;
	}
	/* open datagram socket */
	(*socket_id) = socket(AF_INET,SOCK_DGRAM,0);
	if((*socket_id) < 0)
	{
		socket_errno = errno;
		NGATCil_General_Error_Number = 103;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Open:Failed to create socket (%d:%s).",socket_errno,
			strerror(socket_errno));
		return FALSE;
	}
	/* convert port number to network short */
	network_port_number = htons((short)port_number);
	/* try to convert hostname to address in network byte order */
	/* try numeric address conversion first */
	saddr = inet_addr(hostname);
	if(saddr == INADDR_NONE)
	{
#if NGATCIL_DEBUG > 5
		NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Open",
					   LOG_VERBOSITY_VERBOSE,NULL,
					   "inet_addr didn't work:trying gethostbyname(%s).",
					   hostname);
#endif
		/* try getting by hostname instead */
		if(!Get_Host_By_Name(hostname,&saddr))
		{
			shutdown((*socket_id),SHUT_RDWR);
			(*socket_id) = 0;
			NGATCil_General_Error_Number = 104;
			sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Open:Failed to get host address from (%s).",
				hostname);
			return FALSE;
		}
	}
	/* set up server socket */
	memset((char *) &server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = saddr;
	server.sin_port = network_port_number;
	retval = connect((*socket_id),(struct sockaddr *)&server,sizeof(server));
	if(retval < 0)
	{
		socket_errno = errno;
		shutdown((*socket_id),SHUT_RDWR);
		/*close((*socket_id));*/
		(*socket_id) = 0;
		NGATCil_General_Error_Number = 105;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Open:Failed to connect (%d:%s).",
			socket_errno,strerror(socket_errno));
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Open",
				   LOG_VERBOSITY_VERBOSE,NULL,
				   "returned socket %d:finished.",(*socket_id));
#endif
	return TRUE;
}

/**
 * Change the specified host ordered buffer of ints into network byte order
 * @param message_buf A pointer to an area of memory containing the message change byte order.
 * @param message_buff_len The size of the message to send, in bytes.
 * @return The routine returns TRUE on success, and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_UDP_Raw_To_Network_Byte_Order(void *message_buff,size_t message_buff_len)
{
	int *message_buff_int_ptr = NULL;
	int retval,int_count,i;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_To_Network_Byte_Order",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "started:buffer length = %d.",message_buff_len);
#endif
	if(message_buff == NULL)
	{
		NGATCil_General_Error_Number = 124;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_To_Network_Byte_Order:message_buff was NULL.");
		return FALSE;
	}
	if((message_buff_len % sizeof(int)) != 0)
	{
		NGATCil_General_Error_Number = 125;
		sprintf(NGATCil_General_Error_String,
			"NGATCil_UDP_Raw_To_Network_Byte_Order:message_buff_len %d was not a whole number of ints.",
			message_buff_len);
		return FALSE;
	}
	int_count = message_buff_len/sizeof(int);
	message_buff_int_ptr = (int*)message_buff;
	for(i=0;i<int_count;i++)
	{
		message_buff_int_ptr[i] = htonl(message_buff_int_ptr[i]);
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_To_Network_Byte_Order",
			    LOG_VERBOSITY_VERY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Send the specified data over the specified socket.
 * @param socket_id A previously opened and connected socket to send the buffer over.
 * @param message_buf A pointer to an area of memory containing the message to send.
 * @param message_buff_len The size of the message to send, in bytes.
 * @return The routine returns TRUE on success, and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_UDP_Raw_Send(int socket_id,void *message_buff,size_t message_buff_len)
{
	int retval,send_errno;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_Send",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "started(socket=%d,length=%d).",socket_id,message_buff_len);
#endif
	if(message_buff == NULL)
	{
		NGATCil_General_Error_Number = 106;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Send:message_buff was NULL.");
		return FALSE;
	}
	retval = send(socket_id,message_buff,message_buff_len,0);
	if(retval < 0)
	{
		send_errno = errno;
		NGATCil_General_Error_Number = 100;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Send:"
			"Send failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	if(retval != message_buff_len)
	{
		NGATCil_General_Error_Number = 101;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Send:"
			"Send returned %d vs %d.",retval,message_buff_len);
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_Send",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"finished(%d).",socket_id);
#endif
	return TRUE;
}

/**
 * Send the specified data over the specified socket to the specified endpoint.
 * @param socket_fd A previously opened socket to send the buffer over. It need not have been connected.
 * @param hostname The hostname the socket will talk to, either numeric or via /etc/hosts.
 * @param port_number The port number to send to in host (normal) byte order.
 * @param message_buf A pointer to an area of memory containing the message to send.
 * @param message_buff_len The size of the message to send, in bytes.
 * @return The routine returns TRUE on success, and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #Get_Host_By_Name
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_UDP_Raw_Send_To(int socket_fd,char *hostname,int port_number,void *message_buff,size_t message_buff_len)
{
	int retval,send_errno;
	in_addr_t saddr;
	struct sockaddr_in to_addr;
	struct hostent *host_entry;
	size_t to_len = sizeof(to_addr);

	if(hostname == NULL)
	{
		NGATCil_General_Error_Number = 119;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Send_To:message_buff was NULL.");
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_Send_To",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "started(socket=%d,hostname=%s,port_number=%d,length=%d).",
				   socket_fd,hostname,port_number,message_buff_len);
#endif
	if(message_buff == NULL)
	{
		NGATCil_General_Error_Number = 120;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Send_To:message_buff was NULL.");
		return FALSE;
	}
	/* try to convert hostname to address in network byte order */
	/* try numeric address conversion first */
	saddr = inet_addr(hostname);
	if(saddr == INADDR_NONE)
	{
#if NGATCIL_DEBUG > 10
		NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_Send_To",
					   LOG_VERBOSITY_VERY_VERBOSE,NULL,
					   "inet_addr didn't work:trying gethostbyname(%s).",hostname);
#endif
		/* try getting by hostname instead */
		if(!Get_Host_By_Name(hostname,&saddr))
		{
			NGATCil_General_Error_Number = 121;
			sprintf(NGATCil_General_Error_String,
				"NGATCil_UDP_Raw_Send_To:Failed to get host address from (%s).",hostname);
			return FALSE;
		}
	}
	/* Formulate the socket address */
	to_addr.sin_family = AF_INET;
	to_addr.sin_port = htons((short)port_number);
	to_addr.sin_addr.s_addr = saddr;
#if NGATCIL_DEBUG > 7
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_Send_To",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"sendto(%d,%p,%d,0,{%s},%d).",
				   socket_fd,message_buff,message_buff_len,inet_ntoa(to_addr.sin_addr),to_len);
#endif
	retval = sendto(socket_fd,message_buff,message_buff_len,0,(void*)&to_addr,to_len);
	if(retval < 0)
	{
		send_errno = errno;
		NGATCil_General_Error_Number = 122;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Send_To:"
			"Sendto failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	if(retval != message_buff_len)
	{
		NGATCil_General_Error_Number = 123;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Send_To:"
			"Sendto returned %d vs %d.",retval,message_buff_len);
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_Send_To",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"finished(%d).",socket_fd);
#endif
	return TRUE;
}

/**
 * Get some data from the specified socket. This routine blocks until a message arrives, 
 * if the socket is <b>not</b> set to nonblocking.
 * @param socket_id A previously opened and connected socket to get the buffer from.
 * @param message_buf A pointer to an area of memory of size message_buff_len bytes, to put the received message.
 * @param message_buff_len The size of the message to receive, in bytes.
 * @return The routine returns TRUE on success, and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_UDP_Raw_Recv(int socket_id,void *message_buff,size_t message_buff_len)
{
	int retval,send_errno;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_Recv",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"started(%d).",socket_id);
#endif
	if(message_buff == NULL)
	{
		NGATCil_General_Error_Number = 107;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Recv:message_buff was NULL.");
		return FALSE;
	}
	retval = recv(socket_id,message_buff,message_buff_len,0);
	if(retval < 0)
	{
		send_errno = errno;
		NGATCil_General_Error_Number = 108;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Recv:"
			"Recv failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	if(retval != message_buff_len)
	{
		NGATCil_General_Error_Number = 109;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Raw_Recv:"
			"Recv returned %d vs %d.",retval,message_buff_len);
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Raw_Recv",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"finished(%d).",socket_id);
#endif
	return TRUE;
}

/**
 * Close a previously opened UDP socket.
 * @param socket_id The socket descriptor.
 * @return The routine returns TRUE on success, and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_UDP_Close(int socket_id)
{
	int retval,socket_errno;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Close",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"started(%d).",socket_id);
#endif
	retval = shutdown(socket_id,SHUT_RDWR);
	if(retval < 0)
	{
		socket_errno = errno;
		NGATCil_General_Error_Number = 110;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Close:"
			"Close failed (%d,%d:%s).",retval,socket_errno,strerror(socket_errno));
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Close",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"finished(%d).",socket_id);
#endif
	return TRUE;
}

/**
 * Routine to start a UDP server socket.
 * @param port_number The port number to send to in host (normal) byte order.
 * @param message_length The size of the messages we expect to receive in bytes.
 * @param socket_id The address of an integer to store the socket descriptor.
 * @param connection_handler A function pointer to call when a message is received on the UDP port.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #UDP_Raw_Server_Thread
 * @see #UDP_Raw_Server_Context_Struct
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 */
int NGATCil_UDP_Server_Start(int port_number,size_t message_length,int *socket_id,
			     int (*connection_handler)(int socket_id,void *message_buff,int message_length))
{
	struct UDP_Raw_Server_Context_Struct *server_context = NULL;
	struct sockaddr_in server;
	pthread_t new_thread;
	pthread_attr_t attr;
	unsigned short int network_port_number;
	int retval,socket_errno;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Server_Start",
				   LOG_VERBOSITY_INTERMEDIATE,NULL,"started(%d).",port_number);
#endif
	if(connection_handler == NULL)
	{
		NGATCil_General_Error_Number = 111;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Server_Start:connection_handler was NULL.");
		return FALSE;
	}
	if(socket_id == NULL)
	{
		NGATCil_General_Error_Number = 112;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Server_Start:socket_id was NULL.");
		return FALSE;
	}
	/* convert port number to network short */
	network_port_number = htons((short)port_number);
	/* create socket id*/
	(*socket_id) = socket(AF_INET, SOCK_DGRAM, 0);
	if((*socket_id) < 0)
	{
		socket_errno = errno;
		NGATCil_General_Error_Number = 113;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Server_Start:Failed to create socket (%d:%s).",
			socket_errno,strerror(socket_errno));
		return FALSE;
	}
	/* bind server socket */
	memset((char *) &server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = network_port_number;
	retval = bind((*socket_id),(struct sockaddr *)&server,sizeof(server));
	if(retval < 0)
	{
		socket_errno = errno;
		shutdown((*socket_id),SHUT_RDWR);
		NGATCil_General_Error_Number = 114;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Server_Start:bind failed (%d:%s).",
			socket_errno,strerror(socket_errno));
		return FALSE;
	}
	/* setup server_context */
	server_context = (struct UDP_Raw_Server_Context_Struct*)malloc(sizeof(struct UDP_Raw_Server_Context_Struct));
	if(server_context == NULL)
	{
		shutdown((*socket_id),SHUT_RDWR);
		NGATCil_General_Error_Number = 115;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Server_Start:malloc failed.");
		return FALSE;
	}
	server_context->Socket_Id = (*socket_id);
	server_context->Message_Length = message_length;
	server_context->Connection_Handler = connection_handler;
	/* start a thread to receive packets and pass them onto the connection handler */
	/* create the thread with detatched attrributes */
	/* these next two should really be error checked */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	retval = pthread_create(&new_thread,&attr,&UDP_Raw_Server_Thread,
				(void *)server_context);
	if(retval != 0)
	{
		shutdown((*socket_id),SHUT_RDWR);
		free(server_context);
		NGATCil_General_Error_Number = 116;
		sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Server_Start:Failed to create server thread(%d).",
			retval);
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","NGATCil_UDP_Server_Start",
				   LOG_VERBOSITY_INTERMEDIATE,NULL,"finished(%d).",port_number);
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Thread started to handle UDP packets arriving on the server socket setup by NGATCil_UDP_Server_Start.
 * @param arg The thread argument, should be set to the server context.
 * @return This routine always returns NULL.
 */
static void *UDP_Raw_Server_Thread(void *arg)
{
	struct UDP_Raw_Server_Context_Struct *server_context = NULL;
	struct sockaddr_in client;
	void *message_buff = NULL;
	int *int_message_ptr = NULL;
	int done,retval,socket_errno,current_client_length,recv_message_length,i;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
			    LOG_VERBOSITY_VERY_VERBOSE,NULL,"started.");
#endif
	server_context = (struct UDP_Raw_Server_Context_Struct *)arg;
	if(server_context == NULL)
	{
		NGATCil_General_Error_Number = 117;
		sprintf(NGATCil_General_Error_String,"UDP_Raw_Server_Thread:server_context was NULL.");
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"listening on socket %d.",
				   server_context->Socket_Id);
#endif
	/* allocate recv buffer */
	message_buff = (void *)malloc(server_context->Message_Length*sizeof(char));
	if(message_buff == NULL)
	{
		NGATCil_General_Error_Number = 118;
		sprintf(NGATCil_General_Error_String,"UDP_Raw_Server_Thread:failed to allocate message buffer(%d).",
			server_context->Message_Length);
		return FALSE;
	}
	/* keep receiving packets until done */
	done = FALSE;
	while(done == FALSE)
	{
#if NGATCIL_DEBUG > 9
		NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
					   LOG_VERBOSITY_VERY_VERBOSE,NULL,
					   "Waiting for UDP packet on socket %d.",server_context->Socket_Id);
#endif
		current_client_length = sizeof(client);
		recv_message_length = recvfrom(server_context->Socket_Id,message_buff,server_context->Message_Length,0,
				  (struct sockaddr *)&client,&current_client_length);
		if(recv_message_length < 0)
		{
			socket_errno = errno;
			/* quit server - hopefully error due to closing socket? */
			done = TRUE;
#if NGATCIL_DEBUG > 9
			NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
						   LOG_VERBOSITY_VERY_VERBOSE,NULL,"Terminating:"
						   "recvfrom returned %d (%d:%s).",recv_message_length,socket_errno,
						   strerror(socket_errno));
#endif
		}
		else /* something was received */
		{
			if(recv_message_length != server_context->Message_Length)
			{
#if NGATCIL_DEBUG > 9
				NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
							   LOG_VERBOSITY_VERY_VERBOSE,NULL,
							   "Detected short packet (%d vs %d).",recv_message_length,
							   server_context->Message_Length);
#endif
				/* we seem to get 0 length packets when the server socket is closed */
				if(recv_message_length == 0)
				{
					done = TRUE;
#if NGATCIL_DEBUG > 3
					NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c",
								   "UDP_Raw_Server_Thread",
								   LOG_VERBOSITY_VERY_VERBOSE,NULL,
								   "Zero length packet:"
								   "socket closed?:quiting server thread.");
#endif
				}
			}
			/* network to host */
			if((recv_message_length % sizeof(int)) == 0)
			{
#if NGATCIL_DEBUG > 9
				NGATCil_General_Log("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
						    LOG_VERBOSITY_VERY_VERBOSE,NULL,
						    "Translating packet contents from network to host byte order.");
#endif
				int_message_ptr = (int*)message_buff;
				for(i=0;i < (recv_message_length/sizeof(int));i++)
				{
					int_message_ptr[i] = ntohl(int_message_ptr[i]);
				}
				if(server_context->Connection_Handler != NULL)
				{
					(server_context->Connection_Handler)(server_context->Socket_Id,message_buff,
									     recv_message_length);
				}
				else
				{
#if NGATCIL_DEBUG > 3
					NGATCil_General_Log("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
							    LOG_VERBOSITY_VERY_VERBOSE,NULL,
							 "Packet received but Connection Handler appears to be NULL.");
#endif
				}
			} /* if */
			else
			{
#if NGATCIL_DEBUG > 3
				NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
							   LOG_VERBOSITY_VERY_VERBOSE,NULL,
							   "Packet received with non-word number of bytes %d.",
							   recv_message_length);
#endif
			}
		}/* if */
	}/* while */
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"shutdown on socket %d.",
				   server_context->Socket_Id);
#endif
	shutdown(server_context->Socket_Id,SHUT_RDWR);
	free(server_context);
	free(message_buff);
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","UDP_Raw_Server_Thread",
				   LOG_VERBOSITY_VERY_VERBOSE,NULL,"finished.");
#endif
	return NULL;
}

/**
 * Internal routine to get a host address from it's name. This is traditionally handled by a call
 * to gethostbyname. Unfortunately that routine is not re-entrant because the pointer it returns
 * is to a block of reusable memory in glibc, so a second call to gethostbyname from another thread
 * in the process can lead to the pointers returned from the first call being freed leading to SIGSEGV.
 * This routine wraps gethostbyname_r, the re-entrant version of that routine.
 * @param name The hostname to translate. This should be allocated, zero-terminated and non-null.
 * @param host_addr_zero The address of a in_addr_t (actually an unsigned 32 bit int in Linux).
 *       On return filled with a null-terminated, network byte ordered copy of the first hostent host address
 *       list entry returned by gethostbyname_r. NULL can be returned on failure.
 * @return The routine returns TRUE on success and FALSE on failure.
 */
static int Get_Host_By_Name(const char *name,in_addr_t *host_addr_zero)
{
	struct hostent hostbuf,*hp = NULL;
	size_t hstbuflen;
	char *tmphstbuf = NULL;
	int retval;
	int herr;

	NGATCil_General_Error_Number = 0;
	if(name == NULL)
	{
		NGATCil_General_Error_Number = 126;
		sprintf(NGATCil_General_Error_String,"Get_Host_By_Name:name was NULL.");
		return FALSE;
	}
	if(host_addr_zero == NULL)
	{
		NGATCil_General_Error_Number = 127;
		sprintf(NGATCil_General_Error_String,"Get_Host_By_Name:host_addr_zero was NULL.");
		return FALSE;
	}
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","Get_Host_By_Name",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "Get_Host_By_Name(%s) Started.",name);
#endif /* NGATCIL_DEBUG */
	hstbuflen = 1024;
	/* Allocate buffer, remember to free it to avoid memory leakage.  */
	tmphstbuf = malloc(hstbuflen);
	if(tmphstbuf == NULL)
	{
		NGATCil_General_Error_Number = 128;
		sprintf(NGATCil_General_Error_String,"Get_Host_By_Name:memory allocation of tmphstbuf failed(%d).",
			hstbuflen);
		return FALSE;

	}
	while((retval = gethostbyname_r(name,&hostbuf,tmphstbuf,hstbuflen,&hp,&herr)) == ERANGE)
	{
		/* Enlarge the buffer.  */
		hstbuflen *= 2;
		tmphstbuf = realloc(tmphstbuf, hstbuflen);
		/* check realloc succeeds */
		if(tmphstbuf == NULL)
		{
			NGATCil_General_Error_Number = 129;
			sprintf(NGATCil_General_Error_String,
				"Get_Host_By_Name:memory reallocation of tmphstbuf failed(%d).",hstbuflen);
			return FALSE;
		}
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","Get_Host_By_Name",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "Get_Host_By_Name:gethostbyname_r returned ERANGE:Increasing buffer size to %d.",
				   hstbuflen);
#endif /* NGATCIL_DEBUG */
	}/* while */
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","Get_Host_By_Name",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "Get_Host_By_Name:gethostbyname_r loop exited with retval %d and hp %p.",retval,hp);
#endif /* NGATCIL_DEBUG */
	if(retval != 0)
	{
		if(tmphstbuf != NULL)
			free(tmphstbuf);
		NGATCil_General_Error_Number = 130;
		sprintf(NGATCil_General_Error_String,"Get_Host_By_Name:gethostbyname_r failed to find host %s (%d).",
			name,herr);
		return FALSE;
	}
	if(hp == NULL)
	{
		if(tmphstbuf != NULL)
			free(tmphstbuf);
		NGATCil_General_Error_Number = 131;
		sprintf(NGATCil_General_Error_String,
			"Get_Host_By_Name:gethostbyname_r returned NULL return pointer for hostname %s (%d).",
			name,herr);
		return FALSE;
	}
	/* copy result */
	(*host_addr_zero) = (in_addr_t)(hp->h_addr_list[0]);
	/* free buffer*/
	if(tmphstbuf != NULL)
		free(tmphstbuf);
#if NGATCIL_DEBUG > 5
	NGATCil_General_Log_Format("ngatcil","ngatcil_udp_raw.c","Get_Host_By_Name",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				   "Get_Host_By_Name(%s) Finished and returned %u.%u.%u.%u (network byte ordered).",
				   name,((*host_addr_zero)&0xff),(((*host_addr_zero)>>8)&0xff),
				   (((*host_addr_zero)>>16)&0xff),(((*host_addr_zero)>>24)&0xff));
#endif /* NGATCIL_DEBUG */
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.10  2012/01/26 15:28:43  cjm
** Swapped gethostbyname to Get_Host_By_Name.
**
** Revision 1.9  2011/09/08 09:21:11  cjm
** Added #include <stdlib.h> for malloc under newer kernels.
**
** Revision 1.8  2009/02/02 15:05:31  cjm
** Increased verbosity of UDP_Raw_Server_Thread.
**
** Revision 1.7  2009/02/02 11:02:45  cjm
** Removed old comments.
**
** Revision 1.6  2009/01/30 18:00:52  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.5  2006/08/29 14:07:57  cjm
** Added NGATCil_UDP_Raw_To_Network_Byte_Order.
** Added NGATCil_UDP_Raw_Send_To.
**
** Revision 1.4  2006/06/29 17:02:19  cjm
** Now UDP_Raw_Server_Thread does network to host word conversion.
**
** Revision 1.3  2006/06/07 11:11:25  cjm
** Added logging.
**
** Revision 1.2  2006/06/05 18:56:29  cjm
** Fixed setting host network address.
** Added 0 byte received == termination test.
**
** Revision 1.1  2006/06/01 15:28:06  cjm
** Initial revision
**
*/
