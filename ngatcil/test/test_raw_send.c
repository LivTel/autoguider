/* test_raw_send.c
** $Header: /home/cjm/cvs/autoguider/ngatcil/test/test_raw_send.c,v 1.2 2011-09-08 09:22:24 cjm Exp $
*/
/**
 * Test a raw packet over UDP.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h> /* htons etc */
#include <sys/types.h>
#include <sys/socket.h>

/*
#include "ngatcil_cil.h"
#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"
#include "ngatcil_ags_sdb.h"
*/
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif

/**
 * Max number of int's in packet.
 */
#define MAX_PACKET_LENGTH (256)
/**
 * Socket send buffer size.
 */
#define I_CIL_SNDBUF_SIZE  (9216)
/**
 * Socket receive buffer size.
 */
#define I_CIL_RCVBUF_SIZE  (42624)
/**
 * Default machine name to send SDB data to (mcc). Should be mapped to 192.168.1.1 in /etc/hosts.
 */
#define NGATCIL_AGS_SDB_MCC_DEFAULT      ("mcc")
/**
 * Default port number to send SDB CIL packets to (13011). 
 */
#define NGATCIL_AGS_SDB_CIL_PORT_DEFAULT (13011)
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

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: test_raw_send.c,v 1.2 2011-09-08 09:22:24 cjm Exp $";
/**
 * File descriptor of the UDP socket.
 */
static int Socket_Fd = -1;
/**
 * UDP CIL port number to send packet to.
 * @see #NGATCIL_CIL_AGS_PORT_DEFAULT
 */
static int Local_Port_Number = NGATCIL_CIL_AGS_PORT_DEFAULT;
/**
 * Hostname to send packet to.
 * @see #NGATCIL_CIL_ACC_DEFAULT
 */
static char Local_Hostname[256] = NGATCIL_CIL_ACC_DEFAULT;
/**
 * UDP CIL port number to send packet to.
 * @see ../cdocs/ngatcil_ags_sdb.html#NGATCIL_AGS_SDB_CIL_PORT_DEFAULT
 */
static int Remote_Port_Number = NGATCIL_AGS_SDB_CIL_PORT_DEFAULT;
/**
 * Hostname to send packet to.
 * @see ../cdocs/ngatcil_ags_sdb.html#NGATCIL_AGS_SDB_MCC_DEFAULT
 */
static char Remote_Hostname[256] = NGATCIL_AGS_SDB_MCC_DEFAULT;
/**
 * Contents of packet to send.
 * @see #MAX_PACKET_LENGTH
 */
static int32_t Packet[MAX_PACKET_LENGTH];
/**
 * Number of ints in packet.
 */
static int Packet_Count = 0;
/**
 * Whether to use socket/connect/send/shutdown or socket/sendto.
 */
static int Use_SendTo = FALSE;

/* internal functions */
static int Open(char *hostname,int port_number,int *socket_fd);
static int Send(int socket_id,void *message_buff,size_t message_buff_len);
static int Close(int socket_id);
static int Create_Server_Socket(char *hostname,int port_number,int *socket_fd);
static int SendTo(int socket_fd,char *hostname,int port_number,void *message_buff,size_t message_buff_len);
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Main program.
 * @see #Remote_Hostname
 * @see #Remote_Port_Number
 * @see #Packet_Count
 * @see #Packet
 */
