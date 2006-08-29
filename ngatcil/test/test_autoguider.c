/* test_autoguider.c
** $Header: /home/cjm/cvs/autoguider/ngatcil/test/test_autoguider.c,v 1.3 2006-08-29 14:15:44 cjm Exp $
*/
/**
 * Test server that pretends to be an autoguider, and sends guide packets to a TCS.
 * <pre>
 * test_autoguider
 * </pre>
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
/**
 * This hash define is needed before including math.h to get M_PI.
 */
#define _GNU_SOURCE

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"
#include "ngatcil_tcs_guide_packet.h"
#include "ngatcil_cil.h"

#include "command_server.h"

/* hash defines */
/**
 * Maximum number of guide offset point count's.
 */
#define MAX_GUIDE_OFFSET_POINT_COUNT      (360)

/* internal structure definitions */
/**
 * Structure containing X and Y floats for guide offset point.
 */
struct Guide_Offset_Point_Struct
{
	float X;
	float Y;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: test_autoguider.c,v 1.3 2006-08-29 14:15:44 cjm Exp $";
/**
 * Command server (telnet) port.
 */
static unsigned short Command_Server_Port = 1235;
/**
 * The server context to use for this server.
 */
static Command_Server_Server_Context_T Server_Context = NULL;
/**
 * TCC hostname, the TCC should be running the TCS.
 * @see ../cdocs/ngatcil_tcs_guide.html#NGATCIL_TCS_GUIDE_PACKET_TCC_DEFAULT
 */
static char TCC_Hostname[256] = NGATCIL_TCS_GUIDE_PACKET_TCC_DEFAULT;
/**
 * TCS UDP guide packet port.
 * @see ../cdocs/ngatcil_tcs_guide.html#NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT
 */
static int TCS_UDP_Guide_Port = NGATCIL_TCS_GUIDE_PACKET_PORT_DEFAULT;
/**
 * Boolean stating when to stop guiding.
 */
static int Quit_Guiding = FALSE;
/**
 * Centre X point of any geometric offset applied to fake centroids.
 */
static float Guide_Centre_X = 512.0f;
/**
 * Centre Y point of any geometric offset applied to fake centroids.
 */
static float Guide_Centre_Y = 512.0f;
/**
 * Radius (in pixels) of any geometric offset applied to fake centroids.
 */
static float Guide_Offset_Radius = 5.0f;
/**
 * Number of points in the geometric offset list applied to fake centroids.
 */
static int Guide_Offset_Point_Count = 36;
/**
 * Guide offset point list.
 * @see #Guide_Offset_Point_Struct
 * @see #MAX_GUIDE_OFFSET_POINT_COUNT
 */
static struct Guide_Offset_Point_Struct Guide_Offset_Point_List[MAX_GUIDE_OFFSET_POINT_COUNT];
/**
 * UDP CIL port to wait for TCS commands on.
 * @see ../cdocs/ngatcil_cil.html#NGATCIL_CIL_AGS_PORT_DEFAULT
 */
static int AGS_CIL_UDP_Port = NGATCIL_CIL_AGS_PORT_DEFAULT;
/**
 * UDP CIL Socket file descriptor of port to wait for TCS commands on.
 * @see #AGS_CIL_UDP_Port
 */
static int AGS_CIL_UDP_Socket_Fd = -1;
/**
 * TCS UDP CIL Command port.
 * @see ../cdocs/ngatcil_cil.html#NGATCIL_CIL_TCS_PORT_DEFAULT
 */
static int TCS_UDP_CIL_Port = NGATCIL_CIL_TCS_PORT_DEFAULT;

/* internal routines */
static void Send_Reply(Command_Server_Handle_T connection_handle,char *reply_message);
static void *Guide_Thread(void *user_arg);
static int Setup_Guide_Point_List(void);
static void Test_Autoguider_Server_Connection_Callback(Command_Server_Handle_T connection_handle);
static int Test_Autoguider_CIL_UDP_Server_Callback(int socket_id,void* message_buff,int message_length);
static int Send_CIL_UDP_Autoguider_On_Reply(float pixel_x,float pixel_y,int status,int sequence_number);
static int Send_CIL_UDP_Autoguider_Off_Reply(int status,int sequence_number);
static int Autoguider_On(void);
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Main program.
 * @see #Parse_Arguments
 * @see #Help
 */
int main(int argc, char* argv[])
{
	int retval;

	if(!Parse_Arguments(argc,argv))
		return 1;

	if(Command_Server_Port == 0)
	{
		Help();
		return 3;
	}
	/* setup logging */
	Command_Server_Set_Log_Handler_Function(Command_Server_Log_Handler_Stdout);
	Command_Server_Set_Log_Filter_Function(Command_Server_Log_Filter_Level_Bitwise);
	Command_Server_Set_Log_Filter_Level(COMMAND_SERVER_LOG_BIT_GENERAL);

	NGATCil_General_Set_Log_Handler_Function(NGATCil_General_Log_Handler_Stdout);
	NGATCil_General_Set_Log_Filter_Function(NGATCil_General_Log_Filter_Level_Bitwise);
	NGATCil_General_Set_Log_Filter_Level(NGATCIL_GENERAL_LOG_BIT_GENERAL|NGATCIL_GENERAL_LOG_BIT_UDP_RAW|
					     NGATCIL_GENERAL_LOG_BIT_TCS_GUIDE_PACKET|NGATCIL_GENERAL_LOG_BIT_CIL);
	/* start CIL command server */
	fprintf(stdout, "Starting CIL UDP Command Server on port %d.\n",AGS_CIL_UDP_Port);
	retval = NGATCil_UDP_Server_Start(AGS_CIL_UDP_Port,NGATCIL_CIL_AGS_PACKET_LENGTH,&AGS_CIL_UDP_Socket_Fd,
					  Test_Autoguider_CIL_UDP_Server_Callback);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return 4;
	}
	/* start text command server */
	fprintf(stdout, "Starting command server on port %hu.\n",Command_Server_Port);
	retval = Command_Server_Start_Server(&Command_Server_Port,
					     Test_Autoguider_Server_Connection_Callback,&Server_Context);
	if(retval == FALSE)
	{
		Command_Server_Error();
		return 4;
	}
	fprintf(stdout, "Command Server finished\n");
	fprintf(stdout, "Closing AGS CIL Command UDP socket.\n");
	retval = NGATCil_UDP_Close(AGS_CIL_UDP_Socket_Fd);
	if(retval == FALSE)
	{
		NGATCil_General_Error();
		return 4;
	}
	fprintf(stdout, "test_autoguider finished.\n");
	return 0;
}/* main */

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Fake guide thread started with "autoguider on".
 * @see #Quit_Guiding
 * @see #TCC_Hostname
 * @see #TCS_UDP_Guide_Port
 * @see #Setup_Guide_Point_List
 * @see #Guide_Offset_Point_Count
 * @see #Guide_Offset_Point_List
 * @see #Guide_Offset_Point_Struct
 */
