/* autoguider_command.c
** Autoguider command routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_command.c,v 1.18 2014-01-31 17:17:17 cjm Exp $
*/
/**
 * Command routines for the autoguider program.
 * @author Chris Mottram
 * @version $Revision: 1.18 $
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

#include "log_udp.h"

#include "object.h"

#include "ccd_config.h"
#include "ccd_exposure.h"
#include "ccd_general.h"
#include "ccd_setup.h"
#include "ccd_temperature.h"

#include "command_server.h"

#include "ngatcil_general.h"

#include "autoguider_cil.h"
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
static char rcsid[] = "$Id: autoguider_command.c,v 1.18 2014-01-31 17:17:17 cjm Exp $";

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
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Abort",LOG_VERBOSITY_TERSE,
			       "COMMAND","started.");
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
		Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Abort",
					 LOG_VERBOSITY_TERSE,"COMMAND");
		if(!Autoguider_General_Add_String(reply_string,"1 Abort failed."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_General_Add_String(reply_string,"0 Abort suceeded."))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Abort",LOG_VERBOSITY_TERSE,
			       "COMMAND","finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "agstate <ms>".
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_cil.html#Autoguider_CIL_SDB_State_Set
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Send
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Command_Agstate(char *command_string,char **reply_string)
{
	int retval,agstate;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Agstate",LOG_VERBOSITY_TERSE,
			       "COMMAND","started.");
#endif
	/* parse command */
	retval = sscanf(command_string,"agstate %d",&agstate);
	if(retval != 1)
	{
		Autoguider_General_Error_Number = 329;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Agstate:"
			"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Agstate",
				       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
#endif
		return FALSE;
	}
	if(!Autoguider_CIL_SDB_Packet_State_Set(agstate))
	{
		Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Agstate",
					 LOG_VERBOSITY_TERSE,"COMMAND");
		if(!Autoguider_General_Add_String(reply_string,"1 Setting AgState failed."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_CIL_SDB_Packet_Send())
	{
		Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Agstate",
					 LOG_VERBOSITY_TERSE,"COMMAND");
		if(!Autoguider_General_Add_String(reply_string,"1 Sending AgState to SDB failed."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_General_Add_String(reply_string,"0 Agstate suceeded."))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Agstate",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "autoguide <on [<pixel <x> <y>>|brightest|<rank <n>>]|off>".
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Autoguider_Command_Autoguide_On
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Get_Config_Filename
 */
int Autoguider_Command_Autoguide(char *command_string,char **reply_string)
{
	char onoff_string[64];
	char parameter1_string[64];
	char parameter2_string[64];
	char parameter3_string[64];
	int retval,parameter_count,rank,on_type;
	float pixel_x,pixel_y;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Autoguide",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	/* parse the command */
	parameter_count = sscanf(command_string,"autoguide %63s %63s %63s %63s",onoff_string,parameter1_string,
			parameter2_string,parameter3_string);
	if(parameter_count < 1)
	{
		Autoguider_General_Error_Number = 321;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Autoguide:"
			"Failed to parse autoguide command %s (%d).",command_string,parameter_count);
		return FALSE;
	}
	if(strcmp(onoff_string,"on") == 0)
	{
		/* parameter stuff */
		if(parameter_count < 2)
		{
			Autoguider_General_Error_Number = 322;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Autoguide:"
				"Failed to parse autoguide command %s (%d).",command_string,parameter_count);
			return FALSE;
		}
		if(strcmp(parameter1_string,"brightest") == 0)
		{
			on_type = COMMAND_AG_ON_TYPE_BRIGHTEST;
		}
		else if(strcmp(parameter1_string,"pixel") == 0)
		{
			on_type = COMMAND_AG_ON_TYPE_PIXEL;
			if(parameter_count < 4)
			{
				Autoguider_General_Error_Number = 324;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Autoguide:"
					"Failed to parse autoguide command %s (%d).",command_string,parameter_count);
				return FALSE;
			}
			retval = sscanf(parameter2_string,"%f",&pixel_x);
			if(retval != 1)
			{
				Autoguider_General_Error_Number = 325;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Autoguide:"
					"Failed to parse autoguide pixel x %s (%d).",parameter2_string,retval);
				return FALSE;
			}
			retval = sscanf(parameter3_string,"%f",&pixel_y);
			if(retval != 1)
			{
				Autoguider_General_Error_Number = 326;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Autoguide:"
					"Failed to parse autoguide pixel y %s (%d).",parameter3_string,retval);
				return FALSE;
			}
		}
		else if(strcmp(parameter1_string,"rank") == 0)
		{
			on_type = COMMAND_AG_ON_TYPE_RANK;
			if(parameter_count < 2)
			{
				Autoguider_General_Error_Number = 327;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Autoguide:"
					"Failed to parse autoguide command %s (%d).",command_string,parameter_count);
				return FALSE;
			}
			retval = sscanf(parameter2_string,"%d",&rank);
			if(retval != 1)
			{
				Autoguider_General_Error_Number = 328;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Autoguide:"
					"Failed to parse autoguide rank %s (%d).",parameter2_string,retval);
				return FALSE;
			}
		}
		else
		{
			Autoguider_General_Error_Number = 323;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Autoguide:"
				"Failed to parse autoguide on command (%s) parameter %s.",
				command_string,parameter1_string);
			return FALSE;
		}
		/* turn autoguider on */
		retval = Autoguider_Command_Autoguide_On(on_type,pixel_x,pixel_y,rank);
		if(retval == FALSE)
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Autoguide",
						 LOG_VERBOSITY_TERSE,"COMMAND");
			if(!Autoguider_General_Add_String(reply_string,"1 autoguide on failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Autoguide on suceeded."))
			return FALSE;
		return TRUE;
	}
	else if(strcmp(onoff_string,"off") == 0)
	{
		/* turn guide loop off */
		retval = Autoguider_Guide_Off();
		if(retval == FALSE)
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Autoguide",
						 LOG_VERBOSITY_TERSE,"COMMAND");
			if(!Autoguider_General_Add_String(reply_string,"1 autoguide off:Guide off failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Autoguide off suceeded."))
			return FALSE;
		return TRUE;
	}
	else 
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Autoguide failed:Illegal onoff parameter:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,onoff_string))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_General_Add_String(reply_string,"0 Autoguide suceeded."))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Autoguide",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
	return TRUE;
}

