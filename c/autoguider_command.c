/* autoguider_command.c
** Autoguider command routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_command.c,v 1.1 2006-06-01 15:18:38 cjm Exp $
*/
/**
 * Command routines for the autoguider program.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ccd_exposure.h"
#include "ccd_general.h"
#include "ccd_setup.h"
#include "ccd_temperature.h"

#include "autoguider_general.h"
#include "autoguider_server.h"
#include "autoguider_field.h"
#include "autoguider_get_fits.h"
#include "autoguider_guide.h"
#include "autoguider_object.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_command.c,v 1.1 2006-06-01 15:18:38 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Handle a command of the form: "abort".
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_COMMAND
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Get_Config_Filename
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Abort
 */
int Autoguider_Command_Abort(char *command_string,char **reply_string)
{
	char *config_filename = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Abort:started.");
#endif
	/* are we fielding/guiding etc? */
	if((Autoguider_Field_Is_Fielding() == FALSE)&&(Autoguider_Guide_Is_Guiding() == FALSE))
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Abort failed:No Field or Guide operation underway."))
			return FALSE;
		return TRUE;
	}
	retval = CCD_Exposure_Abort();
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 318;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Abort:CCD_Exposure_Abort failed.");
		Autoguider_General_Error();
		if(!Autoguider_General_Add_String(reply_string,"1 Abort failed."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_General_Add_String(reply_string,"0 Abort suceeded."))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Abort:finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "configload".
 * Note if this is called when <b>anything</b> else is hapenning, you'll probably crash the control system.
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_COMMAND
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Get_Config_Filename
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Shutdown
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Load
 */
int Autoguider_Command_Config_Load(char *command_string,char **reply_string)
{
	char *config_filename = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Config_Load:started.");
#endif
	config_filename = Autoguider_General_Get_Config_Filename();
	/* are we fielding/guiding etc? */
	if(Autoguider_Field_Is_Fielding())
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Config Load failed:Field operation underway."))
			return FALSE;
		return TRUE;
	}
	if(Autoguider_Guide_Is_Guiding())
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Config Load failed:Guide operation underway."))
			return FALSE;
		return TRUE;
	}
	/* shutdown and free the currently loaded config */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Config_Load:"
			       "Calling CCD_Config_Shutdown: Warning this is dangerous operation.");
#endif
	retval = CCD_Config_Shutdown();
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 316;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Config_Load:CCD_Config_Shutdown failed.");
		Autoguider_General_Error();
		if(!Autoguider_General_Add_String(reply_string,"1 Config Load failed."))
			return FALSE;
		return TRUE;
	}
	/* try re loading the config file */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Config_Load:"
			       "Calling CCD_Config_Load: Warning this is dangerous operation.");
