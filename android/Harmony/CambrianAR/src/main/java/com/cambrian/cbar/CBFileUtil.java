package com.cambrian.cbar;

import android.content.res.AssetManager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import timber.log.Timber;

/**
 * Created by Joseph on 11/3/2017.
 */

public class CBFileUtil {


    static void copyDirOrFileFromAssetManager(AssetManager assetManager, String assetDir, File destDir) throws IOException {
        Timber.d("destination dir: " + destDir.toString());

        destDir.mkdirs();

        String[] files = new String[0];
        try {
            files = assetManager.list(assetDir);
        } catch (IOException e) {
            e.printStackTrace();
        }

        for (String file : files) {
            String assetPath = assetDir + File.separator + file;
            if(!assetPath.contains(".")) {
                copyDirOrFileFromAssetManager(assetManager, assetPath, new File(destDir, file));
            } else {
                File destFile = new File(destDir, file);
                copyAssetFile(assetManager, assetPath, destFile.toString());
            }
        }
    }


    private static void copyAssetFile(AssetManager assetManager, String assetFilePath, String destFilePath) throws IOException {
        InputStream input = assetManager.open(assetFilePath);
        FileOutputStream out = new FileOutputStream(destFilePath);

        byte[] buf = new byte[1024];
        int len;
        while((len = input.read(buf)) > 0) {
            out.write(buf, 0, len);
        }
        input.close();
        out.close();
    }

    static void deleteRecursive(File fileOrDirectory) {
        if (fileOrDirectory.isDirectory())
            for (File child : fileOrDirectory.listFiles())
                deleteRecursive(child);

        fileOrDirectory.delete();
    }
}
