package com.cambrian.cbar.remodeling;

import android.content.Context;

import com.cambrian.cbar.CBConfiguration;

/**
 * Created by Joseph on 10/12/2017.
 */

public class CBRemodelingConfiguration extends CBConfiguration {

    static {
        System.loadLibrary("CambrianAR");
    }

    public CBRemodelingConfiguration(String key, Context context) {
        super(key, context);

        //Move files to location accessible by cpp

    }

    public native void setPrimaryAssetPath(String path);
    public native void setSecondaryAssetPath(String path);
}