int main(int argc, char* argv[])
{
	int i;

	if(!Parse_Arguments(argc,argv))
		return 1;
	if(Packet_Count < 1)
	{
		fprintf(stderr,"test_raw_send:No words in packet.\n");
		return 2;
	}
	if(Use_SendTo)
	{
		if(!Create_Server_Socket(Local_Hostname,Local_Port_Number,&Socket_Fd))
		{
			fprintf(stderr,"test_raw_send:Create_Server_Socket(%s:%d) failed.\n",Local_Hostname,
				Local_Port_Number);
			return 3;
		}
	}
	else
	{
		fprintf(stdout,"Opening connection to %s:%d.\n",Remote_Hostname,Remote_Port_Number);
		if(!Open(Remote_Hostname,Remote_Port_Number,&Socket_Fd))
		{
			fprintf(stderr,"test_raw_send:Open(%s:%d) failed.\n",Remote_Hostname,Remote_Port_Number);
			return 3;
		}
	}
	fprintf(stdout,"Packet contains %d words of length %ld bytes.\n",Packet_Count,Packet_Count*sizeof(int));
	for(i=0;i<Packet_Count;i++)
	{
		fprintf(stdout,"Word[%d] = %#x.\n",i,Packet[i]);
	}
	fprintf(stdout,"Swapping packet to network byte order.\n");
	for(i=0;i<Packet_Count;i++)
	{
		/* eCilConvert32bitArray uses ntohl !! */
		Packet[i] = htonl(Packet[i]);
	}
	if(Use_SendTo == FALSE)
	{
		fprintf(stdout,"Sending packet.\n");
		if(!Send(Socket_Fd,Packet,Packet_Count*sizeof(int)))
		{
			fprintf(stderr,"test_raw_send:Send(%d,%p,%ld) failed.\n",Socket_Fd,Packet,
				Packet_Count*sizeof(int));
			Close(Socket_Fd);
			return 3;
		}
	}
	else
	{
		fprintf(stdout,"Sending packet using sendto.\n");
		if(!SendTo(Socket_Fd,Remote_Hostname,Remote_Port_Number,Packet,Packet_Count*sizeof(int)))
		{
			fprintf(stderr,"test_raw_send:Send(%d,%p,%ld) failed.\n",Socket_Fd,Packet,
				Packet_Count*sizeof(int));
			Close(Socket_Fd);
			return 3;
		}
	}
	fprintf(stdout,"Closing socket.\n");
	if(!Close(Socket_Fd))
	{
		fprintf(stderr,"test_raw_send:Close(%d) failed.\n",Socket_Fd);
		return 3;
	}
	fprintf(stdout, "test_raw_send finished.\n");
	return 0;
}/* main */

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */

