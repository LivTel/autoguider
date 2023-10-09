/* pco_test_initialise_finalise.c
 * $Id$
 * Test the initialisation and finalisation of the PCO library.
 */
/**
 * Test the initialisation and finalisation of the PCO library.
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

/* hash definitions */
/**
 * Default number of columns on the detector.
 */
#define DEFAULT_BOARD_NUMBER		(0)

/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id$";
/**
 * Filename for configuration file. 
 */
static char *Config_Filename = NULL;
/**
 * The board number of the camera to connect to.
 * @see #DEFAULT_BOARD_NUMBER
 */
static int Board_Number = DEFAULT_BOARD_NUMBER;

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
 * </ul>
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #Parse_Arguments
 * @see #Config_Filename
 * @see #Board_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Set_Log_Handler_Function
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Handler_Stdout
 * @see ../../cdocs/ccd_general.html#CCD_General_Error
 */
int main(int argc, char *argv[])
{
	int retval;
		
/* parse arguments */
	fprintf(stdout,"Parsing Arguments.\n");
	if(!Parse_Arguments(argc,argv))
		return 1;
/* setup logging */
	CCD_General_Set_Log_Handler_Function(CCD_General_Log_Handler_Stdout);
	/* load config file, this is needed for PCO_Setup_Startup to retrieve the board number */
	/*
	CCD_Config_Initialise();
	if(Config_Filename == NULL)
	{
		fprintf(stderr, "pco_test_initialise_finalise: Config filename was NULL.\n");
		Help();
		return 2;
	}
	retval = CCD_Config_Load(Config_Filename);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 2;
	}
	*/
	if(!PCO_Command_Initialise_Camera())
	{
		CCD_General_Error();
		return 1;
	}	
	/* open a connection to the CCD camera */
	if(!PCO_Command_Open(Board_Number))
	{
		CCD_General_Error();
		return 1;
	}	
	/* initialise grabber reference */
	if(!PCO_Command_Initialise_Grabber())
	{
		CCD_General_Error();
		return 1;
	}

	/* and shut everything down again */
	/* close the open connection to the CCD camera */
	if(!PCO_Command_Close())
	{
		CCD_General_Error();
		return 1;
	}
	/* shutdown the PCO library */
	if(!PCO_Command_Finalise())
	{
		CCD_General_Error();
		return 1;
	}
	fprintf(stdout,"pco_test_initialise_finalise.\n");
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
	fprintf(stdout,"PCO Test initialise finalise:Help.\n");
	fprintf(stdout,"pco_test_initialise_finalise \n");
	fprintf(stdout,"\t[-co[nfig_filename] <filename>]\n");
	fprintf(stdout,"\t[-l[og_level] <verbosity>][-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-help prints out this message and stops the program.\n");
	fprintf(stdout,"\t<filename> should be a valid filename.\n");
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Config_Filename
 * @see #Board_Number
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

