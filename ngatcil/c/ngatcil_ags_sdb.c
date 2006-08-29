/* ngatcil_ags_sdb.c
** NGATCil AGS SDB CIL packet tranmitting/receiving routines.
** $Header: /home/cjm/cvs/autoguider/ngatcil/c/ngatcil_ags_sdb.c,v 1.2 2006-08-29 14:07:57 cjm Exp $
*/
/**
 * NGAT Cil library transmission of AGS SDB packets over UDP.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ngatcil_ags_sdb.h"
#include "ngatcil_general.h"
#include "ngatcil_udp_raw.h"
#include "ngatcil_cil.h"

/* hash defines */
/**
 * Unknown AGS value to put into the SDB.
 */
#define E_AGS_UNKNOWN 0
/**
 * Bodge conversion between QNX typdefs and gnu gcc. int32_t is defined in inttypes.h.
 * See TtlSystem.h for their version.
 */
#define Int32_t int32_t
/**
 * Bodge conversion between QNX typdefs and gnu gcc. uint32_t is defined in inttypes.h 
 * See TtlSystem.h for their version.
 */
#define Uint32_t uint32_t
/**
 * Define first and last datum IDs to be submitted to the SDB.
 */
#define I_AGS_FIRST_DATUMID    D_MCP_FIRST_USER_DATUM
/**
 * Define first and last datum IDs to be submitted to the SDB.
 */
#define I_AGS_FINAL_DATUMID    D_AGS_AGPERCPOW
/**
 * Length of hostname.
 */
#define HOSTNAME_LENGTH        (256)

/* data types */
/**
 * Bool_t typedef. As defined in TtlSystem.h.
 */
typedef int Bool_t;

/**
 * TTL time structure.
 * TtlSystem.h.
 * @see #TTL_TIMESTAMP_OFFSET
 */
struct Ttl_Time_Struct
{
   Int32_t t_sec;      /* Elapsed time in seconds */
   Int32_t t_nsec;     /* Elapsed nanoseconds in present second */
};

/**
 * Typedef of TTL time structure.
 * TtlSystem.h.
 * @see #TTL_TIMESTAMP_OFFSET
 * @see #Ttl_Time_Struct
 */
typedef struct Ttl_Time_Struct eTtlTime_t;

/**
 * States of MCB authorisation request/refused/obtained made via SDB.
 * Mcp.h.
 */
enum eMcpAuthState_e
{

   E_MCP_AUTH_BOL,                     /* Beginning of list */

   E_MCP_AUTH_NONE,                    /* Authorisation is not requested */
   E_MCP_AUTH_REQUEST,                 /* Application requests authorisation */
   E_MCP_AUTH_GRANTED,                 /* Application granted authorisation */
   E_MCP_AUTH_REFUSED,                 /* Application refused authorisation */
   E_MCP_AUTH_SYSREQ_ONLY,             /* Application can make system requests only */

   E_MCP_AUTH_EOL                      /* End of list marker */

};

/**
 * Typedef of states of MCB authorisation request/refused/obtained made via SDB.
 * Mcp.h.
 * @see #eMcpAuthState_e
 */
typedef enum eMcpAuthState_e eMcpAuthState_t;

/**
 * Enumeration of MCP system requests.
 * Mcp.h.
 */
enum eMcpSysReqIndex_e
{
   E_MCP_SYSREQ_NULL = 0,              /* No system request (dummy) */
   E_MCP_SYSREQ_SAFE_STATE,            /* Request for safe-state */
   E_MCP_SYSREQ_ACTIVATE,              /* Request for activate following safe-state */
   E_MCP_SYSREQ_FULL_SHUTDOWN,         /* Request for full shutdown */
   E_MCP_SYSREQ_FULL_STARTUP,          /* Request for full start-up (unused) */
   E_MCP_SYSREQ_PARTIAL_SHUTDOWN,      /* Request for partial shutdown */
   E_MCP_SYSREQ_PARTIAL_STARTUP,       /* Request for partial start-up */
   E_MCP_SYSREQ_START_OBSERVATION,     /* Request for system observation start */
   E_MCP_SYSREQ_STOP_OBSERVATION,      /* Request for system observation stop */
   E_MCP_SYSREQ_IMMED_SHUTDOWN,        /* Request for immediate full shutdown */
   E_MCP_SYSREQ_FULL_RESTART,          /* Request for full system restart */
   E_MCP_SYSREQ_FAIL_OVERRIDE,         /* Request for shutdown on failure override */
   E_MCP_SYSREQ_FAIL_RESTORE,          /* Request to cancel shutdown on failure override */
   E_MCP_SYSREQ_TIME_OVERRIDE,         /* Request for observation time override */
   E_MCP_SYSREQ_TIME_RESTORE,          /* Request to cancel observation time override */
   E_MCP_SYSREQ_WMS_OVERRIDE,          /* Request for bad weather override */
   E_MCP_SYSREQ_WMS_RESTORE,           /* Request to cancel bad weather override */
   E_MCP_SYSREQ_EPT_OVERRIDE,          /* Request for EPT problem override */
   E_MCP_SYSREQ_EPT_RESTORE,           /* Request to cancel EPT problem override */
   E_MCP_SYSREQ_SPT_OVERRIDE,          /* Request for SPT problem override */
   E_MCP_SYSREQ_SPT_RESTORE,           /* Request to cancel SPT problem override */
   E_MCP_SYSREQ_NODE_OVERRIDE,         /* Request for node problem override */
   E_MCP_SYSREQ_NODE_RESTORE,          /* Request to cancel node problem override */
   E_MCP_SYSREQ_CANCEL_START_OBS,      /* Request to cancel a start observation */
   E_MCP_SYSREQ_CANCEL_STOP_OBS,       /* Request to cancel a stop observation */
   E_MCP_SYSREQ_REREAD_OBS,            /* Request to re-read sections of config file */
   E_MCP_SYSREQ_SOFT_SHUTDOWN,         /* Request for software-only shutdown */

   E_MCP_SYSREQ_EOL                    /* End of list marker */


};

/**
 * Typedef of enumeration of MCP system requests.
 * Mcp.h.
 * @see #eMcpSysReqIndex_e
 */
typedef enum eMcpSysReqIndex_e eMcpSysReqIndex_t;

/**
 * Offsets within system requests - same order as eMcpSysRequest_e below.
 * Mcp.h.
 */
enum eMcpSysReqOffset_e
{

