// AgGUIStatus.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/gui/AgGUIStatus.java,v 1.1 2023-08-08 08:55:11 cjm Exp $
package ngat.autoguider.gui;

import java.lang.*;
import java.io.*;
import java.util.*;

import ngat.sound.*;
import ngat.util.logging.FileLogHandler;

/**
 * This class holds status information for the AgGUI program.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class AgGUIStatus
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: AgGUIStatus.java,v 1.1 2023-08-08 08:55:11 cjm Exp $");
	/**
	 * File name containing properties for ccs gui.
	 */
	private final static String PROPERTY_FILE_NAME = new String("./ag_gui.properties");
	/**
	 * The logging level.
	 */
	private int logLevel = 1;//CcsConstants.CCS_LOG_LEVEL_NONE;
	/**
	 * A list of properties held in the properties file. This contains configuration information in ccs_gui
	 * that needs to be changed irregularily.
	 */
	private Properties properties = null;

	/**
	 * Constructor. Constrcuts the properties.
	 * @see #properties
	 */
	public AgGUIStatus()
	{
		super();
		properties = new Properties();
	}

	/**
	 * The load method for the class. This loads the property file from disc, using the default filename.
	 * @see #load(java.lang.String)
	 * @see #PROPERTY_FILE_NAME
	 * @exception FileNotFoundException Thrown if the filename called PROPERTY_FILE_NAME does not exist.
	 * @exception IOException Thrown if there is an error reading from PROPERTY_FILE_NAME.
	 */
	public void load() throws FileNotFoundException,IOException
	{
		load(PROPERTY_FILE_NAME);
	}

	/**
	 * The load method for the class. This loads the property file from disc, from the specified filename.
	 * @param filename The filename of a valid properties file to load.
	 * @see #properties
	 * @exception FileNotFoundException Thrown if the filename does not exist.
	 * @exception IOException Thrown if there is an error reading from the file.
	 */
	public void load(String filename) throws FileNotFoundException,IOException
	{
		FileInputStream fileInputStream = null;

		fileInputStream = new FileInputStream(filename);
		properties.load(fileInputStream);
		fileInputStream.close();
	}

	/**
	 * Set the logging level for Ccs GUI.
	 * @param level The level of logging.
	 */
	public synchronized void setLogLevel(int level)
	{
		logLevel = level;
	}

	/**
	 * Get the logging level for the autoguider GUI.
	 * @return The current log level.
	 * @see #logLevel
	 */	
	public synchronized int getLogLevel()
	{
		return logLevel;
	}

	/**
	 * Play a sample specified by the property.
	 * @param audioThread The audio thread to play the sample with.
	 * @param propertyName The name of a property key. The value contains a sample "name",
	 *      or a list of names is available from the property key (&lt;property key&gt;.&lt;N&gt;),
	 *      and a random name is selected.
	 */
	public void play(SoundThread audioThread,String propertyName)
	{
		Random random = null;
		List sampleNameList = null;
		String sampleName = null;
		int index;
		boolean done;

		sampleName = getProperty(propertyName);
		// if default property value does not exist, see if there is a list to select from.
		if(sampleName == null)
		{
			sampleNameList = new Vector();
			index = 0;
			done = false;
			while(done == false)
			{
				sampleName = getProperty(propertyName+"."+index);
				if(sampleName != null)
					sampleNameList.add(sampleName);
				index++;
				done = (sampleName == null);
			}
			if(sampleNameList.size() > 0)
			{
				random = new Random();
				index = random.nextInt(sampleNameList.size());
				sampleName = (String)(sampleNameList.get(index));
			}
		}
		audioThread.play(sampleName);
	}

	/**
	 * Method to return whether the loaded properties contain the specified keyword.
	 * Calls the proprties object containsKey method. Note assumes the properties object has been initialised.
	 * @param p The property key we wish to test exists.
	 * @return The method returnd true if the specified key is a key in out list of properties,
	 *         otherwise it returns false.
	 * @see #properties
	 */
	public boolean propertyContainsKey(String p)
	{
		return properties.containsKey(p);
	}

	/**
	 * Routine to get a properties value, given a key. Just calls the properties object getProperty routine.
	 * @param p The property key we want the value for.
	 * @return The properties value, as a string object.
	 * @see #properties
	 */
	public String getProperty(String p)
	{
		return properties.getProperty(p);
	}

	/**
	 * Routine to get a properties value, given a key. The value must be a valid integer, else a 
	 * NumberFormatException is thrown.
	 * @param p The property key we want the value for.
	 * @return The properties value, as an integer.
	 * @exception NumberFormatException If the properties value string is not a valid integer, this
	 * 	exception will be thrown when the Integer.parseInt routine is called.
	 * @see #properties
	 */
	public int getPropertyInteger(String p) throws NumberFormatException
	{
		String valueString = null;
		int returnValue = 0;

		valueString = properties.getProperty(p);
		returnValue = Integer.parseInt(valueString);
		return returnValue;
	}

	/**
	 * Routine to get a properties value, given a key. The value must be a valid long, else a 
	 * NumberFormatException is thrown.
	 * @param p The property key we want the value for.
	 * @return The properties value, as a long.
	 * @exception NumberFormatException If the properties value string is not a valid long, this
	 * 	exception will be thrown when the Long.parseLong routine is called.
	 * @see #properties
	 */
	public long getPropertyLong(String p) throws NumberFormatException
	{
		String valueString = null;
		long returnValue = 0;

		valueString = properties.getProperty(p);
		try
		{
			returnValue = Long.parseLong(valueString);
		}
		catch(NumberFormatException e)
		{
			// re-throw exception with more information e.g. keyword
			throw new NumberFormatException(this.getClass().getName()+":getPropertyLong:keyword:"+
				p+":valueString:"+valueString);
		}
		return returnValue;
	}

	/**
	 * Routine to get a properties value, given a key. The value must be a valid double, else a 
	 * NumberFormatException is thrown.
	 * @param p The property key we want the value for.
	 * @return The properties value, as an double.
	 * @exception NumberFormatException If the properties value string is not a valid double, this
	 * 	exception will be thrown when the Double.valueOf routine is called.
	 * @see #properties
	 */
	public double getPropertyDouble(String p) throws NumberFormatException
	{
		String valueString = null;
		Double returnValue = null;

		valueString = properties.getProperty(p);
		returnValue = Double.valueOf(valueString);
		return returnValue.doubleValue();
	}

	/**
	 * Routine to get a properties boolean value, given a key. The properties value should be either 
	 * "true" or "false".
	 * Boolean.valueOf is used to convert the string to a boolean value.
	 * @param p The property key we want the boolean value for.
	 * @return The properties value, as an boolean.
	 * @see #properties
	 */
	public boolean getPropertyBoolean(String p)
	{
		String valueString = null;
		Boolean b = null;

		valueString = properties.getProperty(p);
		b = Boolean.valueOf(valueString);
		return b.booleanValue();
	}

	/**
	 * Routine to get an integer representing a ngat.util.logging.FileLogHandler time period.
	 * The value of the specified property should contain either:'HOURLY_ROTATION', 'DAILY_ROTATION' or
	 * 'WEEKLY_ROTATION'.
	 * @param p The property key we want the time period value for.
	 * @return The properties value, as an FileLogHandler time period (actually an integer).
	 * @exception NullPointerException If the properties value string is null an exception is thrown.
	 * @exception IllegalArgumentException If the properties value string is not a valid time period,
	 *            an exception is thrown.
	 * @see #properties
	 */
	public int getPropertyLogHandlerTimePeriod(String p) throws NullPointerException, IllegalArgumentException
	{
		String valueString = null;
		int timePeriod = 0;
 
		valueString = properties.getProperty(p);
		if(valueString == null)
		{
			throw new NullPointerException(this.getClass().getName()+
						       ":getPropertyLogHandlerTimePeriod:keyword:"+
						       p+":Value was null.");
		}
		if(valueString.equals("HOURLY_ROTATION"))
			timePeriod = FileLogHandler.HOURLY_ROTATION;
		else if(valueString.equals("DAILY_ROTATION"))
			timePeriod = FileLogHandler.DAILY_ROTATION;
		else if(valueString.equals("WEEKLY_ROTATION"))
			timePeriod = FileLogHandler.WEEKLY_ROTATION;
		else
		{
			throw new IllegalArgumentException(this.getClass().getName()+
							   ":getPropertyLogHandlerTimePeriod:keyword:"+
							   p+":Illegal value:"+valueString+".");
		}
		return timePeriod;
	}

}
//
// $Log: not supported by cvs2svn $
//
