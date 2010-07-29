// StatusObjectListCommand.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/command/StatusObjectListCommand.java,v 1.2 2010-07-29 09:21:17 cjm Exp $
package ngat.autoguider.command;

import java.io.*;
import java.lang.*;
import java.net.*;
import java.util.*;

/**
 * The "status object list" command is an extension of the Command, and returns 
 * a list of objects detected by the object detection software on the autoguider CCD.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
 */
public class StatusObjectListCommand extends Command implements Runnable
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: StatusObjectListCommand.java,v 1.2 2010-07-29 09:21:17 cjm Exp $");	/**
	 * The command to send to the autoguider.
	 */
	public final static String COMMAND_STRING = new String("status object list");
	/**
	 * List of StatusObjectListObject instances with a detected autoguider object in each.
	 * @see StatusObjectListObject
	 */
	protected List objectList = null;

	/**
	 * Default constructor.
	 * @see IntegerReplyCommand
	 * @see #commandString
	 * @see #COMMAND_STRING
	 */
	public StatusObjectListCommand()
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
	public StatusObjectListCommand(String address,int portNumber) throws UnknownHostException
	{
		super(address,portNumber,COMMAND_STRING);
	}

	/**
	 * Parse a string returned from the autoguider over the telnet connection.
	 * This string has a header line of the form:
	 * <pre>
	 * Id Frame_Number Index CCD_Pos_X  CCD_Pos_Y Buffer_Pos_X Buffer_Pos_Y Total_Counts Number_of_Pixels Peak_Counts Is_Stellar FWHM_X FWHM_Y
	 * </pre>
	 * and a set of objects lines of the format:
	 * <pre>
	 * %6d %6d %6d %6.2f %6.2f %6.2f %6.2f %6.2f %6d %6.2f %s %6.2f %6.2f
	 * </pre>
	 * See autoguider_object.c, Autoguider_Object_List_Get_Object_List_String for the code that
	 * generates this string.
	 * @see #replyString
	 * @see #parsedReplyString
	 * @see #parsedReplyOk
	 * @see StatusObjectListObject
	 * @see StatusObjectListObject#parse
	 * @link http://ltdevsrv.livjm.ac.uk/~dev/autoguider/cdocs/autoguider_object.html#Autoguider_Object_List_Get_Object_List_String
	 */
	public void parseReplyString() throws Exception
	{
		StatusObjectListObject object = null;
		StringTokenizer lineTokeniser = null;
		String lineString = null;
		boolean firstLine;

		super.parseReplyString();
		if(parsedReplyOk == false)
		{
			objectList = null;
			return;
		}
		try
		{
			objectList = new Vector();
			firstLine = true;
			lineTokeniser = new StringTokenizer(parsedReplyString,"\n",false);
			while(lineTokeniser.hasMoreTokens())
			{
				lineString = lineTokeniser.nextToken();
				// Don't parse the header line...
				if(firstLine)
				{
					firstLine = false;	
				}
				else
				{
					object = new StatusObjectListObject();
					object.parse(lineString);
					objectList.add(object);
				}
			}
		}
		catch(Exception e)
		{
			e.printStackTrace(System.err);
			parsedReplyOk = false;
		        objectList = null;
		}
	}

	/**
	 * Get the list of detected objects in the autoguidier's object list.
	 * @return A list of the detected autoguider objects.
	 * @exception Exception Thrown if getting the data fails, either the run method failed to communicate
	 *         with the autoguider in some way, or the method was called before the command had completed.
	 * @see StatusObjectListObject
	 */
	public List getObjectList() throws Exception
	{
		if(parsedReplyOk)
			return objectList;
		else
		{
			if(runException != null)
				throw runException;
			else
				throw new Exception(this.getClass().getName()+":getObjectList:Unknown Error.");
		}
	}

	/**
	 * Get the number of detected objects in the autoguidier's object list.
	 * @return The number of detected autoguider objects.
	 * @exception Exception Thrown if getting the data fails, either the run method failed to communicate
	 *         with the autoguider in some way, or the method was called before the command had completed.
	 */
	public int getObjectListCount() throws Exception
	{
		if(parsedReplyOk)
			return objectList.size();
		else
		{
			if(runException != null)
				throw runException;
			else
				throw new Exception(this.getClass().getName()+":getObjectListCount:Unknown Error.");
		}
	}

	/**
	 * Get the specified etected object in the autoguidier's object list.
	 * @return The specified object.
	 * @exception Exception Thrown if getting the data fails, either the run method failed to communicate
	 *         with the autoguider in some way, or the method was called before the command had completed.
	 * @see StatusObjectListObject
	 */
	public StatusObjectListObject getObject(int index) throws Exception
	{
		StatusObjectListObject object = null;

		if(parsedReplyOk)
		{
			object = (StatusObjectListObject)(objectList.get(index));
			return object;
		}
		else
		{
			if(runException != null)
				throw runException;
			else
				throw new Exception(this.getClass().getName()+":getObject:Unknown Error.");
		}
	}

	/**
	 * Main test program.
	 * @param args The argument list.
	 */
	public static void main(String args[])
	{
		StatusObjectListCommand command = null;
		StatusObjectListObject object = null;
		List list = null;
		int portNumber = 1234;

		if(args.length != 2)
		{
			System.out.println("java ngat.autoguider.command.StatusObjectListCommand <hostname> <port number>");
			System.exit(1);
		}
		try
		{
			portNumber = Integer.parseInt(args[1]);
			command = new StatusObjectListCommand(args[0],portNumber);
			command.run();
			if(command.getRunException() != null)
			{
				System.err.println("StatusObjectListCommand: Command failed.");
				command.getRunException().printStackTrace(System.err);
				System.exit(1);
			}
			System.out.println("Finished:"+command.getCommandFinished());
			System.out.println("Reply Parsed OK:"+command.getParsedReplyOK());
			list = command.getObjectList();
			System.out.println("Object List:"+list);
			if(list != null)
			{
				for(int i = 0; i < list.size(); i++)
				{
					object = (StatusObjectListObject)(list.get(i));
					System.out.println(object.toString());
				}
			}
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