   E_MCP_SYSREQ_REQ = 0,               /* System request */
   E_MCP_SYSREQ_ACT,                   /* Action in progress */
   E_MCP_SYSREQ_COM,                   /* Action completed */
   E_MCP_SYSREQ_ERR,                   /* Unable to perform action */

   E_MCP_SYSREQ_OFFSET                 /* Size of offset between system requests */

};

/**
 * Typedef of offsets within system requests - same order as eMcpSysRequest_e below.
 * Mcp.h.
 * @see #eMcpSysReqOffset_e
 */
typedef enum eMcpSysReqOffset_e eMcpSysReqOffset_t;

/**
 * States of MCP system requests made in datum for each process in the SDB. 
 * These are the definitions that should be submitted and retrieved to/from the
 * SDB to implement system requests.
 * Mcp.h.
 */
enum eMcpSysRequest_e
{

   E_MCP_SYSREQ_NONE                  = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NULL ),
   E_MCP_SYSREQ_UNAUTHORISED          = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NULL ),

   E_MCP_SYSREQ_REQ_SAFE_STATE        = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SAFE_STATE ),
   E_MCP_SYSREQ_ACT_SAFE_STATE        = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SAFE_STATE ),
   E_MCP_SYSREQ_COM_SAFE_STATE        = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SAFE_STATE ),
   E_MCP_SYSREQ_ERR_SAFE_STATE        = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SAFE_STATE ),

   E_MCP_SYSREQ_REQ_ACTIVATE          = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_ACTIVATE ),
   E_MCP_SYSREQ_ACT_ACTIVATE          = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_ACTIVATE ),
   E_MCP_SYSREQ_COM_ACTIVATE          = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_ACTIVATE ),
   E_MCP_SYSREQ_ERR_ACTIVATE          = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_ACTIVATE ),

   E_MCP_SYSREQ_REQ_FULL_SHUTDOWN     = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_SHUTDOWN ),
   E_MCP_SYSREQ_ACT_FULL_SHUTDOWN     = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_SHUTDOWN ),
   E_MCP_SYSREQ_COM_FULL_SHUTDOWN     = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_SHUTDOWN ),
   E_MCP_SYSREQ_ERR_FULL_SHUTDOWN     = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_SHUTDOWN ),

   E_MCP_SYSREQ_REQ_FULL_STARTUP      = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_STARTUP ),
   E_MCP_SYSREQ_ACT_FULL_STARTUP      = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_STARTUP ),
   E_MCP_SYSREQ_COM_FULL_STARTUP      = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_STARTUP ),
   E_MCP_SYSREQ_ERR_FULL_STARTUP      = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_STARTUP ),
                                             
   E_MCP_SYSREQ_REQ_PARTIAL_SHUTDOWN  = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_PARTIAL_SHUTDOWN ),
   E_MCP_SYSREQ_ACT_PARTIAL_SHUTDOWN  = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_PARTIAL_SHUTDOWN ),
   E_MCP_SYSREQ_COM_PARTIAL_SHUTDOWN  = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_PARTIAL_SHUTDOWN ),
   E_MCP_SYSREQ_ERR_PARTIAL_SHUTDOWN  = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_PARTIAL_SHUTDOWN ),
   
   E_MCP_SYSREQ_REQ_PARTIAL_STARTUP   = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_PARTIAL_STARTUP ),
   E_MCP_SYSREQ_ACT_PARTIAL_STARTUP   = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_PARTIAL_STARTUP ),
   E_MCP_SYSREQ_COM_PARTIAL_STARTUP   = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_PARTIAL_STARTUP ),
   E_MCP_SYSREQ_ERR_PARTIAL_STARTUP   = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_PARTIAL_STARTUP ),

   E_MCP_SYSREQ_REQ_START_OBSERVATION = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_START_OBSERVATION ),
   E_MCP_SYSREQ_ACT_START_OBSERVATION = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_START_OBSERVATION ),
   E_MCP_SYSREQ_COM_START_OBSERVATION = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_START_OBSERVATION ),
   E_MCP_SYSREQ_ERR_START_OBSERVATION = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_START_OBSERVATION ),

   E_MCP_SYSREQ_REQ_STOP_OBSERVATION  = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_STOP_OBSERVATION ),
   E_MCP_SYSREQ_ACT_STOP_OBSERVATION  = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_STOP_OBSERVATION ),
   E_MCP_SYSREQ_COM_STOP_OBSERVATION  = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_STOP_OBSERVATION ),
   E_MCP_SYSREQ_ERR_STOP_OBSERVATION  = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_STOP_OBSERVATION ),

   E_MCP_SYSREQ_REQ_IMMED_SHUTDOWN    = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_IMMED_SHUTDOWN ),
   E_MCP_SYSREQ_ACT_IMMED_SHUTDOWN    = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_IMMED_SHUTDOWN ),
   E_MCP_SYSREQ_COM_IMMED_SHUTDOWN    = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_IMMED_SHUTDOWN ),
   E_MCP_SYSREQ_ERR_IMMED_SHUTDOWN    = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_IMMED_SHUTDOWN ),

   E_MCP_SYSREQ_REQ_FULL_RESTART      = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_RESTART ),
   E_MCP_SYSREQ_ACT_FULL_RESTART      = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_RESTART ),
   E_MCP_SYSREQ_COM_FULL_RESTART      = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_RESTART ),
   E_MCP_SYSREQ_ERR_FULL_RESTART      = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FULL_RESTART ),

   E_MCP_SYSREQ_REQ_FAIL_OVERRIDE     = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FAIL_OVERRIDE ),
   E_MCP_SYSREQ_ACT_FAIL_OVERRIDE     = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FAIL_OVERRIDE ),
   E_MCP_SYSREQ_COM_FAIL_OVERRIDE     = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FAIL_OVERRIDE ),
   E_MCP_SYSREQ_ERR_FAIL_OVERRIDE     = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FAIL_OVERRIDE ),

   E_MCP_SYSREQ_REQ_FAIL_RESTORE      = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FAIL_RESTORE ),
   E_MCP_SYSREQ_ACT_FAIL_RESTORE      = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FAIL_RESTORE ),
   E_MCP_SYSREQ_COM_FAIL_RESTORE      = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FAIL_RESTORE ),
   E_MCP_SYSREQ_ERR_FAIL_RESTORE      = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_FAIL_RESTORE ),

   E_MCP_SYSREQ_REQ_TIME_OVERRIDE     = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_TIME_OVERRIDE ),
   E_MCP_SYSREQ_ACT_TIME_OVERRIDE     = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_TIME_OVERRIDE ),
   E_MCP_SYSREQ_COM_TIME_OVERRIDE     = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_TIME_OVERRIDE ),
   E_MCP_SYSREQ_ERR_TIME_OVERRIDE     = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_TIME_OVERRIDE ),

   E_MCP_SYSREQ_REQ_TIME_RESTORE      = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_TIME_RESTORE ),
   E_MCP_SYSREQ_ACT_TIME_RESTORE      = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_TIME_RESTORE ),
   E_MCP_SYSREQ_COM_TIME_RESTORE      = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_TIME_RESTORE ),
   E_MCP_SYSREQ_ERR_TIME_RESTORE      = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_TIME_RESTORE ),

   E_MCP_SYSREQ_REQ_WMS_OVERRIDE      = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_WMS_OVERRIDE ),
   E_MCP_SYSREQ_ACT_WMS_OVERRIDE      = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_WMS_OVERRIDE ),
   E_MCP_SYSREQ_COM_WMS_OVERRIDE      = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_WMS_OVERRIDE ),
   E_MCP_SYSREQ_ERR_WMS_OVERRIDE      = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_WMS_OVERRIDE ),

   E_MCP_SYSREQ_REQ_WMS_RESTORE       = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_WMS_RESTORE ),
   E_MCP_SYSREQ_ACT_WMS_RESTORE       = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_WMS_RESTORE ),
   E_MCP_SYSREQ_COM_WMS_RESTORE       = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_WMS_RESTORE ),
   E_MCP_SYSREQ_ERR_WMS_RESTORE       = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_WMS_RESTORE ),

   E_MCP_SYSREQ_REQ_EPT_OVERRIDE      = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_EPT_OVERRIDE ),
   E_MCP_SYSREQ_ACT_EPT_OVERRIDE      = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_EPT_OVERRIDE ),
   E_MCP_SYSREQ_COM_EPT_OVERRIDE      = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_EPT_OVERRIDE ),
   E_MCP_SYSREQ_ERR_EPT_OVERRIDE      = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_EPT_OVERRIDE ),
                                                                                                            
   E_MCP_SYSREQ_REQ_EPT_RESTORE       = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_EPT_RESTORE ),
   E_MCP_SYSREQ_ACT_EPT_RESTORE       = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_EPT_RESTORE ),
   E_MCP_SYSREQ_COM_EPT_RESTORE       = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_EPT_RESTORE ),
   E_MCP_SYSREQ_ERR_EPT_RESTORE       = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_EPT_RESTORE ),

   E_MCP_SYSREQ_REQ_SPT_OVERRIDE      = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SPT_OVERRIDE ),
   E_MCP_SYSREQ_ACT_SPT_OVERRIDE      = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SPT_OVERRIDE ),
   E_MCP_SYSREQ_COM_SPT_OVERRIDE      = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SPT_OVERRIDE ),
   E_MCP_SYSREQ_ERR_SPT_OVERRIDE      = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SPT_OVERRIDE ),
                                                                                                            
   E_MCP_SYSREQ_REQ_SPT_RESTORE       = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SPT_RESTORE ),
   E_MCP_SYSREQ_ACT_SPT_RESTORE       = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SPT_RESTORE ),
   E_MCP_SYSREQ_COM_SPT_RESTORE       = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SPT_RESTORE ),
   E_MCP_SYSREQ_ERR_SPT_RESTORE       = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SPT_RESTORE ),

   E_MCP_SYSREQ_REQ_NODE_OVERRIDE     = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NODE_OVERRIDE ),
   E_MCP_SYSREQ_ACT_NODE_OVERRIDE     = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NODE_OVERRIDE ),
   E_MCP_SYSREQ_COM_NODE_OVERRIDE     = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NODE_OVERRIDE ),
   E_MCP_SYSREQ_ERR_NODE_OVERRIDE     = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NODE_OVERRIDE ),
                                                                                                            
   E_MCP_SYSREQ_REQ_NODE_RESTORE      = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NODE_RESTORE ),
   E_MCP_SYSREQ_ACT_NODE_RESTORE      = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NODE_RESTORE ),
   E_MCP_SYSREQ_COM_NODE_RESTORE      = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NODE_RESTORE ),
   E_MCP_SYSREQ_ERR_NODE_RESTORE      = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_NODE_RESTORE ),

   E_MCP_SYSREQ_REQ_CANCEL_START_OBS  = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_CANCEL_START_OBS ),
   E_MCP_SYSREQ_ACT_CANCEL_START_OBS  = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_CANCEL_START_OBS ),
   E_MCP_SYSREQ_COM_CANCEL_START_OBS  = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_CANCEL_START_OBS ),
   E_MCP_SYSREQ_ERR_CANCEL_START_OBS  = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_CANCEL_START_OBS ),

   E_MCP_SYSREQ_REQ_CANCEL_STOP_OBS   = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_CANCEL_STOP_OBS ),
   E_MCP_SYSREQ_ACT_CANCEL_STOP_OBS   = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_CANCEL_STOP_OBS ),
   E_MCP_SYSREQ_COM_CANCEL_STOP_OBS   = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_CANCEL_STOP_OBS ),
   E_MCP_SYSREQ_ERR_CANCEL_STOP_OBS   = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_CANCEL_STOP_OBS ),

   E_MCP_SYSREQ_REQ_REREAD_OBS        = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_REREAD_OBS ),
   E_MCP_SYSREQ_ACT_REREAD_OBS        = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_REREAD_OBS ),
   E_MCP_SYSREQ_COM_REREAD_OBS        = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_REREAD_OBS ),
   E_MCP_SYSREQ_ERR_REREAD_OBS        = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_REREAD_OBS ),

   E_MCP_SYSREQ_REQ_SOFT_SHUTDOWN     = E_MCP_SYSREQ_REQ + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SOFT_SHUTDOWN ),
   E_MCP_SYSREQ_ACT_SOFT_SHUTDOWN     = E_MCP_SYSREQ_ACT + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SOFT_SHUTDOWN ),
   E_MCP_SYSREQ_COM_SOFT_SHUTDOWN     = E_MCP_SYSREQ_COM + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SOFT_SHUTDOWN ),
   E_MCP_SYSREQ_ERR_SOFT_SHUTDOWN     = E_MCP_SYSREQ_ERR + ( E_MCP_SYSREQ_OFFSET * E_MCP_SYSREQ_SOFT_SHUTDOWN )

};

