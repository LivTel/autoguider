/* autoguider_fits_header.c
** Autoguider fits header list handling routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_fits_header.c,v 1.1 2006-07-16 20:13:54 cjm Exp $
*/
/**
 * Routines to look after lists of FITS headers to go into images.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fitsio.h"

#include "autoguider_fits_header.h"
#include "autoguider_general.h"

/* hash defines */
/**
 * Maximum length of FITS header keyword (can go from column 1 to column 8 inclusive), plus a '\0' terminator.
 */
#define FITS_HEADER_KEYWORD_STRING_LENGTH (9)
/**
 * Maximum length of FITS header string value (can go from column 11 to column 80 inclusive), plus a '\0' terminator.
 */
#define FITS_HEADER_VALUE_STRING_LENGTH   (71)
/**
 * Maximum length of FITS header comment (can go from column 10 to column 80 inclusive), plus a '\0' terminator.
 */
#define FITS_HEADER_COMMENT_STRING_LENGTH (72) 

/* data types */
/**
 * Enumeration describing the type of data contained in a FITS header value.
 * <dl>
 * <dt>FITS_HEADER_TYPE_STRING</dt> <dd>String</dd>
 * <dt>FITS_HEADER_TYPE_INTEGER</dt> <dd>Integer</dd>
 * <dt>FITS_HEADER_TYPE_FLOAT</dt> <dd>Floating point (double).</dd>
 * <dt>FITS_HEADER_TYPE_LOGICAL</dt> <dd>Boolean (integer having value 1 (TRUE) or 0 (FALSE).</dd>
 * </dl>
 */
enum Fits_Header_Type_Enum
{
	FITS_HEADER_TYPE_STRING,
	FITS_HEADER_TYPE_INTEGER,
	FITS_HEADER_TYPE_FLOAT,
	FITS_HEADER_TYPE_LOGICAL
};

/**
 * Structure containing information on a FITS header entry.
 * <dl>
 * <dt>Keyword</dt> <dd>Keyword string of length FITS_HEADER_KEYWORD_STRING_LENGTH.</dd>
 * <dt>Type</dt> <dd>Which value in the value union to use.</dd>
 * <dt>Value</dt> <dd>Union containing the following elements:
 *                    <ul>
 *                    <li>String (of length FITS_HEADER_VALUE_STRING_LENGTH).
 *                    <li>Int
 *                    <li>Float (of type double).
 *                    <li>Boolean (an integer, should be 0 (FALSE) or 1 (TRUE)).
 *                    </ul>
 *                </dd>
 * <dt>Comment</dt> <dd>String of length FITS_HEADER_COMMENT_STRING_LENGTH.</dd>
 * </dl>
 * @see #FITS_HEADER_VALUE_STRING_LENGTH
 * @see #FITS_HEADER_KEYWORD_STRING_LENGTH
 * @see #FITS_HEADER_COMMENT_STRING_LENGTH
 */
struct Fits_Header_Card_Struct
{
	char Keyword[FITS_HEADER_KEYWORD_STRING_LENGTH]; /* columns 1-8 */
	enum Fits_Header_Type_Enum Type;
	union
	{
		char String[FITS_HEADER_VALUE_STRING_LENGTH]; /* columns 11-80 */
		int Int;
		double Float;
		int Boolean;
	} Value;
	char Comment[FITS_HEADER_COMMENT_STRING_LENGTH]; /* columns 10-80 */
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_fits_header.c,v 1.1 2006-07-16 20:13:54 cjm Exp $";

/* internal functions */
static int Fits_Header_Add_Card(struct Fits_Header_Struct *header,struct Fits_Header_Card_Struct card);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Routine to clear the fits header of cards. This does <b>not</b> free the card list memory.
 * @param header The address of a Fits_Header_Struct structure to modify.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Fits_Header_Clear(struct Fits_Header_Struct *header)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Clear:started.");
#endif
	if(header == NULL)
	{
		Autoguider_General_Error_Number = 1213;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Clear:Header was NULL.");
		return FALSE;
	}
	/* reset number of cards, without resetting allocated cards */
	header->Card_Count = 0;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Clear:finished.");
#endif
	return TRUE;
}

