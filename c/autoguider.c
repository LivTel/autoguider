/* autoguider.c
** $Header: /home/cjm/cvs/autoguider/c/autoguider.c,v 1.6 2006-09-07 15:37:25 cjm Exp $
*/
/**
 * Autoguider main program.
 * @author $Author: cjm $
 * @version $Revision: 1.6 $
 */
#include <signal.h> /* signal handling */
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "command_server.h"

#include "ngatcil_ags_sdb.h"
#include "ngatcil_general.h"

#include "object.h"

#include "ccd_driver.h"
#include "ccd_exposure.h"
#include "ccd_general.h"
#include "ccd_setup.h"
#include "ccd_temperature.h"

#include "autoguider_buffer.h"
#include "autoguider_cil.h"
#include "autoguider_command.h"
#include "autoguider_dark.h"
#include "autoguider_field.h"
#include "autoguider_flat.h"
#include "autoguider_general.h"
#include "autoguider_guide.h"
#include "autoguider_object.h"
#include "autoguider_server.h"

/* hash definitions */
/**
 * Default temperature to set the CCD to.
 */
#define DEFAULT_TEMPERATURE	(0.0)

/* external variables */

/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id: autoguider.c,v 1.6 2006-09-07 15:37:25 cjm Exp $";

/* internal routines */
static int Autoguider_Initialise_Signal(void);
static int Autoguider_Initialise_Logging(void);
static int Autoguider_Startup_CCD(void);
static int Autoguider_Shutdown_CCD(void);
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #Autoguider_Initialise_Signal
 * @see #Autoguider_Initialise_Logging
 * @see #Autoguider_Startup_CCD
 * @see #Autoguider_Shutdown_CCD
 * @see autoguider_buffer.html#Autoguider_Buffer_Initialise
 * @see autoguider_buffer.html#Autoguider_Buffer_Shutdown
 * @see autoguider_cil.html#Autoguider_CIL_Server_Initialise
 * @see autoguider_cil.html#Autoguider_CIL_Server_Start
 * @see autoguider_cil.html#Autoguider_CIL_Server_Stop
 * @see autoguider_cil.html#Autoguider_CIL_SDB_State_Set
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Send
 * @see autoguider_dark.html#Autoguider_Dark_Initialise
 * @see autoguider_dark.html#Autoguider_Dark_Shutdown
 * @see autoguider_field.html#Autoguider_Field_Initialise
 * @see autoguider_flat.html#Autoguider_Flat_Initialise
 * @see autoguider_flat.html#Autoguider_Flat_Shutdown
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_GENERAL
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Get_Config_Filename
 * @see autoguider_guide.html#Autoguider_Guide_Initialise
 * @see autoguider_object.html#Autoguider_Object_Shutdown
 * @see autoguider_server.html#Autoguider_Server_Initialise
 * @see autoguider_server.html#Autoguider_Server_Start
 * @see ../../ccd/cdocs/ccd_config.html#CCD_Config_Initialise
 * @see ../../ccd/cdocs/ccd_config.html#CCD_Config_Load
 * @see ../../ccd/cdocs/ccd_config.html#CCD_Config_Shutdown
 */
int main(int argc, char *argv[])
{
	int i,retval,value;
	double temperature;
	enum CCD_TEMPERATURE_STATUS temperature_status;

/* parse arguments */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Parsing Arguments.");
#endif
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* initialise signal handling */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Initialise_Signal.");
#endif
	retval = Autoguider_Initialise_Signal();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		return 4;
	}
	/* initialise/load configuration */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:CCD_Config_Load.");
#endif
	CCD_Config_Initialise();
	retval = CCD_Config_Load(Autoguider_General_Get_Config_Filename());
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1;
		sprintf(Autoguider_General_Error_String,"main:CCD_Config_Load(%s) failed.",
			Autoguider_General_Get_Config_Filename());
		Autoguider_General_Error();
		return 2;
	}
	/* set logging options */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Initialise_Logging.");
#endif
	retval = Autoguider_Initialise_Logging();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		return 4;
	}
	/* initialise connection to the CCD */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Startup_CCD.");
#endif
	retval = Autoguider_Startup_CCD();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		return 3;
	}
	/* initialise field and guide buffers */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Buffer_Initialise.");
#endif
	retval = Autoguider_Buffer_Initialise();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 5;
	}
	/* initialise dark handling */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Dark_Initialise.");