/**
 * Typedef of states of MCP system requests made in datum for each process in the SDB. 
 * These are the definitions that should be submitted and retrieved to/from the
 * SDB to implement system requests.
 * Mcp.h.
 * @see #eMcpSysRequest_e
 */
typedef enum eMcpSysRequest_e eMcpSysRequest_t;

/**
 * SDB command set (services offered to other programs) 
 * Sdb.h.
 */
enum eSdbCommands_e
{
	/*E_SDB_HEARTBEAT = E_MCP_HEARTBEAT,*/  /* Heartbeat command */
	/*E_SDB_SHUTDOWN  = E_MCP_SHUTDOWN,*/   /* Shutdown command */
	/*E_SDB_SAFESTATE = E_MCP_SAFESTATE,*/  /* Safestate command */
	/*E_SDB_ACTIVATE  = E_MCP_ACTIVATE,*/   /* Activate command */

   /* next entry was set equal to SERVICE_START(SDB) (TtlSystem.h defines SERVICE_START/SDB */
   E_SDB_PURGE = ((0x000d) << 16),   /* Cause the SDB to delete old entries */
   E_SDB_SUBMIT_1,           /* Submit data for storage */
   E_SDB_RETRIEVE_1,         /* Request data from the database */
   E_SDB_SUBMIT_N,           /* Submit block data for storage */
   E_SDB_RETRIEVE_N,         /* Request block data from the database */
   E_SDB_LISTSOURCES,        /* Request source IDs for all submitted data */
   E_SDB_LISTDATA,           /* Request data IDs for a particular source */
   E_SDB_COUNTSOURCES,       /* Reuqest number of different sources in SDB */
   E_SDB_COUNTDATA,          /* Request number of data definitions for a Src */
   E_SDB_COUNTMSRMENTS,      /* Request number of measurements for a Src:Dat */
   E_SDB_RETRIEVE_1R,        /* Request data from the database (robust mode) */
   E_SDB_RETRIEVE_F,         /* Request data from storage file (robust mode) */
   E_SDB_SUBMIT_1P,          /* Post data for submission - no response */
   E_SDB_CLEAR_S,            /* Clear data with a particular SourceID */
   E_SDB_CLEAR_1,            /* Clear specific data submissions */
   E_SDB_RETRIEVE_L,         /* Request latest data from file (robust mode) */
   E_SDB_COMMAND_EOL,        /* End of enumerated list of commands */
   E_SDB_COMMAND_MAX_VALUE = INT_MAX   /* Req'd to force size to 4 bytes */
};