/**
 * Turn the autoguider 'on'.
 * @param on_type How to select the AG guide object.
 * @param pixel_x If on_type is COMMAND_AG_ON_TYPE_PIXEL, the x pixel position.
 * @param pixel_y If on_type is COMMAND_AG_ON_TYPE_PIXEL, the y pixel position.
 * @param rank If on_type is COMMAND_AG_ON_TYPE_RANK, the rank (ordered index pf brightness).
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #COMMAND_AG_ON_TYPE
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_State_Set
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Send
 * @see autoguider_field.html#Autoguider_Field
 * @see autoguider_field.html#Autoguider_Field_SDB_State_Failed_Then_Idle_Set
 * @see autoguider_field.html#Autoguider_Field_Save_FITS
 * @see autoguider_field.html#Autoguider_Field_Get_Save_FITS_Failed
 * @see autoguider_field.html#Autoguider_Field_Get_Save_FITS_Successful
 * @see autoguider_object.html#Autoguider_Object_Guide_Object_Get
 * @see autoguider_guide.html#Autoguider_Guide_Set_Guide_Object
 * @see autoguider_guide.html#Autoguider_Guide_On
 */
int Autoguider_Command_Autoguide_On(enum COMMAND_AG_ON_TYPE on_type,float pixel_x,float pixel_y,int rank)
{
	int selected_object_index,retval;
	
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Autoguide_On",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	/* do fielding */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Autoguide_On",
			       LOG_VERBOSITY_TERSE,"COMMAND","Fielding.");
#endif
	retval = Autoguider_Field();
	if(retval == FALSE)
	{
		/* SDB should already have been updated by Autoguider_Field in this case. */
		/* save of the failed Field image if configured to do so. */
		if(Autoguider_Field_Get_Save_FITS_Failed())
		{
			if(!Autoguider_Field_Save_FITS(FALSE,-1))
			{
				Autoguider_General_Error("command","autoguider_command.c",
							 "Autoguider_Command_Autoguide_On",
							 LOG_VERBOSITY_TERSE,"COMMAND");
			}
		}
		return FALSE;
	}
	/* check we have an object to guide on */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Autoguide_On",
			       LOG_VERBOSITY_TERSE,"COMMAND","Getting suitable object.");
#endif
	retval = Autoguider_Object_Guide_Object_Get(on_type,pixel_x,pixel_y,rank,&selected_object_index);
	if(retval == FALSE)
	{
		/* update SDB */
		Autoguider_Field_SDB_State_Failed_Then_Idle_Set();
		/* save of the failed Field image if configured to do so. */
		if(Autoguider_Field_Get_Save_FITS_Failed())
		{
			if(!Autoguider_Field_Save_FITS(FALSE,-1))
			{
				Autoguider_General_Error("command","autoguider_command.c",
							 "Autoguider_Command_Autoguide_On",
							 LOG_VERBOSITY_TERSE,"COMMAND");
			}
		}
		return FALSE;
	}
	/* do object selection/ guide setup */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("command","autoguider_command.c","Autoguider_Command_Autoguide_On",
				      LOG_VERBOSITY_TERSE,"COMMAND",
				      "Setting guide object to %d.",selected_object_index);
#endif
	retval = Autoguider_Guide_Set_Guide_Object(selected_object_index);
	if(retval == FALSE)
	{
		/* update SDB */
		Autoguider_Field_SDB_State_Failed_Then_Idle_Set();
		/* save of the failed Field image if configured to do so. */
		if(Autoguider_Field_Get_Save_FITS_Failed())
		{
			if(!Autoguider_Field_Save_FITS(FALSE,-1))
			{
				Autoguider_General_Error("command","autoguider_command.c",
							 "Autoguider_Command_Autoguide_On",
							 LOG_VERBOSITY_TERSE,"COMMAND");
			}
		}
		return FALSE;
	}
	/* save the successful field image if configured to do so */
	if(Autoguider_Field_Get_Save_FITS_Successful())
	{
		if(!Autoguider_Field_Save_FITS(TRUE,selected_object_index))
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Autoguide_On",
						 LOG_VERBOSITY_TERSE,"COMMAND");
		}
	}
	/* turn guide loop on */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Autoguide_On",
			       LOG_VERBOSITY_TERSE,"COMMAND","Turning on guide loop.");
#endif
	retval = Autoguider_Guide_On();
	if(retval == FALSE)
	{
		/* update SDB */
		Autoguider_Field_SDB_State_Failed_Then_Idle_Set();
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Autoguide_On",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
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
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Config_Load",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
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
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Config_Load",
			       LOG_VERBOSITY_TERSE,"COMMAND",
			       "Calling CCD_Config_Shutdown: Warning this is dangerous operation.");
#endif
	retval = CCD_Config_Shutdown();
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 316;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Config_Load:CCD_Config_Shutdown failed.");
		Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Config_Load",
					 LOG_VERBOSITY_TERSE,"COMMAND");
		if(!Autoguider_General_Add_String(reply_string,"1 Config Load failed."))
			return FALSE;
		return TRUE;
	}
	/* try re loading the config file */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Config_Load",
			       LOG_VERBOSITY_TERSE,"COMMAND",
			       "Calling CCD_Config_Load: Warning this is dangerous operation.");
