/* ngatcil_ags_sdb.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_ags_sdb.h,v 1.2 2006-08-29 14:12:06 cjm Exp $
*/
#ifndef NGATCIL_AGS_SDB_H
#define NGATCIL_AGS_SDB_H

/* hash defines */
/**
 * Default machine name to send SDB data to (mcc). Should be mapped to 192.168.1.1 in /etc/hosts.
 */
#define NGATCIL_AGS_SDB_MCC_DEFAULT      ("mcc")
/**
 * Default port number to send SDB CIL packets to (13011). 
 */
#define NGATCIL_AGS_SDB_CIL_PORT_DEFAULT (13011)

/* enums */
/**
 * Global data submitted to the SDB on behalf of each application.
 * TtlSystem.h.
 */
enum eMcpDataId_e
{
   D_MCP_DATAID_BOL = 0,     /* Beginning of data list                        */
   D_MCP_PROC_STATE = 1,     /* Process state                                 */
                             /*  units : PROCSTATE,  enum : eSysProcState_e   */
   D_MCP_AUTH_STATE,         /* Requested/granted authorisation state         */
                             /*  units : AUTH_STATE, enum : eMcpAuthState_e   */
   D_MCP_SYS_REQUEST,        /* System request for MCP (eMcpSysRequest_e)     */
                             /*  units : SYSREQ,     enum : eMcpSysRequest_e  */
   D_MCP_APP_VERSION,        /* Application version                           */
                             /*  units : MVERSION,   enum : -                 */
   D_MCP_FIRST_USER_DATUM,   /* First datum slot available to user            */
   D_MCP_DATAID_EOL          /* End of list marker                            */
};

/**
 * Typedef of global data submitted to the SDB on behalf of each application.
 * TtlSystem.h.
 * @see #eMcpDataId_e
 */
typedef enum eMcpDataId_e eMcpDataId_t;

/**
 * Enumerated list of states reported to the SDB by the autoguider. 
 * Doesn't actually seem to be used - AGSTATE filled in with AggState...
 * Ags.h.
 */
enum eAgsState_e
{
  E_AGS_OFF,                  /* Not ready to accept observing commands. */
  E_AGS_ON_BRIGHTEST,         /* Supplying guiding corrections on brightest */
                              /* non-saturated source. */
  E_AGS_ON_RANGE,             /* Supplying guiding corrections on brightest */
                              /* non-saturated object in supplied range. */ 
  E_AGS_ON_RANK,              /* Supplying guiding corrections on nth */
                              /* brightest non-saturated object. */
  E_AGS_ON_PIXEL,             /* Supplying guiding corrections on object */
                              /* closest to supplied pixel. */
  E_AGS_IDLE,                 /* Ready to accept observing commands. */
  E_AGS_WORKING,              /* Currently acquiring guide source. */
  E_AGS_INITIALISING,         /* Not ready for operational use. */
  E_AGS_FAILED,               /* Failed to find a guide source. */
  E_AGS_INTERACTIVE_WORKING,  /* Currently being used interactively but */
                              /* not supplying guiding data. */
  E_AGS_INTERACTIVE_ON,       /* Currently being used interactively and */
                              /* suppliying guiding data. */
  E_AGS_ERROR,                /* Recoverable error. */
  E_AGS_NON_RECOVERABLE_ERROR /* Non recoverable error. */
};   

/**
 * Typedef of enumerated list of states reported to the SDB by the autoguider. 
 * Ags.h.
 * @see #eAgsState_e
 */
typedef enum eAgsState_e eAgsState_t;

/**
 * Enumerated list of keywords used to build up commands to the AGG. 
 * Agg.h.
 */
enum eAggCmdKeyword_e
{
   E_AGG_ON,                 /* Turn on (ie guiding, loop). */ 
   E_AGG_OFF,                /* Turn off (ie guiding, loop). */
   E_AGG_BRIGHTEST,          /* Guide on brightest source. */
   E_AGG_RANGE,              /* Guide on source within magnitude range. */
   E_AGG_RANK,               /* Guide on source of given rank. */
   E_AGG_PIXEL,              /* Guide on source closest to supplied pixel. */
   E_AGG_CONF_EXPTIME,       /* Configure exposure time. */
   E_AGG_CONF_FRATE,         /* Configure frame rate. */
   E_AGG_CONF_FAVERAGE,      /* Configure frame average. */ 
   E_AGG_CONF_CALIB          /* Configure calibration (grey level->rank). */
};

/**
 * Typedef of enumerated list of keywords used to build up commands to the AGG. 
 * Agg.h.
 * @see #eAggCmdKeyword_e
 */
