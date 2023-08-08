// AgGUIStatusUpdateListener.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/gui/AgGUIStatusUpdateListener.java,v 1.1 2023-08-08 08:55:11 cjm Exp $
package ngat.autoguider.gui;

import java.lang.*;

import ngat.autoguider.gui.*;
/**
 * The AgGUIStatusUpdateListener is an interface for status updates.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public interface AgGUIStatusUpdateListener 
{
	public void setFieldActive(boolean b);
	public void setFieldActive(String s);
	public void setGuideActive(boolean b);
	public void setGuideActive(String s);
	public void setGuideExposureLength(int i);
	public void setGuideExposureLength(String s);
	public void setGuideCadence(double d);
	public void setGuideCadence(String s);
	public void setGuideObjectCount(int c);
	public void setGuideObjectCount(String s);
	public void setGuideObjectId(int n);
	public void setGuideObjectId(String s);
	public void setGuideObjectCCDPositionX(double d);
	public void setGuideObjectCCDPositionX(String s);
	public void setGuideObjectCCDPositionY(double d);
	public void setGuideObjectCCDPositionY(String s);
	public void setGuideObjectBufferPositionX(double d);
	public void setGuideObjectBufferPositionX(String s);
	public void setGuideObjectBufferPositionY(double d);
	public void setGuideObjectBufferPositionY(String s);

}
//
// $Log: not supported by cvs2svn $
//
