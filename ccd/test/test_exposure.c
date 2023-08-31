/* test_exposure.c
 * $Id$
 * Test doing an exposure via the generic CCD library.
 */
/**
 * Test doing an exposure via the generic CCD library.
 * @author $Author: cjm $
 * @version $Revision$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ccd_config.h"
#include "ccd_driver.h"
#include "ccd_exposure.h"
#include "ccd_general.h"
#include "ccd_setup.h"
#include "ccd_temperature.h"

#include "fitsio.h"

/* hash definitions */
/**
 * Default temperature to set the detector to.
 */
#define DEFAULT_TEMPERATURE	(-20.0)
/**
 * Default number of columns on the detector.
 */
#define DEFAULT_SIZE_X		(2048)
/**
 * Default number of rows on the detector.
 */
#define DEFAULT_SIZE_Y		(2048)


/* enums */
/**
 * Enumeration determining which command this program executes. One of:
 * <ul>
 * <li>COMMAND_ID_NONE
 * <li>COMMAND_ID_BIAS
 * <li>COMMAND_ID_DARK
 * <li>COMMAND_ID_EXPOSURE
 * </ul>
 */
enum COMMAND_ID
{
	COMMAND_ID_NONE=0,COMMAND_ID_BIAS,COMMAND_ID_DARK,COMMAND_ID_EXPOSURE
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
 * A boolean, whether to set the detector temperature during initialisation (TRUE) or not (FALSE).
 */
static int Set_Temperature = FALSE;
/**
 * Temperature to set the detector to in degrees centigrade.
 * @see #DEFAULT_TEMPERATURE
 */
static double Temperature = DEFAULT_TEMPERATURE;
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
 */
static struct CCD_Setup_Window_Struct Window;
/**
 * Which type ef exposure command to call.
 * @see #COMMAND_ID
 */
static enum COMMAND_ID Command = COMMAND_ID_NONE;
/**
 * If doing a dark or exposure, the exposure length.
 */
static int Exposure_Length = 0;
/**
 * Filename to store resultant fits image in. 
 */
static char *Fits_Filename = NULL;

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
 * <li>If configured with Set_Temperature, we set the detector temperature with a call to 
 *     CCD_Temperature_Set with Temperature as an argument.
 * <li>We configure the detector dimensions by calling CCD_Setup_Dimensions, with Size_X, Size_Y, Bin_X, Bin_Y, 
 *     Window_Flags and Window as arguments.
 * <li>We allocate an image buffer, using CCD_Setup_Get_NCols and CCD_Setup_Get_NRows to get the image dimensions.
 * <li>Depending on the Command selected in Parse_Arguments, we call CCD_Exposure_Bias or CCD_Exposure_Expose with 
 *     Exposure_Length as an argument to do a bias,dark or exposure.
 * <li>We call Test_Save_Fits_Headers to save some FITS headers to Fits_Filename.
 * <li>We callCCD_Exposure_Save to save the read-out image data to the FITS filename Fits_Filename.
 * <li>We shut the program down by calling CCD_Setup_Shutdown / CCD_Config_Shutdown.
 * </ul>
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #Parse_Arguments
 * @see #Test_Save_Fits_Headers
 * @see #Config_Filename
 * @see #Fits_Filename
 * @see #Size_X
 * @see #Size_Y
 * @see #Bin_X
 * @see #Bin_Y
 * @see #Window_Flags
 * @see #Window
 * @see #Set_Temperature
 * @see #Temperature
 * @see #Exposure_Length
 * @see ../cdocs/ccd_general.html#CCD_General_Set_Log_Handler_Function
 * @see ../cdocs/ccd_general.html#CCD_General_Log_Handler_Stdout
 * @see ../cdocs/ccd_general.html#CCD_General_Error
 * @see ../cdocs/ccd_config.html#CCD_Config_Initialise
 * @see ../cdocs/ccd_config.html#CCD_Config_Load
 * @see ../cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../cdocs/ccd_config.html#CCD_Config_Shutdown
 * @see ../cdocs/ccd_driver.html#CCD_Driver_Register
 * @see ../cdocs/ccd_exposure.html#CCD_Exposure_Bias
 * @see ../cdocs/ccd_exposure.html#CCD_Exposure_Expose
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Dimensions
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Get_NCols
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Get_NRows
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Initialise
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Startup
 * @see ../cdocs/ccd_setup.html#CCD_Setup_Shutdown
 * @see ../cdocs/ccd_temperature.html#CCD_Temperature_Set
 */
int main(int argc, char *argv[])
{
	struct timespec start_time;
	unsigned short *image_buffer = NULL;
	size_t image_buffer_length = 0;
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
	retval = CCD_Setup_Startup();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 7;
	}
	/* set temperature if configured to do so */
	if(Set_Temperature)
	{
		fprintf(stdout,"Setting detector temperature to %.2f C.\n",Temperature);
		retval = CCD_Temperature_Set(Temperature);
		if(retval == FALSE)
		{
			CCD_General_Error();
			return 8;
		}
	}
	/* configure dimensions */
	fprintf(stdout,"Calling CCD_Setup_Dimensions:\n");
	fprintf(stdout,"Chip Size:(%d,%d)\n",Size_X,Size_Y);
	fprintf(stdout,"Binning:(%d,%d)\n",Bin_X,Bin_Y);
	fprintf(stdout,"Window Flags:%d\n",Window_Flags);
	if(Window_Flags > 0)
	{
		fprintf(stdout,"Window:[xs=%d,xe=%d,ys=%d,ye=%d]\n",Window.X_Start,Window.X_End,
			Window.Y_Start,Window.Y_End);
	}
	retval = CCD_Setup_Dimensions(Size_X,Size_Y,Bin_X,Bin_Y,Window_Flags,Window);
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 9;
	}	
	/* allocate buffer for image */
	fprintf(stdout,"Allocating image buffer memory: %d x %d pixels.\n",CCD_Setup_Get_NCols(),CCD_Setup_Get_NRows());
	image_buffer_length = CCD_Setup_Get_NCols()*CCD_Setup_Get_NRows();
	image_buffer = (unsigned short*)malloc(image_buffer_length*sizeof(unsigned short));
	if(image_buffer == NULL)
	{
		fprintf(stderr,"Failed to allocate image buffer of length %ld.\n",image_buffer_length);
		return 10;
	}