#endif
	retval = CCD_Config_Load(Autoguider_General_Get_Config_Filename());
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 317;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Config_Load:CCD_Config_Load(%s) failed.",
			Autoguider_General_Get_Config_Filename());
		Autoguider_General_Error();
		if(!Autoguider_General_Add_String(reply_string,"1 Config Load failed."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_General_Add_String(reply_string,"0 Config Load suceeded."))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Config_Load:finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "status <type> <element>".
 * <ul>
 * <li>status temperature get
 * <li>status temperature status
 * <li>status field &lt;active|dark|flat|object&gt;
 * <li>status guide &lt;active|dark|flat|object&gt;
 * <li>status object &lt;list|count&gt;
 * </ul>
 * @param command_string The status command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_field.html#Autoguider_Field_Is_Fielding
 * @see autoguider_field.html#Autoguider_Field_Get_Do_Dark_Subtract
 * @see autoguider_field.html#Autoguider_Field_Get_Do_Flat_Field
 * @see autoguider_field.html#Autoguider_Field_Get_Do_Object_Detect
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_COMMAND
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_guide.html#Autoguider_Guide_Is_Guiding
 * @see autoguider_guide.html#Autoguider_Guide_Get_Do_Dark_Subtract
 * @see autoguider_guide.html#Autoguider_Guide_Get_Do_Flat_Field
 * @see autoguider_guide.html#Autoguider_Guide_Get_Do_Object_Detect
 * @see autoguider_object.html#Autoguider_Object_List_Get_Count
 * @see autoguider_object.html#Autoguider_Object_List_Get_Object_List_String
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Error
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Get
 */
int Autoguider_Command_Status(char *command_string,char **reply_string)
{
	double current_temperature;
	enum CCD_TEMPERATURE_STATUS temperature_status;
	char type_string[64];
	char element_string[64];
	char buff[256];
	char *object_list_string = NULL;
	int retval,ivalue;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Status:started.");
#endif
	if(command_string == NULL)
	{
		Autoguider_General_Error_Number = 300;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Status:command_string was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Status:parsing command string.");
#endif
	retval = sscanf(command_string,"status %64s %64s",type_string,element_string);
	if(retval != 2)
	{
		Autoguider_General_Error_Number = 301;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Status:"
			"Failed to parse status command %s (%d).",command_string,retval);
		return FALSE;
	}
	if(strcmp(type_string,"temperature") == 0)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Status:"
				       "temperature status detected.");
#endif
		retval = CCD_Temperature_Get(&current_temperature,&temperature_status);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 302;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Status:"
				"CCD_Temperature_Get failed.");
			Autoguider_General_Error();
			if(!Autoguider_General_Add_String(reply_string,"1 Failed to get current temperature status."))
				return FALSE;
			return TRUE;
		}
		if(strcmp(element_string,"get") == 0)
		{
			if(!Autoguider_General_Add_String(reply_string,"0 "))
				return FALSE;
			sprintf(buff,"%.2f",current_temperature);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"status") == 0)
		{
			if(!Autoguider_General_Add_String(reply_string,"0 "))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,
							  CCD_Temperature_Status_To_String(temperature_status)))
				return FALSE;
			return TRUE;
		}
		else
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Unknown temperature element:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,element_string))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
				return FALSE;
			return TRUE;
		}
	}
	else if(strcmp(type_string,"field") == 0)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Status:"
				       "field status detected.");
#endif
		if(strcmp(element_string,"active") == 0)
		{
			if(Autoguider_Field_Is_Fielding())
			{
				if(!Autoguider_General_Add_String(reply_string,"0 true"))
					return FALSE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"0 false"))
					return FALSE;
			}
			return TRUE;
		}
		else if(strcmp(element_string,"dark") == 0)
		{
			if(Autoguider_Field_Get_Do_Dark_Subtract())
			{
				if(!Autoguider_General_Add_String(reply_string,"0 true"))
					return FALSE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"0 false"))
					return FALSE;
			}
			return TRUE;
		}
		else if(strcmp(element_string,"flat") == 0)
		{
			if(Autoguider_Field_Get_Do_Flat_Field())
			{
				if(!Autoguider_General_Add_String(reply_string,"0 true"))
					return FALSE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"0 false"))
					return FALSE;
			}
			return TRUE;
		}
		else if(strcmp(element_string,"object") == 0)
		{
			if(Autoguider_Field_Get_Do_Object_Detect())
			{
				if(!Autoguider_General_Add_String(reply_string,"0 true"))
					return FALSE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"0 false"))
					return FALSE;
			}
			return TRUE;
		}
		else
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Unknown field element:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,element_string))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
				return FALSE;
			return TRUE;
		}
	}
	else if(strcmp(type_string,"guide") == 0)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Status:"
				       "guide status detected.");
#endif
		if(strcmp(element_string,"active") == 0)
		{
			if(Autoguider_Guide_Is_Guiding())
			{
				if(!Autoguider_General_Add_String(reply_string,"0 true"))
					return FALSE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"0 false"))
					return FALSE;
			}
			return TRUE;
		}
		else if(strcmp(element_string,"dark") == 0)
		{
			if(Autoguider_Guide_Get_Do_Dark_Subtract())
			{
				if(!Autoguider_General_Add_String(reply_string,"0 true"))
					return FALSE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"0 false"))
					return FALSE;
			}
			return TRUE;
		}
		else if(strcmp(element_string,"flat") == 0)
		{
			if(Autoguider_Guide_Get_Do_Flat_Field())
			{
				if(!Autoguider_General_Add_String(reply_string,"0 true"))
					return FALSE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"0 false"))
					return FALSE;
			}
			return TRUE;
		}
		else if(strcmp(element_string,"object") == 0)
		{
			if(Autoguider_Guide_Get_Do_Object_Detect())
			{
				if(!Autoguider_General_Add_String(reply_string,"0 true"))
					return FALSE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"0 false"))
					return FALSE;
			}
			return TRUE;
		}
		else
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Unknown guide element:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,element_string))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
				return FALSE;
			return TRUE;
		}
	}
	else if(strcmp(type_string,"object") == 0)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Status:"
				       "object status detected.");
