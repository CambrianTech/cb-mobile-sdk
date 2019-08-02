package com.cambrian.cbar;

import com.cambrian.cbar.types.CBAssetType;

/**
 * Created by joelteply on 8/1/17.
 */

public abstract class CBAugmentedAsset implements ICBJavaObject {

    static {
        System.loadLibrary("CambrianAR");
    }

    public CBAugmentedAsset() {
        this("");
    }

    public CBAugmentedAsset(String assetID) {
        m_coreObject = createNativeInstance(assetID);
    }

    public void finalize() {
        destroyNativeInstance( this.m_coreObject );

        try {
            super.finalize();
        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }

    private long m_coreObject;


    @Override
    public long getCoreObject() {
        return m_coreObject;
    }

    protected abstract long createNativeInstance(String assetID);
    private native void destroyNativeInstance( long p_native_ptr );

    public abstract CBAssetType getAssetType();
    public native String getAssetID();
}