/**
 * Typedef of SDB command set (services offered to other programs).
 * Sdb.h.
 * @see #eSdbCommands_e
 */
typedef enum eSdbCommands_e eSdbCommands_t;

/**
 * SDB units encodings.
 * Sdb.h
 */
enum eSdbUnit_e
{
   E_SDB_INVALID_UNITS = 0,  /* Trap for non-specified units (=start of list) */

   E_SDB_UNSUPPORTED_UNITS,  /* Indicate datum not supported at this time */

   /*
   ** 32-bit signed integer units
   */

   E_SDB_NO_UNITS,           /* Unit-free measurement (counts, etc.) */
   E_SDB_MGAIN,              /* Gain factor x 1000 */
   E_SDB_MAS_UNITS,          /* Milliarcseconds */
   E_SDB_MKELVIN_UNITS,      /* Millikelvin */
   E_SDB_MCELSIUS_UNITS,     /* Millicelsius */
   E_SDB_MVOLT_UNITS,        /* Millivolts */
   E_SDB_MAMP_UNITS,         /* Milliamperes */
   E_SDB_UVOLT_UNITS,        /* Microvolts */
   E_SDB_UAMP_UNITS,         /* Microamperes */
   E_SDB_MAMP_PER_VOLT_UNITS,/* Milliamperes per volt */
   E_SDB_BITS_PER_VOLT_UNITS,/* Bits per volt */
   E_SDB_BITS_UNITS,         /* Bit field */
   E_SDB_MINUTES_UNITS,      /* Minutes */
   E_SDB_SEC_UNITS,          /* Seconds */
   E_SDB_MSEC_UNITS,         /* Milliseconds */
   E_SDB_USEC_UNITS,         /* Microseconds */
   E_SDB_NSEC_UNITS,         /* Nanoseconds */
   E_SDB_MASPERMS_UNITS,     /* Milliarcseconds per millisecond */
   E_SDB_MASPERMSPERMS_UNITS,/* Milliarcseconds per millisecond squared */
   E_SDB_MNEWTONMETRES_UNITS,/* Millinewton metres, Torque */
   E_SDB_MNM_PER_AMP_UNITS,  /* Millinewton metres per ampere */
   E_SDB_PROCSTATE_UNITS,    /* TTL process state */
   E_SDB_TRUEFALSE_UNITS,    /* Boolean True/False */
   E_SDB_ONOFF_UNITS,        /* Boolean On/Off */
   E_SDB_YESNO_UNITS,        /* Boolean Yes/No */
   E_SDB_MPERCENT_UNITS,     /* Milli percentage (1000 = 1%) */
   E_SDB_HERTZ_UNITS,        /* Hertz for frequency */
   E_SDB_SECPERYR_UNITS,     /* Seconds per year (eg proper motion of a star) */
   E_SDB_MYR_UNITS,          /* Milliyear */
   E_SDB_METREPERSEC_UNITS,  /* Metres per second */
   E_SDB_MSEC_PER_YR,        /* Milliseconds per year */
   E_SDB_MAS_PER_YR,         /* Milliarcseconds per year */
   E_SDB_MSEC_PER_DAY,       /* Milliseconds per day */
   E_SDB_MAS_PER_DAY,        /* Milliarcseconds per day */
   E_SDB_SHUTTER_STATE_UNITS,/* State of an enclosure mechanism */
   E_SDB_CIL_ID_UNITS,       /* CIL identifier */
   E_SDB_AUTH_STATE_UNITS,   /* Requested/granted authorisation state */
   E_SDB_BYTES_UNITS,        /* Bytes */
   E_SDB_KBYTES_UNITS,       /* Kilobytes (1024 bytes) */
   E_SDB_MBYTES_UNITS,       /* Megabytes (1024 kilobytes) */
   E_SDB_RPM_UNITS,          /* Revolutions per minute */
   E_SDB_SYSREQ_UNITS,       /* System requests made to MCP */
   E_SDB_MM_PER_SEC_UNITS,   /* Milli-metres per second */
   E_SDB_UHERTZ_UNITS,       /* Micro-hertz for frequency */
   E_SDB_MBAR_UNITS,         /* Milli-bar for (atmospheric) pressure */
   E_SDB_OID_UNITS,          /* Oid contains an Oid as a value, ie CFL */
   E_SDB_INDEX_UNITS,        /* Table or Array Offset */
   E_SDB_BAR_UNITS,          /* Bar for PLC fluid pressures */
   E_SDB_DCELSIUS_UNITS,     /* Deci-celcius for PLC temperatures */
   E_SDB_LTR_PER_MIN_UNITS,  /* Litres per minute for PLC flow-rates */
   E_SDB_MVERSION_UNITS,     /* Milli-version, eg v1.23 is 1230 */
   E_SDB_METRES_UNITS,       /* Metres */
   E_SDB_MILLIMETRES_UNITS,  /* Milli-Metres */
   E_SDB_MICRONS_UNITS,      /* Microns */
   E_SDB_NANOMETRES_UNITS,   /* Nano-metres */
   E_SDB_HOURS_UNITS,        /* Hours */
   E_SDB_MILLIDEGREES_UNITS, /* Milli-degrees */
   E_SDB_ARCSEC_UNITS,       /* Arc-seconds */
   E_SDB_SSE_STRING_UNITS,   /* SSE encoded string */
   E_SDB_NM_PER_CEL_UNITS,   /* Nanometres per degree celsius */
   E_SDB_EPT_DATA_UNITS,     /* EPT Data State Units */
   E_SDB_SPT_DATA_UNITS,     /* SPT Data State Units */
   E_SDB_IET_DATA_UNITS,     /* IET Data State Units */
   E_SDB_PACKAGE_ID_UNITS,   /* TTL Package ID Units */
   E_SDB_TELESCOPE_UNITS,    /* Identification of a telescope, or group of */
   E_SDB_LITRES_UNITS,       /* Litres */
   E_SDB_MILLILITRES_UNITS,  /* Milli-litres */
   E_SDB_MLTR_PER_MIN_UNITS, /* Milli-litres per minute */
   E_SDB_MPIXEL_UNITS,       /* Milli-pixels */
   E_SDB_MSTARMAG_UNITS,     /* Milli-star magnitudes */
   E_SDB_UAS_UNITS,          /* Microarcseconds */
   E_SDB_MPERCENT_RH_UNITS,  /* Milli-percent relative humidity */
                             /* Note that PLC device units must be contiguous */
   E_SDB_PLC_BOL_UNITS,      /* PLC device units beginning of list */
   E_SDB_PLC_2_STATE_UNITS,  /* PLC 2-state device */
   E_SDB_PLC_4_STATE_UNITS,  /* PLC 4-state device */
   E_SDB_PLC_LIMIT_UNITS,    /* PLC Limit device */
   E_SDB_PLC_SENSOR_UNITS,   /* PLC Sensor device */
   E_SDB_PLC_MOTOR_UNITS,    /* PLC Motor device */
   E_SDB_PLC_ACTUATOR_UNITS, /* PLC Actuator device */
   E_SDB_PLC_STATUS_UNITS,   /* PLC status */
   E_SDB_PLC_VERSION_UNITS,  /* PLC version */
   E_SDB_PLC_EOL_UNITS,      /* PLC device units end of list */