static void *Guide_Thread(void *user_arg)
{
	int socket_fd,retval,guide_offset_point_target_index,sleep_time;
	struct Guide_Offset_Point_Struct current_centroid;

	fprintf(stdout,"Guide_Thread:started.\n");
	fprintf(stdout,"Guide_Thread:Opening UDP port to %s:%d.\n",TCC_Hostname,TCS_UDP_Guide_Port);
	retval = NGATCil_UDP_Open(TCC_Hostname,TCS_UDP_Guide_Port,&socket_fd);
	if(retval == FALSE)
	{
		fprintf(stderr,"Guide_Thread:Failed to open UDP port (%s:%d).\n",TCC_Hostname,TCS_UDP_Guide_Port);
		NGATCil_General_Error();
		return NULL;
	}
	fprintf(stdout,"Guide_Thread:UDP port opened (%d).\n",socket_fd);
	fprintf(stdout,"Guide_Thread:Setting up guide point list.\n");
	if(!Setup_Guide_Point_List())
		return NULL;
	current_centroid = Guide_Offset_Point_List[0];
	guide_offset_point_target_index = 1;
	sleep_time = 1;
	fprintf(stdout,"Guide_Thread:starting guiding.\n");
	while(Quit_Guiding == FALSE)
	{
		/* send current centroid */
		fprintf(stdout,"Guide_Thread:Sending guide centroid (%.2f,%.2f).\n",
			current_centroid.X,current_centroid.Y);
		retval = NGATCil_TCS_Guide_Packet_Send(socket_fd,current_centroid.X,current_centroid.Y,FALSE,FALSE,
						       sleep_time*2.0f,'0');
		if(retval == FALSE)
		{
			fprintf(stderr,"Guide_Thread:Failed to send guide packet.\n");
			NGATCil_General_Error();
			return NULL;
		}
		/* move current centroid towards target postion */
		if(current_centroid.X < Guide_Offset_Point_List[guide_offset_point_target_index].X)
		{
			current_centroid.X += MIN(1.0f,Guide_Offset_Point_List[guide_offset_point_target_index].X-
						  current_centroid.X);
		}
		else if(current_centroid.X > Guide_Offset_Point_List[guide_offset_point_target_index].X)
		{
			current_centroid.X -= MIN(1.0f,current_centroid.X - 
						  Guide_Offset_Point_List[guide_offset_point_target_index].X);
		}
		if(current_centroid.Y < Guide_Offset_Point_List[guide_offset_point_target_index].Y)
		{
			current_centroid.Y += MIN(1.0f,Guide_Offset_Point_List[guide_offset_point_target_index].Y-
						  current_centroid.Y);
		}
		else if(current_centroid.Y > Guide_Offset_Point_List[guide_offset_point_target_index].Y)
		{
			current_centroid.Y -= MIN(1.0f,current_centroid.Y - 
						  Guide_Offset_Point_List[guide_offset_point_target_index].Y);
		}
		fprintf(stdout,"Guide_Thread:Next guide centroid (%.2f,%.2f).\n",
			current_centroid.X,current_centroid.Y);
		/* if current centroid at target position, increment target to next index */
		if((fabs(current_centroid.X-Guide_Offset_Point_List[guide_offset_point_target_index].X) < 0.1f) &&
		   (fabs(current_centroid.Y-Guide_Offset_Point_List[guide_offset_point_target_index].Y) < 0.1f))
		{
			fprintf(stdout,"Guide_Thread:Current guide centroid (%.2f,%.2f) at point list[%d](%.2f,%.2f),"
				"incrementing target index.\n",
				current_centroid.X,current_centroid.Y,guide_offset_point_target_index,
				Guide_Offset_Point_List[guide_offset_point_target_index].X,
				Guide_Offset_Point_List[guide_offset_point_target_index].Y);
			guide_offset_point_target_index++;
			if(guide_offset_point_target_index >= Guide_Offset_Point_Count)
				guide_offset_point_target_index = 0;
			fprintf(stdout,"Guide_Thread:target index now %d.\n",guide_offset_point_target_index);
		}
		/* sleep for a bit */
		sleep(sleep_time);
	}
	/* send terminating guide packet */
	fprintf(stdout,"Guide_Thread:Sending terminating guide centroid (%.2f,%.2f).\n",
		current_centroid.X,current_centroid.Y);
	retval = NGATCil_TCS_Guide_Packet_Send(socket_fd,current_centroid.X,current_centroid.Y,TRUE,FALSE,
					       sleep_time*2.0f,'0');
	if(retval == FALSE)
	{
		fprintf(stderr,"Guide_Thread:Failed to send terminating guide packet.\n");
		NGATCil_General_Error();
		return NULL;
	}
	fprintf(stdout,"Guide_Thread:closing UDP port.\n");
	retval = NGATCil_UDP_Close(socket_fd);
	if(retval == FALSE)
	{
		fprintf(stderr,"Guide_Thread:Failed to close UDP port (%s:%d,%d).\n",TCC_Hostname,TCS_UDP_Guide_Port,
			socket_fd);
		NGATCil_General_Error();
		return NULL;
	}
	fprintf(stdout,"Guide_Thread:finished.\n");
	return NULL;
}

