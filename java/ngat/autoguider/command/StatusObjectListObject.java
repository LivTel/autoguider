// StatusObjectListObject.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/command/StatusObjectListObject.java,v 1.1 2009-01-30 18:01:58 cjm Exp $
package ngat.autoguider.command;

import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.*;
import java.util.*;

/**
 * Class containing details of one list of the results of a "status object list" command. i.e.
 * The details of one detected object on the autoguider CCD.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class StatusObjectListObject
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: StatusObjectListObject.java,v 1.1 2009-01-30 18:01:58 cjm Exp $");
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_ID                = 0;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_FRAME_NUMBER      = 1;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_INDEX             = 2;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_CCD_POSITION_X    = 3;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_CCD_POSITION_Y    = 4;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_BUFFER_POSITION_X = 5;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_BUFFER_POSITION_Y = 6;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_TOTAL_COUNTS      = 7;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_NUMBER_OF_PIXELS  = 8;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_PEAK_COUNTS       = 9;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_IS_STELLAR        = 10;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_FWHM_X            = 11;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parse
	 */
	public final static int FIELD_INDEX_FWHM_Y            = 12;
	/**
	 * Object field.
	 */
	protected int id = 0;
	/**
	 * Object field.
	 */
	protected int frameNumber = 0;
	/**
	 * Object field.
	 */
	protected int index = 0;
	/**
	 * Object field.
	 */
	protected double ccdPositionX = 0.0;
	/**
	 * Object field.
	 */
	protected double ccdPositionY = 0.0;
	/**
	 * Object field.
	 */
	protected double bufferPositionX = 0.0;
	/**
	 * Object field.
	 */
	protected double bufferPositionY = 0.0;
	/**
	 * Object field.
	 */
	protected double totalCounts = 0.0;
	/**
	 * Object field.
	 */
	protected int numberOfPixels = 0;
	/**
	 * Object field.
	 */
	protected double peakCounts = 0.0;
	/**
	 * Object field.
	 */
	protected boolean isStellar = false;
	/**
	 * Object field.
	 */
	protected double fwhmX = 0.0;
	/**
	 * Object field.
	 */
	protected double fwhmY = 0.0;

	/**
	 * Default constructor.
	 */
	public StatusObjectListObject()
	{
		super();
	}

	/**
	 * Object field getter.
	 * @return The object id as an integer.
	 */
	public int getId()
	{
		return id;
	}

	/**
	 * Object field getter.
	 * @return The object frame number as an integer.
	 */
	public int getFrameNumber()
	{
		return frameNumber;
	}

	/**
	 * Object field getter.
	 * @return The object index as an integer.
	 */
	public int getIndex()
	{
		return index;
	}

	/**
	 * Object field getter.
	 * @return The object's X position on the CCD as a double.
	 */
	public double getCCDPositionX()
	{
		return ccdPositionX;
	}

	/**
	 * Object field getter.
	 * @return The object's Y position on the CCD as a double.
	 */
	public double getCCDPositionY()
	{
		return ccdPositionY;
	}

	/**
	 * Object field getter.
	 * @return The object's X position in the image buffer as a double.
	 */
	public double getBufferPositionX()
	{
		return bufferPositionX;
	}

	/**
	 * Object field getter.
	 * @return The object's Y position in the image buffer as a double.
	 */
	public double getBufferPositionY()
	{
		return bufferPositionY;
	}

	/**
	 * Object field getter.
	 * @return The object's total integrated counts in ADU as a double.
	 */
	public double getTotalCounts()
	{
		return totalCounts;
	}

	/**
	 * Object field getter.
	 * @return The number of connected pixel making up the object, as an integer. 
	 */
	public int getNumberOfPixels()
	{
		return numberOfPixels;
	}

	/**
	 * Object field getter.
	 * @return The counts, in ADU, of the pixel with the greatest ADU value in the object as a double.
	 */
	public double getPeakCounts()
	{
		return peakCounts;
	}

	/**
	 * Object field getter.
	 * @return A boolean, if true the object detection thinks the object is stellar (round(ish)), false otherwise.
	 */
	public boolean isStellar()
	{
		return isStellar;
	}

	/**
	 * Object field getter.
	 * @return The full width half maximum of the object in X in pixels, as a double.
	 */
	public double getFWHMX()
	{
		return fwhmX;
	}

	/**
	 * Object field getter.
	 * @return The full width half maximum of the object in Y in pixels, as a double.
	 */
	public double getFWHMY()
	{
		return fwhmY;
	}

	/**
	 * Parse a string from "status object list" into the fields of this class.
	 * @param lineString The line to parse.
	 * @exception Exception Thrown if the parsing fails.
	 * @see #FIELD_INDEX_ID
	 * @see #FIELD_INDEX_FRAME_NUMBER
	 * @see #FIELD_INDEX_INDEX
	 * @see #FIELD_INDEX_CCD_POSITION_X
	 * @see #FIELD_INDEX_CCD_POSITION_Y
	 * @see #FIELD_INDEX_BUFFER_POSITION_X
	 * @see #FIELD_INDEX_BUFFER_POSITION_Y
	 * @see #FIELD_INDEX_TOTAL_COUNTS
	 * @see #FIELD_INDEX_NUMBER_OF_PIXELS
	 * @see #FIELD_INDEX_PEAK_COUNTS
	 * @see #FIELD_INDEX_IS_STELLAR
	 * @see #FIELD_INDEX_FWHM_X
	 * @see #FIELD_INDEX_FWHM_Y
	 * @see #id
	 * @see #frameNumber
	 * @see #index
	 * @see #ccdPositionX
	 * @see #ccdPositionY
	 * @see #bufferPositionX
	 * @see #bufferPositionY
	 * @see #totalCounts
	 * @see #numberOfPixels
	 * @see #peakCounts
	 * @see #isStellar
	 * @see #fwhmX
	 * @see #fwhmY
	 */
	public void parse(String lineString) throws Exception
	{
		StringTokenizer fieldTokeniser = null;
		String fieldString = null;
		int fieldIndex;

		fieldIndex = 0;
		fieldTokeniser = new StringTokenizer(lineString," ",false);
		while(fieldTokeniser.hasMoreTokens())
		{
			fieldString = fieldTokeniser.nextToken();
			switch(fieldIndex)
			{
				case FIELD_INDEX_ID:
					id = Integer.parseInt(fieldString);
					break;
				case FIELD_INDEX_FRAME_NUMBER:
					frameNumber = Integer.parseInt(fieldString);
					break;
				case FIELD_INDEX_INDEX:
					index = Integer.parseInt(fieldString);
					break;
				case FIELD_INDEX_CCD_POSITION_X:
					ccdPositionX = Double.parseDouble(fieldString);
					break;
				case FIELD_INDEX_CCD_POSITION_Y:
					ccdPositionY = Double.parseDouble(fieldString);
					break;
				case FIELD_INDEX_BUFFER_POSITION_X:
					bufferPositionX = Double.parseDouble(fieldString);
					break;
				case FIELD_INDEX_BUFFER_POSITION_Y:
					bufferPositionY = Double.parseDouble(fieldString);
					break;
				case FIELD_INDEX_TOTAL_COUNTS:
					totalCounts = Double.parseDouble(fieldString);
					break;
				case FIELD_INDEX_NUMBER_OF_PIXELS:
					numberOfPixels = Integer.parseInt(fieldString);
					break;
				case FIELD_INDEX_PEAK_COUNTS:
					peakCounts = Double.parseDouble(fieldString);
					break;
				case FIELD_INDEX_IS_STELLAR:
					if(fieldString.equals("TRUE"))
						isStellar = true;
					else if(fieldString.equals("FALSE"))
						isStellar = false;
					else
						throw new Exception(this.getClass().getName()+
								    ":parse:Illegal Is Stellar field string:"+
								    fieldString);
					break;
				case FIELD_INDEX_FWHM_X:
				        fwhmX = Double.parseDouble(fieldString);
					break;
				case FIELD_INDEX_FWHM_Y:
				        fwhmY = Double.parseDouble(fieldString);
					break;
				default:
					throw new Exception(this.getClass().getName()+
								    ":parse:Illegal field string:"+
								    fieldString+" at index "+fieldIndex);
			}// switch
			fieldIndex++;
		}// end while on tokens
	}

	/**
	 * Generate a string version of the object.
	 * @return The generated string.
	 */
	public String toString()
	{
		return toString("");
	}

	/**
	 * Generate a string version of the object.
	 * @param prefix A string to prepend to the generated string.
	 * @return The generated string.
	 * @see #id
	 * @see #frameNumber
	 * @see #index
	 * @see #ccdPositionX
	 * @see #ccdPositionY
	 * @see #bufferPositionX
	 * @see #bufferPositionY
	 * @see #totalCounts
	 * @see #numberOfPixels
	 * @see #peakCounts
	 * @see #isStellar
	 * @see #fwhmX
	 * @see #fwhmY
	 */
	public String toString(String prefix)
	{
		DecimalFormat df = null;

		df = new DecimalFormat("0.00");
		return new String(prefix+id+" "+frameNumber+" "+index+" "+
				  df.format(ccdPositionX)+" "+df.format(ccdPositionY)+" "+
				  df.format(bufferPositionX)+" "+df.format(bufferPositionY)+" "+
				  df.format(totalCounts)+" "+numberOfPixels+" "+df.format(peakCounts)+" "+
				  isStellar+" "+df.format(fwhmX)+" "+df.format(fwhmY));
	}

}
//
// $Log: not supported by cvs2svn $
//
