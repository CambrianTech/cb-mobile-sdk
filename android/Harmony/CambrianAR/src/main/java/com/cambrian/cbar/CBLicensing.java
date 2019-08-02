package com.cambrian.cbar;

/**
 * Created by joelteply on 8/1/17.
 */

public class CBLicensing {
    static {
        System.loadLibrary("CambrianAR");
    }

    public native static void enableWithKey(String key);

    public native static boolean isEnabled();
}
