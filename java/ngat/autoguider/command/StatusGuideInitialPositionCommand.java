// StatusGuideInitialPositionCommand.java
package ngat.autoguider.command;

import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.*;
import java.util.*;

/**
 * The "status guide initial_position" command is an extension of Command, and returns 
 * the initial guide object position (in CCD coordinates) at the start of the guide loop.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
 */
public class StatusGuideInitialPositionCommand extends Command implements Runnable
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id$");
	/**
	 * The command to send to the autoguider.
	 */
	public final static String COMMAND_STRING = new String("status guide initial_position");
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_CCD_POSITION_X    = 1;
	/**
	 * Field index in object string, used for parsing.
	 * @see #parseReplyString
	 */
	public final static int FIELD_INDEX_CCD_POSITION_Y    = 2;
	/**
	 * Object field.
	 */
	protected double ccdPositionX = 0.0;
	/**
	 * Object field.
	 */
	protected double ccdPositionY = 0.0;

	/**
	 * Default constructor.
	 * @see #commandString
	 * @see #COMMAND_STRING
	 */
	public StatusGuideInitialPositionCommand()
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
	public StatusGuideInitialPositionCommand(String address,int portNumber) throws UnknownHostException
	{
		super(address,portNumber,COMMAND_STRING);
	}

	/**
	 * Parse a string returned from the autoguider over the telnet connection.
	 * This string has the format:
	 * <pre>
	 * Initial_Object_CCD_X_Position Initial_Object_CCD_Y_Position 
	 * </pre>
	 * See autoguider_command.c, Autoguider_Command_Status for the code that generates this string.
	 * @see #FIELD_INDEX_CCD_POSITION_X
	 * @see #FIELD_INDEX_CCD_POSITION_Y
	 * @see #replyString
	 * @see #parsedReplyString
	 * @see #parsedReplyOk
	 * @see #ccdPositionX
	 * @see #ccdPositionY
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
		}
	}

	/**
	 * Object field getter.
	 * @return The initial CCD X position of the object, in pixels.
	 * @see #ccdPositionX
	 */
	public double getCCDPositionX()
	{
		return ccdPositionX;
	}

	/**
	 * Object field getter.
	 * @return The initial CCD Y position of the object, in pixels.
	 * @see #ccdPositionY
	 */
	public double getCCDPositionY()
	{
		return ccdPositionY;
	}

	/**
	 * Main test program.
	 * @param args The argument list.
	 */
	public static void main(String args[])
	{
		StatusGuideInitialPositionCommand command = null;
		DecimalFormat df = null;
		int portNumber = 1234;

		if(args.length != 2)
		{
			System.out.println("java ngat.autoguider.command.StatusGuideInitialPositionCommand <hostname> <port number>");
			System.exit(1);
		}
		try
		{
			portNumber = Integer.parseInt(args[1]);
			command = new StatusGuideInitialPositionCommand(args[0],portNumber);
			command.run();
			if(command.getRunException() != null)
			{
				System.err.println("StatusGuideInitialPositionCommand: Command failed.");
				command.getRunException().printStackTrace(System.err);
				System.exit(1);
			}
			System.out.println("Finished:"+command.getCommandFinished());
			System.out.println("Reply Parsed OK:"+command.getParsedReplyOK());
			// print out results
			df = new DecimalFormat("0.00");
			System.out.println("Initial Guide Position:ccdx"+df.format(command.ccdPositionX)+
					   " ccdy:"+df.format(command.ccdPositionY));
		}
		catch(Exception e)
		{
			e.printStackTrace(System.err);
			System.exit(1);
		}

		System.exit(0);
	}
}
