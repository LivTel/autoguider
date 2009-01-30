// DoubleReplyCommand.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/command/DoubleReplyCommand.java,v 1.1 2009-01-30 18:01:58 cjm Exp $
package ngat.autoguider.command;

import java.io.*;
import java.lang.*;
import java.net.*;

import ngat.net.TelnetConnection;
import ngat.net.TelnetConnectionListener;

/**
 * The DoubleReplyCommand class is an extension of the base Command class for sending a command and getting a 
 * reply from the
 * LJMU autoguider control system. This is a telnet - type socket interaction. DoubleReplyCommand expects the reply
 * to be '&lt;n&gt; &lt;m&gt;' where &lt;n&gt; is the reply status and &lt;m&gt; is a double value. 
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class DoubleReplyCommand extends Command implements Runnable, TelnetConnectionListener
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: DoubleReplyCommand.java,v 1.1 2009-01-30 18:01:58 cjm Exp $");	/**
	 * The parsed reply string, parsed into a double.
	 */
	protected double parsedReplyDouble = 0;

	/**
	 * Default constructor.
	 * @see Command
	 */
	public DoubleReplyCommand()
	{
		super();
	}

	/**
	 * Constructor.
	 * @param address A string representing the address of the autoguider control computer, i.e. "autoguider1",
	 *     "localhost", "192.168.1.4"
	 * @param portNumber An integer representing the port number the autoguider control software is receiving 
	 *       command on.
	 * @param commandString The string to send to the autoguider as a command.
	 * @see Command
	 * @exception UnknownHostException Thrown if the address in unknown.
	 */
	public DoubleReplyCommand(String address,int portNumber,String commandString) throws UnknownHostException
	{
		super(address,portNumber,commandString);
	}

	/**
	 * Parse a string returned from the autoguider over the telnet connection.
	 * @see #replyString
	 * @see #parsedReplyString
	 * @see #parsedReplyOk
	 */
	public void parseReplyString()
	{
		super.parseReplyString();
		if(parsedReplyOk == false)
		{
			parsedReplyDouble = 0.0;
			return;
		}
		try
		{
			parsedReplyDouble = Double.parseDouble(parsedReplyString);
		}
		catch(Exception e)
		{
			parsedReplyOk = false;
			parsedReplyDouble = 0.0;
		}
	}

	/**
	 * Return the parsed reply.
	 * @return The parsed double.
	 * @see #parsedReplyDouble
	 */
	public double getParsedReplyDouble()
	{
		return parsedReplyDouble;
	}
}

//
// $Log: not supported by cvs2svn $
//