/**
 * Setup guide point list.
 * @see #Guide_Centre_X
 * @see #Guide_Centre_Y
 * @see #Guide_Offset_Radius
 * @see #Guide_Offset_Point_Count
 * @see #Guide_Offset_Point_List
 * @see #MAX_GUIDE_OFFSET_POINT_COUNT
 * @see #Guide_Offset_Point_Struc
 */
static int Setup_Guide_Point_List(void)
{
	double angled;
	int i;

	fprintf(stdout,"Setup_Guide_Point_List:started.\n");
	if((Guide_Offset_Point_Count > MAX_GUIDE_OFFSET_POINT_COUNT) || (Guide_Offset_Point_Count < 2))
	{
		fprintf(stderr,"Setup_Guide_Point_List:Guide offset point count out of range %d.\n",
			Guide_Offset_Point_Count);
		return FALSE;
	}
	for(i=0;i<Guide_Offset_Point_Count; i++)
	{
		angled = ((double)i)*(360.0/Guide_Offset_Point_Count)*(M_PI/180.0);
		fprintf(stdout,"Setup_Guide_Point_List:angle  = %.2f radians.\n",angled);
		Guide_Offset_Point_List[i].X = Guide_Centre_X + (Guide_Offset_Radius*
					       cos(angled));
		Guide_Offset_Point_List[i].Y = Guide_Centre_Y + (Guide_Offset_Radius*
					       sin(angled));
		fprintf(stdout,"Setup_Guide_Point_List:point[%d] = (%.2f,%.2f).\n",i,
			Guide_Offset_Point_List[i].X,Guide_Offset_Point_List[i].Y);
	}
	fprintf(stdout,"Setup_Guide_Point_List:finished.\n");
	return TRUE;
}