   E_SDB_MILLIDEG_PER_VOLT,  /* Milli-degrees per volt */

   E_SDB_MW_PERMPERM_UNITS,  /* Milli-watts per metre squared */
   E_SDB_AGSTATE_UNITS,      /* Autoguider System state units */

   E_SDB_NOMORE_32BIT_UNITS, /* No more 32-bit units in list */

   /*
   ** 64-bit signed integer units
   ** (must be paired in application to be valid)
   */

   E_SDB_MSW_NO_UNITS        /* MSW of signed 64-bit unit free measurement */
                  = 0x10000,
   E_SDB_LSW_NO_UNITS,       /* LSW of signed 64-bit unit free measurement */
   E_SDB_MSW_TTS_UNITS,      /* TTL TimeStamp Most Significant Word (sec) */
   E_SDB_LSW_TTS_UNITS,      /* TTL TimeStamp Least Significant Word (nsec) */
   E_SDB_MSW_RAWENC_UNITS,   /* Most Sig. Word of raw encoder counts */
   E_SDB_LSW_RAWENC_UNITS,   /* Least Sig. Word of raw encoder counts */

   /*
   ** Identifying the use of a composite unit. E.g. any unit that is
   ** stored as a single quantity, but is broken down for transmission
   ** to the SDB.
   */

   E_SDB_TTS_UNITS,          /* TTL TimeStamp structure */
   E_SDB_RAWENC_UNITS,       /* Representation of 48-bit raw encoder counts */

   /*
   ** End of list marker
   ** (never to be used as a units for the applicaion)
   */

   E_SDB_NOMORE_UNITS,       /* End of enumerated list of units */
   E_SDB_UNITS_MAX_VALUE = INT_MAX     /* Req'd to force size to 4 bytes */

};

/**
 * Typedef of SDB units encodings.
 * Sdb.h
 * @see #eSdbUnit_e
 */
typedef enum eSdbUnit_e eSdbUnit_t;

/**
 * Storage structures for submitted/requested SDB data. These are for use
 * by third party applications that wish to access the SDB.
 * Sdb.h.
 */
typedef struct
{             /* -- Measurement (time-value pair) -- */
   eTtlTime_t  TimeStamp;    /* Timestamp associated with value */
   Int32_t     Value;        /* Actual value, encoded as per "Units" */
} eSdbMsrment_t;

/**
 * Storage structures for submitted/requested SDB data. These are for use
 * by third party applications that wish to access the SDB.
 * Sdb.h.
 */
typedef struct
{             /* -- Single datum -- */
   Int32_t     SourceId;     /* Source (parent) ID number */
   Int32_t     DatumId;      /* Data element ID number */
   Int32_t     Units;        /* Measurement units of value (enum) */
   eSdbMsrment_t Msrment;    /* Time-value pair */
} eSdbDatum_t;