/**
 * Routine to delete the specified keyword from the FITS header. 
 * The list is not reallocated, Autoguider_Fits_Header_Free will eventually free the allocated memory.
 * The routine fails (returns FALSE) if a card with the specified keyword is NOT in the list.
 * @param header The address of a Fits_Header_Struct structure to modify.
 * @param keyword The keyword of the FITS header card to remove from the list.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 *
 */
int Autoguider_Fits_Header_Delete(struct Fits_Header_Struct *header,char *keyword)
{
	int found_index,index,done;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Delete:started.");
#endif
	/* check parameters */
	if(header == NULL)
	{
		Autoguider_General_Error_Number = 1214;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Delete:Header was NULL.");
		return FALSE;
	}
	if(keyword == NULL)
	{
		Autoguider_General_Error_Number = 1215;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Delete:Keyword is NULL.");
		return FALSE;
	}
	/* find keyword in header */
	found_index = 0;
	done  = FALSE;
	while((found_index < header->Card_Count) && (done == FALSE))
	{
		if(strcmp(header->Card_List[found_index].Keyword,keyword) == 0)
		{
			done = TRUE;
		}
		else
			found_index++;
	}
	/* if we failed to find the header, then error */
	if(done == FALSE)
	{
		Autoguider_General_Error_Number = 1216;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Delete:"
			"Failed to find Keyword '%s' in header of %d cards.",keyword,header->Card_Count);
		return FALSE;
	}
	/* if we found a card with this keyword, delete it. 
	** Move all cards beyond index down by one. */
	for(index=found_index; index < (header->Card_Count-1); index++)
	{
		header->Card_List[index] = header->Card_List[index+1];
	}
	/* decrement headers in this list */
	header->Card_Count--;
	/* leave memory allocated for reuse - this is deleted in Autoguider_Fits_Header_Free */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Delete:finished.");
#endif
	return TRUE;
}

/**
 * Routine to add a keyword with a string value to a FITS header.
 * @param header The address of a Fits_Header_Struct structure to modify.
 * @param keyword The keyword string, must be at least 1 character less in length than 
 *        FITS_HEADER_KEYWORD_STRING_LENGTH.
 * @param value The value string, which if longer than FITS_HEADER_VALUE_STRING_LENGTH-1 characters will be truncated.
 * @param comment The comment string, which if longer than FITS_HEADER_COMMENT_STRING_LENGTH-1 
 *        characters will be truncated. This parameter can also be NULL.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see #Fits_Header_Struct
 * @see #Fits_Header_Type_Enum
 * @see #FITS_HEADER_KEYWORD_STRING_LENGTH
 * @see #FITS_HEADER_VALUE_STRING_LENGTH
 * @see #FITS_HEADER_COMMENT_STRING_LENGTH
 * @see #Fits_Header_Add_Card
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Fits_Header_Add_String(struct Fits_Header_Struct *header,char *keyword,char *value,char *comment)
{
	struct Fits_Header_Card_Struct card;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Add_String:started.");
#endif
	if(keyword == NULL)
	{
		Autoguider_General_Error_Number = 1200;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_String:"
			"Keyword is NULL.");
		return FALSE;
	}
	if(strlen(keyword) > (FITS_HEADER_KEYWORD_STRING_LENGTH-1))
	{
		Autoguider_General_Error_Number = 1201;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_String:"
			"Keyword %s (%d) was too long.",keyword,strlen(keyword));
		return FALSE;
	}
	if(value == NULL)
	{
		Autoguider_General_Error_Number = 1202;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_String:"
			"Value string is NULL.");
		return FALSE;
	}
	strcpy(card.Keyword,keyword);
	card.Type = FITS_HEADER_TYPE_STRING;
	/* the value will be truncated to FITS_HEADER_VALUE_STRING_LENGTH-1 */
	strncpy(card.Value.String,value,FITS_HEADER_VALUE_STRING_LENGTH-1);
	card.Value.String[FITS_HEADER_VALUE_STRING_LENGTH-1] = '\0';
	/* the comment will be truncated to FITS_HEADER_COMMENT_STRING_LENGTH-1 */
	if(comment != NULL)
	{
		strncpy(card.Comment,comment,FITS_HEADER_COMMENT_STRING_LENGTH-1);
		card.Comment[FITS_HEADER_COMMENT_STRING_LENGTH-1] = '\0';
	}
	else
	{
		strcpy(card.Comment,"");
	}
	if(!Fits_Header_Add_Card(header,card))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Add_String:finished.");
