// StatusGuideLastObjectCommand.java
package ngat.autoguider.command;

import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.*;
import java.util.*;

/**
 * The "status guide last_object" command is an extension of Command, and returns 
 * the guide object last used to send a centroid to the TCS/SDB.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
 */
public class StatusGuideLastObjectCommand extends Command implements Runnable
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id$");
	/**
	 * The command to send to the autoguider.
	 */
	public final static String COMMAND_STRING = new String("status guide last_object");
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_CCD_POSITION_X    = 0;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_CCD_POSITION_Y    = 1;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_BUFFER_POSITION_X = 2;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_BUFFER_POSITION_Y = 3;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_TOTAL_COUNTS      = 4;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_NUMBER_OF_PIXELS  = 5;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_PEAK_COUNTS       = 6;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_FWHM_X            = 7;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_FWHM_Y            = 8;
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
	protected double fwhmX = 0.0;
	/**
	 * Object field.
	 */
	protected double fwhmY = 0.0;

	/**
	 * Default constructor.
	 * @see #commandString
	 * @see #COMMAND_STRING
	 */
	public StatusGuideLastObjectCommand()
	{
		super();
		commandString = COMMAND_STRING;
	}

	/**
	 * Constructor.
	 * @param address A string representing the address of the autoguider control computer, i.e. "autoguider1",
	 *     "localhost", "192.168.1.4"
	 * @param portNumber An integer representing the port number the autoguider control software is receiving 
	 *       command on.
	 * @see IntegerReplyCommand
	 * @see #COMMAND_STRING
	 * @exception UnknownHostException Thrown if the address in unknown.
	 */
	public StatusGuideLastObjectCommand(String address,int portNumber) throws UnknownHostException
	{
		super(address,portNumber,COMMAND_STRING);
	}

	/**
	 * Parse a string returned from the autoguider over the telnet connection.
	 * This string has the format:
	 * <pre>
	 * CCD_X_Position CCD_Y_Position Buffer_X_Position Buffer_Y_Position 
	 * Total_Counts Pixel_Count Peak_Counts FWHM_X FWHM_Y
	 * </pre>
	 * See autoguider_command.c, Autoguider_Command_Status for the code that generates this string.
	 * @see #FIELD_INDEX_CCD_POSITION_X
	 * @see #FIELD_INDEX_CCD_POSITION_Y
	 * @see #FIELD_INDEX_BUFFER_POSITION_X
	 * @see #FIELD_INDEX_BUFFER_POSITION_Y
	 * @see #FIELD_INDEX_TOTAL_COUNTS
	 * @see #FIELD_INDEX_NUMBER_OF_PIXELS
	 * @see #FIELD_INDEX_PEAK_COUNTS
	 * @see #FIELD_INDEX_FWHM_X
	 * @see #FIELD_INDEX_FWHM_Y
	 * @see #replyString
	 * @see #parsedReplyString
	 * @see #parsedReplyOk
	 * @see #ccdPositionX
	 * @see #ccdPositionY
	 * @see #bufferPositionX
	 * @see #bufferPositionY
	 * @see #totalCounts
	 * @see #numberOfPixels
	 * @see #peakCounts
	 * @see #fwhmX
	 * @see #fwhmY
	 * @link http://ltdevsrv.livjm.ac.uk/~dev/autoguider/cdocs/autoguider_command.html#Autoguider_Command_Status
	 */
	public void parseReplyString() throws Exception
	{
		StringTokenizer fieldTokeniser = null;
		String fieldString = null;
		int fieldIndex;

		super.parseReplyString();
		if(parsedReplyOk == false)
		{
			ccdPositionX = 0.0;
			ccdPositionY = 0.0;
			bufferPositionX = 0.0;
			bufferPositionY = 0.0;
			totalCounts = 0.0;
			numberOfPixels = 0;
			peakCounts = 0.0;
			fwhmX = 0.0;
			fwhmY = 0.0;
			return;
		}
		try
		{
			fieldIndex = 0;
			fieldTokeniser = new StringTokenizer(parsedReplyString," ",false);
			while(fieldTokeniser.hasMoreTokens())
			{
				fieldString = fieldTokeniser.nextToken();
				switch(fieldIndex)
				{
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
					case FIELD_INDEX_FWHM_X:
						fwhmX = Double.parseDouble(fieldString);
						break;
					case FIELD_INDEX_FWHM_Y:
						fwhmY = Double.parseDouble(fieldString);
						break;
					default:
						throw new Exception(this.getClass().getName()+
								    ":parseReplyString:Illegal field string:"+
								    fieldString+" at index "+fieldIndex);
				}// switch
				fieldIndex++;
			}// end while on tokens
		}
		catch(Exception e)
		{
			e.printStackTrace(System.err);
			parsedReplyOk = false;
			ccdPositionX = 0.0;
			ccdPositionY = 0.0;
			bufferPositionX = 0.0;
			bufferPositionY = 0.0;
			totalCounts = 0.0;
			numberOfPixels = 0;
			peakCounts = 0.0;
			fwhmX = 0.0;
			fwhmY = 0.0;
		}
	}

	/**
	 * Object field getter.
	 * @return The CCD X position of the object, in pixels.
	 * @see #ccdPositionX
	 */
	public double getCCDPositionX()
	{
		return ccdPositionX;
	}

	/**
	 * Object field getter.
	 * @return The CCD Y position of the object, in pixels.
	 * @see #ccdPositionY
	 */
	public double getCCDPositionY()
	{
		return ccdPositionY;
	}

	/**
	 * Object field getter.
	 * @return The buffer (window) X position of the object, in pixels.
	 * @see #bufferPositionX
	 */
	public double getBufferPositionX()
	{
		return bufferPositionX;
	}

	/**
	 * Object field getter.
	 * @return The buffer (window) Y position of the object, in pixels.
	 * @see #bufferPositionY
	 */
	public double getBufferPositionY()
	{
		return bufferPositionY;
	}

	/**
	 * Object field getter.
	 * @return The total counts in the object.
	 * @see #totalCounts
	 */
	public double getTotalCounts()
	{
		return totalCounts;
	}

	/**
	 * Object field getter.
	 * @return The number of pixels of the object.
	 * @see #numberOfPixels
	 */
	public int getNumberOfPixels()
	{
		return numberOfPixels;
	}

	/**
	 * Object field getter.
	 * @return The peak counts in the object.
	 * @see #peakCounts
	 */
	public double getPeakCounts()
	{
		return peakCounts;
	}

	/**
	 * Object field getter.
	 * @return The full width half maximum in X of the object, in pixels.
	 * @see #fwhmX
	 */
	public double getFWHMX()
	{
		return fwhmX;
	}

	/**
	 * Object field getter.
	 * @return The full width half maximum in Y of the object, in pixels.
	 * @see #fwhmY
	 */
	public double getFWHMY()
	{
		return fwhmY;
	}

	/**
	 * Return a string describing the values returned by executing this command.
	 * @return A string.
	 * @see #toString(java.lang.String)
	 */
	public String toString()
	{
		return toString("");
	}

	/**
	 * Return a string describing the values returned by executing this command.
	 * @param prefix A string to prepend to the returned string.
	 * @return A string.
	 * @see #ccdPositionX
	 * @see #ccdPositionY
	 * @see #bufferPositionX
	 * @see #bufferPositionY
	 * @see #totalCounts
	 * @see #numberOfPixels
	 * @see #peakCounts
	 * @see #fwhmX
	 * @see #fwhmY
	 */
	public String toString(String prefix)
	{
		DecimalFormat df = null;

		df = new DecimalFormat("0.00");
		return new String(prefix+"Last object:ccdx"+df.format(ccdPositionX)+
				  " ccdy:"+df.format(ccdPositionY)+
				  " bufferx:"+df.format(bufferPositionX)+
				  " buffery"+df.format(bufferPositionY)+
				  " total counts:"+df.format(totalCounts)+
				  " number of pixels:"+numberOfPixels+
				  " peak counts:"+df.format(peakCounts)+" "+
				  " fwhmx:"+df.format(fwhmX)+" fwhmy:"+df.format(fwhmY));
	}
	
	/**
	 * Main test program.
	 * @param args The argument list.
	 */
	public static void main(String args[])
	{
		StatusGuideLastObjectCommand command = null;
		DecimalFormat df = null;
		int portNumber = 1234;

		if(args.length != 2)
		{
			System.out.println("java ngat.autoguider.command.StatusGuideLastObjectCommand <hostname> <port number>");
			System.exit(1);
		}
		try
		{
			portNumber = Integer.parseInt(args[1]);
			command = new StatusGuideLastObjectCommand(args[0],portNumber);
			command.run();
			if(command.getRunException() != null)
			{
				System.err.println("StatusGuideLastObjectCommand: Command failed.");
				command.getRunException().printStackTrace(System.err);
				System.exit(1);
			}
			System.out.println("Finished:"+command.getCommandFinished());
			System.out.println("Reply Parsed OK:"+command.getParsedReplyOK());
			// print out results
			df = new DecimalFormat("0.00");
			System.out.println("Last object:ccdx"+df.format(command.ccdPositionX)+
					   " ccdy:"+df.format(command.ccdPositionY)+
					   " bufferx:"+df.format(command.bufferPositionX)+
					   " buffery"+df.format(command.bufferPositionY)+
					   " total counts:"+df.format(command.totalCounts)+
					   " number of pixels:"+command.numberOfPixels+
					   " peak counts:"+df.format(command.peakCounts)+" "+
					   " fwhmx:"+df.format(command.fwhmX)+" fwhmy"+df.format(command.fwhmY));
		}
		catch(Exception e)
		{
			e.printStackTrace(System.err);
			System.exit(1);
		}

		System.exit(0);
	}
}