#endif
	retval = Autoguider_Dark_Initialise();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 5;
	}
	/* initialise flat handling */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Flat_Initialise.");
#endif
	retval = Autoguider_Flat_Initialise();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 5;
	}
	/* initialise field variables - note no equivalent shutdown routine */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Field_Initialise.");
#endif
	retval = Autoguider_Field_Initialise();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 5;
	}
	/* initialise guide variables - note no equivalent shutdown routine */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Guide_Initialise.");
#endif
	retval = Autoguider_Guide_Initialise();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 5;
	}
	/* initialise CIL command server/CIL SDB connection */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_CIL_Server_Initialise.");
#endif
	retval = Autoguider_CIL_Server_Initialise();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 4;
	}
	/* start CIL command server */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_CIL_Server_Start.");
#endif
	retval = Autoguider_CIL_Server_Start();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 4;
	}
	/* write IDLE (ready) to SDB. */
	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
		Autoguider_General_Error(); /* no need to fail */
	if(!Autoguider_CIL_SDB_Packet_Send())
		Autoguider_General_Error(); /* no need to fail */
	/* initialise command server */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Server_Initialise.");
#endif
	retval = Autoguider_Server_Initialise();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 4;
	}
	/* start server */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Server_Start.");
#endif
	retval = Autoguider_Server_Start();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 4;
	}
	/* shutdown cil sdb connection */
	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_OFF))
		Autoguider_General_Error(); /* no need to fail */
	if(!Autoguider_CIL_SDB_Packet_Send())
		Autoguider_General_Error(); /* no need to fail */
	/* shutdown cil server */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_CIL_Server_Stop.");
#endif
	retval = Autoguider_CIL_Server_Stop();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		/* ensure CCD is warmed up */
		Autoguider_Shutdown_CCD();
		return 4;
	}
	/* shutdown */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Shutdown_CCD");
#endif
	retval = Autoguider_Shutdown_CCD();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		return 2;
	}
	/* object handling */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Object_Shutdown.");
#endif
	retval = Autoguider_Object_Shutdown();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		return 6;
	}
	/* free flat handling */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Flat_Shutdown.");
#endif
	retval = Autoguider_Flat_Shutdown();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		return 6;
	}
	/* free dark handling */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Dark_Shutdown.");
#endif
	retval = Autoguider_Dark_Shutdown();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		return 6;
	}
	/* free buffers */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:Autoguider_Buffer_Shutdown.");
#endif
	retval = Autoguider_Buffer_Shutdown();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		return 6;
	}
	/* always call Config shutdown, which free config memory */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:CCD_Config_Shutdown");
#endif
	retval = CCD_Config_Shutdown();
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 2;
		sprintf(Autoguider_General_Error_String,"main:CCD_Config_Shutdown failed.");
		Autoguider_General_Error();
		return 2;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"main:autoguider completed.");
#endif
	return 0;
}

/* -----------------------------------------------------------------------------
**      Internal routines
** ----------------------------------------------------------------------------- */
/**
 * Initialise signal handling. Switches off default "Broken pipe" error, so client
 * crashes do NOT kill the AG control system.
 * DOn't use Logging here, this is called pre-logging.
 */
static int Autoguider_Initialise_Signal(void)
{
	struct sigaction sig_action;

	/* old code
	signal(SIGPIPE, SIG_IGN);
	*/
	sig_action.sa_handler = SIG_IGN;
	sig_action.sa_flags = 0;
	sigemptyset(&sig_action.sa_mask);
	sigaction(SIGPIPE,&sig_action,NULL);
	return TRUE;
}

/**
 * Setup logging. Get directory name from config "logging.directory_name".
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Log_Set_Directory
 * @see autoguider_general.html#Autoguider_General_Set_Log_Handler_Function
 * @see autoguider_general.html#Autoguider_General_Log_Handler_Log_Hourly_File
 * @see autoguider_general.html#Autoguider_General_Set_Log_Filter_Function
 * @see autoguider_general.html#Autoguider_General_Log_Filter_Level_Bitwise
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Set_Log_Handler_Function
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../commandserver/cdocs/command_server.html#Command_Server_Set_Log_Handler_Function
 * @see ../commandserver/cdocs/command_server.html#Command_Server_Set_Log_Filter_Function
 * @see ../commandserver/cdocs/command_server.html#Command_Server_Log_Filter_Level_Bitwise
 * @see ../ngatcil/cdocs/ngatcil_general.html#NGATCil_General_Set_Log_Handler_Function
 * @see ../ngatcil/cdocs/ngatcil_general.html#NGATCil_General_Set_Log_Filter_Function
 * @see ../ngatcil/cdocs/ngatcil_general.html#NGATCil_General_Log_Filter_Level_Bitwise
 * @see ../../libdprt/object/cdocs/object.html#Object_Set_Log_Handler_Function
 * @see ../../libdprt/object/cdocs/object.html#Object_Set_Log_Filter_Function
 * @see ../../libdprt/object/cdocs/object.html#Object_Log_Filter_Level_Bitwise
 */