#endif
	return TRUE;
}

/**
 * Routine to add a keyword with an integer value to a FITS header.
 * @param header The address of a Fits_Header_Struct structure to modify.
 * @param keyword The keyword string, must be at least 1 character less in length than 
 *        FITS_HEADER_KEYWORD_STRING_LENGTH.
 * @param value The integer value.
 * @param comment The comment string, which if longer than FITS_HEADER_COMMENT_STRING_LENGTH-1 
 *        characters will be truncated. This parameter can also be NULL.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see #Fits_Header_Struct
 * @see #Fits_Header_Type_Enum
 * @see #FITS_HEADER_KEYWORD_STRING_LENGTH
 * @see #FITS_HEADER_COMMENT_STRING_LENGTH
 * @see #Fits_Header_Add_Card
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Fits_Header_Add_Int(struct Fits_Header_Struct *header,char *keyword,int value,char *comment)
{
	struct Fits_Header_Card_Struct card;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Add_Int:started.");
#endif
	if(keyword == NULL)
	{
		Autoguider_General_Error_Number = 1203;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_Int:Keyword is NULL.");
		return FALSE;
	}
	if(strlen(keyword) > (FITS_HEADER_KEYWORD_STRING_LENGTH-1))
	{
		Autoguider_General_Error_Number = 1204;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_Int:"
			"Keyword %s (%d) was too long.",keyword,strlen(keyword));
		return FALSE;
	}
	strcpy(card.Keyword,keyword);
	card.Type = FITS_HEADER_TYPE_INTEGER;
	card.Value.Int = value;
	/* the comment will be truncated to FITS_HEADER_COMMENT_STRING_LENGTH-1 */
	if(comment != NULL)
	{
		strncpy(card.Comment,comment,FITS_HEADER_COMMENT_STRING_LENGTH-1);
		card.Comment[FITS_HEADER_COMMENT_STRING_LENGTH-1] = '\0';
	}
	else
	{
		strcpy(card.Comment,"");
	}
	if(!Fits_Header_Add_Card(header,card))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Add_Int:finished.");
#endif
	return TRUE;
}

