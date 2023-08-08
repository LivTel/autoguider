// AgGUIWindowListener.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/gui/AgGUIWindowListener.java,v 1.1 2023-08-08 08:55:11 cjm Exp $
package ngat.autoguider.gui;

import java.lang.*;

import java.awt.event.*;

/**
 * This class is a sub-class of WindowAdapter. An instance of this class is attached to the AgGUI
 * main window to catch windowClosing events. This calls AgGUI's exit method to ensure the program
 * terminates nicely.
 * @see AgGUI#exit
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class AgGUIWindowListener extends WindowAdapter
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: AgGUIWindowListener.java,v 1.1 2023-08-08 08:55:11 cjm Exp $");
	/**
	 * The main program class instance. The instances exit method is called when a windowClosing 
	 * event occurs.
	 */
	private AgGUI parent = null;

	/**
	 * Constructor. The top-level class instance is passed in.
	 * @param p The main program instance.
	 * @see #parent
	 */
	public AgGUIWindowListener(AgGUI p)
	{
		parent = p;
	}

	/**
	 * Method called when a windowClosing event occurs. This just calls AgGUI.exit(0).
	 * @param e The window event that caused this method to be called.
	 * @see #parent
	 * @see AgGUI#exit
	 */
	public void windowClosing(WindowEvent e)
	{
		parent.exit(0);
	}
}
//
// $Log: not supported by cvs2svn $
//
