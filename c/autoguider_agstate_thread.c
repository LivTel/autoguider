/* autoguider_agstate_thread.c
** Start a idle agstate checking thread. This continually (once every n seconds/minutes) checks whether the
** autoguider is idle, and if so writes the AGS AGSTATE datum to the SDB. This is to help the TCS pick up the fact the
** autoguider is idle, to stop 'Autoguider is not accepting commands' TCS errors (Fault #2950)
*/

/**
 * Start a idle agstate checking thread. This continually (once every n seconds/minutes) checks whether the
 * autoguider is idle, and if so writes the AGS AGSTATE datum to the SDB. This is to help the TCS pick up the fact the
 * autoguider is idle, to stop 'Autoguider is not accepting commands' TCS errors (Fault #2950)
 * @author Chris Mottram
 * @version $Revision$
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
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log_udp.h"

#include "ccd_config.h"

#include "autoguider_general.h"
#include "autoguider_cil.h"
#include "autoguider_field.h"
#include "autoguider_guide.h"

/* internal data */
/**
 * Boolean, TRUE if we want to start the Agstate thread.
 */
static int Agstate_Thread_Server_Start = TRUE;
/**
 * The length of time to sleep between checking the autoguider's idle state, in milliseconds.
 */
static int Agstate_Thread_Sleep_Time_Ms = 60000;
/**
 * Boolean, set to FALSE at the start of the agstate thread, Autoguider_Agstate_Thread_Stop
 * sets this to TRUE to stop the agstate thread running.
 */
static int Agstate_Thread_Quit = FALSE;
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/* internal functions */
static void *Agstate_Thread(void *arg);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Autoguider Agstate Thread initialisation routine. Assumes CCD_Config_Load has previously been called
 * to load the configuration file.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #Agstate_Thread_Server_Start
 * @see #Agstate_Thread_Sleep_Time_Ms
 */
int Autoguider_Agstate_Thread_Initialise(void)
{
	int retval;
	
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Autoguider_Agstate_Thread_Initialise",
				      LOG_VERBOSITY_TERSE,"AGSTATE","started.");
#endif
	/* get whether to start the agstate thread */
	retval = CCD_Config_Get_Boolean("agstate.thread.start",&Agstate_Thread_Server_Start);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1300;
		sprintf(Autoguider_General_Error_String,"Autoguider_Agstate_Thread_Initialise:"
			"Failed to find Agstate Thread start (agstate.thread.start) in config file.");
		return FALSE;
	}
 	/* get length of time to sleep between idle checks, in milliseconds */
	retval = CCD_Config_Get_Integer("agstate.thread.sleep_time",&Agstate_Thread_Sleep_Time_Ms);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1301;
		sprintf(Autoguider_General_Error_String,"Autoguider_Agstate_Thread_Initialise:"
			"Failed to find Agstate Thread Sleep time (agstate.thread.sleep_time) in config file.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Autoguider_Agstate_Thread_Initialise",
				      LOG_VERBOSITY_TERSE,
				      "AGSTATE","finished.");
#endif
	return TRUE;
}

/**
 * Agstate Thread start routine.
 * This routine starts the thread. It returns immediately (the idle monitoring is done on a new thread).
 * Use Autoguider_Agstate_Thread_Stop to stop the started server.
 * The server is only started if Agstate_Thread_Server_Start is TRUE.
 * @return The routine returns TRUE if successfull, and FALSE if an error occurs. If an error occurs,
 *        Autoguider_General_Error_Number and Autoguider_General_Error_String are set.
 * @see #Agstate_Thread_Server_Start
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 */
int Autoguider_Agstate_Thread_Start(void)
{
	pthread_t new_thread;
	pthread_attr_t attr;
	int retval;
	
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Autoguider_Agstate_Thread_Start",
				      LOG_VERBOSITY_TERSE,"AGSTATE","started.");
#endif
	if(Agstate_Thread_Server_Start)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Autoguider_Agstate_Thread_Start",
					      LOG_VERBOSITY_TERSE,"AGSTATE","Starting Agstate Thread.");
#endif
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		retval = pthread_create(&new_thread,&attr,&Agstate_Thread,NULL);
		if(retval != 0)
		{
			Autoguider_General_Error_Number = 1302;
			sprintf(Autoguider_General_Error_String,"Autoguider_Agstate_Thread_Start:"
				"Failed to create Agstate thread pthread(%d).",retval);
			return FALSE;
		}
		
	}/* end if Agstate_Thread_Server_Start */
	else
	{
		Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Autoguider_Agstate_Thread_Start",
					      LOG_VERBOSITY_TERSE,"AGSTATE","Not starting Agstate monitoring thread.");
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Autoguider_Agstate_Thread_Start",
				      LOG_VERBOSITY_TERSE,"AGSTATE","finished.");