static int Autoguider_Initialise_Logging(void)
{
	char *log_directory = NULL;
	int retval;

	/* don't log yet - not fully setup yet */
	retval = CCD_Config_Get_String("logging.directory_name",&log_directory);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 17;
		sprintf(Autoguider_General_Error_String,"Autoguider_Initialise_Logging:"
			"Failed to get logging directory.");
		return FALSE;
	}
	retval = Autoguider_General_Log_Set_Directory(log_directory);
	if(retval == FALSE)
		return FALSE;
	/* Autoguider */
	Autoguider_General_Set_Log_Handler_Function(Autoguider_General_Log_Handler_Log_Hourly_File);
	Autoguider_General_Set_Log_Filter_Function(Autoguider_General_Log_Filter_Level_Bitwise);
	/* CCD */
	CCD_General_Set_Log_Handler_Function(Autoguider_General_Log_Handler_Log_Hourly_File);
	/* setup command server logging */
	Command_Server_Set_Log_Handler_Function(Autoguider_General_Log_Handler_Log_Hourly_File);
	Command_Server_Set_Log_Filter_Function(Command_Server_Log_Filter_Level_Bitwise);
	/* setup NGATCil logging */
	NGATCil_General_Set_Log_Handler_Function(Autoguider_General_Log_Handler_Log_Hourly_File);
	NGATCil_General_Set_Log_Filter_Function(NGATCil_General_Log_Filter_Level_Bitwise);
	/* libdprt_object logging */
	Object_Set_Log_Handler_Function(Autoguider_General_Log_Handler_Log_Hourly_File);
	Object_Set_Log_Filter_Function(Object_Log_Filter_Level_Bitwise);
	return TRUE;
}

/**
 * Initialise the CCD conenction, initialise the CCD and set the temperature.
 * Retrieves the shared library from "ccd.driver.shared_library".
 * Retrieves the registration function from "ccd.driver.registration_function".
 * Configures the CCD library based on "ccd.temperature.target", 
 * "ccd.temperature.cooler.on" and "ccd.exposure.loop.pause.length".
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Double
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Boolean
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 * @see ../ccd/cdocs/ccd_driver.html#CCD_Driver_Register
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Error
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Initialise
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Startup
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Set
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Cooler_On
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Cooler_Off
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Loop_Pause_Length_Set
 */
static int Autoguider_Startup_CCD(void)
{
	char *shared_library_name = NULL;
	char *registration_function = NULL;
	double target_temperature;
	int retval,cooler_on,ms;

	/* driver registration */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Startup_CCD:"
			       "Get driver registration data.");
#endif
	retval = CCD_Config_Get_String("ccd.driver.shared_library",&shared_library_name);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 3;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:"
			"Failed to load CCD driver shared library name.");
		return FALSE;
	}
	retval = CCD_Config_Get_String("ccd.driver.registration_function",&registration_function);
	if(retval == FALSE)
	{
		if(shared_library_name != NULL)
			free(shared_library_name);
		Autoguider_General_Error_Number = 4;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:"
			"Failed to load CCD driver registration function name.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Startup_CCD:Registering driver.");
#endif
	retval = CCD_Driver_Register(shared_library_name,registration_function);
	if(retval == FALSE)
	{
		if(shared_library_name != NULL)
			free(shared_library_name);
		if(registration_function != NULL)
			free(registration_function);
		Autoguider_General_Error_Number = 5;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:Failed to register CCD driver.");
		return FALSE;
	}
	if(shared_library_name != NULL)
		free(shared_library_name);
	if(registration_function != NULL)
		free(registration_function);
	/* setup startup */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"CCD_Setup_Initialise.");
