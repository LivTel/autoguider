/* test_setup_startup.c
 * $Id$
 * Test generic CCD_Setup_Startup (and also driver linking/initialisation).
 */
/**
 * Test generic CCD_Setup_Startup (and also driver linking/initialisation).
 * @author $Author: cjm $
 * @version $Revision$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ccd_config.h"
#include "ccd_driver.h"
#include "ccd_general.h"
#include "ccd_setup.h"

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
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 */
int main(int argc, char *argv[])
{
	char *shared_library_name = NULL;
	char *registration_function = NULL;
	int retval;
	
/* parse arguments */
	fprintf(stdout,"Parsing Arguments.\n");
	if(!Parse_Arguments(argc,argv))
		return 1;
/* set text/interface options */
	CCD_General_Set_Log_Handler_Function(CCD_General_Log_Handler_Stdout);
	CCD_Config_Initialise();
	if(Config_Filename == NULL)
	{
		fprintf(stderr, "test_setup_startup: Config filename was NULL.\n");
		Help();
		return 2;
	}
	retval = CCD_Config_Load(Config_Filename);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 2;
	}
	/* get shared library driver to use */
	retval = CCD_Config_Get_String("ccd.driver.shared_library",&shared_library_name);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 3;
	}
	/* get resistration function for shared library */
	retval = CCD_Config_Get_String("ccd.driver.registration_function",&registration_function);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 4;
	}
	/* register (dynamically load) the specified driver and register the interface functions */
	retval = CCD_Driver_Register(shared_library_name,registration_function);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 5;
	}
	if(shared_library_name != NULL)
		free(shared_library_name);
	if(registration_function != NULL)
		free(registration_function);
	/* setup initialise */
	CCD_Setup_Initialise();
	/* setup startup */
	retval = CCD_Setup_Startup();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 6;
	}
	/* and shutdown again */
	retval = CCD_Setup_Shutdown();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 7;
	}
	retval = CCD_Config_Shutdown();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 8;
	}
	fprintf(stdout,"test_setup_startup completed.\n");
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
	fprintf(stdout,"Test Setup Startup:Help.\n");
	fprintf(stdout,"This program calls CCD_Setup_Startup after loading/registering a driver, to test setup and driver linking.\n");
	fprintf(stdout,"test_setup_startup \n");
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
 * @see ../cdocs/ccd_general.html#CCD_General_Set_Log_Filter_Function
 * @see ../cdocs/ccd_general.html#CCD_General_Set_Log_Filter_Level
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

