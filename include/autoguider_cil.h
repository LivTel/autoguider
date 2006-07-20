/* autoguider_cil.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_cil.h,v 1.3 2006-07-20 15:12:02 cjm Exp $
*/
#ifndef AUTOGUIDER_CIL_H
#define AUTOGUIDER_CIL_H
/* for eAgsState_t etc... */
#include "ngatcil_ags_sdb.h"

/* external functions */
extern int Autoguider_CIL_Server_Initialise(void);
extern int Autoguider_CIL_Server_Start(void);
extern int Autoguider_CIL_Server_Stop(void);
extern int Autoguider_CIL_Guide_Packet_Open(void);
extern int Autoguider_CIL_Guide_Packet_Send(float x_pos,float y_pos,int terminating,int unreliable,float timecode_secs,
				     char status_char);
extern int Autoguider_CIL_Guide_Packet_Close(void);
extern int Autoguider_CIL_Guide_Packet_Send_Set(int on);
extern int Autoguider_CIL_Guide_Packet_Send_Get(void);

extern int Autoguider_CIL_SDB_Packet_Open(void);
extern int Autoguider_CIL_SDB_Packet_State_Set(eAgsState_t state);
extern int Autoguider_CIL_SDB_Packet_Exp_Time_Set(int ms);
extern int Autoguider_CIL_SDB_Packet_Centroid_Set(float cx,float cy,float fwhm,float mag);
extern int Autoguider_CIL_SDB_Packet_Send(void);
extern int Autoguider_CIL_SDB_Packet_Close(void);
extern int Autoguider_CIL_SDB_Packet_Send_Set(int on);
extern int Autoguider_CIL_SDB_Packet_Send_Get(void);

/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2006/06/20 13:10:38  cjm
** Added Autoguider_CIL_Guide_Packet_Send_Set/Get.
**
** Revision 1.1  2006/06/12 19:21:13  cjm
** Initial revision
**
*/
#endif