#endif
	CCD_Setup_Initialise();
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"CCD_Setup_Startup.");
#endif
	retval = CCD_Setup_Startup();
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 6;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:CCD_Setup_Startup failed.");
		return FALSE;
	}
	/* temp init */
	retval = CCD_Config_Get_Double("ccd.temperature.target",&target_temperature);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 7;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:"
			"Failed to get target temperature from config.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Setting target temperature to %.2f\n",
				      target_temperature);
#endif
	retval = CCD_Temperature_Set(target_temperature);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 8;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:"
			"Failed to set target temperature.");
		return FALSE;
	}
	/* turn cooler on? */
	retval = CCD_Config_Get_Boolean("ccd.temperature.cooler.on",&cooler_on);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 9;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:"
			"Failed to get cooler configuration.");
		return FALSE;
	}
	if(cooler_on)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Turning cooler on.");
#endif
		retval = CCD_Temperature_Cooler_On();
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 10;
			sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:"
				"Failed to turn cooler on.");
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"NOT turning cooler on");
#endif
	}
	/* setup pause length in exposure loop */
	retval = CCD_Config_Get_Integer("ccd.exposure.loop.pause.length",&ms);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 18;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:"
			"Failed to get exposure loop pause length.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"CCD_Exposure_Loop_Pause_Length_Set(%d).",ms);
#endif
	retval = CCD_Exposure_Loop_Pause_Length_Set(ms);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 19;
		sprintf(Autoguider_General_Error_String,"Autoguider_Startup_CCD:"
			"CCD_Exposure_Loop_Pause_Length_Set failed.");
		return FALSE;
	}
	return TRUE;
}

/**
 * Shutdown the CCD conenction, ramping the temperature to ambient if necessary.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Boolean
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Error
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Get
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Cooler_Off
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Shutdown
 * @see ../ccd/cdocs/ccd_driver.html#CCD_Driver_Close
 */
static int Autoguider_Shutdown_CCD(void)
{
	int cooler_off,ramp_to_ambient;
	double current_temperature;
	enum CCD_TEMPERATURE_STATUS temperature_status;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Shutdown_CCD:started.");
#endif
	/* turn the cooler off */
	retval = CCD_Config_Get_Boolean("ccd.temperature.cooler.off",&cooler_off);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 11;
		sprintf(Autoguider_General_Error_String,"Autoguider_Shutdown_CCD:"
			"Failed to get cooler configuration.");
		return FALSE;
	}
	if(cooler_off)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Shutdown_CCD:"
				       "Turning cooler off");
#endif
		retval = CCD_Temperature_Cooler_Off();
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 12;
			sprintf(Autoguider_General_Error_String,"Autoguider_Shutdown_CCD:"
				"Failed to get turn cooler off.");
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Shutdown_CCD:"
				       "NOT turning cooler off");
#endif
	}
	/* the cooler off command may ramp to ambient - should we wait for this to happen? */
	/* ramp to ambient */
	retval = CCD_Config_Get_Boolean("ccd.temperature.ramp_to_ambient",&ramp_to_ambient);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 13;
		sprintf(Autoguider_General_Error_String,"Autoguider_Shutdown_CCD:"
			"Failed to get ambient ramping configuration.");
		return FALSE;
	}
	if(ramp_to_ambient)
	{
		current_temperature = -999.0;
		temperature_status = CCD_TEMPERATURE_STATUS_OK;
		/* wait until temp controller thinks we are at ambient or current temperature < 0 */
		while((temperature_status != CCD_TEMPERATURE_STATUS_AMBIENT)&&(current_temperature < 0.0))
		{
			/* get current temp info */
			retval = CCD_Temperature_Get(&current_temperature,&temperature_status);
			if(retval == FALSE)
			{
				Autoguider_General_Error_Number = 14;
				sprintf(Autoguider_General_Error_String,"Autoguider_Shutdown_CCD:"
					"Failed to get temperature.");
				Autoguider_General_Error();
				fprintf(stdout,"Failed to read temperature whilst ramping to ambient.\n");
				/* don't return FALSE - may cause CCD to warm up too quickly */
			}
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Shutdown_CCD:"
						      "Current Temperature:%lf.",current_temperature);
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Shutdown_CCD:"
						      "Temperature Status:%s.",
						      CCD_Temperature_Status_To_String(temperature_status));
#endif
			/* wait a bit */
			sleep(1);
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Shutdown_CCD:"
				       "NOT ramping to ambient.");
#endif
	}
	/* setup shutdown */
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Shutdown_CCD:"
				       "CCD_Setup_Shutdown");