#endif
		if(strcmp(element_string,"count") == 0)
		{
			if(!Autoguider_Object_List_Get_Count(&ivalue))
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,"1 Failed to get object list count."))
					return FALSE;
				return TRUE;
			}
			sprintf(buff,"0 %d",ivalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"list") == 0)
		{
			if(!Autoguider_Object_List_Get_Object_List_String(&object_list_string))
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,"1 Failed to get object list."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 \n"))
			{
				if(object_list_string != NULL)
					free(object_list_string);
				return FALSE;
			}
			if(!Autoguider_General_Add_String(reply_string,object_list_string))
			{
				if(object_list_string != NULL)
					free(object_list_string);
				return FALSE;
			}
			/* free allocated string */
			if(object_list_string != NULL)
				free(object_list_string);
			return TRUE;
		}
		else
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Unknown object element:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,element_string))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
				return FALSE;
			return TRUE;
		}
	}
	else
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Unknown type:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,type_string))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
		return TRUE;
	}
	return TRUE;
}

/**
 * Handle a command of the form: "temperature [set <C>|cooler [on|off]".
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_COMMAND
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Set
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Cooler_On
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Cooler_Off
 */
int Autoguider_Command_Temperature(char *command_string,char **reply_string)
{
	double target_temperature;
	char type_string[64];
	char parameter_string[64];
	char buff[256];
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Temperature:started.");
#endif
	if(command_string == NULL)
	{
		Autoguider_General_Error_Number = 303;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:command_string was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Temperature:"
				       "parsing command string.");
#endif
	retval = sscanf(command_string,"temperature %64s %64s",type_string,parameter_string);
	if(retval != 2)
	{
		Autoguider_General_Error_Number = 304;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:"
			"Failed to parse temperature command %s (%d).",command_string,retval);
		Autoguider_General_Error();
		if(!Autoguider_General_Add_String(reply_string,"1 Failed to parse command string:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,command_string))
			return FALSE;
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
					       "Autoguider_Command_Temperature:finished (command parse failed).");
#endif
			return TRUE;
	}
	if(strcmp(type_string,"set") == 0)
	{
		retval = sscanf(parameter_string,"%lf",&target_temperature);
		if(retval != 1)
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Failed to parse target temperature:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,parameter_string))
				return FALSE;
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
					       "Autoguider_Command_Temperature:finished.");
#endif
			return TRUE;
		}
#if AUTOGUIDER_DEBUG > 3
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Temperature:"
				       "Setting target temperature to %lf.",target_temperature);
#endif
		retval = CCD_Temperature_Set(target_temperature);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 305;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:"
				"Failed to set temperature.");
			Autoguider_General_Error();
			if(!Autoguider_General_Add_String(reply_string,"1 Failed to set target temperature."))
				return FALSE;
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
					       "Autoguider_Command_Temperature:finished.");
#endif
			return TRUE;
		}
		/* reply ok */
		if(!Autoguider_General_Add_String(reply_string,"0 target temperature now:"))
			return FALSE;
		sprintf(buff,"%lf",target_temperature);
		if(!Autoguider_General_Add_String(reply_string,buff))
			return FALSE;
	}
	else if(strcmp(type_string,"cooler") == 0)
	{
		if(strcmp(parameter_string,"on")==0)
		{
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Temperature:"
					       "Turning cooler on.");
#endif
			retval = CCD_Temperature_Cooler_On();
			if(retval == FALSE)
			{
				Autoguider_General_Error_Number = 306;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:"
					"Failed to turn cooler on.");
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,"1 Failed to turn cooler on."))
					return FALSE;
#if AUTOGUIDER_DEBUG > 1
				Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
						       "Autoguider_Command_Temperature:finished.");
#endif
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 cooler on"))
				return FALSE;
		}
		else if(strcmp(parameter_string,"off")==0)
		{
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Temperature:"
					       "Turning cooler off.");
#endif
			retval = CCD_Temperature_Cooler_Off();
			if(retval == FALSE)
			{
				Autoguider_General_Error_Number = 307;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:"
					"Failed to turn cooler on.");
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,"1 Failed to turn cooler off."))
					return FALSE;