/**
 * Function invoked in each thread.
 * @param connection_handle Connection handle for this thread.
 * @see #Abort
 * @see #Send_Reply
 */
static void Test_Autoguider_Server_Connection_Callback(Command_Server_Handle_T connection_handle)
{
	char *client_message = NULL;
	int retval;
	int seconds,i;

	/* get message from client */
	retval = Command_Server_Read_Message(connection_handle, &client_message);
	if(retval == FALSE)
	{
		Command_Server_Error();
		return;
	}
	printf("command server: received '%s'\n", client_message);
	/* do something with message */
	if(strcmp(client_message, "help") == 0)
	{
		printf("command server: help detected.\n");
		Send_Reply(connection_handle, "0 help:\n"
			   "\tautoguider on\n"
			   "\tautoguider off\n"
			   "\thelp\n"
			   "\tshutdown\n");
	}
	else if(strcmp(client_message, "shutdown") == 0)
	{
		printf("command server: shutdown detected:about to stop.\n");
		Send_Reply(connection_handle, "0 ok");
		retval = Command_Server_Close_Server(&Server_Context);
		if(retval == FALSE)
			Command_Server_Error();
	}
	else if(strncmp(client_message, "autoguider on", strlen("autoguider on")) == 0)
	{
		printf("command server: autoguider on detected.\n");
		if(!Autoguider_On())
		{
			fprintf(stderr,"command server: Autoguider_On failed.");
			free(client_message);
			return;
		}
		fprintf(stdout,"command server: Guide thread started.\n");
		Send_Reply(connection_handle, "0 Guide thread started.");
	}
	else if(strncmp(client_message, "autoguider off", strlen("autoguider off")) == 0)
	{
		fprintf(stdout,"command server: autoguider off detected.\n");
		Quit_Guiding = TRUE;
		Send_Reply(connection_handle, "0 ok.");
	}
	else
	{
		fprintf(stdout,"command server: message unknown: '%s'\n", client_message);
		Send_Reply(connection_handle, "1 failed message unknown");
	}
	/* free message */
	free(client_message);
}

/**
 * Send a message back to the client.
 * @param connection_handle Connection handle for this thread.
 * @param reply_message The message to send.
 */
static void Send_Reply(Command_Server_Handle_T connection_handle,char *reply_message)
{
	int retval;

	/* send something back to the client */
	printf("command server: about to send '%s'\n", reply_message);
	retval = Command_Server_Write_Message(connection_handle, reply_message);
	if(retval == FALSE)
	{
		Command_Server_Error();
		return;
	}
	printf("command server: sent '%s'\n", reply_message);
}

/**
 * Callback received when a CIL UDP packet arrives from the TCS.
 * @see #Autoguider_On
 * @see #Quit_Guiding
 * @see #Send_CIL_UDP_Autoguider_On_Reply
 * @see #Send_CIL_UDP_Autoguider_Off_Reply
 * @see ../cdocs/ngatcil_cil.html#NGATCil_Cil_Packet_Struct
 * @see ../cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_On_Pixel_Parse
 * @see ../cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_Off_Parse
 * @see ../cdocs/ngatcil_cil.html#NGATCIL_CIL_AGS_PACKET_LENGTH
 * @see ../cdocs/ngatcil_cil.html#E_CIL_CMD_CLASS
 * @see ../cdocs/ngatcil_cil.html#E_AGS_CMD
 * @see ../cdocs/ngatcil_cil.html#E_AGS_GUIDE_ON_PIXEL
 * @see ../cdocs/ngatcil_cil.html#E_AGS_GUIDE_OFF
 */