#endif
	retval = CCD_Setup_Shutdown();
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 15;
		sprintf(Autoguider_General_Error_String,"Autoguider_Shutdown_CCD:CCD_Setup_Shutdown failed.");
		return FALSE;
	}
	/* driver shutdown */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GENERAL,"Autoguider_Shutdown_CCD:CCD_Driver_Close");
#endif
	retval = CCD_Driver_Close();
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 16;
		sprintf(Autoguider_General_Error_String,"Autoguider_Shutdown_CCD:Failed to close CCD driver.");
		return FALSE;
	}
	return TRUE;
}

/**
 * Help routine.
 */
static void Help(void)
{
	fprintf(stdout,"Autoguider:Help.\n");
	fprintf(stdout,"autoguider \n");
	fprintf(stdout,"\t[-co[nfig_filename] <filename>]\n");
	fprintf(stdout,"\t[-[al|autoguider_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-[ccdl|ccd_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-[csl|command_server_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-[ncl|ngatcil_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-ol|-object_log_level] <bit field>]\n");
	fprintf(stdout,"\t[-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-help prints out this message and stops the program.\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t<config filename> should be a valid configuration filename.\n");
	fprintf(stdout,"\tAutoguider log levels defined in /home/dev/src/autoguider/include/autoguider_general.h.\n");
	fprintf(stdout,"\tCCD log levels defined in /home/dev/src/autoguider/ccd/include/ccd_general.h.\n");
	fprintf(stdout,"\tAndor CCD log levels defined in /home/dev/src/autoguider/ccd/andor/include/andor_general.h.\n");
	fprintf(stdout,"\tCommand Server log levels defined in /home/dev/src/autoguider/commmand_server/include/command_server.h.\n");
	fprintf(stdout,"\tNGATCil log levels defined in /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h.\n");
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see autoguider_general.html#Autoguider_General_Set_Config_Filename
 * @see autoguider_general.html#Autoguider_General_Set_Log_Filter_Level
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Set_Log_Filter_Function
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Set_Log_Filter_Level
 * @see ../commandserver/cdocs/command_server.html#Command_Server_Set_Log_Filter_Level
 * @see ../ngatcil/cdocs/ngatcil_general.html#NGATCil_General_Set_Log_Filter_Level
 * @see ../../libdprt/object/cdocs/object.html#Object_Set_Log_Filter_Level
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-autoguider_log_level")==0)||(strcmp(argv[i],"-al")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&log_level);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing log level %s failed.\n",argv[i+1]);
					return FALSE;
				}
				Autoguider_General_Set_Log_Filter_Level(log_level);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Log Level requires a level.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-ccd_log_level")==0)||(strcmp(argv[i],"-ccdl")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&log_level);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing log level %s failed.\n",argv[i+1]);
					return FALSE;
				}
				CCD_General_Set_Log_Filter_Function(CCD_General_Log_Filter_Level_Bitwise);
				CCD_General_Set_Log_Filter_Level(log_level);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Log Level requires a level.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-command_server_log_level")==0)||(strcmp(argv[i],"-csl")==0))
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
		else if((strcmp(argv[i],"-config_filename")==0)||(strcmp(argv[i],"-co")==0))
		{
			if((i+1)<argc)
			{
				if(!Autoguider_General_Set_Config_Filename(argv[i+1]))
				{
					fprintf(stderr,"Parse_Arguments:"
						"Autoguider_General_Set_Config_Filename failed.\n");
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:config filename required.\n");
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
		else if((strcmp(argv[i],"-object_log_level")==0)||(strcmp(argv[i],"-ol")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&log_level);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing log level %s failed.\n",argv[i+1]);
					return FALSE;
				}
				Object_Set_Log_Filter_Level(log_level);
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Log Level requires a level.\n");
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

/*
** $Log: not supported by cvs2svn $
** Revision 1.5  2006/08/29 13:55:42  cjm
** Added calls to set SDB AG_STATE.
**
** Revision 1.4  2006/07/20 15:14:31  cjm
** Added SDB Open/Close initial Status setting.
**
** Revision 1.3  2006/06/20 18:42:38  cjm
** Added setting of autoguider logging to bitwise filtering.
**
** Revision 1.2  2006/06/12 19:22:13  cjm
** Added -ngatcil_log_level.
** Added NGATCil logging setup.
** Added starting/stopping of CIL server.
**
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