static int Open(char *hostname,int port_number,int *socket_fd)
{
	int socket_errno,retval;
	unsigned short int network_port_number;
	in_addr_t saddr;
	struct hostent *host_entry;
	struct sockaddr_in server;

	if(socket_fd == NULL)
	{
		fprintf(stderr,"Open:socket_fd was NULL.");
		return FALSE;
	}
	/* open datagram socket */
	(*socket_fd) = socket(AF_INET,SOCK_DGRAM,0);
	if((*socket_fd) < 0)
	{
		socket_errno = errno;
		fprintf(stderr,"Open:Failed to create socket (%d:%s).",socket_errno,
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
		fprintf(stdout,"Open:inet_addr didn't work:trying gethostbyname(%s).",hostname);
		/* try getting by hostname instead */
		host_entry = gethostbyname(hostname);
		if(host_entry == NULL)
		{
			shutdown((*socket_fd),SHUT_RDWR);
			(*socket_fd) = 0;
			fprintf(stderr,"Open:Failed to get host address from (%s).",hostname);
			return FALSE;
		}
		memcpy(&saddr,host_entry->h_addr_list[0],host_entry->h_length);
	}
	/* set up socket */
	memset((char *) &server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = saddr;
	server.sin_port = network_port_number;
	retval = connect((*socket_fd),(struct sockaddr *)&server,sizeof(server));
	if(retval < 0)
	{
		socket_errno = errno;
		shutdown((*socket_fd),SHUT_RDWR);
		/*close((*socket_id));*/
		(*socket_fd) = 0;
		fprintf(stderr,"Open:Failed to connect (%d:%s).",socket_errno,strerror(socket_errno));
		return FALSE;
	}
	fprintf(stdout,"Open(%s,%d):returned socket %d:finished.",hostname,port_number,(*socket_fd));
	return TRUE;
}

static int Send(int socket_id,void *message_buff,size_t message_buff_len)
{
	int retval,send_errno;

	fprintf(stdout,"Send(socket=%d,length=%ld):started.",socket_id,message_buff_len);
	if(message_buff == NULL)
	{
		fprintf(stdout,"Send:message_buff was NULL.");
		return FALSE;
	}
	retval = send(socket_id,message_buff,message_buff_len,0);
	if(retval < 0)
	{
		send_errno = errno;
		fprintf(stderr,"Send:Send failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	if(retval != message_buff_len)
	{
		fprintf(stderr,"Send:Send returned %d vs %ld.",retval,message_buff_len);
		return FALSE;
	}
	fprintf(stdout,"Send(%d):finished.",socket_id);
	return TRUE;
}

static int Create_Server_Socket(char *hostname,int port_number,int *socket_fd)
{
	in_addr_t in_addr;
	struct sockaddr_in saddr;
	struct hostent *host_entry;
	int retval,send_errno;
	int buffer,buffer_size;

	if(socket_fd == NULL)
	{
		fprintf(stderr,"Create_Server_Socket:socket_fd was NULL.");
		return FALSE;
	}
	/* Create an unbound socket */
	(*socket_fd) = socket(AF_INET, SOCK_DGRAM, 0);
	if((*socket_fd) < 0)
	{
		send_errno = errno;
		fprintf(stderr,"Create_Server_Socket:Failed to create socket%d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	/* bind for server side (iCilCreateUdpSocket) */
	memset(&saddr, '\0', sizeof(saddr));
	/* try to convert hostname to address in network byte order */
	/* try numeric address conversion first */
	in_addr = inet_addr(hostname);
	if(in_addr == INADDR_NONE)
	{
		fprintf(stdout,"Create_Server_Socket:inet_addr didn't work:trying gethostbyname(%s).",hostname);
		/* try getting by hostname instead */
		host_entry = gethostbyname(hostname);
		if(host_entry == NULL)
		{
			fprintf(stderr,"Create_Server_Socket:Failed to get host address from (%s).",hostname);
			return FALSE;
		}
		memcpy(&in_addr,host_entry->h_addr_list[0],host_entry->h_length);
	}
	/* Formulate the server socket address */
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons((short)port_number);
	saddr.sin_addr.s_addr = in_addr;
	/* bind server socket */
	retval = bind((*socket_fd),(void *) &saddr,sizeof(saddr));
	if(retval < 0)
	{
		send_errno = errno;
		fprintf(stderr,"Create_Server_Socket:bind failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	/* Set receive buffer size.*/
	buffer_size = sizeof(buffer);
	retval = getsockopt((*socket_fd),SOL_SOCKET, SO_RCVBUF,(void*)&buffer,&buffer_size);
	if(retval != 0)
	{
		send_errno = errno;
		fprintf(stderr,"Create_Server_Socket:getsockopt(recv) failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	fprintf(stdout,"Create_Server_Socket:socket send buffer size was %d.",buffer);
	buffer = I_CIL_RCVBUF_SIZE;
	retval = setsockopt((*socket_fd),SOL_SOCKET,SO_RCVBUF,(void*)&buffer,buffer_size);
	if(retval != 0)
	{
		send_errno = errno;
		fprintf(stderr,"Create_Server_Socket:setsockopt(recv) failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	retval = getsockopt((*socket_fd),SOL_SOCKET, SO_RCVBUF,(void*)&buffer,&buffer_size);
	if(retval != 0)
	{
		send_errno = errno;
		fprintf(stderr,"Create_Server_Socket:getsockopt(recv2) failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	fprintf(stdout,"Create_Server_Socket:socket recv buffer size is now %d.",buffer);
	/* Set send buffer size.*/
	buffer_size = sizeof(buffer);
	retval = getsockopt((*socket_fd),SOL_SOCKET, SO_SNDBUF,(void*)&buffer,&buffer_size);
	if(retval != 0)
	{
		send_errno = errno;
		fprintf(stderr,"Create_Server_Socket:getsockopt(send) failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	fprintf(stdout,"Create_Server_Socket:socket send buffer size was %d.",buffer);
	buffer = I_CIL_SNDBUF_SIZE;
	retval = setsockopt((*socket_fd),SOL_SOCKET,SO_SNDBUF,(void*)&buffer,buffer_size);
	if(retval != 0)
	{
		send_errno = errno;
		fprintf(stderr,"Create_Server_Socket:setsockopt(send) failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	retval = getsockopt((*socket_fd),SOL_SOCKET, SO_SNDBUF,(void*)&buffer,&buffer_size);
	if(retval != 0)
	{
		send_errno = errno;
		fprintf(stderr,"Create_Server_Socket:getsockopt(send2) failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	fprintf(stdout,"Create_Server_Socket:socket send buffer size is now %d.",buffer);

	return TRUE;
}

static int SendTo(int socket_fd,char *hostname,int port_number,void *message_buff,size_t message_buff_len)
{
	int retval,send_errno;
	struct sockaddr_in to_addr;
	in_addr_t saddr;
	struct hostent *host_entry;
	size_t to_len = sizeof(to_addr);
	int buffer,buffer_size;

	fprintf(stdout,"SendTo(socket_fd = %d,hostname=%s,port_number=%d,length=%ld):started.",
		socket_fd,hostname,port_number,message_buff_len);
	if(hostname == NULL)
	{
		fprintf(stdout,"SendTo:hostname was NULL.");
		return FALSE;
	}
	if(message_buff == NULL)
	{
		fprintf(stdout,"SendTo:message_buff was NULL.");
		return FALSE;
	}
	/* try to convert hostname to address in network byte order */
	/* try numeric address conversion first */
	saddr = inet_addr(hostname);
	if(saddr == INADDR_NONE)
	{
		fprintf(stdout,"SendTo:inet_addr didn't work:trying gethostbyname(%s).",hostname);
		/* try getting by hostname instead */
		host_entry = gethostbyname(hostname);
		if(host_entry == NULL)
		{
			fprintf(stderr,"SendTo:Failed to get host address from (%s).",hostname);
			return FALSE;
		}
		memcpy(&saddr,host_entry->h_addr_list[0],host_entry->h_length);
	}
	/* Formulate the socket address */
	to_addr.sin_family = AF_INET;
	to_addr.sin_port = htons((short)port_number);
	to_addr.sin_addr.s_addr = saddr;
	retval = sendto(socket_fd,message_buff,message_buff_len,0,(void*)&to_addr,to_len);
	if(retval < 0)
	{
		send_errno = errno;
		fprintf(stderr,"SendTo:Send failed %d (%s).",send_errno,strerror(send_errno));
		return FALSE;
	}
	if(retval != message_buff_len)
	{
		fprintf(stderr,"SendTo:Send returned %d vs %ld.",retval,message_buff_len);
		return FALSE;
	}
	fprintf(stdout,"SendTo(%d):finished.",socket_fd);
	return TRUE;
}

static int Close(int socket_id)
{
	int retval, socket_errno;

	fprintf(stdout,"Close(%d):started.",socket_id);
	retval = shutdown(socket_id,SHUT_RDWR);
	if(retval < 0)
	{
		socket_errno = errno;
		fprintf(stderr,"Close:Close failed (%d,%d:%s).",retval,socket_errno,strerror(socket_errno));
		return FALSE;
	}
	fprintf(stdout,"Close(%d):finished.",socket_id);
	return TRUE;
}


/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Packet_Count
 * @see #Packet
 * @see #MAX_PACKET_LENGTH
 * @see #Use_SendTo
 * @see #Local_Hostname
 * @see #Local_Port_Number
 * @see #Remote_Hostname
 * @see #Remote_Port_Number
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,value;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"-h")==0))
		{
			Help();
			exit(0);
		}
		else if((strcmp(argv[i],"-local_hostname")==0)||(strcmp(argv[i],"-local_host")==0))
		{
			if((i+1)<argc)
			{
				strncpy(Local_Hostname,argv[i+1],256);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Host requires a hostname.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-local_port")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Local_Port_Number);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing port number %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Port requires a number.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-remote_hostname")==0)||(strcmp(argv[i],"-remote_host")==0))
		{
			if((i+1)<argc)
			{
				strncpy(Remote_Hostname,argv[i+1],256);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Host requires a hostname.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-remote_port")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Remote_Port_Number);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing port number %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Port requires a number.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-word")==0)||(strcmp(argv[i],"-w")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%i",&value);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:value %s is not an integer.\n",argv[i+1]);
					return FALSE;
				}
				if(Packet_Count >= MAX_PACKET_LENGTH)
				{
					fprintf(stderr,"Parse_Arguments:Packet list run out of room.\n");
					return FALSE;
				}
				Packet[Packet_Count] = value;
				Packet_Count++;
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:-word requires a value.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-sendto")==0))
		{
			Use_SendTo = TRUE;
		}
		else
		{
			fprintf(stderr,"Parse_Arguments:argument '%s' not recognized.\n",argv[i]);
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Help routine.
 */
static void Help(void)
{
	fprintf(stdout,"Test Raw Send:Help.\n");
	fprintf(stdout,"test_raw_send\n");
	fprintf(stdout,"\t[-w[ord] <word value>]\n");
	fprintf(stdout,"\t[-h[elp]]\n");
	fprintf(stdout,"\t[-local_host[name] <hostname>]\n");
	fprintf(stdout,"\t[-local_port <port_number>]\n");
	fprintf(stdout,"\t[-remote_host[name] <hostname>]\n");
	fprintf(stdout,"\t[-remote_port <port_number>]\n");
	fprintf(stdout,"\t[-sendto]\n");
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/08/29 14:15:44  cjm
** Initial revision
**
*/