#endif
	retval = CCD_Config_Load(Autoguider_General_Get_Config_Filename());
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 317;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Config_Load:CCD_Config_Load(%s) failed.",
			Autoguider_General_Get_Config_Filename());
		Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Config_Load",
					 LOG_VERBOSITY_TERSE,"COMMAND");
		if(!Autoguider_General_Add_String(reply_string,"1 Config Load failed."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_General_Add_String(reply_string,"0 Config Load suceeded."))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Config_Load",
			       LOG_VERBOSITY_TERSE,"COMMAND","Autoguider_Command_Config_Load:finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "object <variable> <n>". This allows us to set various object variables
 * used to determine the treshold and object detection dynamically.
 * <ul>
 * <li>object sigma <n>
 * <li>object sigma_reject <n>
 * <li>object ellipticity_limit <n>
 * <li>object min_con_pix <n>
 * </ul>
 * @param command_string The status command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_object.html#Autoguider_Object_Threshold_Sigma_Set
 * @see autoguider_object.html#Autoguider_Object_Threshold_Sigma_Reject_Set
 * @see autoguider_object.html#Autoguider_Object_Ellipticity_Limit_Set
 * @see autoguider_object.html#Autoguider_Object_Min_Connected_Pixel_Count_Set
 */
int Autoguider_Command_Object(char *command_string,char **reply_string)
{
	char variable_string[65];
	char value_string[65];
	char buff[64];
	float fvalue;
	int retval,ivalue;
	
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Object",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	if(command_string == NULL)
	{
		Autoguider_General_Error_Number = 331;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Object:command_string was NULL.");
		return FALSE;
	}
	if(reply_string == NULL)
	{
		Autoguider_General_Error_Number = 332;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Object:reply_string was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Object",
			       LOG_VERBOSITY_TERSE,"COMMAND","Autoguider_Command_Object:parsing command string.");
#endif
	retval = sscanf(command_string,"object %64s %64s",variable_string,value_string);
	if(retval != 2)
	{
		Autoguider_General_Error_Number = 333;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Object:"
			"Failed to parse object command '%s' (%d).",command_string,retval);
		return FALSE;
	}
	/* usually the value is a float */
	retval = sscanf(value_string,"%f",&fvalue);
	if(retval != 1)
	{
		Autoguider_General_Error_Number = 334;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Object:"
			"Failed to parse value string '%s' (%d).",value_string,retval);
		return FALSE;
	}
	/* do something based on variable */
	if(strcmp(variable_string,"sigma") == 0)
	{
		if(!Autoguider_Object_Threshold_Sigma_Set(fvalue))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"0 object sigma set to:"))
			return FALSE;
		sprintf(buff,"%.3f",fvalue);
		if(!Autoguider_General_Add_String(reply_string,buff))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
		return TRUE;
	}
	else if(strcmp(variable_string,"sigma_reject") == 0)
	{
		if(!Autoguider_Object_Threshold_Sigma_Reject_Set(fvalue))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"0 object sigma reject set to:"))
			return FALSE;
		sprintf(buff,"%.3f",fvalue);
		if(!Autoguider_General_Add_String(reply_string,buff))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
		return TRUE;
	}
	else if(strcmp(variable_string,"ellipticity_limit") == 0)
	{
		if(!Autoguider_Object_Ellipticity_Limit_Set(fvalue))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"0 object ellipticity limit set to:"))
			return FALSE;
		sprintf(buff,"%.3f",fvalue);
		if(!Autoguider_General_Add_String(reply_string,buff))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
		return TRUE;
	}
	else if(strcmp(variable_string,"min_con_pix") == 0)
	{
		/* reparse value_string as an integer in this case */
		retval = sscanf(value_string,"%d",&ivalue);
		if(retval != 1)
		{
			Autoguider_General_Error_Number = 335;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Object:"
				"Failed to parse value string '%s' as an integer (%d).",value_string,retval);
			return FALSE;
		}
		if(!Autoguider_Object_Min_Connected_Pixel_Count_Set(ivalue))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"0 object minimum connected pixels set to:"))
			return FALSE;
		sprintf(buff,"%d",ivalue);
		if(!Autoguider_General_Add_String(reply_string,buff))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
		return TRUE;
	}
	else
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Unknown variable:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,variable_string))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
		return TRUE;
	}
	return TRUE;
}


