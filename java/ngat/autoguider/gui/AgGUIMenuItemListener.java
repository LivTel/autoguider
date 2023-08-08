// AgGUIMenuItemListener.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/gui/AgGUIMenuItemListener.java,v 1.1 2023-08-08 08:55:11 cjm Exp $
package ngat.autoguider.gui;

import java.lang.*;
import java.lang.reflect.*;
import java.io.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

/**
 * This class is an ActionListener for the AgGUI menu bar items.
 */
public class AgGUIMenuItemListener implements ActionListener
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: AgGUIMenuItemListener.java,v 1.1 2023-08-08 08:55:11 cjm Exp $");
	/**
	 * The parent to the menu item listener. The instance of the main program.
	 */
	private AgGUI parent = null;

	/**
	 * Constructor. This sets the parent to be main program class.
	 * @param p The parent object which owns the menu items.
	 * @see #parent
	 */
	public AgGUIMenuItemListener(AgGUI p)
	{
		super();

		parent = p;
	}
	/**
	 * Routine that is called when a menu item is pressed.
	 * If Exit is called, the program is terminated.
	 * @param event The event that caused this call to the listener.
	 */
	public void actionPerformed(ActionEvent event)
	{
		String commandString = event.getActionCommand();

		if(commandString.equals("Exit"))
		{
			parent.exit(0);
		}
		if(commandString.equals("Audio"))
		{
			JCheckBoxMenuItem cbmi = (JCheckBoxMenuItem)event.getSource();

			if(cbmi.getState())
			{
				parent.setAudioFeedback(true);
				parent.log(1,"Audio feedback enabled.");
			}
			else
			{
				parent.setAudioFeedback(false);
				parent.log(1,"Audio feedback disabled.");
			}
			return;
		}
		if(commandString.equals("Status Active"))
		{
			JCheckBoxMenuItem cbmi = (JCheckBoxMenuItem)event.getSource();

			if(cbmi.getState())
			{
				parent.startUpdateThread();
			}
			else
			{
				parent.stopUpdateThread();
			}
			return;
		}
		if(commandString.equals("Field Status Active"))
		{
			JCheckBoxMenuItem cbmi = (JCheckBoxMenuItem)event.getSource();

			if(parent.getStatusThread() != null)
				parent.getStatusThread().setCheckFieldStatusActive(cbmi.getState());
			return;
		}
		if(commandString.equals("Guide Status Active"))
		{
			JCheckBoxMenuItem cbmi = (JCheckBoxMenuItem)event.getSource();

			if(parent.getStatusThread() != null)
				parent.getStatusThread().setCheckGuideStatusActive(cbmi.getState());
			return;
		}
		if(commandString.equals("Status Cadence"))
		{
			parent.log(1,"Status Cadence not avilable yet.");
			JOptionPane.showMessageDialog((Component)null,
						      (Object)("The Status Cadence Dialog is not avilable yet."),
						      "Message",JOptionPane.INFORMATION_MESSAGE);
		}
	}
}
//
// $Log: not supported by cvs2svn $
//
