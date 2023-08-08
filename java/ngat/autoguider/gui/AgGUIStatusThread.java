// AgGUIStatusThread.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/gui/AgGUIStatusThread.java,v 1.1 2023-08-08 08:55:11 cjm Exp $
package ngat.autoguider.gui;

import java.lang.*;
import java.net.*;
import javax.swing.*;

import ngat.autoguider.command.*;
import ngat.autoguider.gui.*;

/**
 * The AgGUIStatusThread extends Thread.
 * The thread sleeps for a pre determined length.
 * It then queries the autoguider general status.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class AgGUIStatusThread extends Thread
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: AgGUIStatusThread.java,v 1.1 2023-08-08 08:55:11 cjm Exp $");
	/**
	 * The IcsGUI object.
	 */
	private AgGUI parent = null;
	/**
	 * The time to sleep (in milliseconds) between getting the status.
	 */
	private long updateTime = 0;
	/**
	 * Boolean to quit the thread.
	 */
	private boolean quit = false;
	/**
	 * The IP address of the autoguider control system.
	 */
	protected InetAddress agAddress = null;
	/**
	 * The port number of the autoguider control system text xontrol port.
	 */
	protected int agPortNumber = 0;
	/**
	 * Update listener.
	 * @see AgGUIStatusUpdateListener
	 */
	private AgGUIStatusUpdateListener statusUpdateListener = null;
	/**
	 * Thread used for polling field status.
	 */
	//private AgGUIFieldStatusThread fieldStatusThread = null;
	/**
	 * Thread used for polling guide status.
	 */
	private AgGUIGuideStatusThread guideStatusThread = null;

	protected boolean checkFieldStatusActive = true;
	protected boolean checkGuideStatusActive = true;


	/**
	 * A constructor for this class. Calls the parent constructor. Sets the parent reference.
	 * Sets the auto-update time.
	 * @param p The parent object.
	 */
	public AgGUIStatusThread(AgGUI p)
	{
		super();
		parent = p;
		quit = false;
	}

	/**
	 * Set how long to wait between calls.
	 * @param ms The sleep time in ms.
	 * @see #updateTime
	 */
	public void setUpdateTime(int ms)
	{
		updateTime = ms;
	}

	/**
	 * Set The update listener to call when new status is received
	 * @param l The listener
	 * @see #statusUpdateListener
	 * @see AgGUIStatusUpdateListener
	 */
	public void setUpdateListener(AgGUIStatusUpdateListener l)
	{
		statusUpdateListener = l;	
	}

	/**
	 * Method to set checkFieldStatusActive: used to determine whether to 
	 * query the autoguider as to whether the fielding is ongoing.
	 * @param b Boolean, if true do the query when this thread is running.
	 * @see #checkFieldStatusActive
	 * @see #run
	 */
	public void setCheckFieldStatusActive(boolean b)
	{
		checkFieldStatusActive = b;
	}

	/**
	 * Method to set checkGuideStatusActive: used to determine whether to 
	 * query the autoguider as to whether the guiding is ongoing.
	 * @param b Boolean, if true do the query when this thread is running.
	 * @see #checkGuideStatusActive
	 * @see #run
	 */
	public void setCheckGuideStatusActive(boolean b)
	{
		checkGuideStatusActive = b;
	}

	/**
	 * Run method. 
	 * This thread runs in a loop until quit is true.  The thread then sleeps for the updateTime,
	 * @see #quit
	 * @see #updateTime
	 * @see #agAddress
	 * @see #agPortNumber
	 * @see #checkFieldStatusActive
	 * @see #checkGuideStatusActive
	 * @see #stopGuideStatusThread
	 * @see #guideStatusThread
	 * @see ngat.autoguider.command.StatusFieldActiveCommand
	 * @see ngat.autoguider.command.StatusGuideActiveCommand
	 */
	public void run()
	{
		boolean fieldActive = false;
		boolean guideActive = false;

		agAddress = parent.getAgAddress();
		agPortNumber = parent.getAgPortNumber();
		while(!quit)
		{
		// status field active
			fieldActive = sendStatusFieldActive();
		// status guide active
			guideActive = sendStatusGuideActive();
		// sleep for a bit
			try
			{
				Thread.sleep(updateTime);
			}
			catch(InterruptedException e)
			{
				parent.error("The status thread was interruped(1):",e);
			}
		}// end while
		parent.log(1,"Status Thread:Terminating.");
		if(guideStatusThread != null)
			stopGuideStatusThread();
	}

	/**
	 * Quit method. Sets quit to true, so that the run method will exit.
	 */
	public void quit()
	{
		quit = true;
		parent.log(1,"Status Thread:Ordered to terminate.");
	}

	/**
	 * Method to start a guide status thread if the autoguider is guiding.
	 */
	protected void startGuideStatusThread()
	{
		JCheckBoxMenuItem guideStatusActiveMenuItem = null;
		int guideUpdateTime;

		guideStatusActiveMenuItem = parent.getGuideStatusActiveMenuItem();
		if(guideStatusActiveMenuItem.getState())
		{
			parent.log(1,"Status Thread:Start Guide Status Thread.");
			guideUpdateTime = parent.getStatus().
				getPropertyInteger("ag_gui.status.guide.thread.cadence.default");
			guideStatusThread = new AgGUIGuideStatusThread(parent);
			guideStatusThread.setUpdateTime(guideUpdateTime);
			guideStatusThread.setUseGuideCadenceUpdateTime(true);//diddly checkbox
			guideStatusThread.setUpdateListener(statusUpdateListener);
			guideStatusThread.start();
			parent.log(1,"Status Thread:Guide Status Thread Started.");
		}
		else
			parent.log(1,"Status Thread:Guide Status Thread NOT Started:Checkbox NOT set.");
	}

	/**
	 * Method to stop the update thread.
	 * @see #guideStatusThread
	 * @see AgGUIGuideStatusThread#quit
	 */
	public void stopGuideStatusThread()
	{
		if(guideStatusThread != null)
		{
		        parent.log(1,"Quiting guide status thread.");
			guideStatusThread.quit();
			parent.log(1,"Guide status thread ordered to quit.");
			guideStatusThread = null;
		}
		else
			parent.log(1,"Guide status thread not running.");
	}

	/**
	 * Send "status field active" command and evaluate result. Update relevant Swing GUI.
	 * Return state.
	 * @return A boolean, true if the autoguider if fielding, and false if it is not (or an error occured).
	 * @see #checkFieldStatusActive
	 * @see #statusUpdateListener
	 */
	protected boolean sendStatusFieldActive()
	{
		StatusFieldActiveCommand fieldActiveCommand = null;
		boolean retval;

		if(checkFieldStatusActive)
		{
			parent.log(1,"Status Thread:Sending 'status field active'");
			fieldActiveCommand = new StatusFieldActiveCommand();
			fieldActiveCommand.setAddress(agAddress);
			fieldActiveCommand.setPortNumber(agPortNumber);
			try
			{
				fieldActiveCommand.sendCommand();
				if(fieldActiveCommand.getParsedReplyOK())
				{
					if(statusUpdateListener != null)
						statusUpdateListener.setFieldActive(fieldActiveCommand.isFielding());
					retval = fieldActiveCommand.isFielding();
					parent.log(1,"Status Thread:'status field active' returned:"+
						   fieldActiveCommand.isFielding());
				}
				else
				{
					parent.error(this.getClass().getName()+
						     ":run:Sending 'status field active' failed and returned:"+
						     fieldActiveCommand.getReply());
					if(statusUpdateListener != null)
						statusUpdateListener.setFieldActive("Unknown");
					retval = false;
				}
			}
			catch(Exception e)
			{
				parent.error(this.getClass().getName()+":run:Sending 'status field active' failed:",e);
				if(statusUpdateListener != null)
					statusUpdateListener.setFieldActive("Unknown");
				retval = false;
			}
		}
		else
		{
			if(statusUpdateListener != null)
				statusUpdateListener.setFieldActive("Unknown");
			retval = false;
		}
		return retval;
	}

	/**
	 * Send "status guide active" command and evaluate result. Update relevant Swing GUI.
	 * Return state.
	 * @return A boolean, true if the autoguider if guiding, and false if it is not (or an error occured).
	 * @see #checkGuideStatusActive
	 * @see #statusUpdateListener
	 */
	protected boolean sendStatusGuideActive()
	{
		StatusGuideActiveCommand guideActiveCommand = null;
		boolean retval;

		if(checkGuideStatusActive)
		{
			parent.log(1,"Status Thread:Sending 'status guide active'");
			guideActiveCommand = new StatusGuideActiveCommand();
			guideActiveCommand.setAddress(agAddress);
			guideActiveCommand.setPortNumber(agPortNumber);
			try
			{
				guideActiveCommand.sendCommand();
				if(guideActiveCommand.getParsedReplyOK())
				{
					if(statusUpdateListener != null)
						statusUpdateListener.setGuideActive(guideActiveCommand.isGuiding());
					retval = guideActiveCommand.isGuiding();
					parent.log(1,"Status Thread:'status guide active' returned:"+
						   guideActiveCommand.isGuiding());
					if(guideActiveCommand.isGuiding()&& (guideStatusThread == null))
						startGuideStatusThread();
					if((guideActiveCommand.isGuiding() == false) && (guideStatusThread != null))
						stopGuideStatusThread();
				}
				else
				{
					parent.error(this.getClass().getName()+
						     ":run:Sending 'status guide active' failed and returned:"+
						     guideActiveCommand.getReply());
					if(statusUpdateListener != null)
						statusUpdateListener.setGuideActive("Unknown");
					retval = false;
				}
			}
			catch(Exception e)
			{
				parent.error(this.getClass().getName()+":run:Sending 'status guide active' failed:",e);
				if(statusUpdateListener != null)
					statusUpdateListener.setGuideActive("Unknown");
				retval = false;
			}
		}
		else
		{
			if(statusUpdateListener != null)
				statusUpdateListener.setGuideActive("Unknown");
			retval = false;
		}
		return retval;
	}

}
//
// $Log: not supported by cvs2svn $
//