/**
 * Handle a command of the form: "status <type> <element>".
 * <ul>
 * <li>status temperature get
 * <li>status temperature status
 * <li>status field &lt;active|dark|flat|object&gt;
 * <li>status guide &lt;active|dark|flat|object|packet|cadence|timecode_scaling|exposure_length|window&gt;
 * <li>status guide &lt;last_object|initial_position&gt;
 * <li>status object &lt;list|count|median|mean|background_standard_deviation|threshold&gt;
 * <li>status object &lt;sigma|sigma_reject|ellipticity_limit|min_con_pix&gt;
 * </ul>
 * @param command_string The status command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_cil.html#Autoguider_CIL_Guide_Packet_Send_Get
 * @see autoguider_field.html#Autoguider_Field_Is_Fielding
 * @see autoguider_field.html#Autoguider_Field_Get_Do_Dark_Subtract
 * @see autoguider_field.html#Autoguider_Field_Get_Do_Flat_Field
 * @see autoguider_field.html#Autoguider_Field_Get_Do_Object_Detect
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_guide.html#Autoguider_Guide_Is_Guiding
 * @see autoguider_guide.html#Autoguider_Guide_Get_Do_Dark_Subtract
 * @see autoguider_guide.html#Autoguider_Guide_Get_Do_Flat_Field
 * @see autoguider_guide.html#Autoguider_Guide_Get_Do_Object_Detect
 * @see autoguider_guide.html#Autoguider_Guide_Loop_Cadence_Get
 * @see autoguider_guide.html#Autoguider_Guide_Exposure_Length_Get
 * @see autoguider_guide.html#Autoguider_Guide_Window_Get
 * @see autoguider_guide.html#Autoguider_Guide_Timecode_Scaling_Get
 * @see autoguider_guide.html#Autoguider_Guide_Last_Object_Get
 * @see autoguider_guide.html#Autoguider_Guide_Initial_Object_CCD_X_Position_Get
 * @see autoguider_guide.html#Autoguider_Guide_Initial_Object_CCD_Y_Position_Get
 * @see autoguider_object.html#Autoguider_Object_Struct
 * @see autoguider_object.html#Autoguider_Object_List_Get_Count
 * @see autoguider_object.html#Autoguider_Object_List_Get_Object_List_String
 * @see autoguider_object.html#Autoguider_Object_Median_Get
 * @see autoguider_object.html#Autoguider_Object_Mean_Get
 * @see autoguider_object.html#Autoguider_Object_Background_Standard_Deviation_Get
 * @see autoguider_object.html#Autoguider_Object_Threshold_Get
 * @see autoguider_object.html#Autoguider_Object_Threshold_Sigma_Get
 * @see autoguider_object.html#Autoguider_Object_Threshold_Sigma_Reject_Get
 * @see autoguider_object.html#Autoguider_Object_Ellipticity_Limit_Get
 * @see autoguider_object.html#Autoguider_Object_Min_Connected_Pixel_Count_Get
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Error
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Get_Time_String
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Get
 * @see ../ccd/cdocs/ccd_general.html#CCD_Temperature_Cached_Temperature_Get
 */
int Autoguider_Command_Status(char *command_string,char **reply_string)
{
	double temperature;
	enum CCD_TEMPERATURE_STATUS temperature_status;
	struct CCD_Setup_Window_Struct window;
	struct Autoguider_Object_Struct last_object;
	struct timespec temperature_time_stamp;
	char type_string[65];
	char element_string[65];
	char time_string[32];
	char buff[256];
	char *object_list_string = NULL;
	double dvalue;
	float x,y,fvalue;
	int retval,ivalue;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Status",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	if(command_string == NULL)
	{
		Autoguider_General_Error_Number = 300;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Status:command_string was NULL.");
		return FALSE;
	}
	if(reply_string == NULL)
	{
		Autoguider_General_Error_Number = 319;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Status:reply_string was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Status",
			       LOG_VERBOSITY_TERSE,"COMMAND","Autoguider_Command_Status:parsing command string.");
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
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Status",
				       LOG_VERBOSITY_TERSE,"COMMAND","temperature status detected.");
#endif
		retval = CCD_Temperature_Get(&temperature,&temperature_status);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 302;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Status:"
				"CCD_Temperature_Get failed.");
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Status",
						 LOG_VERBOSITY_TERSE,"COMMAND");
			/* get last cached temperature instead */
			CCD_Temperature_Cached_Temperature_Get(&temperature,&temperature_status,
							       &temperature_time_stamp);
		}
		else /* set the temperature time stamp to be now */
			clock_gettime(CLOCK_REALTIME,&(temperature_time_stamp));
		if(strcmp(element_string,"get") == 0)
		{
			if(!Autoguider_General_Add_String(reply_string,"0 "))
				return FALSE;
			CCD_General_Get_Time_String(temperature_time_stamp,time_string,31);
			sprintf(buff,"%s %.2f",time_string,temperature);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"status") == 0)
		{
			if(!Autoguider_General_Add_String(reply_string,"0 "))
				return FALSE;
			CCD_General_Get_Time_String(temperature_time_stamp,time_string,31);
			sprintf(buff,"%s %.2f %s",time_string,temperature,
				CCD_Temperature_Status_To_String(temperature_status));
			if(!Autoguider_General_Add_String(reply_string,buff))
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
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Status",
				       LOG_VERBOSITY_TERSE,"COMMAND","field status detected.");
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
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Status",
				       LOG_VERBOSITY_TERSE,"COMMAND","guide status detected.");
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
		else if(strcmp(element_string,"packet") == 0)
		{
			if(Autoguider_CIL_Guide_Packet_Send_Get())
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
		else if(strcmp(element_string,"cadence") == 0)
		{
			dvalue = Autoguider_Guide_Loop_Cadence_Get();
			sprintf(buff,"0 %.2f",dvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"timecode_scaling") == 0)
		{
			dvalue = Autoguider_Guide_Timecode_Scaling_Get();
			sprintf(buff,"0 %.2f",dvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"exposure_length") == 0)
		{
			ivalue = Autoguider_Guide_Exposure_Length_Get();
			sprintf(buff,"0 %d",ivalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"window") == 0)
		{
			window = Autoguider_Guide_Window_Get();
			sprintf(buff,"0 %d %d %d %d",window.X_Start,window.Y_Start,window.X_End,window.Y_End);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"last_object") == 0)
		{
			last_object = Autoguider_Guide_Last_Object_Get();
			/* 0 CCD_X_Position CCD_Y_Position Buffer_X_Position Buffer_Y_Position 
			** Total_Counts Pixel_Count Peak_Counts Is_Stellar FWHM_X FWHM_Y */
			sprintf(buff,"0 %.2f %.2f %.2f %.2f %.2f %d %.2f %s %.2f %.2f",
				last_object.CCD_X_Position,last_object.CCD_Y_Position,
				last_object.Buffer_X_Position,last_object.Buffer_Y_Position,
				last_object.Total_Counts,last_object.Pixel_Count,last_object.Peak_Counts,
				(last_object.Is_Stellar ? "TRUE" : "FALSE"),
				last_object.FWHM_X,last_object.FWHM_Y);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"initial_position") == 0)
		{
			x = Autoguider_Guide_Initial_Object_CCD_X_Position_Get();
			y = Autoguider_Guide_Initial_Object_CCD_Y_Position_Get();
			/* 0 Initial_Object_CCD_X_Position Initial_Object_CCD_Y_Position  */
			sprintf(buff,"0 %.2f %.2f",x,y);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
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
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Status",
				       LOG_VERBOSITY_TERSE,"COMMAND","object status detected.");