typedef enum eAggCmdKeyword_e eAggCmdKeyword_t;

/**
 * Enumerated list of Agg states.
 * Agg.h.
 */
enum eAggState_e
{
   E_AGG_STATE_UNKNOWN          = 0x0,
   E_AGG_STATE_OK               = 0x10,
   E_AGG_STATE_OFF,
   E_AGG_STATE_STANDBY,
   E_AGG_STATE_IDLE,
   E_AGG_STATE_WORKING,
   E_AGG_STATE_INIT,
   E_AGG_STATE_FAILED,
   E_AGG_STATE_INTWORK,
   E_AGG_STATE_ERROR            = 0x100,
   E_AGG_STATE_NONRECERR,
   E_AGG_STATE_LOOP_RUNNING     = 0x1000,
   E_AGG_STATE_GUIDEONBRIGHT    = E_AGG_STATE_LOOP_RUNNING + E_AGG_BRIGHTEST,
   E_AGG_STATE_GUIDEONRANGE     = E_AGG_STATE_LOOP_RUNNING + E_AGG_RANGE,
   E_AGG_STATE_GUIDEONRANK      = E_AGG_STATE_LOOP_RUNNING + E_AGG_RANK,
   E_AGG_STATE_GUIDEONPIXEL     = E_AGG_STATE_LOOP_RUNNING + E_AGG_PIXEL,
   E_AGG_STATE_INTON,
   E_AGG_TSTATE_AT_TEMP         = 0x10000,
   E_AGG_TSTATE_ABOVE_TEMP,
   E_AGG_TSTATE_BELOW_TEMP,
   E_AGG_TSTATE_ERROR,
   E_AGG_STATE_EOL
};

/**
 * Typedef of enumerated list of AGG states reported to the SDB by the autoguider. 
 * Ags.h.
 * @see #eAggState_e
 */
typedef enum eAggState_e eAggState_t;   


/**
 * Global data for submission to the Status Database (SDB).
 * Ags.h.
 */
enum eAgsDataId_e
{
   D_AGS_DATAID_BOL = 0,     /* Begining of data list */

   D_AGS_PROC_STATE          /* Process state datum */
      = D_MCP_PROC_STATE,
   D_AGS_AUTH_STATE          /* Requested/granted authorisation state */
      = D_MCP_AUTH_STATE,
   D_AGS_SYS_REQUEST         /* System requests made to MCP */
      = D_MCP_SYS_REQUEST,
   D_AGS_APP_VERSION         /* Application version number */
      = D_MCP_APP_VERSION,
   D_AGS_AGSTATE = D_MCP_FIRST_USER_DATUM,  /* State of autoguider subsystem. */
   D_AGS_WINDOW_TLX,         /* X pixel of top left guide win coord. */
   D_AGS_WINDOW_TLY,         /* Y pixel of top left guide win coord. */
   D_AGS_WINDOW_BRX,         /* X pixel of bottom right guide win coord. */
   D_AGS_WINDOW_BRY,         /* Y pixel of bottom right guide win coord. */
   D_AGS_INTTIME,            /* Int time for current/last guide loop. */
   D_AGS_FRAMESKIP,          /* Frame skip for current/last guide loop. */
   D_AGS_GUIDEMAG,           /* Star magnitude of guide star on loop start. */
   D_AGS_CENTROIDX,          /* X pixel of current/last centroid. */
   D_AGS_CENTROIDY,          /* Y pixel of current/last centroid. */
   D_AGS_FWHM,               /* Measured FWHM of current/last centroid. */
   D_AGS_PEAKPIXEL,          /* Star magnitude of current/last centroid. */
   D_AGS_AGTEMP,             /* Chip temperature. */
   D_AGS_AGCASETEMP,         /* Autoguider case temperature. */
   D_AGS_AGPERCPOW,          /* Autoguider cooler power. */
   D_AGS_AGFRAMEMEAN,        /* Autoguider frame mean */
   D_AGS_AGFRAMERMS,         /* Autoguider frame rms */
   D_AGS_DATAID_EOL          /* End of list.                                */
};

/**
 * Typedef of global data for submission to the Status Database (SDB).
 * Ags.h.
 * @see #eAgsDataId_e
 */
typedef enum eAgsDataId_e eAgsDataId_t;

/* external function declarations */
extern int NGATCil_AGS_SDB_Initialise(void);
extern int NGATCil_AGS_SDB_Remote_Host_Set(char *hostname,int port_number);
extern int NGATCil_AGS_SDB_Status_Send(int socket_id);
extern int NGATCil_AGS_SDB_Value_Set(eAgsDataId_t datum_id,int value);

#endif