static int Test_Autoguider_CIL_UDP_Server_Callback(int socket_id,void* message_buff,int message_length)
{
	struct NGATCil_Cil_Packet_Struct cil_packet;
	struct NGATCil_Ags_Packet_Struct ags_cil_packet;
	float pixel_x,pixel_y;
	int sequence_number,status;

	if(message_length != NGATCIL_CIL_AGS_PACKET_LENGTH)
	{
		fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:"
			"received CIL packet of wrong length %d vs %d.\n",message_length,
			NGATCIL_CIL_AGS_PACKET_LENGTH);
		return FALSE;
	}
	/* diddly won't work if NGATCIL_CIL_AGS_PACKET_LENGTH != sizeof(struct NGATCil_Ags_Packet_Struct) */
	memcpy(&ags_cil_packet,message_buff,message_length);
	if(ags_cil_packet.Cil_Base.Class != E_CIL_CMD_CLASS)
	{
		fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:"
			"received CIL packet for wrong class %d vs %d.\n",ags_cil_packet.Cil_Base.Class,E_CIL_CMD_CLASS);
		return FALSE;

	}
	if(ags_cil_packet.Cil_Base.Service != E_AGS_CMD)
	{
		fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:"
			"received CIL packet for wrong service %d vs %d.\n",ags_cil_packet.Cil_Base.Service,E_AGS_CMD);
		return FALSE;

	}
	switch(ags_cil_packet.Command)
	{
		case E_AGS_GUIDE_ON_PIXEL:
			fprintf(stdout,"Test_Autoguider_CIL_UDP_Server_Callback:Command is autoguider on pixel.\n");
			if(!NGATCil_Cil_Autoguide_On_Pixel_Parse(ags_cil_packet,&pixel_x,&pixel_y,&sequence_number))
			{
				fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:"
					"Failed to parse autoguider on pixel command.\n");
				NGATCil_General_Error();
				return FALSE;
			}
			/* start guide thread */
			if(!Autoguider_On())
			{
				fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:Autoguider_On failed.\n");
				Send_CIL_UDP_Autoguider_On_Reply(pixel_x,pixel_y,1,sequence_number);
				return FALSE;
			}
			if(!Send_CIL_UDP_Autoguider_On_Reply(pixel_x,pixel_y,SYS_NOMINAL,sequence_number))
			{
				fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:"
					"Failed to send autoguider on pixel reply.\n");
				return FALSE;
			}
			break;
		case E_AGS_GUIDE_OFF:
			fprintf(stdout,"Test_Autoguider_CIL_UDP_Server_Callback:Command is autoguider off.\n");
			if(!NGATCil_Cil_Autoguide_Off_Parse(ags_cil_packet,&status,&sequence_number))
			{
				fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:"
					"Failed to parse autoguider off command.\n");
				NGATCil_General_Error();
				return FALSE;
			}
			/* stop guide thread */
			Quit_Guiding = TRUE;
			if(!Send_CIL_UDP_Autoguider_Off_Reply(SYS_NOMINAL,sequence_number))
			{
				fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:"
					"Failed to send autoguider off reply.\n");
				return FALSE;
			}
			break;
		default:
			fprintf(stderr,"Test_Autoguider_CIL_UDP_Server_Callback:"
				"received CIL packet with unknown command %d.\n",ags_cil_packet.Command);
			return FALSE;
			break;
	}
	return TRUE;
}

/**
 * Send a reply to an "autoguider on pixel" command sent to the AGS CIL port,
 * by sending a CIL UDP reply packet to the TCS UDP CIL command port.
 * @see #TCC_Hostname
 * @see #TCS_UDP_CIL_Port
 * @see #AGS_CIL_UDP_Socket_Fd
 * @see ../cdocs/ngatcil_cil.html#NGATCil_UDP_Open
 * @see ../cdocs/ngatcil_cil.html#NGATCil_UDP_Close
 * @see ../cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_On_Pixel_Reply_Send
 */
static int Send_CIL_UDP_Autoguider_On_Reply(float pixel_x,float pixel_y,int status,int sequence_number)
{
	int retval;

	retval = NGATCil_Cil_Autoguide_On_Pixel_Reply_Send(AGS_CIL_UDP_Socket_Fd,TCC_Hostname,TCS_UDP_CIL_Port,
							   pixel_x,pixel_y,status,sequence_number);
	if(retval == FALSE)
	{
		fprintf(stderr,"Send_CIL_UDP_Autoguider_On_Reply:Failed to send autoguide on pixel reply packet.\n");
		NGATCil_General_Error();
		return FALSE;
	}
	return TRUE;
}

