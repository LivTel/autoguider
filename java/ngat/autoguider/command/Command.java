// Command.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/command/Command.java,v 1.2 2010-07-29 09:21:17 cjm Exp $
package ngat.autoguider.command;

import java.io.*;
import java.lang.*;
import java.net.*;

import ngat.net.TelnetConnection;
import ngat.net.TelnetConnectionListener;

/**
 * The Command class is the base class for sending a command and getting a reply from the
 * LJMU autoguider control system. This is a telnet - type socket interaction.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
 */
public class Command implements Runnable, TelnetConnectionListener
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: Command.java,v 1.2 2010-07-29 09:21:17 cjm Exp $");
	/**
	 * ngat.net.TelnetConnection instance.
	 */
	protected TelnetConnection telnetConnection = null;
	/**
	 * The command to send to the autoguider.
	 */
	protected String commandString = null;
	/**
	 * Exception generated by errors generated in sendCommand, if called via the run method.
	 * @see #sendCommand
	 * @see #run
	 */
	protected Exception runException = null;
	/**
	 * Boolean set to true, when a command has been sent to the autoguider server and
	 * a reply string has been sent.
	 * @see #sendCommand
	 */
	protected boolean commandFinished = false;
	/**
	 * A string containing the reply from the autoguider.
	 */
	protected String replyString = null;
	/**
	 * The parsed reply string, this is the reply with the initial number and space stripped off.
	 */
	protected String parsedReplyString = null;
	/**
	 * Whether the reply string started with a number, and that number was '0', indicating the
	 * command suceeded in some sense.
	 */
	protected boolean parsedReplyOk = false;

	/**
	 * Default constructor.
	 * @see #telnetConnection
	 */
	public Command()
	{
		super();
		telnetConnection = new TelnetConnection();
		telnetConnection.setListener(this);
	}

	/**
	 * Constructor.
	 * @param address A string representing the address of the autoguider control computer, i.e. "autoguider1",
	 *     "localhost", "192.168.1.4"
	 * @param portNumber An integer representing the port number the autoguider control software is receiving 
	 *       command on.
	 * @param commandString The string to send to the autoguider as a command.
	 * @see #telnetConnection
	 * @see #commandString
	 * @exception UnknownHostException Thrown if the address in unknown.
	 */
	public Command(String address,int portNumber,String commandString) throws UnknownHostException
	{
		super();
		telnetConnection = new TelnetConnection(address,portNumber);
		telnetConnection.setListener(this);
		this.commandString = commandString;
	}

	/**
	 * Set the address.
	 * @param address A string representing the address of the autoguider control computer, i.e. "autoguider1",
	 *     "localhost", "192.168.1.4"
	 * @see #telnetConnection
	 * @exception UnknownHostException Thrown if the address in unknown.
	 */
	public void setAddress(String address) throws UnknownHostException
	{
		telnetConnection.setAddress(address);
	}

	/**
	 * Set the address.
	 * @param address A instance of InetAddress representing the address of the autoguider control computer.
	 * @see #telnetConnection
	 */
	public void setAddress(InetAddress address)
	{
		telnetConnection.setAddress(address);
	}

	/**
	 * Set the port number.
	 * @param portNumber An integer representing the port number the autoguider control software is receiving 
	 *       command on.
	 * @see #telnetConnection
	 */
	public void setPortNumber(int portNumber)
	{
		telnetConnection.setPortNumber(portNumber);
	}

	/**
	 * Set the command.
	 * @param command The string to send to the autoguider as a command.
	 * @see #commandString
	 */
	public void setCommand(String command)
	{
		commandString = command;
	}

	/**
	 * Run thread. Uses sendCommand to send the specified command over a telnet connection to the specified
	 * address and port number.
	 * Catches any errors and puts them into runException. commandFinished indicates when the command
	 * has finished processing, replyString, parsedReplyString, parsedReplyOk contain the autoguider replies.
	 * @see #commandString
	 * @see #sendCommand
	 * @see #replyString
	 * @see #parsedReplyString
	 * @see #parsedReplyOk
	 * @see #runException
	 * @see #commandFinished
	 */
	public void run()
	{
		try
		{
			sendCommand();
		}
		catch(Exception e)
		{
			runException = e;
			commandFinished = false;
		}
	}

	/**
	 * Routine to send the specified command over a telnet connection to the specified
	 * address and port number, wait for a reply from the autoguider, and try to parse the reply.
	 * @exception Exception Thrown if sending the command failed.
	 * @see #telnetConnection
	 * @see #commandString
	 * @see #commandFinished
	 * @see #parseReplyString
	 */
	public void sendCommand() throws Exception
	{
		Thread thread = null;

		commandFinished = false;
		telnetConnection.open();
		thread = new Thread(telnetConnection,"Reader thread");
		thread.start();
		telnetConnection.sendLine(commandString);
		thread.join();
		telnetConnection.close();
		parseReplyString();
		commandFinished = true;
	}

	/**
	 * TelnetConnectionListener interface implementation.
	 * Called for each line of text read by the TelnetConnection instance.
	 * The string is copied/appended to the replyString.
	 * @param line The string read from the TelnetConnection.
	 * @see #replyString
	 */
	public void lineRead(String line)
	{
		if(replyString == null)
			replyString = line;
		else
			replyString = new String(replyString+line);
	}

	/**
	 * Parse a string returned from the autoguider over the telnet connection.
	 * @exception Exception Thrown if replyString is null, or there was no space between 
	 *            the return error code and message.
	 * @see #replyString
	 * @see #parsedReplyString
	 * @see #parsedReplyOk
	 */
	public void parseReplyString() throws Exception
	{
		int sindex;
		String okString = null;

		if(replyString == null)
		{
			throw new Exception(this.getClass().getName()+
					    ":parseReplyString:Reply string to command '"+commandString+"'was null.");
		}
		sindex = replyString.indexOf(' ');
		if(sindex < 0)
		{
			parsedReplyString = null;
			parsedReplyOk = false;
			throw new Exception(this.getClass().getName()+
					    ":parseReplyString:Failed to detect space between error code and data in:"+
					    replyString);
		}
		okString = replyString.substring(0,sindex);
		parsedReplyString = replyString.substring(sindex+1);
		if(okString.equals("0"))
			parsedReplyOk = true;
		else
			parsedReplyOk = false;
	}

	/**
	 * Return the reply string
	 * @return The FULL string returned from the autoguider.
	 * @see #replyString
	 */
	public String getReply()
	{
		return replyString;
	}

	/**
	 * Return the reply string
	 * @return The PARSED string returned from the autoguider.
	 * @see #parsedReplyString
	 */
	public String getParsedReply()
	{
		return parsedReplyString;
	}

	/**
	 * Return the reply string was parsed OK.
	 * @return A boolean, true if the reply started with a '0'.
	 * @see #parsedReplyOk
	 */
	public boolean getParsedReplyOK()
	{
		return parsedReplyOk;
	}

	/**
	 * Get any exception resulating from running the command.
	 * This is only filled in if the command was sent using the run method, rather than the sendCommand method.
	 * @return An exception if the command failed in some way, or null if no error occured.
	 * @see #run
	 * @see #sendCommand
	 * @see #runException
	 */
	public Exception getRunException()
	{
		return runException;
	}

	/**
	 * Get whether the command has been completed.
	 * @return A Boolean, true if a command has been sent, and a reply received and parsed. false if the
	 *     command has not been sent yet, or we are still waiting for a reply.
	 * @see #commandFinished
	 */
	public boolean getCommandFinished()
	{
		return commandFinished;
	}

	/**
	 * Main test program.
	 * @param args The argument list.
	 */
	public static void main(String args[])
	{
		Command command = null;
		int portNumber = 1234;

		if(args.length != 3)
		{
			System.out.println("java ngat.autoguider.command.Command <hostname> <port number> <command>");
			System.exit(1);
		}
		try
		{
			portNumber = Integer.parseInt(args[1]);
			command = new Command(args[0],portNumber,args[2]);
			command.run();
			if(command.getRunException() != null)
			{
				System.err.println("Command: Command "+args[2]+" failed.");
				command.getRunException().printStackTrace(System.err);
				System.exit(1);
			}
			System.out.println("Reply:"+command.getReply());
			System.out.println("Finished:"+command.getCommandFinished());
			System.out.println("Reply Parsed OK:"+command.getParsedReplyOK());
			System.out.println("Parsed Reply:"+command.getParsedReply());
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
// Revision 1.1  2009/01/30 18:01:58  cjm
// Initial revision
//
//