/**
 * Structure for SDB submissions .
 * AgsPrivate.h.
 * @see #eAgsDataId_t
 * @see #eSdbDatum_t
 */
typedef struct iAgsSdbDat_e
{
	Uint32_t NumElts;                   /* Element count for submission to SDB */
	/* Datums to be submitted to the SDB */
	eSdbDatum_t Datum[ D_AGS_DATAID_EOL ]; 
} iAgsSdbData_t;

/**
 * Structure defining packet contents of a AGS SDB CIL command packet.
 * All the integers should be sent over UDP in network byte order (htonl).
 * Packet contents are derived from "Generic 2.0m Telescope, Autoguider to TCS Interface Control Document,
 * Version 0.01, 6th October 2005", and AgsPrivate.h.
 * NB Assumes sizeof(int) == 32, should perhaps replace with Int32_t.
 * <dl>
 * <dt>Cil_Base</dt> <dd>NGATCil_Cil_Packet_Struct, base Cil details.</dd>
 * <dt>Datums</dt> <dd>Of type iAgsSdbDat_e</dd>
 * <dt>Param2</dt> <dd></dd>
 * </dl>
 * @see ngatcil_cil.html#NGATCil_Cil_Packet_Struct
 * @see #iAgsSdbDat_e
 */
struct NGATCil_AGS_SDB_Packet_Struct
{
	struct NGATCil_Cil_Packet_Struct Cil_Base;
	struct iAgsSdbDat_e Datums;
};

/**
 * AgsPrivate.h.
 */
typedef struct iAgsOidTable_s
{
   Int32_t     Oid;             /* Identification index - same as a DatumId */
   eSdbUnit_t  Units;           /* Units associated with the object  */
   Int32_t     Value;           /* Value to be submitted  */
   Bool_t      Changed;         /* If Changed submit to SBD.  */
   eTtlTime_t  TimeStamp;       /* Time of last change of this value.. */
   Int32_t (*CvtFn)( Int32_t ); /* Pointer to conversion function */
} iAgsOidTable_t;

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ngatcil_ags_sdb.c,v 1.2 2006-08-29 14:07:57 cjm Exp $";

/**
 * Internal data set of AGS OID's that can me modified and sent to the SDB.
 * AgsPrivate.h.
 */
static iAgsOidTable_t iAgsOidTable[] =
{
/*__Oid_________________Units_____________________Value_______________Changed__TimeStamp__CvtFn*/

{D_AGS_DATAID_BOL     , E_SDB_INVALID_UNITS     , E_AGS_UNKNOWN     , FALSE  , {0,0},     NULL },
{D_AGS_PROC_STATE     , E_SDB_PROCSTATE_UNITS   , SYS_INVALID_STATE , FALSE  , {0,0},     NULL },
{D_AGS_AUTH_STATE     , E_SDB_AUTH_STATE_UNITS  , E_MCP_AUTH_NONE   , FALSE  , {0,0},     NULL },
{D_AGS_SYS_REQUEST    , E_SDB_SYSREQ_UNITS      , E_MCP_SYSREQ_NONE , FALSE  , {0,0},     NULL },
{D_AGS_APP_VERSION    , E_SDB_MVERSION_UNITS    , 0                 , FALSE  , {0,0},     NULL },
{D_AGS_AGSTATE        , E_SDB_NO_UNITS          , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_WINDOW_TLX     , E_SDB_MPIXEL_UNITS      , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_WINDOW_TLY     , E_SDB_MPIXEL_UNITS      , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_WINDOW_BRX     , E_SDB_MPIXEL_UNITS      , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_WINDOW_BRY     , E_SDB_MPIXEL_UNITS      , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_INTTIME        , E_SDB_MSEC_UNITS        , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_FRAMESKIP      , E_SDB_NO_UNITS          , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_GUIDEMAG       , E_SDB_MSTARMAG_UNITS    , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_CENTROIDX      , E_SDB_MPIXEL_UNITS      , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_CENTROIDY      , E_SDB_MPIXEL_UNITS      , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_FWHM           , E_SDB_MPIXEL_UNITS      , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_PEAKPIXEL      , E_SDB_NO_UNITS          , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_AGTEMP         , E_SDB_MCELSIUS_UNITS    , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_AGCASETEMP     , E_SDB_MCELSIUS_UNITS    , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },
{D_AGS_AGPERCPOW      , E_SDB_MPERCENT_UNITS    , E_AGS_UNKNOWN     ,  TRUE  , {0,0},     NULL },

{D_AGS_DATAID_EOL     , E_SDB_INVALID_UNITS     , E_AGS_UNKNOWN     , FALSE  , {0,0},     NULL }
};

/**
 * UDP CIL port number to send packet to.
 * @see #NGATCIL_AGS_SDB_CIL_PORT_DEFAULT
 */
static int Remote_Port_Number = NGATCIL_AGS_SDB_CIL_PORT_DEFAULT;
/**
 * Hostname to send packet to.
 * @see #NGATCIL_AGS_SDB_MCC_DEFAULT
 * @see #HOSTNAME_LENGTH
 */
static char Remote_Hostname[HOSTNAME_LENGTH] = NGATCIL_AGS_SDB_MCC_DEFAULT;
/**
 * CIL packet sequence number.
 */
static int Sequence_Number = 0;

/* internal function declarations */
static int AGS_SDB_Packet_Send_To(int socket_id,char *hostname,int port_number,
			       struct NGATCil_AGS_SDB_Packet_Struct packet,int packet_length);
static int AGS_SDB_Packet_To_Network_Byte_Order(struct NGATCil_AGS_SDB_Packet_Struct *sdb_packet,int packet_length);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise. Set all timestamps in iAgsOidTable.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #TTL_TIMESTAMP_OFFSET
 * @see #eTtlTime_t
 * @see #iAgsOidTable
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_AGS_SDB
 */
int NGATCil_AGS_SDB_Initialise(void)
{
	struct timespec current_time;
	eTtlTime_t time_now;
	int oid_index;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"NGATCil_AGS_SDB_Initialise:started.");