#endif
		if(strcmp(element_string,"count") == 0)
		{
			if(!Autoguider_Object_List_Get_Count(&ivalue))
			{
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Status",
							 LOG_VERBOSITY_TERSE,"COMMAND");
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
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Status",
							 LOG_VERBOSITY_TERSE,"COMMAND");
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
		else if(strcmp(element_string,"median") == 0)
		{
			fvalue = Autoguider_Object_Median_Get();
			sprintf(buff,"0 %.6f",fvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"mean") == 0)
		{
			fvalue = Autoguider_Object_Mean_Get();
			sprintf(buff,"0 %.6f",fvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"background_standard_deviation") == 0)
		{
			fvalue = Autoguider_Object_Background_Standard_Deviation_Get();
			sprintf(buff,"0 %.6f",fvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"threshold") == 0)
		{
			fvalue = Autoguider_Object_Threshold_Get();
			sprintf(buff,"0 %.6f",fvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"sigma") == 0)
		{
			fvalue = Autoguider_Object_Threshold_Sigma_Get();
			sprintf(buff,"0 %.6f",fvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"sigma_reject") == 0)
		{
			fvalue = Autoguider_Object_Threshold_Sigma_Reject_Get();
			sprintf(buff,"0 %.6f",fvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"ellipticity_limit") == 0)
		{
			fvalue = Autoguider_Object_Ellipticity_Limit_Get();
			sprintf(buff,"0 %.6f",fvalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
			return TRUE;
		}
		else if(strcmp(element_string,"min_con_pix") == 0)
		{
			ivalue = Autoguider_Object_Min_Connected_Pixel_Count_Get();
			sprintf(buff,"0 %d",ivalue);
			if(!Autoguider_General_Add_String(reply_string,buff))
				return FALSE;
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
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	if(command_string == NULL)
	{
		Autoguider_General_Error_Number = 303;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:command_string was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
			       LOG_VERBOSITY_TERSE,"COMMAND","parsing command string.");
#endif
	retval = sscanf(command_string,"temperature %64s %64s",type_string,parameter_string);
	if(retval != 2)
	{
		Autoguider_General_Error_Number = 304;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:"
			"Failed to parse temperature command %s (%d).",command_string,retval);
		Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Temperature",
			       LOG_VERBOSITY_TERSE,"COMMAND");
		if(!Autoguider_General_Add_String(reply_string,"1 Failed to parse command string:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,command_string))
			return FALSE;
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
					       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
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
			Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
					       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
			return TRUE;
		}
#if AUTOGUIDER_DEBUG > 3
		Autoguider_General_Log_Format("command","autoguider_command.c","Autoguider_Command_Temperature",
					      LOG_VERBOSITY_TERSE,"COMMAND","Setting target temperature to %lf.",
					      target_temperature);
#endif
		retval = CCD_Temperature_Set(target_temperature);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 305;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:"
				"Failed to set temperature.");
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Temperature",
			       LOG_VERBOSITY_TERSE,"COMMAND");
			if(!Autoguider_General_Add_String(reply_string,"1 Failed to set target temperature."))
				return FALSE;
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
					       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
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
			Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
					       LOG_VERBOSITY_TERSE,"COMMAND","Turning cooler on.");
#endif
			retval = CCD_Temperature_Cooler_On();
			if(retval == FALSE)
			{
				Autoguider_General_Error_Number = 306;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:"
					"Failed to turn cooler on.");
				Autoguider_General_Error("command","autoguider_command.c",
							 "Autoguider_Command_Temperature",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,"1 Failed to turn cooler on."))
					return FALSE;
#if AUTOGUIDER_DEBUG > 1
				Autoguider_General_Log("command","autoguider_command.c",
						       "Autoguider_Command_Temperature",LOG_VERBOSITY_TERSE,"COMMAND",
						       "finished.");
#endif
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 cooler on"))
				return FALSE;
		}
		else if(strcmp(parameter_string,"off")==0)
		{
#if AUTOGUIDER_DEBUG > 3
			Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
					       LOG_VERBOSITY_TERSE,"COMMAND","Turning cooler off.");
#endif
			retval = CCD_Temperature_Cooler_Off();
			if(retval == FALSE)
			{
				Autoguider_General_Error_Number = 307;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Temperature:"
					"Failed to turn cooler on.");
				Autoguider_General_Error("command","autoguider_command.c",
							 "Autoguider_Command_Temperature",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,"1 Failed to turn cooler off."))
					return FALSE;
#if AUTOGUIDER_DEBUG > 1
				Autoguider_General_Log("command","autoguider_command.c",
						       "Autoguider_Command_Temperature",LOG_VERBOSITY_TERSE,"COMMAND",
						       "finished.");
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
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
				       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
		return TRUE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Temperature",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
	return TRUE;
}

/**
 * Handle commands of the form: 
 * <ul>
 * <li>"field [ms [lock]]"
 * <li>"field <dark|flat|object> <on|off>"
 * </ul>
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_State_Set
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Send
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_field.html#Autoguider_Field
 */
