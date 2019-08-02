package com.cambrian.cbar;

import android.content.Context;
import android.content.SharedPreferences;

import com.cambrian.BuildConfig;

import java.io.File;
import java.io.IOException;

/**
 * Created by Joseph on 10/12/2017.
 */

public class CBConfiguration {

    private static final String FILE_KEY = "FILE_KEY";
    private static final String VERSION_KEY = "VERSION_KEY";

    static {
        System.loadLibrary("CambrianAR");
    }

    public CBConfiguration(String key, Context context) {

        CBLicensing.enableWithKey(key);

        String cbAssets = "CBAssets";

        File appPath = context.getExternalFilesDir(null);
        File assetPath = new File(appPath, "assets");
        File cbAssetPath = new File(assetPath, cbAssets);
        File debugPath = new File(appPath, "debug");

        setCBAssetPath(cbAssetPath.toString());
        setCBLoggingPath(debugPath.toString());
        cleanDebug(debugPath);

        SharedPreferences settings = context.getSharedPreferences("CB_Config", Context.MODE_PRIVATE);

        int currentCode = BuildConfig.VERSION_CODE;
        int savedCode = settings.getInt(VERSION_KEY, -1);

        if(!settings.getBoolean(FILE_KEY, false) || savedCode != currentCode) {
            try {
                CBFileUtil.copyDirOrFileFromAssetManager(context.getAssets(), cbAssets, cbAssetPath);
            } catch (IOException e) {
                e.printStackTrace();
            }

            settings.edit().putInt(VERSION_KEY, currentCode).apply();
            settings.edit().putBoolean(FILE_KEY, true).apply();
        }

    }

    private void cleanDebug(File debugPath) {
        CBFileUtil.deleteRecursive(debugPath);
    }

    public native void initialize();

    public native void setCBAssetPath(String path);
    public native void setCBLoggingPath(String path);
    public native void setWorkingPath(String path);
}
