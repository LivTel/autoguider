// StatusObjectMeanCommand.java
// $Header$
package ngat.autoguider.command;

import java.io.*;
import java.lang.*;
import java.net.*;

/**
 * The "status object mean" command is an extension of the DoubleReplyCommand, and returns the 
 * mean ADU of the last image passed to the autoguider object detection software.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class StatusObjectMeanCommand extends DoubleReplyCommand implements Runnable
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id$");
	/**
	 * The command to send to the autoguider.
	 */
	public final static String COMMAND_STRING = new String("status object mean");

	/**
	 * Default constructor.
	 * @see DoubleReplyCommand
	 * @see #commandString
	 * @see #COMMAND_STRING
	 */
	public StatusObjectMeanCommand()
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
	 * @see DoubleReplyCommand
	 * @see #COMMAND_STRING
	 * @exception UnknownHostException Thrown if the address in unknown.
	 */
	public StatusObjectMeanCommand(String address,int portNumber) throws UnknownHostException
	{
		super(address,portNumber,COMMAND_STRING);
	}

	/**
	 * Get the mean ADU in the last image passed to the autoguider object detection software.
	 * @return A double, the mean ADU in counts.
	 * @exception Exception Thrown if getting the data fails, either the run method failed to communicate
	 *         with the autoguider in some way, or the method was called before the command had completed.
	 */
	public double getMean() throws Exception
	{
		if(parsedReplyOk)
			return super.getParsedReplyDouble();
		else
		{
			if(runException != null)
				throw runException;
			else
				throw new Exception(this.getClass().getName()+":getMean:Unknown Error.");
		}
	}

	/**
	 * Main test program.
	 * @param args The argument list.
	 */
	public static void main(String args[])
	{
		StatusObjectMeanCommand command = null;
		int portNumber = 1234;

		if(args.length != 2)
		{
			System.out.println("java ngat.autoguider.command.StatusObjectMeanCommand <hostname> <port number>");
			System.exit(1);
		}
		try
		{
			portNumber = Integer.parseInt(args[1]);
			command = new StatusObjectMeanCommand(args[0],portNumber);
			command.run();
			if(command.getRunException() != null)
			{
				System.err.println("StatusObjectMeanCommand: Command failed.");
				command.getRunException().printStackTrace(System.err);
				System.exit(1);
			}
			System.out.println("Finished:"+command.getCommandFinished());
			System.out.println("Reply Parsed OK:"+command.getParsedReplyOK());
			System.out.println("Mean ADU of the last object detection image (counts):"+
					   command.getMean());
		}
		catch(Exception e)
		{
			e.printStackTrace(System.err);
			System.exit(1);
		}
		System.exit(0);
	}
}