int Autoguider_Command_Field(char *command_string,char **reply_string)
{
	char parameter_string1[64];
	char parameter_string2[64];
	int parameter_count,retval,exposure_length,doit,lockit;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Field",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	/* parse command */
	parameter_count = sscanf(command_string,"field %64s %64s",parameter_string1,parameter_string2);
	if((strcmp(parameter_string1,"dark") == 0)||(strcmp(parameter_string1,"flat") == 0)||
	   (strcmp(parameter_string1,"object") == 0))
	{
		if(parameter_count != 2)
		{
			if(!Autoguider_General_Add_String(reply_string,"1 Field parameter 2 not on or off:"))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,parameter_string2))
				return FALSE;
			if(!Autoguider_General_Add_String(reply_string,"."))
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
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Field",
							 LOG_VERBOSITY_TERSE,"COMMAND");
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
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Field",
							 LOG_VERBOSITY_TERSE,"COMMAND");
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
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Field",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting field object detect failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Field Do Object Detect set."))
				return FALSE;
		}
	}
	else
	{
		lockit = FALSE;
		/* if at least one parameter, first parameter is exposure length */
		if(parameter_count > 0)
		{
			retval = sscanf(parameter_string1,"%d",&exposure_length);
			if(retval != 1)
			{
				Autoguider_General_Error_Number = 313;
				sprintf(Autoguider_General_Error_String,"Autoguider_Command_Field:"
					"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
				Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Field",
						       LOG_VERBOSITY_TERSE,"COMMAND",
						       "finished (command parse failed).");
#endif
				return FALSE;
			}
			if(parameter_count > 1)
			{
				if(strcmp(parameter_string2,"lock") == 0)
				{
					lockit = TRUE;
				}
				else
				{
					if(!Autoguider_General_Add_String(reply_string,
									  "1 Field parameter 2 not lock:"))
						return FALSE;
					if(!Autoguider_General_Add_String(reply_string,parameter_string2))
						return FALSE;
					if(!Autoguider_General_Add_String(reply_string,"."))
						return FALSE;
					return TRUE;
				}
			}/* end if 2 parameters */
			retval = Autoguider_Field_Exposure_Length_Set(exposure_length,lockit);
			if(retval == FALSE)
			{
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Field",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting field exposure length failed."))
					return FALSE;
				return TRUE;
			}
		}/* end if parameter_count > 0 */
		/* do actual field operation if "field" or "field <n> or "field <n> lock" */
		retval = Autoguider_Field();
		if(retval == FALSE)
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Field",
						 LOG_VERBOSITY_TERSE,"COMMAND");
			if(!Autoguider_General_Add_String(reply_string,"1 Field failed."))
				return FALSE;
			return TRUE;
		}
		/* update SDB */
		if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Field",
						 LOG_VERBOSITY_TERSE,"COMMAND"); /* no need to fail */
		}
		if(!Autoguider_CIL_SDB_Packet_Send())
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Field",
						 LOG_VERBOSITY_TERSE,"COMMAND"); /* no need to fail */
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Field suceeded."))
			return FALSE;
	}/* end else */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Field",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
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
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_field.html#Autoguider_Field_Expose
 * @see autoguider_field.html#Autoguider_Field_Exposure_Length_Set
 */
int Autoguider_Command_Expose(char *command_string,char **reply_string)
{
	int retval,exposure_length;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Expose",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	/* parse command */
	retval = sscanf(command_string,"expose %d",&exposure_length);
	if(retval != 1)
	{
		Autoguider_General_Error_Number = 315;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Expose:"
			"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Expose",
				       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
#endif
		return FALSE;
	}
	retval = Autoguider_Field_Exposure_Length_Set(exposure_length,FALSE);
	if(retval == FALSE)
	{
		Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Expose",
					 LOG_VERBOSITY_TERSE,"COMMAND");
		if(!Autoguider_General_Add_String(reply_string,"1 Setting field exposure length failed."))
			return FALSE;
		return TRUE;
	}
	retval = Autoguider_Field_Expose();
	if(retval == FALSE)
	{
		Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Expose",
					 LOG_VERBOSITY_TERSE,"COMMAND");
		if(!Autoguider_General_Add_String(reply_string,"1 Expose failed."))
			return FALSE;
		return TRUE;
	}
	if(!Autoguider_General_Add_String(reply_string,"0 Expose suceeded."))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Expose",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
	return TRUE;
}

/**
 * Handle a commands of the form: 
 * <ul>
 * <li>guide [on|off]
 * <li>guide window <sx> <sy> [<ex> <ey>]
 * <li>guide exposure_length <ms> [lock]
 * <li>guide <dark|flat|object|packet|window_track> <on|off>
 * <li>guide <object> <index>
 * <li>guide timecode_scaling <float value>
 * </ul>
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_cil.html#Autoguider_CIL_Guide_Packet_Send_Set
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_guide.html#Autoguider_Guide_On
 * @see autoguider_guide.html#Autoguider_Guide_Off
 * @see autoguider_guide.html#Autoguider_Guide_Window_Set
 * @see autoguider_guide.html#Autoguider_Guide_Window_Set_From_XY
 * @see autoguider_guide.html#Autoguider_Guide_Exposure_Length_Set
 * @see autoguider_guide.html#Autoguider_Guide_Set_Do_Dark_Subtract
 * @see autoguider_guide.html#Autoguider_Guide_Set_Do_Flat_Field
 * @see autoguider_guide.html#Autoguider_Guide_Set_Do_Object_Detect
 * @see autoguider_guide.html#Autoguider_Guide_Set_Guide_Object
 * @see autoguider_guide.html#Autoguider_Guide_Set_Guide_Window_Tracking
 * @see autoguider_guide.html#Autoguider_Guide_Timecode_Scaling_Set
 */
