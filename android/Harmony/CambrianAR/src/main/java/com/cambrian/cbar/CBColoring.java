package com.cambrian.cbar;

/**
 * Created by joelteply on 8/1/17.
 */

public class CBColoring {

    static {
        System.loadLibrary("CambrianAR");
    }

    public native static int[] getComplementsForColor(int color, int count, int angleSpan);

    public native static int[] getAdjacentColors(int color, int count, int angleSpan);

    public native static int[] getShadesOfColor(int color, int count);

    public native static double getEuclideanDistance(int colorA, int colorB);

    public native static double getEuclideanDistance(int colorA, int colorB, boolean asHSV, float[] coeficient);

    //http://www.compuphase.com/cmetric.htm
    public native static double getHumanPerceptiveDistance(int colorA, int colorB);
}
