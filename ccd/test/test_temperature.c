/* test_temperature.c
 * $Id$
 * Test detector temperature control via the generic CCD library.
 */
/**
 * Test detector temperature control via the generic CCD library.
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
#include "ccd_temperature.h"

#include "fitsio.h"

/* hash definitions */
/**
 * Default temperature to set the detector to.
 */
#define DEFAULT_TEMPERATURE	(-20.0)


/* enums */
/**
 * Enumeration determining which command this program executes. One of:
 * <ul>
 * <li>COMMAND_ID_NONE
 * <li>COMMAND_ID_GET
 * <li>COMMAND_ID_ON
 * <li>COMMAND_ID_OFF
 * <li>COMMAND_ID_SET
 * </ul>
 */
enum COMMAND_ID
{
	COMMAND_ID_NONE=0,COMMAND_ID_GET,COMMAND_ID_ON,COMMAND_ID_OFF,COMMAND_ID_SET
};

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
 * Temperature to set the detector to in degrees centigrade.
 * @see #DEFAULT_TEMPERATURE
 */
static double Temperature = DEFAULT_TEMPERATURE;
/**
 * Which type ef exposure command to call.
 * @see #COMMAND_ID
 */
static enum COMMAND_ID Command = COMMAND_ID_NONE;
/**
 * Boolean whether to call CCD_Setup_Shutdown at the end of the test.
 * This may switch off the cooler. Default value is TRUE.
 */
static int Call_Setup_Shutdown = TRUE;

/* internal routines */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);
static int Test_Save_Fits_Headers(int exposure_time,int ncols,int nrows,char *filename);
static void Test_Fits_Header_Error(int status);

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
 * <li>We get the shared library to use from the config data, by retrieving the 
 *     "ccd.driver.shared_library" key using CCD_Config_Get_String.
 * <li>We get the driver registration function to use for the shared library from the config data, by retrieving the
 *     "ccd.driver.registration_function" string using CCD_Config_Get_String.
 * <li>We register (dynamically load) the specified driver and register the interface functions by calling 
 *     CCD_Driver_Register.
 * <li>We initialise the setup module by calling CCD_Setup_Initialise.
 * <li>We make a connection to the camera, and initially configure it, by calling CCD_Setup_Startup.
 * <li>Depending on Command, we turn the cooler on (CCD_Temperature_Cooler_On), 
 *     turn the cooler off (CCD_Temperature_Cooler_Off), 
 *     set the detector target temperature (CCD_Temperature_Set with Temperature parameter), 
 *     or retrieve the current detector temperature (CCD_Temperature_Get).
 * <li>If configured with Call_Setup_Shutdown, we shut the program down by calling CCD_Setup_Shutdown.
 * <li>We free up the config module memory by calling CCD_Config_Shutdown.
 * </ul>
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #Parse_Arguments
 * @see #Config_Filename
 * @see #Temperature
 * @see #Call_Setup_Shutdown
 * @see ../cdocs/ccd_general.html#CCD_General_Set_Log_Handler_Function
 * @see ../cdocs/ccd_general.html#CCD_General_Log_Handler_Stdout
 * @see ../cdocs/ccd_general.html#CCD_General_Error
 * @see ../cdocs/ccd_config.html#CCD_Config_Initialise
 * @see ../cdocs/ccd_config.html#CCD_Config_Load
 * @see ../cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../cdocs/ccd_config.html#CCD_Config_Shutdown
 * @see ../cdocs/ccd_driver.html#CCD_Driver_Register
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Initialise
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Startup
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Shutdown
 * @see ../cdocs/ccd_temperature.html#CCD_Temperature_Set
 * @see ../cdocs/ccd_temperature.html#CCD_Temperature_Get
 * @see ../cdocs/ccd_temperature.html#CCD_Temperature_Cooler_On
 * @see ../cdocs/ccd_temperature.html#CCD_Temperature_Cooler_Off
 */
int main(int argc, char *argv[])
{
	char *shared_library_name = NULL;
	char *registration_function = NULL;
	double temperature;
	enum CCD_TEMPERATURE_STATUS temperature_status;
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
		fprintf(stderr, "test_exposure: Config filename was NULL.\n");
		Help();
		return 2;
	}
	retval = CCD_Config_Load(Config_Filename);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 3;
	}
	/* get shared library driver to use */
	retval = CCD_Config_Get_String("ccd.driver.shared_library",&shared_library_name);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 4;
	}
	/* get resistration function for shared library */
	retval = CCD_Config_Get_String("ccd.driver.registration_function",&registration_function);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 5;
	}
	/* register (dynamically load) the specified driver and register the interface functions */
	retval = CCD_Driver_Register(shared_library_name,registration_function);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 6;
	}
	if(shared_library_name != NULL)
		free(shared_library_name);
	if(registration_function != NULL)
		free(registration_function);
	/* setup initialise */
	CCD_Setup_Initialise();
	/* setup startup */
	fprintf(stdout,"Calling CCD_Setup_Startup.\n");
	retval = CCD_Setup_Startup();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 7;
	}