/**
 * Routine to add a keyword with an float (double) value to a FITS header.
 * @param header The address of a Fits_Header_Struct structure to modify.
 * @param keyword The keyword string, must be at least 1 character less in length than 
 *        FITS_HEADER_KEYWORD_STRING_LENGTH.
 * @param value The float value of type double.
 * @param comment The comment string, which if longer than FITS_HEADER_COMMENT_STRING_LENGTH-1 
 *        characters will be truncated. This parameter can also be NULL.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see #Fits_Header_Struct
 * @see #Fits_Header_Type_Enum
 * @see #FITS_HEADER_KEYWORD_STRING_LENGTH
 * @see #FITS_HEADER_COMMENT_STRING_LENGTH
 * @see #Fits_Header_Add_Card
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Fits_Header_Add_Float(struct Fits_Header_Struct *header,char *keyword,double value,char *comment)
{
	struct Fits_Header_Card_Struct card;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Add_Float:started.");
#endif
	if(keyword == NULL)
	{
		Autoguider_General_Error_Number = 1205;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_Float:Keyword is NULL.");
		return FALSE;
	}
	if(strlen(keyword) > (FITS_HEADER_KEYWORD_STRING_LENGTH-1))
	{
		Autoguider_General_Error_Number = 1206;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_Float:"
			"Keyword %s (%d) was too long.",keyword,strlen(keyword));
		return FALSE;
	}
	strcpy(card.Keyword,keyword);
	card.Type = FITS_HEADER_TYPE_FLOAT;
	card.Value.Float = value;
	/* the comment will be truncated to FITS_HEADER_COMMENT_STRING_LENGTH-1 */
	if(comment != NULL)
	{
		strncpy(card.Comment,comment,FITS_HEADER_COMMENT_STRING_LENGTH-1);
		card.Comment[FITS_HEADER_COMMENT_STRING_LENGTH-1] = '\0';
	}
	else
	{
		strcpy(card.Comment,"");
	}
	if(!Fits_Header_Add_Card(header,card))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Add_Float:finished.");
#endif
	return TRUE;
}

/**
 * Routine to add a keyword with a boolean value to a FITS header.
 * @param header The address of a Fits_Header_Struct structure to modify.
 * @param keyword The keyword string, must be at least 1 character less in length than 
 *        FITS_HEADER_KEYWORD_STRING_LENGTH.
 * @param value The boolean value, an integer with value 0 (FALSE) or 1 (TRUE).
 * @param comment The comment string, which if longer than FITS_HEADER_COMMENT_STRING_LENGTH-1 
 *        characters will be truncated. This parameter can also be NULL.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see #Fits_Header_Struct
 * @see #Fits_Header_Type_Enum
 * @see #FITS_HEADER_KEYWORD_STRING_LENGTH
 * @see #FITS_HEADER_COMMENT_STRING_LENGTH
 * @see #Fits_Header_Add_Card
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Fits_Header_Add_Logical(struct Fits_Header_Struct *header,char *keyword,int value,char *comment)
{
	struct Fits_Header_Card_Struct card;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Add_Logical:started.");
#endif
	if(keyword == NULL)
	{
		Autoguider_General_Error_Number = 1207;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_Logical:Keyword is NULL.");
		return FALSE;
	}
	if(strlen(keyword) > (FITS_HEADER_KEYWORD_STRING_LENGTH-1))
	{
		Autoguider_General_Error_Number = 1208;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_Logical:"
			"Keyword %s (%d) was too long.",keyword,strlen(keyword));
		return FALSE;
	}
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(value))
	{
		Autoguider_General_Error_Number = 1209;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Add_Logical:"
			"Value (%d) was not a boolean.",value);
		return FALSE;
	}
	strcpy(card.Keyword,keyword);
	card.Type = FITS_HEADER_TYPE_LOGICAL;
	card.Value.Boolean = value;
	/* the comment will be truncated to FITS_HEADER_COMMENT_STRING_LENGTH-1 */
	if(comment != NULL)
	{
		strncpy(card.Comment,comment,FITS_HEADER_COMMENT_STRING_LENGTH-1);
		card.Comment[FITS_HEADER_COMMENT_STRING_LENGTH-1] = '\0';
	}
	else
	{
		strcpy(card.Comment,"");
	}
	if(!Fits_Header_Add_Card(header,card))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Add_Logical:finished.");
#endif
	return TRUE;
}

