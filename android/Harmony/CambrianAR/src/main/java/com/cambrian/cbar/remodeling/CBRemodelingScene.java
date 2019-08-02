package com.cambrian.cbar.remodeling;

import android.support.annotation.Nullable;

import com.cambrian.cbar.CBAugmentedAsset;
import com.cambrian.cbar.CBAugmentedScene;
import com.cambrian.cbar.types.CBAssetType;

import java.io.File;

/**
 * Created by joelteply on 8/1/17.
 */

public class CBRemodelingScene extends CBAugmentedScene {

    //@Override
    //protected native long[] initWithID(String sceneID);

    public CBRemodelingScene() {
        super();
    }

    public CBRemodelingScene(String sceneID) {
        super(sceneID);
    }

    public CBRemodelingScene(File projectPath) {
        super(projectPath);
    }

    public CBRemodelingScene(String sceneID, String path) {
        super(sceneID, path);
    }

    public boolean hasFloor() {
        for (CBAugmentedAsset asset : getAssets().values()) {
            if(asset.getAssetType() == CBAssetType.Floor) return true;
        }
        return false;
    }

    @Nullable
    public CBAugmentedAsset getFloor() {
        for (CBAugmentedAsset asset : getAssets().values()) {
            if(asset.getAssetType() == CBAssetType.Floor) return asset;
        }
        return null;
    }

    public boolean getIsMasked() {
        return false;
    }
}