#endif
	clock_gettime(CLOCK_REALTIME,&current_time);
	time_now.t_sec = current_time.tv_sec-TTL_TIMESTAMP_OFFSET;
	time_now.t_nsec = current_time.tv_nsec;
	for ( oid_index = D_AGS_DATAID_BOL; oid_index < D_AGS_DATAID_EOL; oid_index++ )
	{
		iAgsOidTable[oid_index].TimeStamp = time_now;
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"NGATCil_AGS_SDB_Initialise:finished.");
#endif
	return TRUE;
}

/**
 * Set the name and port number of the remote connection to contact when sending SDB entries.
 * @param hostname A string respresenting the host name.
 * @param port_number The port number.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #HOSTNAME_LENGTH
 * @see #Remote_Hostname
 * @see #Remote_Port_Number
 * @see #NGATCIL_AGS_SDB_MCC_DEFAULT
 * @see #NGATCIL_AGS_SDB_CIL_PORT_DEFAULT
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_AGS_SDB
 */
int NGATCil_AGS_SDB_Remote_Host_Set(char *hostname,int port_number)
{
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"NGATCil_AGS_SDB_Remote_Host_Set:started.");
#endif
	if(hostname == NULL)
	{
		NGATCil_General_Error_Number = 401;
		sprintf(NGATCil_General_Error_String,"NGATCil_AGS_SDB_Remote_Host_Set: Hostname was NULL.");
		return FALSE;
	}
	if(strlen(hostname) >= HOSTNAME_LENGTH)
	{
		NGATCil_General_Error_Number = 402;
		sprintf(NGATCil_General_Error_String,
			"NGATCil_AGS_SDB_Remote_Host_Set: Hostname was too long (%d vs %d.",
			strlen(hostname),HOSTNAME_LENGTH);
		return FALSE;
	}
	strcpy(Remote_Hostname,hostname);
	Remote_Port_Number = port_number;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"NGATCil_AGS_SDB_Remote_Host_Set:finished.");
#endif
	return TRUE;
}
/**
 * Send an SDB packet containing latest status.
 * @param socket_id The file descriptor of an open socket to send the packet on.
 * @param hostname A string representing the remote host to send the status to (mcc).
 * @param port_number The remote port number to send the packet to (NGATCIL_AGS_SDB_CIL_PORT_DEFAULT).
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #AGS_SDB_Packet_To_Network_Byte_Order
 * @see #AGS_SDB_Packet_Send_To
 * @see #Remote_Hostname
 * @see #Remote_Port_Number
 * @see #NGATCIL_AGS_SDB_CIL_PORT_DEFAULT
 * @see #TTL_TIMESTAMP_OFFSET
 * @see #iAgsOidTable
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_AGS_SDB
 */
int NGATCil_AGS_SDB_Status_Send(int socket_id)
{
	struct NGATCil_AGS_SDB_Packet_Struct sdb_packet;
	struct timespec current_time;
	int retval,oid_index,sdb_index,data_length,packet_length;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"NGATCil_AGS_SDB_Status_Send:started.");
#endif
	sdb_index = 0;
	for ( oid_index = I_AGS_FIRST_DATUMID; oid_index <= I_AGS_FINAL_DATUMID; oid_index++ )
	{
		if(iAgsOidTable[oid_index].Changed == TRUE)
		{
#if NGATCIL_DEBUG > 5
			NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,
						   "NGATCil_AGS_SDB_Status_Send:Found changed OID %d at index %d.",
						   iAgsOidTable[oid_index].Oid,oid_index);
#endif
			sdb_packet.Datums.Datum[sdb_index].SourceId          = E_CIL_AGS;
			sdb_packet.Datums.Datum[sdb_index].DatumId           = iAgsOidTable[oid_index].Oid;
			sdb_packet.Datums.Datum[sdb_index].Msrment.TimeStamp = iAgsOidTable[oid_index].TimeStamp;
			sdb_packet.Datums.Datum[sdb_index].Msrment.Value     = iAgsOidTable[oid_index].Value;
			sdb_packet.Datums.Datum[sdb_index].Units             = iAgsOidTable[oid_index].Units;
			sdb_index++;
		}
	}
	if(sdb_index > 0)
	{
		sdb_packet.Cil_Base.Source_Id = E_CIL_AGS;
		sdb_packet.Cil_Base.Dest_Id = E_CIL_SDB;
		sdb_packet.Cil_Base.Class = E_CIL_CMD_CLASS;
		sdb_packet.Cil_Base.Service = E_SDB_SUBMIT_1;
		sdb_packet.Cil_Base.Seq_Num = Sequence_Number++;
		clock_gettime(CLOCK_REALTIME,&current_time);
		sdb_packet.Cil_Base.Timestamp_Seconds = current_time.tv_sec-TTL_TIMESTAMP_OFFSET;
		sdb_packet.Cil_Base.Timestamp_Nanoseconds = current_time.tv_nsec;
		sdb_packet.Datums.NumElts = (Uint32_t)sdb_index;
#if NGATCIL_DEBUG > 5
		NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,
					   "NGATCil_AGS_SDB_Status_Send:Found %d changed OIDs.",sdb_index);
#endif
		/* length of Datum data to submit */
		data_length = sizeof(Int32_t) + (sdb_index * sizeof(eSdbDatum_t));
#if NGATCIL_DEBUG > 5
		NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,
					   "NGATCil_AGS_SDB_Status_Send:Data length %d.",data_length);
#endif
		/* packet has 7 int header (CilPrivate.h:I_CIL_HDRBLK_SIZE  28) */
		packet_length = data_length + (7 * sizeof(int)); 
#if NGATCIL_DEBUG > 5
		NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,
					   "NGATCil_AGS_SDB_Status_Send:Packet length %d.",packet_length);
#endif
		/* change to network byte order */
		if(!AGS_SDB_Packet_To_Network_Byte_Order(&sdb_packet,packet_length))
			return FALSE;
		/* submit to sdb */
		if(!AGS_SDB_Packet_Send_To(socket_id,Remote_Hostname,Remote_Port_Number,sdb_packet,packet_length))
			return FALSE;
	}
	/* clear changed values if SDB has been updated */
	for ( oid_index = I_AGS_FIRST_DATUMID; oid_index <= I_AGS_FINAL_DATUMID; oid_index++ )
	{
		if(iAgsOidTable[oid_index].Changed == TRUE)
		{
			iAgsOidTable[oid_index].Changed = FALSE;
		}
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"NGATCil_AGS_SDB_Status_Send:finished.");
#endif
	return TRUE;
}