#if AUTOGUIDER_DEBUG > 1
				Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
						       "Autoguider_Command_Temperature:finished.");
#endif
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 cooler off"))
				return FALSE;
		}
		else
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Unknown Parameter."))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,parameter_string))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
				return FALSE;
		}
	}
	else
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Unknown type:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,type_string))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
				       "Autoguider_Command_Temperature:finished.");
#endif
		return TRUE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
			       "Autoguider_Command_Temperature:finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "field".
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_COMMAND
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_field.html#Autoguider_Field
 */
int Autoguider_Command_Field(char *command_string,char **reply_string)
{
	char parameter_string1[64];
	char parameter_string2[64];
	int parameter_count,retval,exposure_length,doit;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Field:started.");
#endif
	/* parse command */
	parameter_count = sscanf(command_string,"field %64s %64s",parameter_string1,parameter_string2);
	if(parameter_count == 2)
	{
		/* check parameter 2 is on or off and set as appropriate */
		if(strcmp(parameter_string2,"on") == 0)
		{
			doit = TRUE;
		}
		else if(strcmp(parameter_string2,"off") == 0)
		{
			doit = FALSE;
		}
		else
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Field parameter 2 not on or off:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,parameter_string2))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
				return FALSE;
			return TRUE;
		}
		/* check parameter 1  */
		if(strcmp(parameter_string1,"dark") == 0)
		{
			retval = Autoguider_Field_Set_Do_Dark_Subtract(doit);
			if(retval == FALSE)
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting field dark subtraction failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Field Do Dark Subtraction set."))
				return FALSE;
		}
		else if(strcmp(parameter_string1,"flat") == 0)
		{
			retval = Autoguider_Field_Set_Do_Flat_Field(doit);
			if(retval == FALSE)
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting field flat field failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Field Do Flat Fielding set."))
				return FALSE;
		}
		else if(strcmp(parameter_string1,"object") == 0)
		{
			retval = Autoguider_Field_Set_Do_Object_Detect(doit);
			if(retval == FALSE)
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting field object detect failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Field Do Object Detect set."))
				return FALSE;
		}
		else
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Unknown field parameter 1:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,parameter_string1))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
				return FALSE;
			return TRUE;
		}
	}
	else if(parameter_count == 1)
	{
		retval = sscanf(command_string,"field %d",&exposure_length);
		if(retval != 1)
		{
			Autoguider_General_Error_Number = 313;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Field:"
				"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
					       "Autoguider_Command_Field:finished (command parse failed).");
#endif
			return FALSE;
		}
		retval = Autoguider_Field_Exposure_Length_Set(exposure_length);
		if(retval == FALSE)
		{
			Autoguider_General_Error();
			if(!Autoguider_General_Add_String(reply_string,"1 Setting field exposure length failed."))
				return FALSE;
			return TRUE;
		}
	}
	/* do actual field operation if "field" or "field <n>" */
	if((parameter_count == 0) || (parameter_count == 1))
	{
		retval = Autoguider_Field();
		if(retval == FALSE)
		{
			Autoguider_General_Error();
			if(!Autoguider_General_Add_String(reply_string,"1 Field failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Field suceeded."))
			return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Field:finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "expose <ms>".
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_COMMAND
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_field.html#Autoguider_Field_Expose
 * @see autoguider_field.html#Autoguider_Field_Exposure_Length_Set
 */
int Autoguider_Command_Expose(char *command_string,char **reply_string)
{
	int retval,exposure_length;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Expose:started.");
#endif
	/* parse command */
	retval = sscanf(command_string,"expose %d",&exposure_length);
	if(retval != 1)
	{
		Autoguider_General_Error_Number = 315;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Expose:"
			"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
				       "Autoguider_Command_Expose:finished (command parse failed).");
#endif
		return FALSE;
	}
	retval = Autoguider_Field_Exposure_Length_Set(exposure_length);
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		if(!Autoguider_General_Add_String(reply_string,"1 Setting field exposure length failed."))
			return FALSE;
		return TRUE;
	}
	retval = Autoguider_Field_Expose();
	if(retval == FALSE)
	{
		Autoguider_General_Error();
		if(!Autoguider_General_Add_String(reply_string,"1 Expose failed."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_General_Add_String(reply_string,"0 Expose suceeded."))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Expose:finished.");
#endif
	return TRUE;
}

/**
 * Handle a commands of the form: 
 * <ul>
 * <li>guide [on|off]
 * <li>guide window <sx> <sy> <ex> <ey>
 * <li>guide exposure_length <ms>
 * <li>guide <dark|flat|object> <on|off>
 * <li>guide <object> <index>
 * </ul>
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_COMMAND
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_guide.html#Autoguider_Guide_On
 * @see autoguider_guide.html#Autoguider_Guide_Off
 * @see autoguider_guide.html#Autoguider_Guide_Window_Set
 * @see autoguider_guide.html#Autoguider_Guide_Exposure_Length_Set
 * @see autoguider_guide.html#Autoguider_Guide_Set_Do_Dark_Subtract
 * @see autoguider_guide.html#Autoguider_Guide_Set_Do_Flat_Field
 * @see autoguider_guide.html#Autoguider_Guide_Set_Do_Object_Detect
 * @see autoguider_guide.html#Autoguider_Guide_Set_Guide_Object
 */
int Autoguider_Command_Guide(char *command_string,char **reply_string)
{
	char parameter_string1[64];
	char parameter_string2[64];
	char parameter_string3[64];
	char parameter_string4[64];
	char parameter_string5[64];
	int parameter_count,retval,sx,sy,ex,ey,exposure_length,doit,object_index;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Guide:started.");
#endif
	/* parse command */
	parameter_count = sscanf(command_string,"guide %64s %64s %64s %64s %64s",parameter_string1,parameter_string2,
				 parameter_string3,parameter_string4,parameter_string5);
	if(parameter_count < 1)
	{
		Autoguider_General_Error_Number = 310;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Guide:"
			"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
				       "Autoguider_Command_Guide:finished (command parse failed).");
#endif
		return FALSE;
	}
	if(strcmp(parameter_string1,"on") == 0)
	{
		retval = Autoguider_Guide_On();
		if(retval == FALSE)
		{
			Autoguider_General_Error();
			if(!Autoguider_General_Add_String(reply_string,"1 Guide on failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Guide on suceeded."))
			return FALSE;
	}
	else if(strcmp(parameter_string1,"off") == 0)
	{
		retval = Autoguider_Guide_Off();
		if(retval == FALSE)
		{
			Autoguider_General_Error();
			if(!Autoguider_General_Add_String(reply_string,"1 Guide off failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Guide off suceeded."))
			return FALSE;
	}
	else if(strncmp(parameter_string1,"window",6) == 0)
	{
		retval = sscanf(command_string,"guide window %d %d %d %d",&sx,&sy,&ex,&ey);
		if(retval != 4)
		{
			Autoguider_General_Error_Number = 311;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Guide:"
				"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
					       "Autoguider_Command_Guide:finished (command parse failed).");
#endif
			return FALSE;
		}
		retval = Autoguider_Guide_Window_Set(sx,sy,ex,ey);
		if(retval == FALSE)
		{
			Autoguider_General_Error();
			if(!Autoguider_General_Add_String(reply_string,"1 Guide window failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Guide window suceeded."))
			return FALSE;
	}
	else if(strncmp(parameter_string1,"exposure_length",15) == 0)
	{
		retval = sscanf(command_string,"guide exposure_length %d",&exposure_length);
		if(retval != 1)
		{
			Autoguider_General_Error_Number = 312;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Guide:"
				"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
					       "Autoguider_Command_Guide:finished (command parse failed).");
#endif
			return FALSE;
		}
		retval = Autoguider_Guide_Exposure_Length_Set(exposure_length,FALSE);
		if(retval == FALSE)
		{
			Autoguider_General_Error();
			if(!Autoguider_General_Add_String(reply_string,"1 Setting guide exposure length failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Setting guide exposure length suceeded."))
			return FALSE;
	}
	else if((strncmp(parameter_string1,"dark",4) == 0)||(strncmp(parameter_string1,"flat",4) == 0))
	{
		if(parameter_count != 2)
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Wrong number of parameters specified."))
				return FALSE;
			return TRUE;
		}
		/* check parameter 2 is on or off and set as appropriate */
		if(strcmp(parameter_string2,"on") == 0)
		{
			doit = TRUE;
		}
		else if(strcmp(parameter_string2,"off") == 0)
		{
			doit = FALSE;
		}
		else
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Guide parameter 2 not on or off:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,parameter_string2))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
				return FALSE;
			return TRUE;
		}
		/* check parameter 1  */
		if(strcmp(parameter_string1,"dark") == 0)
		{
			retval = Autoguider_Guide_Set_Do_Dark_Subtract(doit);
			if(retval == FALSE)
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting guide dark subtraction failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Guide Do Dark Subtraction set."))
				return FALSE;
		}
		else if(strcmp(parameter_string1,"flat") == 0)
		{
			retval = Autoguider_Guide_Set_Do_Flat_Field(doit);
			if(retval == FALSE)
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting guide flat field failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Guide Do Flat Fielding set."))
				return FALSE;
		}
	}
	else if(strncmp(parameter_string1,"object",6) == 0)
	{
		if(parameter_count != 2)
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Wrong number of parameters specified."))
				return FALSE;
			return TRUE;
		}
		/* check parameter 2 is on or off OR an object index and set as appropriate */
		object_index = -1;
		if(strcmp(parameter_string2,"on") == 0)
		{
			doit = TRUE;
		}
		else if(strcmp(parameter_string2,"off") == 0)
		{
			doit = FALSE;
		}
		else
		{
			retval = sscanf(parameter_string2,"%d",&object_index);
			if(retval != 1)
			{
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Guide parameter 2 not on/off/number:"))
					return FALSE;
				if(!Autoguider_General_Add_String(reply_string,parameter_string2))
					return FALSE;
				if(!Autoguider_General_Add_String(reply_string,"."))
					return FALSE;
				return TRUE;
			}
		}
		/* turning object detection on/off */
		if((strcmp(parameter_string2,"on") == 0)||(strcmp(parameter_string2,"off") == 0))
		{
			retval = Autoguider_Guide_Set_Do_Object_Detect(doit);
			if(retval == FALSE)
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting guide object detect failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Guide Do Object Detect set."))
				return FALSE;
		}
		else
		{
			/* setting to guide on object index object_index */
			retval = Autoguider_Guide_Set_Guide_Object(object_index);
			if(retval == FALSE)
			{
				Autoguider_General_Error();
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting guide object failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Setting guide object ok."))
				return FALSE;
		}
	}
	else
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Guide command failed:Unknown parameter:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,parameter_string1))
			return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Guide:finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "getfits [field|guide] [raw|reduced]".
 * @param command_string The command. This is not changed during this routine.
 * @param buffer_ptr The address of a pointer to allocate and store a FITS image in memory.
 * @param buffer_length The address of a word to store the length of the created returned data.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_COMMAND
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_get_fits.html#Autoguider_Get_Fits
 */