#endif
	return TRUE;
}

/**
 * Routine to stop a started Agstate thread. This sets Agstate_Thread_Quit to TRUE, which is regularily checked
 * by Agstate_Thread.
 * @see #Agstate_Thread_Quit
 */
int Autoguider_Agstate_Thread_Stop(void)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Autoguider_Agstate_Thread_Stop",
				      LOG_VERBOSITY_TERSE,"AGSTATE","Stopping Agstate Thread.");
#endif
	Agstate_Thread_Quit = TRUE;
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Agstate monitoring thread.
 * This continually (once every n seconds/minutes) checks whether the
 * autoguider is idle, and if so writes the AGS AGSTATE datum to the SDB. This is to help the TCS pick up the fact the
 * autoguider is idle, to stop 'Autoguider is not accepting commands' TCS errors (Fault #2950). The TCS is meant
 * to always pick up the latest SDB AGSTATE entry, but due to networking glitches this is currently sometimes failing.
 * The thread runs every Agstate_Thread_Sleep_Time_Ms milliseconds, until Agstate_Thread_Quit is set to TRUE.
 * @see #Agstate_Thread_Sleep_Time_Ms
 * @see #Agstate_Thread_Quit
 */
static void *Agstate_Thread(void *arg)
{
	struct timespec sleep_time;
	
	Agstate_Thread_Quit = FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Agstate_Thread",
				      LOG_VERBOSITY_TERSE,"AGSTATE","started.");
#endif
	while(Agstate_Thread_Quit == FALSE)
	{
		/* are we guiding?
		** autoguider_guide:Guide_Thread sets Is_Guiding to TRUE whilst the guide thread is running.
		*/
		if(Autoguider_Guide_Is_Guiding() == TRUE)
		{
			/* we are currently guiding and NOT idle */
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Agstate_Thread",
						      LOG_VERBOSITY_VERBOSE,"AGSTATE",
						      "We are currently guiding, and are therefore NOT idle.");
#endif
		}
		else
		{
			/* We are currently NOT guiding.
			** What about fielding?
			** Autoguider_Field changes SDB AGSTATE to E_AGG_STATE_WORKING, so if we are fielding
			** we musn't reset the SDB AGSTATE to IDLE */
			if(Autoguider_Field_Is_Fielding() == TRUE)
			{
				/* we are currently fielding and therefore NOT idle */
#if AUTOGUIDER_DEBUG > 5
				Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Agstate_Thread",
							      LOG_VERBOSITY_VERBOSE,"AGSTATE",
							      "We are currently fielding, and are therefore NOT idle.");
#endif
			}
			else
			{
				/* we are NOT guiding OR fielding, and therfore must be IDLE ? */
#if AUTOGUIDER_DEBUG > 5
				Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Agstate_Thread",
							      LOG_VERBOSITY_VERBOSE,"AGSTATE",
							      "We are currently IDLE, reseting AGSTATE in SDB.");
#endif
				/* note we used to have to 'twiddle' the AGSTATE before sending the SDB packet,
				** as Autoguider_CIL_SDB_Packet_Send only sent datums with modified values.
				** Autoguider_CIL_SDB_Packet_State_Set->NGATCil_AGS_SDB_Value_Set
				** now treats D_AGS_AGSTATE as a special datum that is always
				** sent to the SDB even if it's value is unchanged */
				if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
				{
					 /* no need to fail */
					Autoguider_General_Error("agstate","autoguider_agstate_thread.c",
								 "Agstate_Thread",LOG_VERBOSITY_VERY_TERSE,"STARTUP");
				}
				if(!Autoguider_CIL_SDB_Packet_Send())
				{
					 /* no need to fail */
					Autoguider_General_Error("agstate","autoguider_agstate_thread.c",
								 "Agstate_Thread",LOG_VERBOSITY_VERY_TERSE,"STARTUP");
				}
			}/* end else not fielding */
		}/* end else not guiding */
		/* sleep for Agstate_Thread_Sleep_Time_Ms milliseconds */
		sleep_time.tv_sec = Agstate_Thread_Sleep_Time_Ms/AUTOGUIDER_GENERAL_ONE_SECOND_MS;
		sleep_time.tv_nsec = (Agstate_Thread_Sleep_Time_Ms%AUTOGUIDER_GENERAL_ONE_SECOND_MS)*AUTOGUIDER_GENERAL_ONE_MILLISECOND_NS;
		nanosleep(&sleep_time,NULL);
	}/* end while */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("agstate","autoguider_agstate_thread.c","Agstate_Thread",
				      LOG_VERBOSITY_TERSE,"AGSTATE","finished.");
#endif
}