/* do command */
	switch(Command)
	{
		case COMMAND_ID_ON:
			retval = CCD_Temperature_Cooler_On();
			break;
		case COMMAND_ID_OFF:
			retval = CCD_Temperature_Cooler_Off();
			break;
		case COMMAND_ID_SET:
			retval = CCD_Temperature_Set(Temperature);
			break;
		case COMMAND_ID_GET:
			retval = CCD_Temperature_Get(&temperature,&temperature_status);
			if(retval == TRUE)
			{
				fprintf(stdout,"Temperature:%lf.\n",temperature);
				fprintf(stdout,"Temperature Status:%s (%d).\n",
					CCD_Temperature_Status_To_String(temperature_status),temperature_status);
				
			}
			break;
		default:
			fprintf(stderr,"Unknown/No command specified (%d).\n",Command);
			return 3;
			break;
	}
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 11;
	}
	/* and shutdown again */
	fprintf(stdout,"Shutting down....\n");
	if(Call_Setup_Shutdown)
	{
		fprintf(stdout,"Calling CCD_Setup_Shutdown.\n");
		retval = CCD_Setup_Shutdown();
		if(retval == FALSE)
		{
			CCD_General_Error();
			return 14;
		}
	}
	else
		fprintf(stdout,"NOT calling CCD_Setup_Shutdown.\n");
	/* always call Config shutdown, which frees config memory */
	retval = CCD_Config_Shutdown();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 15;
	}
	fprintf(stdout,"test_temperature completed.\n");
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
	fprintf(stdout,"Test Temperature:Help.\n");
	fprintf(stdout,"This program allows us to control/monitor detector temperature/cooling using the generic autoguider CCD interface.\n");
	fprintf(stdout,"test_temperature \n");
	fprintf(stdout,"\t[-co[nfig_filename] <filename>]\n");
	fprintf(stdout,"\t[-l[og_level] <verbosity>][-h[elp]]\n");
	fprintf(stdout,"\t[-s[et_temperature] <temperature>]\n");
	fprintf(stdout,"\t[-g[et_temperature]]\n");
	fprintf(stdout,"\t[-on]\n");
	fprintf(stdout,"\t[-off]\n");
	fprintf(stdout,"\t[-noshutdown]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-help prints out this message and stops the program.\n");
	fprintf(stdout,"\t<filename> should be a valid filename.\n");
	fprintf(stdout,"\t<temperature> should be a valid double, a temperature in degrees Celcius.\n");
	fprintf(stdout,"\t<verbosity> should be an integer log level between 0 and 5 inclusive.\n");
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Config_Filename
 * @see #Temperature
 * @see #Command
 * @see #Call_Setup_Shutdown
 * @see ../cdocs/ccd_general.html#CCD_General_Set_Log_Filter_Function
 * @see ../cdocs/ccd_general.html#CCD_General_Log_Filter_Level_Absolute
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
		else if((strcmp(argv[i],"-get")==0)||(strcmp(argv[i],"-g")==0))
		{
			Command = COMMAND_ID_GET;
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
		else if((strcmp(argv[i],"-noshutdown")==0))
		{
			Call_Setup_Shutdown = FALSE;
		}
		else if((strcmp(argv[i],"-on")==0))
		{
			Command = COMMAND_ID_ON;
		}
		else if((strcmp(argv[i],"-off")==0))
		{
			Command = COMMAND_ID_OFF;
		}
		else if((strcmp(argv[i],"-set_temperature")==0)||(strcmp(argv[i],"-s")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%lf",&Temperature);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing temperature %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				Command = COMMAND_ID_SET;
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:temperature required.\n");
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
 * Internal routine that saves some basic FITS headers to the relevant filename.
 * This is needed as CCD_Exposure_Expose/Bias routines need saved FITS headers to
 * not give an error.
 * @param exposure_time The amount of time, in milliseconds, of the exposure.
 * @param ncols The number of columns in the FITS file.
 * @param nrows The number of rows in the FITS file.
 * @param filename The filename to save the FITS headers in.
 * @return The routine returns TRUE on success and FALSE on failure.
 */
static int Test_Save_Fits_Headers(int exposure_time,int ncols,int nrows,char *filename)
{
	static fitsfile *fits_fp = NULL;
	int status = 0,retval,ivalue;
	double dvalue;

/* open file */
	if(fits_create_file(&fits_fp,filename,&status))
	{
		Test_Fits_Header_Error(status);
		return FALSE;
	}
/* SIMPLE keyword */
	ivalue = TRUE;
	retval = fits_update_key(fits_fp,TLOGICAL,(char*)"SIMPLE",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Header_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* BITPIX keyword */
	ivalue = 16;
	retval = fits_update_key(fits_fp,TINT,(char*)"BITPIX",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Header_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* NAXIS keyword */
	ivalue = 2;
	retval = fits_update_key(fits_fp,TINT,(char*)"NAXIS",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Header_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* NAXIS1 keyword */
	ivalue = ncols;
	retval = fits_update_key(fits_fp,TINT,(char*)"NAXIS1",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Header_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* NAXIS2 keyword */
	ivalue = nrows;
	retval = fits_update_key(fits_fp,TINT,(char*)"NAXIS2",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Header_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* BZERO keyword */
	dvalue = 32768.0;
	retval = fits_update_key_fixdbl(fits_fp,(char*)"BZERO",dvalue,6,
		(char*)"Number to offset data values by",&status);
	if(retval != 0)
	{
		Test_Fits_Header_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* BSCALE keyword */
	dvalue = 1.0;
	retval = fits_update_key_fixdbl(fits_fp,(char*)"BSCALE",dvalue,6,
		(char*)"Number to multiply data values by",&status);
	if(retval != 0)
	{
		Test_Fits_Header_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* close file */
	if(fits_close_file(fits_fp,&status))
	{
		Test_Fits_Header_Error(status);
		return FALSE;
	}
	return TRUE;
}

/**
 * Internal routine to write the complete CFITSIO error stack to stderr.
 * @param status The status returned by CFITSIO.
 */
static void Test_Fits_Header_Error(int status)
{
	/* report the whole CFITSIO error message stack to stderr. */
	fits_report_error(stderr, status);
}
