/* pco_test_get_roi_step_size.c
 * $Id$
 * Create a connection to the PCO camera and do an initial setup, and then retrieve the ROI (region of interest)
 * pixel step size.
 */
/**
 * Create a connection to the PCO camera and do an initial setup, and then retrieve the ROI (region of interest)
 * pixel step size.
 * @author $Author: cjm $
 * @version $Revision$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ccd_config.h"
#include "ccd_general.h"
#include "pco_command.h"
#include "pco_setup.h"

/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id$";
/**
 * Filename for configuration file. 
 */
static char *Config_Filename = NULL;

/* internal routines */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/* -----------------------------------------------------------------------------
**      External routines
** ----------------------------------------------------------------------------- */
/**
 * Main program.
 * <ul>
 * <li>We call Parse_Arguments to parse the command line, setup logging and config filenames etc...
 * <li>We set the CCD library log handler function (CCD_General_Set_Log_Handler_Function) 
 *     to stdout (CCD_General_Log_Handler_Stdout).
 * <li>We intialise the config module (CCD_Config_Initialise).
 * <li>We load the config filename (Config_Filename) from disk using CCD_Config_Load.
 * <li>We make a connection to the camera, and initially configure it, by calling PCO_Setup_Startup.
 * <li>We call PCO_Command_Description_Get_ROI_Horizontal_Step_Size to get the horizontal ROI step size.
 * <li>We call PCO_Command_Description_Get_ROI_Vertical_Step_Size to get the vertical ROI step size.
 * <li>We print the step sizes out.
 * <li>We call PCO_Setup_Shutdown to close the connection to the camera.
 * <li>We call CCD_Config_Shutdown to free allocated meemory in the config module.
 * </ul>
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #Parse_Arguments
 * @see #Config_Filename
 * @see ../cdocs/pco_command.html#PCO_Command_Description_Get_ROI_Horizontal_Step_Size
 * @see ../cdocs/pco_command.html#PCO_Command_Description_Get_ROI_Vertical_Step_Size
 * @see ../cdocs/pco_setup.html#PCO_Setup_Startup
 * @see ../cdocs/pco_setup.html#PCO_Setup_Shutdown
 * @see ../../cdocs/ccd_general.html#CCD_General_Set_Log_Handler_Function
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Handler_Stdout
 * @see ../../cdocs/ccd_general.html#CCD_General_Error
 * @see ../../cdocs/ccd_config.html#CCD_Config_Initialise
 * @see ../../cdocs/ccd_config.html#CCD_Config_Load
 * @see ../../cdocs/ccd_config.html#CCD_Config_Shutdown
 */
int main(int argc, char *argv[])
{
	int retval,hor_step_size,ver_step_size;
	
/* parse arguments */
	fprintf(stdout,"Parsing Arguments.\n");
	if(!Parse_Arguments(argc,argv))
		return 1;
/* set text/interface options */
	CCD_General_Set_Log_Handler_Function(CCD_General_Log_Handler_Stdout);
	/* load config file, this is needed for PCO_Setup_Startup to retrieve the board number */
	CCD_Config_Initialise();
	if(Config_Filename == NULL)
	{
		fprintf(stderr, "pco_test_get_roi_step_size: Config filename was NULL.\n");
		Help();
		return 2;
	}
	retval = CCD_Config_Load(Config_Filename);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 2;
	}
	/* setup startup */
	retval = PCO_Setup_Startup();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 6;
	}
	/* get roi step sizes */
	retval = PCO_Command_Description_Get_ROI_Horizontal_Step_Size(&hor_step_size);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 6;
	}
	retval = PCO_Command_Description_Get_ROI_Vertical_Step_Size(&ver_step_size);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 6;
	}
	fprintf(stdout, "pco_test_get_roi_step_size: Horizontal Step Size = %d, Vertical Step Size = %d.\n",
		hor_step_size,ver_step_size);
	/* and shutdown again */
	retval = PCO_Setup_Shutdown();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 9;
	}
	retval = CCD_Config_Shutdown();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 10;
	}
	fprintf(stdout,"pco_test_get_roi_step_size completed.\n");
	return 0;
}

/* -----------------------------------------------------------------------------
**      Internal routines
** ----------------------------------------------------------------------------- */
/**
 * Help routine.
 */
static void Help(void)
{
	fprintf(stdout,"PCO Test get Region of Interest step size:Help.\n");
	fprintf(stdout,"This program retrieves the region of interest step sizes after setting up the PCO camera.\n");
	fprintf(stdout,"pco_test_get_roi_step_size \n");
	fprintf(stdout,"\t[-co[nfig_filename] <filename>]\n");
	fprintf(stdout,"\t[-l[og_level] <verbosity>][-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-help prints out this message and stops the program.\n");
	fprintf(stdout,"\t<filename> should be a valid filename.\n");
	fprintf(stdout,"\t<verbosity> should be an integer log level between 0 and 5 inclusive.\n");
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Config_Filename
 * @see ../../cdocs/ccd_general.html#CCD_General_Set_Log_Filter_Function
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Filter_Level_Absolute
 * @see ../../cdocs/ccd_general.html#CCD_General_Set_Log_Filter_Level
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-config_filename")==0)||(strcmp(argv[i],"-co")==0))
		{
			if((i+1)<argc)
			{
				Config_Filename = argv[i+1];
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
		else if((strcmp(argv[i],"-log_level")==0)||(strcmp(argv[i],"-l")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&log_level);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing log level %s failed.\n",argv[i+1]);
					return FALSE;
				}
				CCD_General_Set_Log_Filter_Function(CCD_General_Log_Filter_Level_Absolute);
				CCD_General_Set_Log_Filter_Level(log_level);
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