/**
 * Send a reply to an "autoguider off" command sent to the AGS CIL port,
 * by sending a CIL UDP reply packet to the TCS UDP CIL command port.
 * @see #TCC_Hostname
 * @see #TCS_UDP_CIL_Port
 * @see #AGS_CIL_UDP_Socket_Fd
 * @see ../cdocs/ngatcil_cil.html#NGATCil_UDP_Open
 * @see ../cdocs/ngatcil_cil.html#NGATCil_UDP_Close
 * @see ../cdocs/ngatcil_cil.html#NGATCil_Cil_Autoguide_Off_Reply_Send
 */
static int Send_CIL_UDP_Autoguider_Off_Reply(int status,int sequence_number)
{
	int retval;

	retval = NGATCil_Cil_Autoguide_Off_Reply_Send(AGS_CIL_UDP_Socket_Fd,TCC_Hostname,TCS_UDP_CIL_Port,
						      status,sequence_number);
	if(retval == FALSE)
	{
		fprintf(stderr,"Send_CIL_UDP_Autoguider_Off_Reply:Failed to send autoguide off reply packet.\n");
		NGATCil_General_Error();
		return FALSE;
	}
	return TRUE;
}

/**
 * Start the fake autoguider guide thread.
 * @see #Guide_Thread
 */
static int Autoguider_On(void)
{
	pthread_t guide_thread;
	pthread_attr_t attr;
	int retval;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	retval = pthread_create(&guide_thread,&attr,&Guide_Thread,(void *)NULL);
	if(retval != 0)
	{
		fprintf(stderr,"Autoguider_On:Failed to create guide thread.");
		return FALSE;
	}
	fprintf(stdout,"Autoguider_On: Guide thread started.\n");
	return TRUE;
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Guide_Centre_X
 * @see #Guide_Centre_Y
 * @see #Guide_Offset_Radius
 * @see #Guide_Offset_Point_Count
 * @see #TCC_Hostname
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-command_server_log_level")==0)||(strcmp(argv[i],"-csl")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&log_level);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing log level %s failed.\n",argv[i+1]);
					return FALSE;
				}
				Command_Server_Set_Log_Filter_Level(log_level);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Log Level requires a level.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-centre_x")==0)||(strcmp(argv[i],"-cx")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%f",&Guide_Centre_X);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing centre X %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Centre X requires a pixel value.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-centre_y")==0)||(strcmp(argv[i],"-cy")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%f",&Guide_Centre_Y);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing centre Y %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Centre Y requires a pixel value.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"-h")==0))
		{
			Help();
			exit(0);
		}
		else if((strcmp(argv[i],"-ngatcil_log_level")==0)||(strcmp(argv[i],"-ncl")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&log_level);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing log level %s failed.\n",argv[i+1]);
					return FALSE;
				}
				NGATCil_General_Set_Log_Filter_Level(log_level);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Log Level requires a level.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-point_count")==0)||(strcmp(argv[i],"-pc")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Guide_Offset_Point_Count);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing point count %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Point Count requires a number.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-radius")==0)||(strcmp(argv[i],"-r")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%f",&Guide_Offset_Radius);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing radius %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Radius requires a pixel value.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-tcc_hostname")==0)||(strcmp(argv[i],"-tcc")==0))
		{
			if((i+1)<argc)
			{
				strcpy(TCC_Hostname,argv[i+1]);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:TCC hostname not specified.\n");
				return FALSE;
			}
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
	fprintf(stdout,"Test TCS:Help.\n");
	fprintf(stdout,"test_autoguider\n");
	fprintf(stdout,"\t[-[csl|command_server_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-[ncl|ngatcil_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-[cx|centre_x] <pixel>][-[cy|centre_y] <pixel>]\n");
	fprintf(stdout,"\t[-[r[adius] <pixels>][-[pc|point_count] <n>]\n");
	fprintf(stdout,"\t[-tcc[_hostname] <string>]\n");
	fprintf(stdout,"\t[-h[elp]]\n");
}
/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2006/06/07 13:43:05  cjm
** Added CIL command handling.
**
** Revision 1.1  2006/06/05 18:56:45  cjm
** Initial revision
**
*/
