package com.cambrian.cbar.remodeling;

import com.cambrian.cbar.CBAugmentedAsset;
import com.cambrian.cbar.types.CBAssetType;

/**
 * Created by joelteply on 8/1/17.
 */

public class CBRemodelingPaint extends CBAugmentedAsset {

    public CBRemodelingPaint() {
        super();
    }

    public CBRemodelingPaint(String assetID) {
        super(assetID);
    }

    @Override
    protected native long createNativeInstance(String assetID);

    @Override
    public CBAssetType getAssetType() {
        return CBAssetType.Paint;
    }

    public native int getColor();

    public native void setColor(int color);

    private native int _getTransparency();

    public CBRemodelingTypes.CBTransparency getTransparency() {
        return CBRemodelingTypes.CBTransparency.fromOrdinal(_getTransparency());
    }

    private native void _setTransparency(int transparency);

    public void setTransparency(CBRemodelingTypes.CBTransparency transparency) {
        _setTransparency(transparency.ordinal());
    }

    private native int _getSheen();
    public CBRemodelingTypes.CBPaintSheen getSheen() {
        return CBRemodelingTypes.CBPaintSheen.fromOrdinal(_getSheen());
    }

    private native void _setSheen(int value);
    public void setSheen(CBRemodelingTypes.CBPaintSheen sheen) {
        _setSheen(sheen.ordinal());
    }
}
