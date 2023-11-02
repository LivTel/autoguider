// AgGUI.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/gui/AgGUI.java,v 1.1 2023-08-08 08:55:11 cjm Exp $
package ngat.autoguider.gui;

import java.lang.*;
import java.io.*;
import java.net.*;
import java.text.*;
import java.util.*;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.plaf.metal.*;

import ngat.sound.*;
import ngat.swing.*;
import ngat.util.*;
import ngat.util.logging.*;

/**
 * This class is the start point for the Autoguider GUI.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class AgGUI
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: AgGUI.java,v 1.1 2023-08-08 08:55:11 cjm Exp $");
	/**
	 * The stream to write error messages to - defaults to System.err.
	 */
	private PrintWriter errorStream = new PrintWriter(System.err,true);
	/**
	 * The stream to write log messages to - defaults to System.out.
	 */
	private PrintWriter logStream = new PrintWriter(System.out,true);
	/** 
	 * Simple date format in a style similar to ISO 8601.
	 */
	private SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd 'T' HH:mm:ss.SSS z");
	/**
	 * Top level frame for the program.
	 */
	private JFrame frame = null;
	/**
	 * The property filename to load status properties from.
	 * If NULL, the status's internal default filename is used.
	 */
	private String propertyFilename = null;
	/**
	 * The port number to send autoguider commands to.
	 */
	private int agPortNumber = 0;
	/**
	 * The ip address of the machine the autoguider is running on, to send autoguider commands to.
	 */
	private InetAddress agAddress = null;
	/**
	 * Whether to optimise swing to work over a remote connection or not.
	 * This means turning off double buffering, and making scroll panes scroll simply.
	 * See http://developer.java.sun.com/developer/bugParade/bugs/4204845.html
	 * for details.
	 */
	private boolean remoteX = false;
	/**
	 * Whether the GUI provides audio feedback to various events.
	 */
	private boolean audioFeedback = true;
	/**
	 * Instance of sound thread used to control sounds.
	 */
	private SoundThread audioThread = null;
	/**
	 * The logging logger.
	 */
	protected Logger logLogger = null;
	/**
	 * The error logger.
	 */
	protected Logger errorLogger = null;
	/**
	 * The filter used to filter messages sent to the logging logger.
	 * @see #logLogger
	 */
	protected BitFieldLogFilter logFilter = null;
	/**
	 * AgGUI status information.
	 */
	private AgGUIStatus status = null;
	/**
	 * Label for whether the autoguider is fielding.
	 */
	private JLabel isFieldingLabel = null;
	/**
	 * Label for whether the autoguider is guiding.
	 */
	private JLabel isGuidingLabel = null;
	/**
	 * Label for guide exposure length.
	 */
	private JLabel guideExposureLengthLabel = null;
	/**
	 * Label for guide cadence.
	 */
	private JLabel guideCadenceLabel = null;
	/**
	 * Label for guide count.
	 */
	private JLabel guideObjectCountLabel = null;
	/**
	 * Label for guide id.
	 */
	private JLabel guideObjectIdLabel = null;
	/**
	 * Label for guide frame number.
	 */
	private JLabel guideObjectFrameNumberLabel = null;
	/**
	 * Label for guide index.
	 */
	private JLabel guideObjectIndexLabel = null;
	/**
	 * Label for guide CCD Position X.
	 */
	private JLabel guideObjectCCDPositionXLabel = null;
	/**
	 * Label for guide CCD Position Y.
	 */
	private JLabel guideObjectCCDPositionYLabel = null;
	/**
	 * Label for guide Buffer Position X.
	 */
	private JLabel guideObjectBufferPositionXLabel = null;
	/**
	 * Label for guide Buffer Position Y.
	 */
	private JLabel guideObjectBufferPositionYLabel = null;
	/**
	 * Label for guide Total Counts.
	 */
	private JLabel guideObjectTotalCountsLabel = null;
	/**
	 * Label for guide peak counts.
	 */
	private JLabel guideObjectPeakCountsLabel = null;
	/**
	 * Label for guide cadence.
	 */
	private JLabel guideObjectNoOfPixelsLabel = null;
	/**
	 * Label for guide is stellar.
	 */
	private JLabel guideObjectIsStellarLabel = null;
	/**
	 * Label for guide FWHM X.
	 */
	private JLabel guideObjectFWHMXLabel = null;
	/**
	 * Label for guide FWHM Y.
	 */
	private JLabel guideObjectFWHMYLabel = null;
	/**
	 * Label to put object Median background value into.
	 */
	private JLabel objectMedianLabel = null;
	/**
	 * Label to put object Mean background value into.
	 */
	private JLabel objectMeanLabel = null;
	/**
	 * Label to put object background standard deviation value into.
	 */
	private JLabel objectBackgroundStandardDeviationLabel = null;
	/**
	 * Label to put object Threshold value into.
	 */
	private JLabel objectThresholdLabel = null;
	/**
	 * Should we poll field status when fielding.
	 */
	private JCheckBoxMenuItem fieldStatusActiveMenuItem = null;
	/**
	 * Should we poll guide status when guiding.
	 */
	private JCheckBoxMenuItem guideStatusActiveMenuItem = null;
	/**
	 * Thread for generating basic status.
	 */
	private AgGUIStatusThread statusThread = null;

	/**
	 * Default constructor.
	 */
	public AgGUI()
	{
		super();
		propertyFilename = null;
	}

	/**
	 * The main routine, called when IcsGUI is executed.
	 * This does the following:
	 * <ul>
	 * <li>Sets up the look and feel.
	 * <li>Starts a splash screen.
	 * <li>Constructs an instance of IcsGUI.
	 * <li>Calls parsePropertyFilenameArgument, which changes the property filename initStatus uses if
	 *     a relevant argument is in the argument list.
	 * <li>Calls initStatus, to load the properties.
	 * <li>Calls parseArguments, to look at all the other command line arguments.
	 * <li>Calls initGUI, to create the screens.
	 * <li>Calls run.
	 * </ul>
	 * @see #parsePropertyFilenameArgument
	 * @see #initStatus
	 * @see #parseArguments
	 * @see #initGUI
	 * @see #initAudio
	 * @see #run
	 */
	public static void main(String[] args)
	{
		MetalLookAndFeel metalLookAndFeel = null;

		System.err.println("Starting AgGUI...");
	// setup LTMetalTheme
		System.err.println("Setting LT Metal Theme...");
		try
		{
			Class cl = Class.forName(UIManager.getCrossPlatformLookAndFeelClassName());
			metalLookAndFeel = (MetalLookAndFeel)(cl.newInstance());
			metalLookAndFeel.setCurrentTheme(new LTMetalTheme());
			UIManager.setLookAndFeel(metalLookAndFeel);
		}
		catch(Exception e)
		{
			System.err.println("UIManager.setLookAndFeel failed:"+e);
		}
	// create and display a splash screen whilst we are initialising
		System.err.println("Starting splash screen...");
		ngat.swing.SplashScreen splashScreen = new ngat.swing.SplashScreen(350,200,"lt.gif",
			"Liverpool John Moores University",null);
		splashScreen.show(30000);
	// construct gui main object
		System.err.println("Constructing AgGUI...");
		AgGUI agGUI = new AgGUI();
		try
		{
			System.err.println("Parsing Property Filename.");
			agGUI.parsePropertyFilenameArgument(args);
			System.err.println("Initialising Status.");
			agGUI.initStatus();
			System.err.println("Initialising Loggers.");
			agGUI.initLoggers();
			agGUI.log(1,"Parsing command line arguments.");
			agGUI.parseArguments(args);
		}
		catch(Exception e)
		{
			agGUI.error(e.toString());
			System.exit(1);
		}
		agGUI.log(1,"Intialising GUI.");
		agGUI.initGUI();
		agGUI.log(1,"Intialising Audio.");
		agGUI.initAudio();
		agGUI.log(1,"Calling main run.");
		agGUI.run();
		agGUI.log(1,"End of main.");
	}
	
	/**
	 * Initialise the program status, from the configuration files.
	 * Creates the status object and initialises it.
	 * @exception FileNotFoundException Thrown if a configuration filename not found.
	 * @exception IOException Thrown if a configuration file has an IO error during reading.
	 * @see #propertyFilename
	 * @see #status
	 * @see AgGUIStatus#load
	 */
	private void initStatus() throws FileNotFoundException,IOException
	{
		status = new AgGUIStatus();
		if(propertyFilename != null)
			status.load(propertyFilename);
		else
			status.load();
	}

	/**
	 * Initialise log handlers. 
	 * @see #initLogHandlers
	 * @see #copyLogHandlers
	 * @see #errorLogger
	 * @see #logLogger
	 * @see #logFilter
	 */
	protected void initLoggers()
	{
	// errorLogger setup
		errorLogger = LogManager.getLogger("error");
		initLogHandlers(errorLogger);
		errorLogger.setLogLevel(Logging.ALL);
	// ngat.net error loggers
		//copyLogHandlers(errorLogger,LogManager.getLogger("ngat.net.TCPServer"),null);
	// logLogger setup
		logLogger = LogManager.getLogger("log");
		initLogHandlers(logLogger);
		logLogger.setLogLevel(Logging.ALL);
		logFilter = new BitFieldLogFilter(status.getLogLevel());
		logLogger.setFilter(logFilter);
		//copyLogHandlers(logLogger,LogManager.getLogger("ngat.ccd.CCDLibrary"),logFilter);
	}

	/**
	 * Method to create and add all the handlers for the specified logger.
	 * These handlers are in the status properties:
	 * "ag_gui.log."+l.getName()+".handler."+index+".name" retrieves the relevant class name
	 * for each handler.
	 * @param l The logger.
	 * @see #initFileLogHandler
	 * @see #initConsoleLogHandler
	 */
	protected void initLogHandlers(Logger l)
	{
		LogHandler handler = null;
		String handlerName = null;
		int index = 0;

		do
		{
			handlerName = status.getProperty("ag_gui.log."+l.getName()+".handler."+index+".name");
			if(handlerName != null)
			{
				try
				{
					handler = null;
					if(handlerName.equals("ngat.util.logging.FileLogHandler"))
					{
						handler = initFileLogHandler(l,index);
					}
					else if(handlerName.equals("ngat.util.logging.ConsoleLogHandler"))
					{
						handler = initConsoleLogHandler(l,index);
					}
					else if(handlerName.equals("ngat.util.logging.MulticastLogHandler"))
					{
						handler = initMulticastLogHandler(l,index);
					}
					else if(handlerName.equals("ngat.util.logging.MulticastLogRelay"))
					{
						handler = initMulticastLogRelay(l,index);
					}
					else
					{
						error("initLogHandlers:Unknown handler:"+handlerName);
					}
					if(handler != null)
					{
						handler.setLogLevel(Logging.ALL);
						l.addHandler(handler);
					}
				}
				catch(Exception e)
				{
					error("initLogHandlers:Adding Handler failed:",e);
				}
				index++;
			}
		}
		while(handlerName != null);
	}

	/**
	 * Routine to add a FileLogHandler to the specified logger.
	 * This method expects either 3 or 6 constructor parameters to be in the status properties.
	 * If there are 6 parameters, we create a record limited file log handler with parameters:
	 * <ul>
	 * <li><b>param.0</b> is the filename.
	 * <li><b>param.1</b> is the formatter class name.
	 * <li><b>param.2</b> is the record limit in each file.
	 * <li><b>param.3</b> is the start index for file suffixes.
	 * <li><b>param.4</b> is the end index for file suffixes.
	 * <li><b>param.5</b> is a boolean saying whether to append to files.
	 * </ul>
	 * If there are 3 parameters, we create a time period file log handler with parameters:
	 * <ul>
	 * <li><b>param.0</b> is the filename.
	 * <li><b>param.1</b> is the formatter class name.
	 * <li><b>param.2</b> is the time period, either 'HOURLY_ROTATION','DAILY_ROTATION' or 'WEEKLY_ROTATION'.
	 * </ul>
	 * @param l The logger to add the handler to.
	 * @param index The index in the property file of the handler we are adding.
	 * @return A LogHandler of the relevant class is returned, if no exception occurs.
	 * @exception NumberFormatException Thrown if the numeric parameters in the properties
	 * 	file are not valid numbers.
	 * @exception FileNotFoundException Thrown if the specified filename is not valid in some way.
	 * @exception NullPointerException Thrown if a property value is null.
	 * @exception IllegalArgumentException Thrown if a property value is not legal.
	 * @see #status
	 * @see #initLogFormatter
	 * @see AgGUIStatus#getProperty
	 * @see AgGUIStatus#getPropertyInteger
	 * @see AgGUIStatus#getPropertyBoolean
	 * @see AgGUIStatus#propertyContainsKey
	 * @see AgGUIStatus#getPropertyLogHandlerTimePeriod
	 */
	protected LogHandler initFileLogHandler(Logger l,int index) throws NumberFormatException,
	          FileNotFoundException, NullPointerException, IllegalArgumentException
	{
		LogFormatter formatter = null;
		LogHandler handler = null;
		String fileName;
		int recordLimit,fileStart,fileLimit,timePeriod;
		boolean append;

		fileName = status.getProperty("ag_gui.log."+l.getName()+".handler."+index+".param.0");
		formatter = initLogFormatter("ag_gui.log."+l.getName()+".handler."+index+".param.1");
		// if we have more then 3 parameters, we are using a recordLimit FileLogHandler
		// rather than a time period log handler.
		if(status.propertyContainsKey("ag_gui.log."+l.getName()+".handler."+index+".param.3"))
		{
			recordLimit = status.getPropertyInteger("ag_gui.log."+l.getName()+".handler."+index+".param.2");
			fileStart = status.getPropertyInteger("ag_gui.log."+l.getName()+".handler."+index+".param.3");
			fileLimit = status.getPropertyInteger("ag_gui.log."+l.getName()+".handler."+index+".param.4");
			append = status.getPropertyBoolean("ag_gui.log."+l.getName()+".handler."+index+".param.5");
			handler = new FileLogHandler(fileName,formatter,recordLimit,fileStart,fileLimit,append);
		}
		else
		{
			// This is a time period log handler.
			timePeriod = status.getPropertyLogHandlerTimePeriod("ag_gui.log."+l.getName()+".handler."+
									    index+".param.2");
			handler = new FileLogHandler(fileName,formatter,timePeriod);
		}
		return handler;
	}

	/**
	 * Routine to add a MulticastLogHandler to the specified logger.
	 * The parameters to the constructor are stored in the status properties:
	 * <ul>
	 * <li>param.0 is the multicast group name i.e. "228.0.0.1".
	 * <li>param.1 is the port number i.e. 5000.
	 * <li>param.2 is the formatter class name.
	 * </ul>
	 * @param l The logger to add the handler to.
	 * @param index The index in the property file of the handler we are adding.
	 * @return A LogHandler of the relevant class is returned, if no exception occurs.
	 * @exception IOException Thrown if the multicast socket cannot be created for some reason.
	 */
	protected LogHandler initMulticastLogHandler(Logger l,int index) throws IOException
	{
		LogFormatter formatter = null;
		LogHandler handler = null;
		String groupName = null;
		int portNumber;

		groupName = status.getProperty("ag_gui.log."+l.getName()+".handler."+index+".param.0");
		portNumber = status.getPropertyInteger("ag_gui.log."+l.getName()+".handler."+index+".param.1");
		formatter = initLogFormatter("ag_gui.log."+l.getName()+".handler."+index+".param.2");
		handler = new MulticastLogHandler(groupName,portNumber,formatter);
		return handler;
	}

	/**
	 * Routine to add a MulticastLogRelay to the specified logger.
	 * The parameters to the constructor are stored in the status properties:
	 * <ul>
	 * <li>param.0 is the multicast group name i.e. "228.0.0.1".
	 * <li>param.1 is the port number i.e. 5000.
	 * </ul>
	 * @param l The logger to add the handler to.
	 * @param index The index in the property file of the handler we are adding.
	 * @return A LogHandler of the relevant class is returned, if no exception occurs.
	 * @exception IOException Thrown if the multicast socket cannot be created for some reason.
	 */
	protected LogHandler initMulticastLogRelay(Logger l,int index) throws IOException
	{
		LogHandler handler = null;
		String groupName = null;
		int portNumber;

		groupName = status.getProperty("ag_gui.log."+l.getName()+".handler."+index+".param.0");
		portNumber = status.getPropertyInteger("ag_gui.log."+l.getName()+".handler."+index+".param.1");
		handler = new MulticastLogRelay(groupName,portNumber);
		return handler;
	}

	/**
	 * Routine to add a ConsoleLogHandler to the specified logger.
	 * The parameters to the constructor are stored in the status properties:
	 * <ul>
	 * <li>param.0 is the formatter class name.
	 * </ul>
	 * @param l The logger to add the handler to.
	 * @param index The index in the property file of the handler we are adding.
	 * @return A LogHandler of class FileLogHandler is returned, if no exception occurs.
	 */
	protected LogHandler initConsoleLogHandler(Logger l,int index)
	{
		LogFormatter formatter = null;
		LogHandler handler = null;

		formatter = initLogFormatter("ag_gui.log."+l.getName()+".handler."+index+".param.0");
		handler = new ConsoleLogHandler(formatter);
		return handler;
	}

	/**
	 * Method to create an instance of a LogFormatter, given a property name
	 * to retrieve it's details from. If the property does not exist, or the class does not exist
	 * or an instance cannot be instansiated we try to return a ngat.util.logging.BogstanLogFormatter.
	 * @param propertyName A property name, present in the status's properties, 
	 * 	which has a value of a valid LogFormatter sub-class name. i.e.
	 * 	<pre>ag_gui.log.log.handler.0.param.1 =ngat.util.logging.BogstanLogFormatter</pre>
	 * @return An instance of LogFormatter is returned.
	 */
	protected LogFormatter initLogFormatter(String propertyName)
	{
		LogFormatter formatter = null;
		String formatterName = null;
		Class formatterClass = null;

		formatterName = status.getProperty(propertyName);
		if(formatterName == null)
		{
			error("initLogFormatter:NULL formatter for:"+propertyName);
			formatterName = "ngat.util.logging.BogstanLogFormatter";
		}
		try
		{
			formatterClass = Class.forName(formatterName);
		}
		catch(ClassNotFoundException e)
		{
			error("initLogFormatter:Unknown class formatter:"+formatterName+
				" from property "+propertyName);
			formatterClass = BogstanLogFormatter.class;
		}
		try
		{
			formatter = (LogFormatter)formatterClass.newInstance();
		}
		catch(Exception e)
		{
			error("initLogFormatter:Cannot create instance of formatter:"+formatterName+
				" from property "+propertyName);
			formatter = (LogFormatter)new BogstanLogFormatter();
		}
	// set better date format if formatter allows this.
	// Note we really need LogFormatter to generically allow us to do this
		if(formatter instanceof BogstanLogFormatter)
		{
			BogstanLogFormatter blf = (BogstanLogFormatter)formatter;

			blf.setDateFormat(new SimpleDateFormat("yyyy/MM/dd HH:mm:ss.SSS z"));
		}
		if(formatter instanceof SimpleLogFormatter)
		{
			SimpleLogFormatter slf = (SimpleLogFormatter)formatter;

			slf.setDateFormat(new SimpleDateFormat("yyyy/MM/dd HH:mm:ss.SSS z"));
		}
		return formatter;
	}

	/**
	 * Method to copy handlers from one logger to another.
	 * @param inputLogger The logger to copy handlers from.
	 * @param outputLogger The logger to copy handlers to.
	 * @param lf The log filter to apply to the output logger. If this is null, the filter is not set.
	 */
	protected void copyLogHandlers(Logger inputLogger,Logger outputLogger,LogFilter lf)
	{
		LogHandler handlerList[] = null;
		LogHandler handler = null;

		handlerList = inputLogger.getHandlers();
		for(int i = 0; i < handlerList.length; i++)
		{
			handler = handlerList[i];
			outputLogger.addHandler(handler);
		}
		outputLogger.setLogLevel(inputLogger.getLogLevel());
		if(lf != null)
			outputLogger.setFilter(lf);
	}

	/**
	 * Initialise the program.
	 * Creates the frame and creates the widgets associated with it.
	 * Creates the menu bar.
	 * @see #remoteX
	 * @see #frame
	 * @see #initMenuBar
	 * @see #initMainPanel
	 */
	private void initGUI()
	{
		Image image = null;
		String titleString = null;

		log(1,"initGUI:Started.");
		System.err.println("initGUI:Started.");
        // optimise for remote X?
		if(remoteX)
		{
			RepaintManager.currentManager(null).setDoubleBufferingEnabled(false);
		}
	// create the frame level layout manager
		GridBagLayout gridBagLayout = new GridBagLayout();
        	GridBagConstraints gridBagCon = new GridBagConstraints();
	// Create the top-level container.
		if(agAddress != null)
		{
			titleString = new String(status.getProperty("ag_gui.title")+":"+agAddress.getHostName());
		}
		else
		{
			titleString = new String(status.getProperty("ag_gui.title")+"None");
		}
		frame = new MinimumSizeFrame(titleString,new Dimension(400,300));
	// set icon image
		image = Toolkit.getDefaultToolkit().getImage(status.getProperty("ag_gui.icon.filename"));
		frame.setIconImage(image);

		frame.getContentPane().setLayout(gridBagLayout);
		initMenuBar();
		/*
		 * An easy way to put space between a top-level container
		 * and its contents is to put the contents in a JPanel
		 * that has an "empty" border.
		 */
		JPanel panel = new JPanel();

		initMainPanel(panel);

	// Add the JPanel to the frame.
		gridBagCon.gridx = GridBagConstraints.RELATIVE;
		gridBagCon.gridy = GridBagConstraints.RELATIVE;
		gridBagCon.gridwidth = GridBagConstraints.REMAINDER;
		gridBagCon.gridheight = GridBagConstraints.REMAINDER;
		gridBagCon.fill = GridBagConstraints.BOTH;
		gridBagCon.weightx = 1.0;
		gridBagCon.weighty = 1.0;
		gridBagCon.anchor = GridBagConstraints.NORTHWEST;
		gridBagLayout.setConstraints(panel,gridBagCon);
		frame.getContentPane().add(panel);

	//Finish setting up the frame, and show it.
		frame.addWindowListener(new AgGUIWindowListener(this));
		log(1,"initGUI:Finished.");
		System.err.println("initGUI:Finished.");
 	}
 	
	/**
	 * Initialise the main panel. This consists of setting the panel layout, and then 
	 * adding the status panel and the log panel.
	 * @see #initStatusPanel
	 * @see #initGuidePanel
	 * @see #initObjectStatsPanel
	 */
	private void initMainPanel(JPanel panel)
	{
		log(1,"initMainPanel:Started.");
		System.err.println("initMainPanel:Started.");
 	// create the frame level layout manager
		GridBagLayout gridBagLayout = new GridBagLayout();
	// setup panel
		panel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		panel.setLayout(gridBagLayout);
		panel.setMinimumSize(new Dimension(600,320));
		panel.setPreferredSize(new Dimension(600,320));
		panel.setMaximumSize(new Dimension(1024,1024));
	//  status labels
		initStatusPanel(panel,gridBagLayout);
	// guide panel
		initGuidePanel(panel,gridBagLayout);
	// object stats panel
		initObjectStatsPanel(panel,gridBagLayout);
	}

	/**
	 * Initialise status area.
	 * @param panel The panel to put the status panel in.
	 * @param gridBagLayout The grid bag layout to set constraints for, before adding things
	 * 	to the panel.
	 */
	private void initStatusPanel(JPanel panel,GridBagLayout gridBagLayout)
	{
		JPanel isStatusPanel = new JPanel();
		JLabel label = null;
        	GridBagConstraints gridBagCon = new GridBagConstraints();

		log(1,"initStatusPanel:Started.");
		System.err.println("initStatusPanel:Started.");
 		isStatusPanel.setLayout(new GridLayout(0,4));
		isStatusPanel.setMinimumSize(new Dimension(600,40));
		isStatusPanel.setPreferredSize(new Dimension(600,40));
		isStatusPanel.setMaximumSize(new Dimension(1024,40));
	// isFielding
		label = new JLabel("Fielding:");
		isStatusPanel.add(label);
		isFieldingLabel = new JLabel("Unknown");
		isFieldingLabel.setForeground(Color.RED);
		isStatusPanel.add(isFieldingLabel);
	// isGuiding
		label = new JLabel("Guiding:");
		isStatusPanel.add(label);
		isGuidingLabel = new JLabel("Unknown");
		isGuidingLabel.setForeground(Color.RED);
		isStatusPanel.add(isGuidingLabel);
	// add border
		isStatusPanel.setBorder(new TitledSmallerBorder("Status"));
	// these constraints mean that the GridBagLayout can't alter the size of isStatusPanel
		gridBagCon.gridx = 0;
		gridBagCon.gridy = 0;
		gridBagCon.gridwidth = GridBagConstraints.REMAINDER;
		gridBagCon.gridheight = 1;
		gridBagCon.fill = GridBagConstraints.HORIZONTAL;
		gridBagCon.weightx = 1.0;
		gridBagCon.weighty = 0.0;
		gridBagCon.anchor = GridBagConstraints.NORTH;
		gridBagLayout.setConstraints(isStatusPanel,gridBagCon);
		panel.add(isStatusPanel);
	}

	/**
	 * Initialise guide area.
	 * @param panel The panel to put the status panel in.
	 * @param gridBagLayout The grid bag layout to set constraints for, before adding things
	 * 	to the panel.
	 */
	private void initGuidePanel(JPanel panel,GridBagLayout gridBagLayout)
	{
		JPanel guidePanel = new JPanel();
		JLabel label = null;
        	GridBagConstraints gridBagCon = new GridBagConstraints();

		log(1,"initGuidePanel:Started.");
		System.err.println("initGuidePanel:Started.");
 		guidePanel.setLayout(new GridLayout(0,4));
		guidePanel.setMinimumSize(new Dimension(400,200));
		guidePanel.setPreferredSize(new Dimension(1024,200));
		guidePanel.setMaximumSize(new Dimension(1024,200));
	// Guide Exposure Length
		label = new JLabel("Guide Exposure Length:");
		guidePanel.add(label);
		guideExposureLengthLabel = new JLabel("Unknown");
		guidePanel.add(guideExposureLengthLabel);
	// Guide Cadence
		label = new JLabel("Guide Cadence:");
		guidePanel.add(label);
		guideCadenceLabel = new JLabel("Unknown");
		guidePanel.add(guideCadenceLabel);
	// Object Count
		label = new JLabel("Object Count:");
		guidePanel.add(label);
		guideObjectCountLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectCountLabel);
	// Object Id
		label = new JLabel("Object Id:");
		guidePanel.add(label);
		guideObjectIdLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectIdLabel);
	// Object Frame Number
		label = new JLabel("Object Frame Number:");
		guidePanel.add(label);
		guideObjectFrameNumberLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectFrameNumberLabel);
	// Object Index
		label = new JLabel("Object Index:");
		guidePanel.add(label);
		guideObjectIndexLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectIndexLabel);
	// Object CCD Position X
		label = new JLabel("Object CCD Position X:");
		guidePanel.add(label);
		guideObjectCCDPositionXLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectCCDPositionXLabel);
	// Object CCD Position Y
		label = new JLabel("Object CCD Position Y:");
		guidePanel.add(label);
		guideObjectCCDPositionYLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectCCDPositionYLabel);
	// Object Buffer Position X
		label = new JLabel("Object Buffer Position X:");
		guidePanel.add(label);
		guideObjectBufferPositionXLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectBufferPositionXLabel);
	// Object Buffer Position Y
		label = new JLabel("Object Buffer Position Y:");
		guidePanel.add(label);
		guideObjectBufferPositionYLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectBufferPositionYLabel);
	// Object Total Counts
		label = new JLabel("Object Integrated Counts:");
		guidePanel.add(label);
		guideObjectTotalCountsLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectTotalCountsLabel);
	// Object Peak Counts
		label = new JLabel("Object Peak Counts:");
		guidePanel.add(label);
		guideObjectPeakCountsLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectPeakCountsLabel);
	// Object Number of Pixels
		label = new JLabel("Object Number of Pixels:");
		guidePanel.add(label);
		guideObjectNoOfPixelsLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectNoOfPixelsLabel);
	// Object isStellar
		label = new JLabel("Object Is Stellar:");
		guidePanel.add(label);
		guideObjectIsStellarLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectIsStellarLabel);
	// Object FWHM X
		label = new JLabel("Object FWHM X:");
		guidePanel.add(label);
		guideObjectFWHMXLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectFWHMXLabel);
	// Object FWHM Y
		label = new JLabel("Object FWHM Y:");
		guidePanel.add(label);
		guideObjectFWHMYLabel = new JLabel("Unknown");
		guidePanel.add(guideObjectFWHMYLabel);

	// add border
		guidePanel.setBorder(new TitledSmallerBorder("Guide"));
	// these constraints mean that the GridBagLayout can't alter the size of guidePanel
		gridBagCon.gridx = 0;
		gridBagCon.gridy = 1;
		gridBagCon.gridwidth = GridBagConstraints.REMAINDER;
		gridBagCon.gridheight = 1;
		gridBagCon.fill = GridBagConstraints.HORIZONTAL;
		gridBagCon.weightx = 1.0;
		gridBagCon.weighty = 0.0;
		gridBagCon.anchor = GridBagConstraints.NORTH;
		gridBagLayout.setConstraints(guidePanel,gridBagCon);
		panel.add(guidePanel);
	}
	
	/**
	 * Initialise object stats area.
	 * @param panel The panel to put the object stats panel in.
	 * @param gridBagLayout The grid bag layout to set constraints for, before adding things
	 * 	to the panel.
	 * @see #objectMedianLabel
	 * @see #objectMeanLabel
	 * @see #objectBackgroundStandardDeviationLabel
	 * @see #objectThresholdLabel
	 */
	private void initObjectStatsPanel(JPanel panel,GridBagLayout gridBagLayout)
	{
		JPanel objectStatsPanel = new JPanel();
		JLabel label = null;
        	GridBagConstraints gridBagCon = new GridBagConstraints();

		log(1,"initObjectStatsPanel:Started.");
		System.err.println("initObjectStatsPanel:Started.");
 		objectStatsPanel.setLayout(new GridLayout(0,4));
		objectStatsPanel.setMinimumSize(new Dimension(400,60));
		objectStatsPanel.setPreferredSize(new Dimension(1024,60));
		objectStatsPanel.setMaximumSize(new Dimension(1024,60));
	// Median Background Counts
		label = new JLabel("Median Background:");
		objectStatsPanel.add(label);
		objectMedianLabel = new JLabel("Unknown");
		objectStatsPanel.add(objectMedianLabel);
	// Mean Background Counts
		label = new JLabel("Mean Background:");
		objectStatsPanel.add(label);
		objectMeanLabel = new JLabel("Unknown");
		objectStatsPanel.add(objectMeanLabel);
	// Background Standard Deviation
		label = new JLabel("Background S.D.:");
		objectStatsPanel.add(label);
		objectBackgroundStandardDeviationLabel = new JLabel("Unknown");
		objectStatsPanel.add(objectBackgroundStandardDeviationLabel);
	// Threshold
		label = new JLabel("Threshold:");
		objectStatsPanel.add(label);
		objectThresholdLabel = new JLabel("Unknown");
		objectStatsPanel.add(objectThresholdLabel);
	// add border
		objectStatsPanel.setBorder(new TitledSmallerBorder("Object Stats"));
	// these constraints mean that the GridBagLayout can't alter the size of objectStatsPanel
		gridBagCon.gridx = 0;
		gridBagCon.gridy = 2;
		gridBagCon.gridwidth = GridBagConstraints.REMAINDER;
		gridBagCon.gridheight = 1;
		gridBagCon.fill = GridBagConstraints.HORIZONTAL;
		gridBagCon.weightx = 1.0;
		gridBagCon.weighty = 0.0;
		gridBagCon.anchor = GridBagConstraints.NORTH;
		gridBagLayout.setConstraints(objectStatsPanel,gridBagCon);
		panel.add(objectStatsPanel);
	}

	/**
	 * Method to initialise the top-level menu bar.
	 */
	private void initMenuBar()
	{
		JMenuBar menuBar;
		JMenu menu, submenu;
		JMenuItem menuItem;
		AgGUIMenuItemListener menuItemListener = new AgGUIMenuItemListener(this);

	//Create the menu bar.
		menuBar = new JMenuBar();
		frame.setJMenuBar(menuBar);

	//Build the general menu.
		menu = new JMenu("File");
		menu.setMnemonic(KeyEvent.VK_F);
		menu.getAccessibleContext().setAccessibleDescription("The File Menu");
		menuBar.add(menu);
        // Audio
		menuItem = new JCheckBoxMenuItem("Audio",true);// default value?
		menuItem.getAccessibleContext().setAccessibleDescription("Turns audio feedback on/off");
		menuItem.addActionListener(menuItemListener);
		menu.add(menuItem);
        // Exit
		menuItem = new JMenuItem("Exit",KeyEvent.VK_X);
		menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X,ActionEvent.CTRL_MASK));
		menuItem.getAccessibleContext().setAccessibleDescription("Exits the program");
		menuItem.addActionListener(menuItemListener);
		menu.add(menuItem);
	// Build the status menu.
		menu = new JMenu("Status");
		menu.setMnemonic(KeyEvent.VK_S);
		menu.getAccessibleContext().setAccessibleDescription("The Status Menu");
		menuBar.add(menu);
        // Cadence
		menuItem = new JMenuItem("Status Cadence",KeyEvent.VK_C);
		menuItem.getAccessibleContext().setAccessibleDescription("Set the status query interval");
		menuItem.addActionListener(menuItemListener);
		menu.add(menuItem);
        // Status Active
		menuItem = new JCheckBoxMenuItem("Status Active",true);// default value?
		menuItem.getAccessibleContext().setAccessibleDescription("Enable/Disable Basic Status");
		menuItem.addActionListener(menuItemListener);
		menu.add(menuItem);
        // Field Status Active
		fieldStatusActiveMenuItem = new JCheckBoxMenuItem("Field Status Active",true);// default value?
		fieldStatusActiveMenuItem.getAccessibleContext().
			setAccessibleDescription("Enable/Disable Field Status");
		fieldStatusActiveMenuItem.addActionListener(menuItemListener);
		menu.add(fieldStatusActiveMenuItem);
        // Guide Status Active
		guideStatusActiveMenuItem = new JCheckBoxMenuItem("Guide Status Active",true);// default value?
		guideStatusActiveMenuItem.getAccessibleContext().
			setAccessibleDescription("Enable/Disable Guide Status");
		guideStatusActiveMenuItem.addActionListener(menuItemListener);
		menu.add(guideStatusActiveMenuItem);
	}

	/**
	 * Initialise the audio feedback thread, and pre-load relevant samples.
	 * Assumes the properties are already setup.
	 * @see #audioThread
	 * @see #audioFeedback
	 */
	private void initAudio()
	{
		Thread t = null;
		File sampleFile = null;
		String dirString = null;
		String sampleName = null;
		String sampleFilename = null;
		boolean done = false;
		int index;

		// init ialse threads and SoundThread instance
		audioThread = new SoundThread();
		t = new Thread(audioThread);
		t.start();
		dirString = status.getProperty("ag_gui.audio.sample.directory");
		// load relevant samples
		done = false;
		index = 0;
		while(done == false)
		{
			sampleName = status.getProperty("ag_gui.audio.sample.name."+index);
			sampleFilename = new String(dirString+
						    status.getProperty("ag_gui.audio.sample.filename."+index));
			if(sampleName != null)
			{
				sampleFile = new File(sampleFilename);
				try
				{
					audioThread.load(sampleName,sampleFile);
				}
				catch(MalformedURLException e)
				{
					error(this.getClass().getName()+":initAudio:Failed to load:"+
					      sampleFilename+":"+e);
					// non-fatal error - continue
				}
				index++;
			}
			else
				done = true;
		}// end while
		// call initial sample
		if(audioFeedback)
			status.play(audioThread,"ag_gui.audio.event.welcome");
	}

	/**
 	 * The run routine. Sets the main frame visible.
	 * Starts the update thread.
	 * @see #frame
	 * @see #startUpdateThread
	 */
	private void run()
	{
		frame.pack();
		frame.setVisible(true);
		startUpdateThread();
	}

	/**
	 * Method to start the update thread.
	 * @see #statusThread
	 * @see AgGUIStatusThread
	 */
	public void startUpdateThread()
	{
		AgGUIStatusUpdater statusUpdater = null;
		int updateTime;

		if(statusThread == null)
		{
			log(1,"Starting new status thread.");
			statusThread = new AgGUIStatusThread(this);
			updateTime = status.getPropertyInteger("ag_gui.update_thread.cadence");
			statusThread.setUpdateTime(updateTime);
			statusUpdater = new AgGUIStatusUpdater(this);
			statusThread.setUpdateListener(statusUpdater);
			statusThread.start();
			log(1,"Status thread Started.");
		}
		else
			log(1,"Status thread already running.");
	}

	/**
	 * Method to stop the update thread.
	 * @see #statusThread
	 * @see AgGUIStatusThread#quit
	 */
	public void stopUpdateThread()
	{
		if(statusThread != null)
		{
		        log(1,"Quiting status thread.");
			statusThread.quit();
			log(1,"Status thread ordered to quit.");
			statusThread = null;
		}
		else
			log(1,"Status thread not running.");
	}

	/**
	 * Set the contents of the fielding label.
	 * @param s The string to use.
	 * @see #isFieldingLabel
	 */
	public void setIsFieldingLabel(String s)
	{
		isFieldingLabel.setText(s);
	}

	/**
	 * Set the contents of the guiding label.
	 * @param s The string to use.
	 * @see #isGuidingLabel
	 */
	public void setIsGuidingLabel(String s)
	{
		isGuidingLabel.setText(s);
	}

	/**
	 * Set the contents of the guide exposure length label.
	 * @param s The string to use.
	 * @see #guideExposureLengthLabel
	 */
	public void setGuideExposureLengthLabel(String s)
	{
		guideExposureLengthLabel.setText(s);
	}

	/**
	 * Set the contents of the guide cadence label.
	 * @param s The string to use.
	 * @see #guideCadenceLabel
	 */
	public void setGuideCadenceLabel(String s)
	{
		guideCadenceLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object count label.
	 * @param s The string to display in the label.
	 * @see #guideObjectCountLabel
	 */
	public void setGuideObjectCountLabel(String s)
	{
		guideObjectCountLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object id label.
	 * @param s The string to display in the label.
	 * @see #guideObjectIdLabel
	 */
	public void setGuideObjectIdLabel(String s)
	{
		guideObjectIdLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object frame number label.
	 * @param s The string to display in the label.
	 * @see #guideObjectFrameNumberLabel
	 */
	public void setGuideObjectFrameNumberLabel(String s)
	{
		guideObjectFrameNumberLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object index label.
	 * @param s The string to display in the label.
	 * @see #guideObjectIndexLabel
	 */
	public void setGuideObjectIndexLabel(String s)
	{
		guideObjectIndexLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object CCD X Position label.
	 * @param s The string to display in the label.
	 * @see #guideObjectCCDPositionXLabel
	 */
	public void setGuideObjectCCDPositionXLabel(String s)
	{
		guideObjectCCDPositionXLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object CCD Y Position label.
	 * @param s The string to display in the label.
	 * @see #guideObjectCCDPositionYLabel
	 */
	public void setGuideObjectCCDPositionYLabel(String s)
	{
		guideObjectCCDPositionYLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object buffer X position label.
	 * @param s The string to display in the label.
	 * @see #guideObjectBufferPositionXLabel
	 */
	public void setGuideObjectBufferPositionXLabel(String s)
	{
		guideObjectBufferPositionXLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object buffer Y position label.
	 * @param s The string to display in the label.
	 * @see #guideObjectBufferPositionYLabel
	 */
	public void setGuideObjectBufferPositionYLabel(String s)
	{
		guideObjectBufferPositionYLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object total count label.
	 * @param s The string to display in the label.
	 * @see #guideObjectTotalCountsLabel
	 */
	public void setGuideObjectTotalCountsLabel(String s)
	{
		guideObjectTotalCountsLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object peak counts label.
	 * @param s The string to display in the label.
	 * @see #guideObjectPeakCountsLabel
	 */
	public void setGuideObjectPeakCountsLabel(String s)
	{
		guideObjectPeakCountsLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object no. of pixels label.
	 * @param s The string to display in the label.
	 * @see #guideObjectNoOfPixelsLabel
	 */
	public void setGuideObjectNoOfPixelsLabel(String s)
	{
		guideObjectNoOfPixelsLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object is stellar label.
	 * @param s The string to display in the label.
	 * @see #guideObjectIsStellarLabel
	 */
	public void setGuideObjectIsStellarLabel(String s)
	{
		guideObjectIsStellarLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object FWHM X label.
	 * @param s The string to display in the label.
	 * @see #guideObjectFWHMXLabel
	 */
	public void setGuideObjectFWHMXLabel(String s)
	{
		guideObjectFWHMXLabel.setText(s);
	}

	/**
	 * Set the contents of the guide object FWHM Y label.
	 * @param s The string to display in the label.
	 * @see #guideObjectFWHMYLabel
	 */
	public void setGuideObjectFWHMYLabel(String s)
	{
		guideObjectFWHMYLabel.setText(s);
	}

	/**
	 * Set the contents of the object status Median Counts label.
	 * @param s The string to display in the label.
	 * @see #objectMedianLabel
	 */
	public void setObjectMedianLabel(String s)
	{
		objectMedianLabel.setText(s);
	}

	/**
	 * Set the contents of the object status Mean Counts label.
	 * @param s The string to display in the label.
	 * @see #objectMeanLabel
	 */
	public void setObjectMeanLabel(String s)
	{
		objectMeanLabel.setText(s);
	}

	/**
	 * Set the contents of the object status Background Standard Deviation label.
	 * @param s The string to display in the label.
	 * @see #objectBackgroundStandardDeviationLabel
	 */
	public void setObjectBackgroundStandardDeviationLabel(String s)
	{
		objectBackgroundStandardDeviationLabel.setText(s);
	}

	/**
	 * Set the contents of the object status Threshold label.
	 * @param s The string to display in the label.
	 * @see #objectThresholdLabel
	 */
	public void setObjectThresholdLabel(String s)
	{
		objectThresholdLabel.setText(s);
	}

	/**
	 * Main program exit routine. Waits for command to complete before exiting, if n is zero,
	 * otherwise just terminates.
	 * @param n The return exit value to return to the calling shell/program.
	 */
	public void exit(int n)
	{
		System.exit(n);
	}

	/**
	 * Method to set the level of logging filtered by the log level filter.
	 * @param level An integer, used as a bit-field. Each bit set will allow
	 * any messages with that level bit set to be logged. e.g. 0 logs no messages,
	 * 127 logs any messages with one of the first 8 bits set.
	 * @see #logFilter
	 */
	public void setLogLevelFilter(int level)
	{
		logFilter.setLevel(level);
	}

	/**
	 * Routine to write the string to the relevant logger. If the relevant logger has not been
	 * created yet the error gets written to System.out.
	 * @param level The level of logging this message belongs to.
	 * @param s The string to write.
	 * @see #logLogger
	 */
	public void log(int level,String s)
	{
		if(logLogger != null)
			logLogger.log(level,s);
		else
		{
			if((status.getLogLevel()&level) > 0)
				System.out.println(s);
		}
	}

	/**
	 * Routine to write the string to the relevant logger. If the relevant logger has not been
	 * created yet the error gets written to System.err.
	 * @param s The string to write.
	 * @see #errorLogger
	 */
	public void error(String s)
	{
		if(errorLogger != null)
			errorLogger.log(1,s);
		else
			System.err.println(s);
	}

	/**
	 * Routine to write the string to the relevant logger. If the relevant logger has not been
	 * created yet the error gets written to System.err.
	 * @param s The string to write.
	 * @param e An exception that caused the error to occur.
	 * @see #errorLogger
	 */
	public void error(String s,Exception e)
	{
		if(errorLogger != null)
		{
			errorLogger.log(1,s,e);
			errorLogger.dumpStack(1,e);
		}
		else
			System.err.println(s+e);
	}

	/**
	 * Return the GUI's parent frame.
	 * @see #frame
	 */
	public JFrame getFrame()
	{
		return frame;
	}

	/**
	 * Return the status object
	 * @see #status
	 * @see AgGUIStatus
	 */
	public AgGUIStatus getStatus()
	{
		return status;
	}

	/**
	 * Return the Autoguider control systems's control computer address.
	 * @return The current internet address of the Autoguider control systems's control computer.
	 * @see #agAddress
	 */
	public InetAddress getAgAddress()
	{
		return agAddress;
	}

	/**
	 * Return the Autoguider command port number.
	 * @return The current autoguider command port number.
	 * @see #agPortNumber
	 */
	public int getAgPortNumber()
	{
		return agPortNumber;
	}

	/**
	 * Method to get whether to optimise swing components for running over a remote X server.
	 * @return Returns the value of the remoteX field.
	 * @see #remoteX
	 */
	public boolean getRemoteX()
	{
		return remoteX;
	}

	/**
	 * Method to set whether to to provide audio feedback to various GUI events.
	 * @param b True if we want audio feedback, false otherwise.
	 * @see #audioFeedback
	 */
	public void setAudioFeedback(boolean b)
	{
		audioFeedback = b;
	}

	/**
	 * Method to get whether to provide audio feedback to various GUI events.
	 * @return Retrurns the value of the audioFeedback field.
	 * @see #audioFeedback
	 */
	public boolean getAudioFeedback()
	{
		return audioFeedback;
	}

	/**
	 * Method to get the sound thread instance, used to play audio data.
	 * @return Retrurns the value of the audioThread field.
	 * @see #audioThread
	 */
	public SoundThread getAudioThread()
	{
		return audioThread;
	}

	/**
	 * Get the status thread.
	 * @return The current value of statusThread, which may be null.
	 * @see #statusThread
	 */
	public AgGUIStatusThread getStatusThread()
	{
		return statusThread;
	}

	/**
	 * Get the field status active checkbox.
	 * @see #fieldStatusActiveMenuItem
	 */
	public JCheckBoxMenuItem getFieldStatusActiveMenuItem()
	{
		return fieldStatusActiveMenuItem;
	}

	/**
	 * Get the guide status active checkbox.
	 * @see #guideStatusActiveMenuItem
	 */
	public JCheckBoxMenuItem getGuideStatusActiveMenuItem()
	{
		return guideStatusActiveMenuItem;
	}

	/**
	 * This routine parses arguments passed into the GUI. It only looks for property filename argument
	 * config, which is needed before the status is inited, and therfore needs a special parseArguments
	 * method (as other arguments affect the status).
	 * @param args The list of arguments to parse.
	 * @see #propertyFilename
	 * @see #parseArguments
	 */
	private void parsePropertyFilenameArgument(String[] args) throws NumberFormatException,UnknownHostException
	{
	// look through the argument list.
		for(int i = 0; i < args.length;i++)
		{
			if(args[i].equals("-config")||args[i].equals("-co"))
			{
				if((i+1)< args.length)
				{
					propertyFilename = args[i+1];
					i++;
				}
				else
					error("-config requires a filename");
			}
		}
	}


	/**
	 * This routine parses arguments passed into the GUI. It gets some default arguments from the configuration
	 * file. These are the Autoguider numbers, and the Autoguider internet address.
	 * It then reads through the arguments in the list. This routine will stop the program 
	 * if a `-help' is one of the arguments. It ignores the property filename argument, as this is parsed in
	 * parsePropertyFilenameArgument.
	 * @param args The list of arguments to parse.
	 * @see #agAddress
	 * @see #agPortNumber
	 * @see #remoteX
	 * @see #help
	 * @see #parsePropertyFilenameArgument
	 */
	private void parseArguments(String[] args) throws NumberFormatException,UnknownHostException
	{
	// initialise port numbers from properties file
		try
		{
			agPortNumber = status.getPropertyInteger("ag_gui.net.autoguider.port_number");
		}
		catch(NumberFormatException e)
		{
			error(this.getClass().getName()+":parseArguments:initialsing port number:"+e);
			throw e;
		}
	// initialise address's from properties file
		try
		{
			agAddress = InetAddress.getByName(status.getProperty("ag_gui.net.autoguider.address"));
		}
		catch(UnknownHostException e)
		{
			error(this.getClass().getName()+":illegal internet address:"+e);
			throw e;
		}
	// look through the argument list.
		for(int i = 0; i < args.length;i++)
		{
			if(args[i].equals("-config")||args[i].equals("-co"))
			{
				// do nothing here, see parsePropertyFilenameArgument
				// but move over filename argument.
				if((i+1)< args.length)
					i++;
			}
			else if(args[i].equals("-log"))
			{
				if((i+1)< args.length)
				{
					status.setLogLevel(Integer.parseInt(args[i+1]));
					i++;
				}
				else
					error("-log requires a log level");
			}
			else if(args[i].equals("-agport"))
			{
				if((i+1)< args.length)
				{
					agPortNumber = Integer.parseInt(args[i+1]);
					i++;
				}
				else
					error("-agport requires a port number");
			}
			else if(args[i].equals("-agip")||args[i].equals("-agaddress"))
			{
				if((i+1)< args.length)
				{
					try
					{
						agAddress = InetAddress.getByName(args[i+1]);
					}
					catch(UnknownHostException e)
					{
						error(this.getClass().getName()+
							":illegal Autoguider address:"+args[i+1]+":"+e);
					}
					i++;
				}
				else
					error("-agaddress requires a valid ip address");
			}
			else if(args[i].equals("-h")||args[i].equals("-help"))
			{
				help();
				System.exit(0);
			}
			else if(args[i].equals("-remotex"))
			{
				remoteX = true;
			}
			else
				error(this.getClass().getName()+"'"+args[i]+"' not a recognised option.");
		}
	}

	/**
	 * Help message routine.
	 */
	private void help()
	{
		System.out.println(this.getClass().getName()+" Help:");
		System.out.println("IcsGUI is the `Instrument Control System Graphical User Interface'.");
		System.out.println("Options are:");
		System.out.println("\t-[co]nfig <filename> - Change the property file to load.");
		System.out.println("\t-[agip]|[agaddress] <address> - Address to send Autoguider commands to.");
		System.out.println("\t-agport <port number> - Autoguider server port number.");
		System.out.println("\t-log <log level> - log level.");
		System.out.println("\t-remotex - Optimise swing for running over a remote X connection.");
	}
}
//
// $Log: not supported by cvs2svn $
//