/* do command */
	start_time.tv_sec = 0;
	start_time.tv_nsec = 0;
	switch(Command)
	{
		case COMMAND_ID_BIAS:
			fprintf(stdout,"Calling CCD_Exposure_Bias.\n");
			retval = CCD_Exposure_Bias(image_buffer,image_buffer_length);
			break;
		case COMMAND_ID_DARK:
			fprintf(stdout,
				"Calling CCD_Exposure_Expose with open_shutter FALSE and exposure length %d ms.\n",
				Exposure_Length);
			retval = CCD_Exposure_Expose(FALSE,start_time,Exposure_Length,
						       image_buffer,image_buffer_length);
			break;
		case COMMAND_ID_EXPOSURE:
			fprintf(stdout,
				"Calling CCD_Exposure_Expose with open_shutter TRUE and exposure length %d ms.\n",
				Exposure_Length);
			retval = CCD_Exposure_Expose(TRUE,start_time,Exposure_Length,
						       image_buffer,image_buffer_length);
			break;
		case COMMAND_ID_NONE:
			fprintf(stdout,"Please select a command to execute (-bias | -dark | -expose).\n");
			Help();
			return 5;
	}
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 11;
	}
	fprintf(stdout,"Saving fits headers to '%s'.\n",Fits_Filename);
	if(!Test_Save_Fits_Headers(Exposure_Length,CCD_Setup_Get_NCols(),CCD_Setup_Get_NRows(),Fits_Filename))
	{
		fprintf(stdout,"Saving FITS headers failed.\n");
		return 12;
	}
	fprintf(stdout,"Saving image data to '%s'.\n",Fits_Filename);
	retval = CCD_Exposure_Save(Fits_Filename,image_buffer,image_buffer_length,
				   CCD_Setup_Get_NCols(),CCD_Setup_Get_NRows());
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 13;
	}
	/* and shutdown again */
	fprintf(stdout,"Shutting down....\n");
	if(image_buffer != NULL)
		free(image_buffer);
	retval = CCD_Setup_Shutdown();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 14;
	}
	retval = CCD_Config_Shutdown();
	if(retval == FALSE)
	{
		CCD_General_Error();
		return 15;
	}
	fprintf(stdout,"test_exposure completed.\n");
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
	fprintf(stdout,"Test Exposure:Help.\n");
	fprintf(stdout,"This program takes a bias/dark/exposure using the generic autoguider CCD interface.\n");
	fprintf(stdout,"test_exposure \n");
	fprintf(stdout,"\t[-co[nfig_filename] <filename>]\n");
	fprintf(stdout,"\t[-l[og_level] <verbosity>][-h[elp]]\n");
	fprintf(stdout,"\t[-temperature <temperature>]\n");
	fprintf(stdout,"\t[-xs[ize] <no. of pixels>][-ys[ize] <no. of pixels>]\n");
	fprintf(stdout,"\t[-xb[in] <binning factor>][-yb[in] <binning factor>]\n");
	fprintf(stdout,"\t[-w[indow] <xstart> <ystart> <xend> <yend>]\n");
	fprintf(stdout,"\t[-f[its_filename] <filename>]\n");
	fprintf(stdout,"\t[-b[ias]][-d[ark] <exposure length>][-e[xpose] <exposure length>]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-help prints out this message and stops the program.\n");
	fprintf(stdout,"\t<filename> should be a valid filename.\n");
	fprintf(stdout,"\t<temperature> should be a valid double, a temperature in degrees Celcius.\n");
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
 * @see #Set_Temperature
 * @see #Temperature
 * @see #Size_X
 * @see #Size_Y
 * @see #Bin_X
 * @see #Bin_Y
 * @see #Window_Flags
 * @see #Window
 * @see #Command
 * @see #Exposure_Length
 * @see #Fits_Filename
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
		else if((strcmp(argv[i],"-bias")==0)||(strcmp(argv[i],"-b")==0))
		{
			Command = COMMAND_ID_BIAS;
		}
		else if((strcmp(argv[i],"-dark")==0)||(strcmp(argv[i],"-d")==0))
		{
			Command = COMMAND_ID_DARK;
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Exposure_Length);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing exposure length %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:dark requires exposure length.\n");
				return FALSE;
			}

		}
		else if((strcmp(argv[i],"-expose")==0)||(strcmp(argv[i],"-e")==0))
		{
			Command = COMMAND_ID_EXPOSURE;
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Exposure_Length);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing exposure length %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:exposure requires exposure length.\n");
				return FALSE;
			}

		}
		else if((strcmp(argv[i],"-fits_filename")==0)||(strcmp(argv[i],"-f")==0))
		{
			if((i+1)<argc)
			{
				Fits_Filename = argv[i+1];
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:filename required.\n");
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
		else if(strcmp(argv[i],"-temperature")==0)
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
				Set_Temperature = TRUE;
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:temperature required.\n");
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
