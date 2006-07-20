/* ngatcil_ags_sdb.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_ags_sdb.h,v 1.1 2006-07-20 15:12:17 cjm Exp $
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
typedef enum eMcpDataId_e
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
} eMcpDataId_t;

/**
 * Enumerated list of states reported to the SDB by the autoguider. 
 * Ags.h.
 */
typedef enum eAgsState_e
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
} eAgsState_t;   

/**
 * Global data for submission to the Status Database (SDB).
 * Ags.h.
 */
typedef enum eAgsDataId_e
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
} eAgsDataId_t;


/* external function declarations */
extern int NGATCil_AGS_SDB_Initialise(void);
extern int NGATCil_AGS_SDB_Open_Default(int *socket_id);
extern int NGATCil_AGS_SDB_Status_Send(int socket_id);
extern int NGATCil_AGS_SDB_Value_Set(eAgsDataId_t datum_id,int value);

#endif
