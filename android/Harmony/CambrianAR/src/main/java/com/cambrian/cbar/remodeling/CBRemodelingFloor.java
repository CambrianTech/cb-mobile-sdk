package com.cambrian.cbar.remodeling;

import com.cambrian.cbar.CBAugmentedAsset;
import com.cambrian.cbar.types.CBAssetType;

/**
 * Created by joelteply on 8/1/17.
 */

public class CBRemodelingFloor extends CBAugmentedAsset {

    public CBRemodelingFloor() {
        super();
    }

    public CBRemodelingFloor(String assetID) {
        super(assetID);
    }

    @Override
    protected native long createNativeInstance(String assetID);

    @Override
    public CBAssetType getAssetType() {
        return CBAssetType.Floor;
    }

    public native void setScale(float scale);
    public native void setDiffuse(byte[] diffuse);
    public native void setNormalRoughness(byte[] normal, byte[] roughness);

}
