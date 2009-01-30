// StatusGuideActiveCommand.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/command/StatusGuideActiveCommand.java,v 1.1 2009-01-30 18:01:58 cjm Exp $
package ngat.autoguider.command;

import java.io.*;
import java.lang.*;
import java.net.*;

/**
 * The "status guide active" command is an extension of the BooleanReplyCommand,
 * which returns "true" if the autoguider is currently guiding, and "false" if it is not.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class StatusGuideActiveCommand extends BooleanReplyCommand implements Runnable
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: StatusGuideActiveCommand.java,v 1.1 2009-01-30 18:01:58 cjm Exp $");	/**
	 * The command to send to the autoguider.
	 */
	public final static String COMMAND_STRING = new String("status guide active");

	/**
	 * Default constructor.
	 * @see Command
	 * @see #commandString
	 * @see #COMMAND_STRING
	 */
	public StatusGuideActiveCommand()
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
	 * @see BooleanReplyCommand
	 * @see #COMMAND_STRING
	 * @exception UnknownHostException Thrown if the address in unknown.
	 */
	public StatusGuideActiveCommand(String address,int portNumber) throws UnknownHostException
	{
		super(address,portNumber,COMMAND_STRING);
	}

	/**
	 * Get whether the autoguider is guiding.
	 * @return A boolean, "true" if the autoguider is guiding, and "false" if it is not.
	 * @exception Exception Thrown if getting the data fails, either the run method failed to communicate
	 *         with the autoguider in some way, or the method was called before the command had completed.
	 */
	public boolean isGuiding() throws Exception
	{
		if(parsedReplyOk)
			return super.getParsedReplyBoolean();
		else
		{
			if(runException != null)
				throw runException;
			else
				throw new Exception(this.getClass().getName()+":isGuiding:Unknown Error.");
		}
	}

	/**
	 * Main test program.
	 * @param args The argument list.
	 */
	public static void main(String args[])
	{
		StatusGuideActiveCommand command = null;
		int portNumber = 1234;

		if(args.length != 2)
		{
			System.out.println("java ngat.autoguider.command.StatusGuideActiveCommand <hostname> <port number>");
			System.exit(1);
		}
		try
		{
			portNumber = Integer.parseInt(args[1]);
			command = new StatusGuideActiveCommand(args[0],portNumber);
			command.run();
			if(command.getRunException() != null)
			{
				System.err.println("StatusGuideActiveCommand: Command failed.");
				command.getRunException().printStackTrace(System.err);
				System.exit(1);
			}
			System.out.println("Finished:"+command.getCommandFinished());
			System.out.println("Reply Parsed OK:"+command.getParsedReplyOK());
			System.out.println("Is Guiding:"+command.isGuiding());
		}
		catch(Exception e)
		{
			e.printStackTrace(System.err);
			System.exit(1);
		}

		System.exit(0);

	}
}
//
// $Log: not supported by cvs2svn $
//