int Autoguider_Command_Guide(char *command_string,char **reply_string)
{
	char parameter_string1[64];
	char parameter_string2[64];
	char parameter_string3[64];
	char parameter_string4[64];
	char parameter_string5[64];
	int parameter_count,retval,sx,sy,ex,ey,exposure_length,doit,object_index;
	float float_value;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Guide",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
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
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Guide",
				       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
#endif
		return FALSE;
	}
	if(strcmp(parameter_string1,"on") == 0)
	{
		retval = Autoguider_Guide_On();
		if(retval == FALSE)
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
						 LOG_VERBOSITY_TERSE,"COMMAND");
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
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
						 LOG_VERBOSITY_TERSE,"COMMAND");
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
		if((retval != 4)&&(retval != 2))
		{
			Autoguider_General_Error_Number = 311;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Guide:"
				"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Guide",
					       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
#endif
			return FALSE;
		}
		if(retval == 2) /* retval is 2, parameters are centre x/y of window */
		{
			retval = Autoguider_Guide_Window_Set_From_XY(sx,sy);
			if(retval == FALSE)
			{
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,"1 Guide window failed."))
					return FALSE;
				return TRUE;
			}			
		}
		else /* retval is 4, set guide window from corners */
		{
			retval = Autoguider_Guide_Window_Set(sx,sy,ex,ey);
			if(retval == FALSE)
			{
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,"1 Guide window failed."))
					return FALSE;
				return TRUE;
			}
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Guide window suceeded."))
			return FALSE;
	}
	else if(strncmp(parameter_string1,"exposure_length",15) == 0)
	{
		doit = FALSE;/* lock exp length */
		retval = sscanf(command_string,"guide exposure_length %d %s",&exposure_length,parameter_string2);
		if(retval < 1)
		{
			Autoguider_General_Error_Number = 312;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Guide:"
				"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Guide",
					       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
#endif
			return FALSE;
		}
		if(retval == 2)
		{
			if(strcmp(parameter_string2,"lock") == 0)
			{
				doit = TRUE;
			}
			else
			{
				if(!Autoguider_General_Add_String(reply_string,"1 Setting guide exposure length:"
								  "Illegal parameter 2:."))
					return FALSE;
				if(!Autoguider_General_Add_String(reply_string,parameter_string2))
					return FALSE;
				if(!Autoguider_General_Add_String(reply_string,"."))
					return FALSE;
				return TRUE;
			}
		}
		retval = Autoguider_Guide_Exposure_Length_Set(exposure_length,doit);
		if(retval == FALSE)
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
						 LOG_VERBOSITY_TERSE,"COMMAND");
			if(!Autoguider_General_Add_String(reply_string,"1 Setting guide exposure length failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Setting guide exposure length suceeded."))
			return FALSE;
	}
	else if((strncmp(parameter_string1,"dark",4) == 0)||(strncmp(parameter_string1,"flat",4) == 0)||
		(strncmp(parameter_string1,"packet",6) == 0)||(strncmp(parameter_string1,"window_track",12) == 0))
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
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
							 LOG_VERBOSITY_TERSE,"COMMAND");
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
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting guide flat field failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Guide Do Flat Fielding set."))
				return FALSE;
		}
		else if(strcmp(parameter_string1,"packet") == 0)
		{
			retval = Autoguider_CIL_Guide_Packet_Send_Set(doit);
			if(retval == FALSE)
			{
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting guide packet send failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Guide Packet Send set."))
				return FALSE;
		}
		else if(strcmp(parameter_string1,"window_track") == 0)
		{
			retval = Autoguider_Guide_Set_Guide_Window_Tracking(doit);
			if(retval == FALSE)
			{
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting guide window tracking failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Guide Window Tracking set."))
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
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
							 LOG_VERBOSITY_TERSE,"COMMAND");
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
				Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
							 LOG_VERBOSITY_TERSE,"COMMAND");
				if(!Autoguider_General_Add_String(reply_string,
								  "1 Setting guide object failed."))
					return FALSE;
				return TRUE;
			}
			if(!Autoguider_General_Add_String(reply_string,"0 Setting guide object ok."))
				return FALSE;
		}
	}
	else if(strncmp(parameter_string1,"timecode_scaling",16) == 0)
	{
		retval = sscanf(command_string,"guide timecode_scaling %f",&float_value);
		if(retval < 1)
		{
			Autoguider_General_Error_Number = 330;
			sprintf(Autoguider_General_Error_String,"Autoguider_Command_Guide:"
				"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Guide",
					       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
#endif
			return FALSE;
		}
		retval = Autoguider_Guide_Timecode_Scaling_Set(float_value);
		if(retval == FALSE)
		{
			Autoguider_General_Error("command","autoguider_command.c","Autoguider_Command_Guide",
						 LOG_VERBOSITY_TERSE,"COMMAND");
			if(!Autoguider_General_Add_String(reply_string,"1 Setting guide timecode scaling failed."))
				return FALSE;
			return TRUE;
		}
		if(!Autoguider_General_Add_String(reply_string,"0 Setting guide timecode scaling suceeded."))
			return FALSE;
	}
	else
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Guide command failed:Unknown parameter:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,parameter_string1))
			return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Guide",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "getfits <field|guide|object> <raw|reduced>".
 * @param command_string The command. This is not changed during this routine.
 * @param buffer_ptr The address of a pointer to allocate and store a FITS image in memory.
 * @param buffer_length The address of a word to store the length of the created returned data.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
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
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Get_Fits",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	/* parse command */
	retval = sscanf(command_string,"getfits %64s %64s",type_parameter_string,state_parameter_string);
	if(retval != 2)
	{
		Autoguider_General_Error_Number = 308;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Get_Fits:"
			"Failed to parse command %s (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Get_Fits",
				       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
#endif
		return FALSE;
	}
	/* parse type parameter */
	if(strcmp(type_parameter_string,"field") == 0)
		buffer_type = AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD;
	else if(strcmp(type_parameter_string,"guide") == 0)
		buffer_type = AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE;
	else if(strcmp(type_parameter_string,"object") == 0)
		buffer_type = AUTOGUIDER_GET_FITS_BUFFER_TYPE_OBJECT;
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
	retval = Autoguider_Get_Fits(buffer_type,buffer_state,-1,buffer_ptr,buffer_length);
	if(retval == FALSE)
	{
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Get_Fits",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
	return TRUE;
}

