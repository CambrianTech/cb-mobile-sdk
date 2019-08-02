package com.cambrian.cbar;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Size;

import com.cambrian.cbar.types.CBLightingType;

import java.io.File;
import java.util.Map;
import java.util.UUID;

/**
 * Created by joelteply on 8/1/17.
 */

public abstract class CBAugmentedScene implements ICBJavaObject {

    static {
        System.loadLibrary("CambrianAR");
    }

    private long m_coreObject;
    private CBAugmentedAssetSyncMap m_assets;

    public CBAugmentedScene() {
        this(UUID.randomUUID().toString());
    }

    public CBAugmentedScene(String sceneID) {
        long[] pointers = initWithID(sceneID);
        m_coreObject = pointers[0];
        m_assets = new CBAugmentedAssetSyncMap(m_coreObject);//use scene in underlying CBAugmentedAssetSyncMap
    }

    public CBAugmentedScene(String sceneID, String imagePath) {
        long[] pointers = initWithImagePath(sceneID, imagePath);
        m_coreObject = pointers[0];
        m_assets = new CBAugmentedAssetSyncMap(m_coreObject);//use scene in underlying CBAugmentedAssetSyncMap
    }

    public CBAugmentedScene(File projectPath) {
        this();
        loadFromDirectory(projectPath.getPath());
    }


    Size getFrameSize() {
        int[] frameInfo = _getFrameInfo();
        if (frameInfo != null) {
            return new Size(frameInfo[0],frameInfo[1]);
        }
        return new Size(0,0);
    }

    public void finalize() {
        destroyNativeInstance( this.m_coreObject );

        try {
            super.finalize();
        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }

    @Override
    public long getCoreObject() {
        return m_coreObject;
    }


    public void appendAsset(CBAugmentedAsset asset) {
        if(canAppendAsset()) {
            m_assets.put(asset.getAssetID(), asset);
        }
    }

    public void removeAsset(String id) {
        m_assets.remove(id);
    }

    public boolean canAppendAsset() {
        return m_assets.size() < 4;
    }

    @NonNull
    public Map<String, CBAugmentedAsset> getAssets() {
        return m_assets;
    }
    public CBAugmentedAsset getAsset(String key) {
        return m_assets.get(key);
    }

    /**
     * Return the currently selected asset
     */
    public CBAugmentedAsset getSelectedAsset() {
        if (getSelectedAssetID() != null) {
            return getAssets().get(getSelectedAssetID());
        }
        return null;
    }

    /**
     * Lighting adjustment for this scene
     */
    public CBLightingType getLightingAdjustment() {
        return CBLightingType.fromOrdinal(_getLightingAdjustment());
    }
    public void setLightingAdjustment(CBLightingType adj) {
        _setLightingAdjustment(adj.ordinal());
    }



    /***
     * Save the current project either compressed or not, with a completion callback.
     */
    protected void saveToDirectory(final File path, final boolean doCompression) {
        _saveToDirectory(path.toString(), doCompression);
    }

    protected native long[] initWithImagePath(String sceneID, String path);
    protected native long[] initWithID(String sceneID);
    private native void loadFromDirectory(String path);
    private native void _saveToDirectory(String path, boolean doCompression);

    private native void destroyNativeInstance( long p_native_ptr );

    public native String getSceneID();

    private native int[] _getFrameInfo();
    private native int _getLightingAdjustment();
    private native void _setLightingAdjustment(int adjustment);


    @Nullable
    public native String getSelectedAssetID();
    public native boolean setSelectedAssetID(String id);
    public native void setSelectedAsset(CBAugmentedAsset asset);
    public native boolean hasWorldTransform();
}
