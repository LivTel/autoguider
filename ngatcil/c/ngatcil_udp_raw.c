/* ngatcil_udp_raw.c
** NGATCil UDP raw transmission routines
** $Header: /home/cjm/cvs/autoguider/ngatcil/c/ngatcil_udp_raw.c,v 1.3 2006-06-07 11:11:25 cjm Exp $
*/
/**
 * NGAT Cil library raw UDP packet transmission.
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

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h> /* htons etc */
#include <sys/types.h>
#include <sys/socket.h>

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
static char rcsid[] = "$Id: ngatcil_udp_raw.c,v 1.3 2006-06-07 11:11:25 cjm Exp $";

/* internal function declaration */
static void *UDP_Raw_Server_Thread(void *);

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
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_UDP_RAW
 */
int NGATCil_UDP_Open(char *hostname,int port_number,int *socket_id)
{
	int socket_errno,retval;
	unsigned short int network_port_number;
	in_addr_t saddr;
	struct hostent *host_entry;
	struct sockaddr_in server;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Open(%s,%d):started.",
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
		NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,
					   "NGATCil_UDP_Open:inet_addr didn't work:trying gethostbyname(%s).",
					   hostname);
#endif
		/* try getting by hostname instead */
		host_entry = gethostbyname(hostname);
		if(host_entry == NULL)
		{
			shutdown((*socket_id),SHUT_RDWR);
			(*socket_id) = 0;
			NGATCil_General_Error_Number = 104;
			sprintf(NGATCil_General_Error_String,"NGATCil_UDP_Open:Failed to get host address from (%s).",
				hostname);
			return FALSE;
		}
		memcpy(&saddr,host_entry->h_addr_list[0],host_entry->h_length);
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
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,
				   "NGATCil_UDP_Open(%s,%d):returned socket %d:finished.",
				   hostname,port_number,(*socket_id));
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_UDP_RAW
 */
int NGATCil_UDP_Raw_Send(int socket_id,void *message_buff,size_t message_buff_len)
{
	int retval,send_errno;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Raw_Send(%d):started.",socket_id);
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
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Raw_Send(%d):finished.",socket_id);
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_UDP_RAW
 */
int NGATCil_UDP_Raw_Recv(int socket_id,void *message_buff,size_t message_buff_len)
{
	int retval,send_errno;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Raw_Recv(%d):started.",socket_id);
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
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Raw_Recv(%d):finished.",socket_id);
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_UDP_RAW
 */
int NGATCil_UDP_Close(int socket_id)
{
	int retval,socket_errno;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Close(%d):started.",socket_id);
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
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Close(%d):finished.",socket_id);
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
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_UDP_RAW
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
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Server_Start(%d):started.",
				   port_number);
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
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"NGATCil_UDP_Server_Start(%d):finished.",
				   port_number);
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
	int done,retval,socket_errno,current_client_length;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"UDP_Raw_Server_Thread:started.");
#endif
	server_context = (struct UDP_Raw_Server_Context_Struct *)arg;
	if(server_context == NULL)
	{
		NGATCil_General_Error_Number = 117;
		sprintf(NGATCil_General_Error_String,"UDP_Raw_Server_Thread:server_context was NULL.");
		return FALSE;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"UDP_Raw_Server_Thread:listening on socket %d.",
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
#if NGATCIL_DEBUG > 3
		NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,
					   "UDP_Raw_Server_Thread:Waiting for UDP packeton socket %d.",
					   server_context->Socket_Id);
#endif
		current_client_length = sizeof(client);
		retval = recvfrom(server_context->Socket_Id,message_buff,server_context->Message_Length,0, 
				  (struct sockaddr *)&client,&current_client_length);
		if(retval < 0)
		{
			socket_errno = errno;
			/* quit server - hopefully error due to closing socket? */
			done = TRUE;
#if NGATCIL_DEBUG > 3
			NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"UDP_Raw_Server_Thread:Terminating:"
						   "recvfrom returned %d (%d:%s).",retval,socket_errno,
						   strerror(socket_errno));
#endif
		}
		else /* something was received */
		{
			if(retval != server_context->Message_Length)
			{
#if NGATCIL_DEBUG > 3
				NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"UDP_Raw_Server_Thread:"
							   "Detected short packet (%d vs %d).",retval,
							   server_context->Message_Length);
#endif
				/* we seem to get 0 length packets when the server socket is closed */
				if(retval == 0)
				{
					done = TRUE;
#if NGATCIL_DEBUG > 3
					NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,
							       "UDP_Raw_Server_Thread:Zero length packet:"
								   "socket closed?:quiting server thread.");
#endif
				}
			}
			if(server_context->Connection_Handler != NULL)
			{
				(server_context->Connection_Handler)(server_context->Socket_Id,message_buff,retval);
			}
			else
			{
#if NGATCIL_DEBUG > 3
				NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"UDP_Raw_Server_Thread:"
						    "Packet received but Connection Handler appears to be NULL.");
#endif
			}
		}/* if */
	}/* while */
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"UDP_Raw_Server_Thread:shutdown on socket %d.",
				   server_context->Socket_Id);
#endif
	shutdown(server_context->Socket_Id,SHUT_RDWR);
	free(server_context);
	free(message_buff);
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_UDP_RAW,"UDP_Raw_Server_Thread:finished.");
#endif
	return NULL;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2006/06/05 18:56:29  cjm
** Fixed setting host network address.
** Added 0 byte received == termination test.
**
** Revision 1.1  2006/06/01 15:28:06  cjm
** Initial revision
**
*/