/**
 * Handle a command of the form: "log_level <autoguider|ccd|command_server|object|ngatcil> <n>".
 * @param command_string The command. This is not changed during this routine.
 * @param reply_string The address of a pointer to allocate and set the reply string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Set_Log_Filter_Level
 * @see ../commandserver/cdocs/command_server.html#Command_Server_Set_Log_Filter_Level
 * @see ../ngatcil/cdocs/ngatcil_general.html#NGATCil_General_Set_Log_Filter_Level
 * @see ../../libdprt/object/cdocs/object.html#Object_Set_Log_Filter_Level
 */
int Autoguider_Command_Log_Level(char *command_string,char **reply_string)
{
	char parameter_buff[32];
	int retval,log_level;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Log_Level",
			       LOG_VERBOSITY_TERSE,"COMMAND","started.");
#endif
	/* parse command */
	retval = sscanf(command_string,"log_level %32s %d",parameter_buff,&log_level);
	if(retval != 2)
	{
		Autoguider_General_Error_Number = 320;
		sprintf(Autoguider_General_Error_String,"Autoguider_Command_Log_Level:"
			"Failed to parse command '%s' (%d).",command_string,retval);
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Log_Level",
				       LOG_VERBOSITY_TERSE,"COMMAND","finished (command parse failed).");
#endif
		return FALSE;
	}
	if(strcmp(parameter_buff,"autoguider") == 0)
	{
		Autoguider_General_Set_Log_Filter_Level(log_level);
		if(!Autoguider_General_Add_String(reply_string,"0 Setting autoguider log level suceeded."))
			return FALSE;
	}
	else if(strcmp(parameter_buff,"ccd") == 0)
	{
		CCD_General_Set_Log_Filter_Level(log_level);
		if(!Autoguider_General_Add_String(reply_string,"0 Setting ccd log level suceeded."))
			return FALSE;
	}
	else if(strcmp(parameter_buff,"command_server") == 0)
	{
		Command_Server_Set_Log_Filter_Level(log_level);
		if(!Autoguider_General_Add_String(reply_string,"0 Setting command_server log level suceeded."))
			return FALSE;
	}
	else if(strcmp(parameter_buff,"object") == 0)
	{
		Object_Set_Log_Filter_Level(log_level);
		if(!Autoguider_General_Add_String(reply_string,"0 Setting object log level suceeded."))
			return FALSE;
	}
	else if(strcmp(parameter_buff,"ngatcil") == 0)
	{
		NGATCil_General_Set_Log_Filter_Level(log_level);
		if(!Autoguider_General_Add_String(reply_string,"0 Setting ngatcil log level suceeded."))
			return FALSE;
	}
	else
	{
		if(!Autoguider_General_Add_String(reply_string,"1 Setting log level failed:Illegal parameter:"))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,parameter_buff))
			return FALSE;
		if(!Autoguider_General_Add_String(reply_string,"."))
			return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("command","autoguider_command.c","Autoguider_Command_Log_Level",
			       LOG_VERBOSITY_TERSE,"COMMAND","finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.17  2012/03/07 14:56:26  cjm
** Autoguider_Field_SDB_State_Failed_Then_Idle_Set now called, rather than
** Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE), in Autoguider_Command_Autoguide_On
** when the fielding process fails.
**
** Revision 1.16  2011/06/23 11:02:00  cjm
** Fixed dodgy sprintf in "status temperature status" command.
**
** Revision 1.15  2010/07/29 09:53:33  cjm
** CHanged "status temperature get" command.
** If CCD_Temperature_Get fails (because we are acquiring an image),
** we call CCD_Temperature_Cached_Temperature_Get to get the last
** known temperature and return that.
** "status temperature get" return string has changed to include a timestamp.
**
** Revision 1.14  2009/04/29 10:54:43  cjm
** Change to Autoguider_Field_Save_FITS calls to allow saving of guide star FITS header information.
** Also Autoguider_Get_Fits calls for the same reason.
**
** Revision 1.13  2009/01/30 18:01:33  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.12  2007/11/05 18:24:32  cjm
** Added calls to Autoguider_Field_Get_Save_FITS_Failed, Autoguider_Field_Get_Save_FITS_Successful and
** Autoguider_Field_Save_FITS to save the field image in a FITS file if configured to do so.
**
** Revision 1.11  2007/02/09 14:41:05  cjm
** Added timecode_scaling get and set commands.
** Also getters for status on guide window.
**
** Revision 1.10  2007/01/19 14:26:34  cjm
** Added guide window_track <on|off> command for guide window tracking
** control.
**
** Revision 1.9  2006/08/29 13:55:42  cjm
** Added text agstate command.
** Added some SDB calls to set AG_STATE to idle on error.
**
** Revision 1.8  2006/06/29 17:04:34  cjm
** Moved silly is-use check from AG on command.
** More logging
**
** Revision 1.7  2006/06/27 20:43:52  cjm
** Added Autoguider_Command_Autoguide_On.
**
** Revision 1.6  2006/06/22 15:52:42  cjm
** Added status commands to get guide loop cadence/exposure_length.
**
** Revision 1.5  2006/06/21 14:09:01  cjm
** Added guide exposure length locking.
** "autoguide on/off" still not finished yet.
**
** Revision 1.4  2006/06/20 13:05:21  cjm
** Added TCS guide packet sending control/status.
** Start of Autoguide on/off implmentation - THIS IS NOT YET FINISHED.
**
** Revision 1.3  2006/06/12 19:22:13  cjm
** Added log_level command handling.
**
** Revision 1.2  2006/06/02 13:44:59  cjm
** Added extra tests and allow for \0 in some command argument strings.
**
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