/**
 * Set a datum value.
 * @param datum_id The datum to set, of type eAgsDataId_t.
 * @param value The value as an integer.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see #iAgsOidTable
 * @see ngatcil_general.html#NGATCil_General_Error_Number
 * @see ngatcil_general.html#NGATCil_General_Error_String
 * @see ngatcil_general.html#NGATCil_General_Log
 * @see ngatcil_general.html#NGATCIL_GENERAL_LOG_BIT_AGS_SDB
 */
int NGATCil_AGS_SDB_Value_Set(eAgsDataId_t datum_id,int value)
{
	struct timespec current_time;
	eTtlTime_t time_now;
	Int32_t old_value;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,
				   "NGATCil_AGS_SDB_Value_Set:started: Setting OID %d to %d.",datum_id,value);
#endif
	clock_gettime(CLOCK_REALTIME,&current_time);
	time_now.t_sec = current_time.tv_sec-TTL_TIMESTAMP_OFFSET;
	time_now.t_nsec = current_time.tv_nsec;
	if((datum_id <= D_AGS_DATAID_BOL) || (datum_id >= D_AGS_DATAID_EOL))
	{
		NGATCil_General_Error_Number = 400;
		sprintf(NGATCil_General_Error_String,"NGATCil_AGS_SDB_Value_Set: Illegal datum %d (%d..%d).",datum_id,
			D_AGS_DATAID_BOL,D_AGS_DATAID_EOL);
		return FALSE;
	}
	old_value = iAgsOidTable[datum_id].Value;
	/* If the value is being changed set the changed flag. */
	if (iAgsOidTable[datum_id].Value != value) 
	{
		iAgsOidTable[datum_id].Changed = TRUE;
	}
	else if((datum_id == D_AGS_GUIDEMAG) ||
		(datum_id == D_AGS_CENTROIDX) || (datum_id == D_AGS_CENTROIDY) ||
		(datum_id == D_AGS_FWHM) || (datum_id == D_AGS_INTTIME))
	{
		iAgsOidTable[datum_id].Changed = TRUE;
	}
	iAgsOidTable[datum_id].Value = value;
	iAgsOidTable[datum_id].TimeStamp = time_now;
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"NGATCil_AGS_SDB_Value_Set:finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Convert a AGS SDB packet to network byte order.
 * @param sdb_packet The packet, the contents of which on entry are in host byte order. On exit the
 *        contents are in network byte order.
 * @param packet_length The length of packet to be converted (only filled in datums need conversion). Not
 *        used atm, we use the Datums.NumElts field instead.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 */
static int AGS_SDB_Packet_To_Network_Byte_Order(struct NGATCil_AGS_SDB_Packet_Struct *sdb_packet,int packet_length)
{
	int datum_count,i;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"AGS_SDB_Packet_To_Network_Byte_Order:started.");
#endif
	sdb_packet->Cil_Base.Source_Id             = htonl(sdb_packet->Cil_Base.Source_Id);
	sdb_packet->Cil_Base.Dest_Id               = htonl(sdb_packet->Cil_Base.Dest_Id);
	sdb_packet->Cil_Base.Class                 = htonl(sdb_packet->Cil_Base.Class);
	sdb_packet->Cil_Base.Service               = htonl(sdb_packet->Cil_Base.Service);
	sdb_packet->Cil_Base.Seq_Num               = htonl(sdb_packet->Cil_Base.Seq_Num);
	sdb_packet->Cil_Base.Timestamp_Seconds     = htonl(sdb_packet->Cil_Base.Timestamp_Seconds);
	sdb_packet->Cil_Base.Timestamp_Nanoseconds = htonl(sdb_packet->Cil_Base.Timestamp_Nanoseconds);
	datum_count                                = sdb_packet->Datums.NumElts;
	sdb_packet->Datums.NumElts                 = htonl(sdb_packet->Datums.NumElts);
	for(i=0;i<datum_count;i++)
	{
		sdb_packet->Datums.Datum[i].SourceId = htonl(sdb_packet->Datums.Datum[i].SourceId);
		sdb_packet->Datums.Datum[i].DatumId = htonl(sdb_packet->Datums.Datum[i].DatumId);
		sdb_packet->Datums.Datum[i].Units = htonl(sdb_packet->Datums.Datum[i].Units);
		sdb_packet->Datums.Datum[i].Msrment.TimeStamp.t_sec = htonl(sdb_packet->Datums.Datum[i].
									    Msrment.TimeStamp.t_sec);
		sdb_packet->Datums.Datum[i].Msrment.TimeStamp.t_nsec = htonl(sdb_packet->Datums.Datum[i].
									     Msrment.TimeStamp.t_nsec);
		sdb_packet->Datums.Datum[i].Msrment.Value = htonl(sdb_packet->Datums.Datum[i].Msrment.Value);
	}
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"AGS_SDB_Packet_To_Network_Byte_Order:finished.");
#endif
	return TRUE;
}

/**
 * Send an AGS SDB CIL packet. 
 * @param socket_id The socket file descriptor to use.
 * @param hostname The hostname of the host to send the packet to.
 * @param port_number The port number of the host to send the packet to.
 * @param packet The packet to send. The contents should have been put into network byte order
 *        (NGATCil_AGS_SDB_Status_Send does this automatically).
 * @param packet_length How much of the packet to send.
 * @return The routine returns TRUE on success and FALSE on failure. If the routine failed,
 *      NGATCil_General_Error_Number and NGATCil_General_Error_String should be set.
 * @see ngatcil_udp_raw.html#NGATCil_UDP_Raw_Send_To
 */
static int AGS_SDB_Packet_Send_To(int socket_id,char *hostname,int port_number,
			       struct NGATCil_AGS_SDB_Packet_Struct packet,int packet_length)
{
	int retval;

#if NGATCIL_DEBUG > 1
	NGATCil_General_Log_Format(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,
		     "AGS_SDB_Packet_Send_To:started (socket_id=%d,hostname=%s,port_number=%d,Packet_length=%d).",
				   socket_id,hostname,port_number,packet_length);
#endif
	retval = NGATCil_UDP_Raw_Send_To(socket_id,hostname,port_number,(void*)&packet,packet_length);
#if NGATCIL_DEBUG > 1
	NGATCil_General_Log(NGATCIL_GENERAL_LOG_BIT_AGS_SDB,"AGS_SDB_Packet_Send_To:finished.");
#endif
	return retval;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/07/20 15:15:15  cjm
** Initial revision
**
*/
