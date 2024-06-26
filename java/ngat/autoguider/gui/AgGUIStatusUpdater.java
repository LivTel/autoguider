// AgGUIStatusUpdater.java
// $Header: /home/cjm/cvs/autoguider/java/ngat/autoguider/gui/AgGUIStatusUpdater.java,v 1.1 2023-08-08 08:55:11 cjm Exp $
package ngat.autoguider.gui;

import java.lang.*;
import java.net.*;
import java.text.*;

import javax.swing.*;

/**
 * The AgGUIStatusUpdater updates the swing GUI.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
public class AgGUIStatusUpdater implements AgGUIStatusUpdateListener
{
	/**
	 * Revision Control System id string, showing the version of the Class.
	 */
	public final static String RCSID = new String("$Id: AgGUIStatusUpdater.java,v 1.1 2023-08-08 08:55:11 cjm Exp $");
	/**
	 * The IcsGUI object.
	 */
	private AgGUI parent = null;

	/**
	 * A constructor for this class. 
	 * @param p The parent object.
	 */
	public AgGUIStatusUpdater(AgGUI p)
	{
		super();
		parent = p;
	}

	/**
	 * Set Fielding label based on boolean to either ACTIVE or INACTIVE.
	 * @param b A boolean, true if the autoguider is fielding.
	 */
	public void setFieldActive(boolean b)
	{
		if(b)
			parent.setIsFieldingLabel("ACTIVE");
		else
			parent.setIsFieldingLabel("INACTIVE");
	}

	/**
	 * Set Fielding label to the specified string.
	 * @param s The string.
	 */
	public void setFieldActive(String s)
	{
		parent.setIsFieldingLabel(s);
	}

	/**
	 * Set Guiding label based on boolean to either ACTIVE or INACTIVE.
	 * @param b A boolean, true if the autoguider is guiding.
	 */
	public void setGuideActive(boolean b)
	{
		if(b)
			parent.setIsGuidingLabel("ACTIVE");
		else
			parent.setIsGuidingLabel("INACTIVE");
	}

	/**
	 * Set Guiding label to the specified string.
	 * @param s The string.
	 */
	public void setGuideActive(String s)
	{
		parent.setIsGuidingLabel(s);
	}

	/**
	 * Set Guide cadence label.
	 * @param i The guide exposure length, in milliseconds.
	 */
	public void setGuideExposureLength(int i)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(i);
		setGuideExposureLength(s);
	}

	/**
	 * Set Guide exposure length label to the specified string.
	 * @param s The string.
	 */
	public void setGuideExposureLength(String s)
	{
		parent.setGuideExposureLengthLabel(s);
	}

	/**
	 * Set Guide cadence label.
	 * @param d The guide cadence, in decimal seconds.
	 */
	public void setGuideCadence(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideCadence(s);
	}

	/**
	 * Set Guide cadence label to the specified string.
	 * @param s The string.
	 */
	public void setGuideCadence(String s)
	{
		parent.setGuideCadenceLabel(s);
	}

	/**
	 * Set Guide object count label.
	 * @param c The guide cadence, in decimal seconds.
	 */
	public void setGuideObjectCount(int c)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0");
		s = decimalFormat.format(c);
		setGuideObjectCount(s);
	}

	/**
	 * Set Guide object count label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectCount(String s)
	{
		parent.setGuideObjectCountLabel(s);
	}

	/**
	 * Set Guide object count label.
	 * @param n The guide cadence, in decimal seconds.
	 */
	public void setGuideObjectId(int n)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0");
		s = decimalFormat.format(n);
		setGuideObjectId(s);
	}

	/**
	 * Set Guide object id label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectId(String s)
	{
		parent.setGuideObjectIdLabel(s);
	}

	/**
	 * Set Guide object CCD Position X label.
	 * @param d The guide object ccd X position, in pixels.
	 */
	public void setGuideObjectCCDPositionX(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideObjectCCDPositionX(s);
	}

	/**
	 * Set Guide object CCD Position X label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectCCDPositionX(String s)
	{
		parent.setGuideObjectCCDPositionXLabel(s);
	}

	/**
	 * Set Guide object CCD Position Y label.
	 * @param d The guide object ccd Y position, in pixels.
	 */
	public void setGuideObjectCCDPositionY(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideObjectCCDPositionY(s);
	}

	/**
	 * Set Guide object CCD Position Y label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectCCDPositionY(String s)
	{
		parent.setGuideObjectCCDPositionYLabel(s);
	}

	/**
	 * Set Guide object buffer Position X label.
	 * @param d The guide object buffer X position, in pixels.
	 */
	public void setGuideObjectBufferPositionX(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideObjectBufferPositionX(s);
	}

	/**
	 * Set Guide object Buffer Position X label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectBufferPositionX(String s)
	{
		parent.setGuideObjectBufferPositionXLabel(s);
	}

	/**
	 * Set Guide object buffer Position Y label.
	 * @param d The guide object buffer Y position, in pixels.
	 */
	public void setGuideObjectBufferPositionY(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideObjectBufferPositionY(s);
	}

	/**
	 * Set Guide object Buffer Position Y label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectBufferPositionY(String s)
	{
		parent.setGuideObjectBufferPositionYLabel(s);
	}

	/**
	 * Set Guide object total counts label.
	 * @param d The guide object total counts, in ADU.
	 */
	public void setGuideObjectTotalCounts(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideObjectTotalCounts(s);
	}

	/**
	 * Set Guide object total counts label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectTotalCounts(String s)
	{
		parent.setGuideObjectTotalCountsLabel(s);
	}
	
	/**
	 * Set Guide object peak counts label.
	 * @param d The guide object peak counts, in ADU.
	 */
	public void setGuideObjectPeakCounts(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideObjectPeakCounts(s);
	}

	/**
	 * Set Guide object peak counts label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectPeakCounts(String s)
	{
		parent.setGuideObjectPeakCountsLabel(s);
	}
	
	/**
	 * Set Guide object No. of pixels label.
	 * @param n The number of pixels in the guide object, in pixels.
	 */
	public void setGuideObjectNoOfPixels(int n)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("#######0");
		s = decimalFormat.format(n);
		setGuideObjectNoOfPixels(s);
	}

	/**
	 * Set Guide object peak counts label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectNoOfPixels(String s)
	{
		parent.setGuideObjectNoOfPixelsLabel(s);
	}
	
	/**
	 * Set Guide object is stellar label.
	 * @param b A boolea, true if the guide object is stellar, false if it is not.
	 */
	public void setGuideObjectIsStellar(boolean b)
	{
		if(b)
			setGuideObjectIsStellar("true");
		else
			setGuideObjectIsStellar("false");
	}
	
	/**
	 * Set Guide object is stellar label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectIsStellar(String s)
	{
		parent.setGuideObjectIsStellarLabel(s);
	}
	
	/**
	 * Set Guide object FWHM X label.
	 * @param d The guide object FWHM in X, in pixels.
	 */
	public void setGuideObjectFWHMX(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideObjectFWHMX(s);
	}

	/**
	 * Set Guide object FWHM X label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectFWHMX(String s)
	{
		parent.setGuideObjectFWHMXLabel(s);
	}
	
	/**
	 * Set Guide object FWHM Y label.
	 * @param d The guide object FWHM in Y, in pixels.
	 */
	public void setGuideObjectFWHMY(double d)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(d);
		setGuideObjectFWHMY(s);
	}

	/**
	 * Set Guide object FWHM Y label to the specified string.
	 * @param s The string.
	 */
	public void setGuideObjectFWHMY(String s)
	{
		parent.setGuideObjectFWHMYLabel(s);
	}
	
	/**
	 * Set the last object detection image median ADU value label.
	 * @param m The last object detection image median ADU value, in counts.
	 */
	public void setObjectMedian(double m)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(m);
		setObjectMedian(s);
	}
	
	/**
	 * Set object median label to the specified string.
	 * @param s The string.
	 * @see #parent
	 * @see AgGUI#setObjectMedianLabel
	 */
	public void setObjectMedian(String s)
	{
		parent.setObjectMedianLabel(s);
	}

	/**
	 * Set the last object detection image mean ADU value label.
	 * @param m The last object detection image mean ADU value, in counts.
	 */
	public void setObjectMean(double m)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(m);
		setObjectMean(s);
	}

	/**
	 * Set object mean label to the specified string.
	 * @param s The string.
	 * @see #parent
	 * @see AgGUI#setObjectMeanLabel
	 */
	public void setObjectMean(String s)
	{
		parent.setObjectMeanLabel(s);
	}

	/**
	 * Set the last object detection image standard deviation value label.
	 * @param sd The last object detection image standard deviation value, in counts.
	 */
	public void setObjectBackgroundStandardDeviation(double sd)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(sd);
		setObjectBackgroundStandardDeviation(s);
	}

	/**
	 * Set object background standard deviation label to the specified string.
	 * @param s The string.
	 * @see #parent
	 * @see AgGUI#setObjectBackgroundStandardDeviationLabel
	 */
	public void setObjectBackgroundStandardDeviation(String s)
	{
		parent.setObjectBackgroundStandardDeviationLabel(s);
	}

	/**
	 * Set the last object detection image threshold value label.
	 * @param t The last object detection image threshold value, in counts.
	 */
	public void setObjectThreshold(double t)
	{
		DecimalFormat decimalFormat = null;
		String s = null;

		decimalFormat = new DecimalFormat("####0.00");
		s = decimalFormat.format(t);
		setObjectThreshold(s);
	}
	
	/**
	 * Set object threshold label to the specified string.
	 * @param s The string.
	 * @see #parent
	 * @see AgGUI#setObjectThresholdLabel
	 */
	public void setObjectThreshold(String s)
	{
		parent.setObjectThresholdLabel(s);
	}

	/**
	 * Add the specified ccd x position to the graph.
	 * @param d An object centroid X position in pixels.
	 * @see #parent
	 * @see AgGUI#addCCDXPositionToGraph
	 */
	public void addCCDXPositionToGraph(double d)
	{
		parent.addCCDXPositionToGraph((float)d);
	}

	/**
	 * Add the specified ccd y position to the graph.
	 * @param d An object centroid Y position in pixels.
	 * @see #parent
	 * @see AgGUI#addCCDYPositionToGraph
	 */
	public void addCCDYPositionToGraph(double d)
	{
		parent.addCCDYPositionToGraph((float)d);
	}
}
