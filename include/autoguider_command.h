/* autoguider_command.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_command.h,v 1.5 2006-08-29 14:07:04 cjm Exp $
*/
#ifndef AUTOGUIDER_COMMAND_H
#define AUTOGUIDER_COMMAND_H

/* enum */
/**
 * Enum describing type of "autoguider on" command.:
 * <ul>
 * <li>COMMAND_AG_ON_TYPE_BRIGHTEST
 * <li>COMMAND_AG_ON_TYPE_PIXEL
 * <li>COMMAND_AG_ON_TYPE_RANK
 * </ul>
 */
enum COMMAND_AG_ON_TYPE
{
	COMMAND_AG_ON_TYPE_BRIGHTEST,
	COMMAND_AG_ON_TYPE_PIXEL,
	COMMAND_AG_ON_TYPE_RANK
};

extern int Autoguider_Command_Abort(char *command_string,char **reply_string);
extern int Autoguider_Command_Agstate(char *command_string,char **reply_string);
extern int Autoguider_Command_Autoguide(char *command_string,char **reply_string);
extern int Autoguider_Command_Autoguide_On(enum COMMAND_AG_ON_TYPE on_type,float pixel_x,float pixel_y,int rank);
extern int Autoguider_Command_Config_Load(char *command_string,char **reply_string);
extern int Autoguider_Command_Object(char *command_string,char **reply_string);
extern int Autoguider_Command_Status(char *command_string,char **reply_string);
extern int Autoguider_Command_Temperature(char *command_string,char **reply_string);
extern int Autoguider_Command_Expose(char *command_string,char **reply_string);
extern int Autoguider_Command_Field(char *command_string,char **reply_string);
extern int Autoguider_Command_Guide(char *command_string,char **reply_string);
extern int Autoguider_Command_Get_Fits(char *command_string,void **buffer_ptr,size_t *buffer_length);
extern int Autoguider_Command_Log_Level(char *command_string,char **reply_string);

/*
** $Log: not supported by cvs2svn $
** Revision 1.4  2006/06/27 20:44:43  cjm
** Added Autoguider_Command_Autoguide_On.
** Added COMMAND_AG_ON_TYPE enum.
**
** Revision 1.3  2006/06/20 13:10:38  cjm
** Added Autoguider_Command_Autoguide.
**
** Revision 1.2  2006/06/12 19:24:55  cjm
** Added Autoguider_Command_Log_Level.
**
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
