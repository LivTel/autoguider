/* autoguider_command.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_command.h,v 1.2 2006-06-12 19:24:55 cjm Exp $
*/
#ifndef AUTOGUIDER_COMMAND_H
#define AUTOGUIDER_COMMAND_H

extern int Autoguider_Command_Abort(char *command_string,char **reply_string);
extern int Autoguider_Command_Config_Load(char *command_string,char **reply_string);
extern int Autoguider_Command_Status(char *command_string,char **reply_string);
extern int Autoguider_Command_Temperature(char *command_string,char **reply_string);
extern int Autoguider_Command_Expose(char *command_string,char **reply_string);
extern int Autoguider_Command_Field(char *command_string,char **reply_string);
extern int Autoguider_Command_Guide(char *command_string,char **reply_string);
extern int Autoguider_Command_Get_Fits(char *command_string,void **buffer_ptr,size_t *buffer_length);
extern int Autoguider_Command_Log_Level(char *command_string,char **reply_string);

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
