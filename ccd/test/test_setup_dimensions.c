/* test_setup_dimensions.c
 * $Id$
 * Test generic CCD_Setup_Dimensions (and also driver linking/initialisation/ CCD_Setup_Startup).
 */
/**
 * Test generic CCD_Setup_Dimensions (and also driver linking/initialisation/ CCD_Setup_Startup).
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

/* hash definitions */
/**
 * Default number of columns on the detector.
 */
#define DEFAULT_SIZE_X		(2048)
/**
 * Default number of rows on the detector.
 */
#define DEFAULT_SIZE_Y		(2048)

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
 * The number of columns on the detector.
 * @see #DEFAULT_SIZE_X
 */
static int Size_X = DEFAULT_SIZE_X;
/**
 * The number of rows on the detector.
 * @see #DEFAULT_SIZE_Y
 */
static int Size_Y = DEFAULT_SIZE_Y;
/**
 * The binning factor in X.
 */
static int Bin_X = 1;
/**
 * The binning factor in Y.
 */
static int Bin_Y = 1;
/**
 * Window flags specifying whether to use the specified window, or not.
 */
static int Window_Flags = FALSE;
/**
 * Window data.
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 */
static struct CCD_Setup_Window_Struct Window;

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
 * <li>We get the shared library to use from the config data, by retrieving the 
 *     "ccd.driver.shared_library" key using CCD_Config_Get_String.
 * <li>We get the driver registration function to use for the shared library from the config data, by retrieving the
 *     "ccd.driver.registration_function" string using CCD_Config_Get_String.
 * <li>We register (dynamically load) the specified driver and register the interface functions by calling 
 *     CCD_Driver_Register.
 * <li>We initialise the setup module by calling CCD_Setup_Initialise.
 * <li>We make a connection to the camera, and initially configure it, by calling CCD_Setup_Startup.
 * <li>We configure the detector dimensions by calling CCD_Setup_Dimensions, with Size_X, Size_Y, Bin_X, Bin_Y, 
 *     Window_Flags and Window as arguments.
 * <li>We shut the program down by calling CCD_Setup_Shutdown / CCD_Config_Shutdown.
 * </ul>
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #Parse_Arguments
 * @see #Config_Filename
 * @see #Size_X
 * @see #Size_Y
 * @see #Bin_X
 * @see #Bin_Y
 * @see #Window_Flags
 * @see #Window
 * @see ../cdocs/ccd_general.html#CCD_General_Set_Log_Handler_Function
 * @see ../cdocs/ccd_general.html#CCD_General_Log_Handler_Stdout
 * @see ../cdocs/ccd_general.html#CCD_General_Error
 * @see ../cdocs/ccd_config.html#CCD_Config_Initialise
 * @see ../cdocs/ccd_config.html#CCD_Config_Load
 * @see ../cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../cdocs/ccd_config.html#CCD_Config_Shutdown
 * @see ../cdocs/ccd_driver.html#CCD_Driver_Register
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Dimensions
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Initialise
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Startup
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Shutdown
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
		fprintf(stderr, "test_setup_dimensions: Config filename was NULL.\n");
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
	/* configure dimensions */
	retval = CCD_Setup_Dimensions(Size_X,Size_Y,Bin_X,Bin_Y,Window_Flags,Window);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 8;
	}	
	/* and shutdown again */
	retval = CCD_Setup_Shutdown();
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
	fprintf(stdout,"test_setup_dimensions completed.\n");
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
	fprintf(stdout,"Test Setup Dimensions:Help.\n");
	fprintf(stdout,"This program calls CCD_Setup_Dimensions after loading/registering a driver, and setting up the camera, to test setup dimension code.\n");
	fprintf(stdout,"test_setup_dimensions \n");
	fprintf(stdout,"\t[-co[nfig_filename] <filename>]\n");
	fprintf(stdout,"\t[-l[og_level] <verbosity>][-h[elp]]\n");
	fprintf(stdout,"\t[-xs[ize] <no. of pixels>][-ys[ize] <no. of pixels>]\n");
	fprintf(stdout,"\t[-xb[in] <binning factor>][-yb[in] <binning factor>]\n");
	fprintf(stdout,"\t[-w[indow] <xstart> <ystart> <xend> <yend>]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-help prints out this message and stops the program.\n");
	fprintf(stdout,"\t<filename> should be a valid filename.\n");
	fprintf(stdout,"\t<verbosity> should be an integer log level between 0 and 5 inclusive.\n");
	fprintf(stdout,"\t<no. of pixels> and <binning factor> is a positive integer.\n");
	fprintf(stdout,"\tThe window dimensions are positive integers and inclusive.\n");
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Config_Filename
 * @see #Size_X
 * @see #Size_Y
 * @see #Bin_X
 * @see #Bin_Y
 * @see #Window_Flags
 * @see #Window
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
		else if((strcmp(argv[i],"-window")==0)||(strcmp(argv[i],"-w")==0))
		{
			if((i+4)<argc)
			{
				
				Window_Flags = TRUE;
				/* x start */
				retval = sscanf(argv[i+1],"%d",&(Window.X_Start));
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing Window Start X (%s) failed.\n",
						argv[i+1]);
					return FALSE;
				}
				/* y start */
				retval = sscanf(argv[i+2],"%d",&(Window.Y_Start));
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing Window Start Y (%s) failed.\n",
						argv[i+2]);
					return FALSE;
				}
				/* x end */
				retval = sscanf(argv[i+3],"%d",&(Window.X_End));
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing Window End X (%s) failed.\n",
						argv[i+3]);
					return FALSE;
				}
				/* y end */
				retval = sscanf(argv[i+4],"%d",&(Window.Y_End));
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing Window End Y (%s) failed.\n",
						argv[i+4]);
					return FALSE;
				}
				i+= 4;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:-window requires 4 argument:%d supplied.\n",argc-i);
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-xsize")==0)||(strcmp(argv[i],"-xs")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Size_X);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing X Size %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:size required.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-ysize")==0)||(strcmp(argv[i],"-ys")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Size_Y);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing Y Size %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:size required.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-xbin")==0)||(strcmp(argv[i],"-xb")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Bin_X);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing X Bin %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:bin required.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-ybin")==0)||(strcmp(argv[i],"-yb")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Bin_Y);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing Y Bin %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:bin required.\n");
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