/**
 * Routine to free an allocated FITS header list.
 * @param header The address of a Fits_Header_Struct structure to modify.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Fits_Header_Free(struct Fits_Header_Struct *header)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Free:started.");
#endif
	if(header == NULL)
	{
		Autoguider_General_Error_Number = 1210;
		sprintf(Autoguider_General_Error_String,"Autoguider_Fits_Header_Free:Header was NULL.");
		return FALSE;
	}
	if(header->Card_List != NULL)
		free(header->Card_List);
	header->Card_List = NULL;
	header->Card_Count = 0;
	header->Allocated_Card_Count = 0;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Autoguider_Fits_Header_Free:finished.");
#endif
	return TRUE;
}

/**
 * Write the information contained in the header structure to the specified fitsfile.
 * @param header The Fits_Header_Struct structure containing the headers to insert.
 * @param fits_fp A previously created CFITSIO file pointer to write the headers into.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Fits_Header_Write_To_Fits(struct Fits_Header_Struct header,fitsfile *fits_fp)
{
	int i;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,
			       "Autoguider_Fits_Header_Write_To_Fits:started.");
#endif
	for(i=0;i<header.Card_Count;i++)
	{
		switch(header.Card_List[i].Type)
		{
			case FITS_HEADER_TYPE_STRING:

				break;
			case FITS_HEADER_TYPE_INTEGER:
				break;
			case FITS_HEADER_TYPE_FLOAT:
				break;
			case FITS_HEADER_TYPE_LOGICAL:
				break;

			default:
				break;
		}
		/*diddly*/
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,
			       "Autoguider_Fits_Header_Write_To_Fits:finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Routine to add a card to the list. If the keyword already exists, that card will be updated with the new value,
 * otherwise a new card will be allocated (if necessary) and added to the lsit.
 * @param header The address of a Fits_Header_Struct structure to modify.
 * @return The routine returns TRUE on success, and FALSE on failure. On failure, Autoguider_General_Error_Number
 *         and Autoguider_General_Error_String should be filled in with suitable values.
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Log_Format
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Fits_Header_Add_Card(struct Fits_Header_Struct *header,struct Fits_Header_Card_Struct card)
{
	int index,done;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Fits_Header_Add_Card:started.");
#endif
	if(header == NULL)
	{
		Autoguider_General_Error_Number = 1211;
		sprintf(Autoguider_General_Error_String,"Fits_Header_Add_Card:"
			"Header was NULL for keyword %s.",card.Keyword);
		return FALSE;
	}
	index = 0;
	done  = FALSE;
	while((index < header->Card_Count) && (done == FALSE))
	{
		if(strcmp(header->Card_List[index].Keyword,card.Keyword) == 0)
		{
			done = TRUE;
		}
		else
			index++;
	}
	/* if we found a card with this keyword, update it. */
	if(done == TRUE)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,
				       "Fits_Header_Add_Card:Found keyword %s at index %d:Card updated.",
				       card.Keyword,index);
#endif
		header->Card_List[index] = card;
		return TRUE;
	}
	/* add the card to the list */
	/* if we need to allocate more memory... */
	if((header->Card_Count+1) >= header->Allocated_Card_Count)
	{
		/* allocate card list memory to be the current card count +1 */
		if(header->Card_List == NULL)
		{
			header->Card_List = (struct Fits_Header_Card_Struct *)malloc((header->Card_Count+1)*
									  sizeof(struct Fits_Header_Card_Struct));
		}
		else
		{
			header->Card_List = (struct Fits_Header_Card_Struct *)realloc(header->Card_List,
					    (header->Card_Count+1)*sizeof(struct Fits_Header_Card_Struct));
		}
		if(header->Card_List == NULL)
		{
			header->Card_Count = 0;
			header->Allocated_Card_Count = 0;
			Autoguider_General_Error_Number = 1212;
			sprintf(Autoguider_General_Error_String,"Fits_Header_Add_Card:"
				"Failed to reallocate card list (%d,%d).",(header->Card_Count+1),
				header->Allocated_Card_Count);
			return FALSE;
		}
		/* upcate allocated card count */
		header->Allocated_Card_Count = header->Card_Count+1;
	}/* end if more memory needed */
	/* add the card to the list */
	header->Card_List[header->Card_Count] = card;
	header->Card_Count++;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER,"Fits_Header_Add_Card:finished.");
#endif
	return TRUE;

}
/*
** $Log: not supported by cvs2svn $
*/