int Autoguider_Command_Get_Fits(char *command_string,void **buffer_ptr,size_t *buffer_length)
{
	char type_parameter_string[64];
	char state_parameter_string[64];
	int retval,buffer_type,buffer_state;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Get_Fits:started.");
#endif
	/* parse command */
	retval = sscanf(command_string,"getfits %64s %64s",type_parameter_string,state_parameter_string);
	if(retval != 2)
	{
		Autoguider_General_Error_Number = 308;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Get_Fits:"
			"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,
				       "Autoguider_Command_Get_Fits:finished (command parse failed).");
#endif
		return FALSE;
	}
	/* parse type parameter */
	if(strcmp(type_parameter_string,"field") == 0)
		buffer_type = AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD;
	else if(strcmp(type_parameter_string,"guide") == 0)
		buffer_type = AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE;
	else
	{
		Autoguider_General_Error_Number = 309;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Get_Fits:"
			"Get FITS had illegal type parameter:%s.",type_parameter_string);
		return FALSE;
	}
	/* parse state parameter */
	if(strcmp(state_parameter_string,"raw") == 0)
		buffer_state = AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW;
	else if(strcmp(state_parameter_string,"reduced") == 0)
		buffer_state = AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED;
	else
	{
		Autoguider_General_Error_Number = 314;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Get_Fits:"
			"Get FITS had illegal state parameter:%s.",state_parameter_string);
		return FALSE;
	}
	/* get FITS in memory buffer */
	retval = Autoguider_Get_Fits(buffer_type,buffer_state,buffer_ptr,buffer_length);
	if(retval == FALSE)
	{
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_COMMAND,"Autoguider_Command_Get_Fits:finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/