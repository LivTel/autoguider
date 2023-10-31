// AgGUIGuideStatusThread.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/gui/AgGUIGuideStatusThread.java,v 1.1 2023-08-08 08:55:11 cjm Exp $
package ngat.autoguider.gui;

import java.lang.*;
import java.net.*;

import javax.swing.*;

import ngat.autoguider.command.*;
import ngat.autoguider.gui.*;

/**
 * The AgGUIGuideStatusThread extends Thread.
 * The thread sleeps for a pre determined length.
 * It then queries the autoguider guide status.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class AgGUIGuideStatusThread extends Thread
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: AgGUIGuideStatusThread.java,v 1.1 2023-08-08 08:55:11 cjm Exp $");
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
	private boolean useGuideCadenceUpdateTime = false;
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
	 * A constructor for this class. Calls the parent constructor. Sets the parent reference.
	 * Sets the auto-update time.
	 * @param p The parent object.
	 */
	public AgGUIGuideStatusThread(AgGUI p)
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


	public void setUseGuideCadenceUpdateTime(boolean b)
	{
		useGuideCadenceUpdateTime = b;
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
	 * Run method. 
	 * This thread runs in a loop until quit is true.  The thread then sleeps for the updateTime,
	 * @see #quit
	 * @see #updateTime
	 * @see #agAddress
	 * @see #agPortNumber
	 * @see #statusUpdateListener
	 * @see #sendStatusGuideExposureLength
	 * @see #sendStatusGuideCadence
	 * @see #sendStatusObjectCount
	 * @see #sendStatusGuideLastObject
	 */
	public void run()
	{

		agAddress = parent.getAgAddress();
		agPortNumber = parent.getAgPortNumber();
		while(!quit)
		{
			// guide exposure length
			sendStatusGuideExposureLength();
			// guide loop cadence
			sendStatusGuideCadence();
			// number of objects detected
			sendStatusObjectCount();
			// objects detected
			//sendStatusObjectList();
			// Last guide object used to send a centroid to the TCS/SDB
			sendStatusGuideLastObject();
			// diddly if(useGuideCadenceUpdateTime) update updateTime
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
		// reset data to unknown
		if(statusUpdateListener != null)
		{
			statusUpdateListener.setGuideExposureLength("Unknown");
			statusUpdateListener.setGuideCadence("Unknown");
			statusUpdateListener.setGuideObjectCount("Unknown");
			setGuideObjectBlank();
		}
	}

	/**
	 * Quit method. Sets quit to true, so that the run method will exit.
	 */
	public void quit()
	{
		quit = true;
	}

	/**
	 * Send "status guide exposure_length" command and evaluate result. Update relevant Swing GUI.
	 * @see #statusUpdateListener
	 * @see #agAddress
	 * @see #agPortNumber
	 */
	protected void sendStatusGuideExposureLength()
	{
		StatusGuideExposureLengthCommand guideExposureLengthCommand = null;
		boolean retval;

		parent.log(1,"sendStatusGuideExposureLength:Sending 'status guide exposure_length'");
		guideExposureLengthCommand = new StatusGuideExposureLengthCommand();
		guideExposureLengthCommand.setAddress(agAddress);
		guideExposureLengthCommand.setPortNumber(agPortNumber);
		try
		{
			guideExposureLengthCommand.sendCommand();
			if(guideExposureLengthCommand.getParsedReplyOK())
			{
				if(statusUpdateListener != null)
				{
					statusUpdateListener.
					   setGuideExposureLength(guideExposureLengthCommand.getGuideExposureLength());
				}
				parent.log(1,"sendStatusGuideExposureLength:'status guide exposure_length' "+
					   "returned:"+guideExposureLengthCommand.getGuideExposureLength());
			}
			else
			{
				parent.error(this.getClass().getName()+
				             ":sendStatusGuideExposureLength:Sending 'status guide exposure_Length' "+
					     "failed and returned:"+guideExposureLengthCommand.getReply());
				if(statusUpdateListener != null)
					statusUpdateListener.setGuideExposureLength("Unknown");
			}
		}
		catch(Exception e)
		{
			parent.error(this.getClass().getName()+":sendStatusGuideExposureLength:"+
				     "Sending 'status guide exposure_length' failed:",e);
			if(statusUpdateListener != null)
				statusUpdateListener.setGuideExposureLength("Unknown");
		}
	}

	/**
	 * Send "status guide cadence" command and evaluate result. Update relevant Swing GUI.
	 * @see #statusUpdateListener
	 * @see #agAddress
	 * @see #agPortNumber
	 */
	protected void sendStatusGuideCadence()
	{
		StatusGuideCadenceCommand guideCadenceCommand = null;
		boolean retval;

		parent.log(1,"sendStatusGuideCadence:Sending 'status guide cadence'");
		guideCadenceCommand = new StatusGuideCadenceCommand();
		guideCadenceCommand.setAddress(agAddress);
		guideCadenceCommand.setPortNumber(agPortNumber);
		try
		{
			guideCadenceCommand.sendCommand();
			if(guideCadenceCommand.getParsedReplyOK())
			{
				if(statusUpdateListener != null)
					statusUpdateListener.setGuideCadence(guideCadenceCommand.getGuideCadence());
				parent.log(1,"sendStatusGuideCadence:'status guide cadence' "+
					   "returned:"+guideCadenceCommand.getGuideCadence());
			}
			else
			{
				parent.error(this.getClass().getName()+
			       	     ":sendStatusGuideCadence:Sending 'status guide cadence' failed and returned:"+
					     guideCadenceCommand.getReply());
				if(statusUpdateListener != null)
					statusUpdateListener.setGuideCadence("Unknown");
			}
		}
		catch(Exception e)
		{
			parent.error(this.getClass().getName()+
				     ":sendStatusGuideCadence:Sending 'status guide cadence' failed:",e);
			if(statusUpdateListener != null)
				statusUpdateListener.setGuideCadence("Unknown");
		}
	}

	/**
	 * Send "status object count" command and evaluate result. Update relevant Swing GUI.
	 * @see #statusUpdateListener
	 * @see #agAddress
	 * @see #agPortNumber
	 */
	protected void sendStatusObjectCount()
	{
		StatusObjectCountCommand objectCountCommand = null;
		boolean retval;

		parent.log(1,"sendStatusObjectCount:Sending 'status object count'");
		objectCountCommand = new StatusObjectCountCommand();
		objectCountCommand.setAddress(agAddress);
		objectCountCommand.setPortNumber(agPortNumber);
		try
		{
			objectCountCommand.sendCommand();
			if(objectCountCommand.getParsedReplyOK())
			{
				if(statusUpdateListener != null)
					statusUpdateListener.setGuideObjectCount(objectCountCommand.getObjectCount());
				parent.log(1,"sendStatusObjectCount:'status object count' "+
					   "returned:"+objectCountCommand.getObjectCount());
			}
			else
			{
				parent.error(this.getClass().getName()+
					   ":sendStatusObjectCount:Sending 'status object count' failed and returned:"+
					     objectCountCommand.getReply());
				if(statusUpdateListener != null)
					statusUpdateListener.setGuideObjectCount("Unknown");
			}
		}
		catch(Exception e)
		{
			parent.error(this.getClass().getName()+":sendStatusObjectCount:"+
				     "Sending 'status object count' failed:",e);
			if(statusUpdateListener != null)
				statusUpdateListener.setGuideObjectCount("Unknown");
		}
	}

	/**
	 * Send "status object list" command and evaluate result. Update relevant Swing GUI.
	 * @see #statusUpdateListener
	 * @see #agAddress
	 * @see #agPortNumber
	 * @see #setGuideObject
	 */
	protected void sendStatusObjectList()
	{
		StatusObjectListCommand objectListCommand = null;
		StatusObjectListObject object = null;
		boolean retval;

		parent.log(1,"sendStatusObjectList:Sending 'status object list'");
		objectListCommand = new StatusObjectListCommand();
		objectListCommand.setAddress(agAddress);
		objectListCommand.setPortNumber(agPortNumber);
		try
		{
			objectListCommand.sendCommand();
			if(objectListCommand.getParsedReplyOK())
			{
				if(objectListCommand.getObjectListCount() == 1)
				{
					object = objectListCommand.getObject(0);
					setGuideObject(object);
					parent.log(1,"sendStatusObjectCount:'status object list' "+
						   "returned:"+object);
				}
				else
				{
					setGuideObjectBlank();
					parent.log(1,"sendStatusObjectCount:'status object list' "+
						   "returned wrong number of objects:"+
						   objectListCommand.getObjectListCount());
				}
			}
			else
			{
				parent.error(this.getClass().getName()+
					     ":sendStatusObjectList:Sending 'status object list' failed and returned:"+
					     objectListCommand.getReply());
				setGuideObjectBlank();
			}
		}
		catch(Exception e)
		{
			parent.error(this.getClass().getName()+":sendStatusObjectList:"+
				     "Sending 'status object list' failed:",e);
			setGuideObjectBlank();
		}
	}

	/**
	 * Send "status guide last_object" command and evaluate result. Update relevant Swing GUI.
	 * @see #statusUpdateListener
	 * @see #agAddress
	 * @see #agPortNumber
	 * @see #setGuideObject
	 */
	protected void sendStatusGuideLastObject()
	{
		StatusGuideLastObjectCommand lastObjectCommand = null;
		boolean retval;

		parent.log(1,"sendStatusGuideLastObject:Sending 'status guide last_object'");
		lastObjectCommand = new StatusGuideLastObjectCommand();
		lastObjectCommand.setAddress(agAddress);
		lastObjectCommand.setPortNumber(agPortNumber);
		try
		{
			lastObjectCommand.sendCommand();
			if(lastObjectCommand.getParsedReplyOK())
			{
				setGuideObject(lastObjectCommand);
				parent.log(1,"sendStatusGuideLastObject:'status guide last_object' "+
					   "returned:"+lastObjectCommand);
			}
			else
			{
				parent.error(this.getClass().getName()+
					     ":sendStatusObjectList:Sending 'status guide last_object' failed and returned:"+
					     lastObjectCommand.getReply());
				setGuideObjectBlank();
			}
		}
		catch(Exception e)
		{
			parent.error(this.getClass().getName()+":sendStatusGuideLastObject:"+
				     "Sending 'status guide last_object' failed:",e);
			setGuideObjectBlank();
		}
	}

	/**
	 * Method to set the guide object Swing data.
	 * @param object The object to set from, this can be null to set the fields to unknown.
	 * @see #statusUpdateListener
	 * @see ngat.autoguider.command.StatusObjectListObject
	 */
	protected void setGuideObject(StatusObjectListObject object)
	{
		if(statusUpdateListener != null)
		{
			if(object != null)
			{
				statusUpdateListener.setGuideObjectId(object.getId());
				statusUpdateListener.setGuideObjectCCDPositionX(object.getCCDPositionX());
				statusUpdateListener.setGuideObjectCCDPositionY(object.getCCDPositionY());
				statusUpdateListener.setGuideObjectBufferPositionX(object.getBufferPositionX());
				statusUpdateListener.setGuideObjectBufferPositionY(object.getBufferPositionY());
			}
			else
			{
				statusUpdateListener.setGuideObjectId("Unknown");
				statusUpdateListener.setGuideObjectCCDPositionX("Unknown");
				statusUpdateListener.setGuideObjectCCDPositionY("Unknown");
				statusUpdateListener.setGuideObjectBufferPositionX("Unknown");
				statusUpdateListener.setGuideObjectBufferPositionY("Unknown");
			}
		}
	}

	/**
	 * Method to set the guide object Swing data.
	 * @param object The object to set from, this can be null to set the fields to unknown.
	 * @see #statusUpdateListener
	 * @see ngat.autoguider.command.StatusGuideLastObjectCommand
	 */
	protected void setGuideObject(StatusGuideLastObjectCommand object)
	{
		if(statusUpdateListener != null)
		{
			if(object != null)
			{
				statusUpdateListener.setGuideObjectId("Unknown");
				statusUpdateListener.setGuideObjectCCDPositionX(object.getCCDPositionX());
				statusUpdateListener.setGuideObjectCCDPositionY(object.getCCDPositionY());
				statusUpdateListener.setGuideObjectBufferPositionX(object.getBufferPositionX());
				statusUpdateListener.setGuideObjectBufferPositionY(object.getBufferPositionY());
			}
			else
			{
				statusUpdateListener.setGuideObjectId("Unknown");
				statusUpdateListener.setGuideObjectCCDPositionX("Unknown");
				statusUpdateListener.setGuideObjectCCDPositionY("Unknown");
				statusUpdateListener.setGuideObjectBufferPositionX("Unknown");
				statusUpdateListener.setGuideObjectBufferPositionY("Unknown");
			}
		}
	}
	
	/**
	 * Method to set the guide object Swing data to unknowns.
	 * @see #statusUpdateListener
	 */
	protected void setGuideObjectBlank()
	{
		if(statusUpdateListener != null)
		{
			statusUpdateListener.setGuideObjectId("Unknown");
			statusUpdateListener.setGuideObjectCCDPositionX("Unknown");
			statusUpdateListener.setGuideObjectCCDPositionY("Unknown");
			statusUpdateListener.setGuideObjectBufferPositionX("Unknown");
			statusUpdateListener.setGuideObjectBufferPositionY("Unknown");
		}
	}
}
