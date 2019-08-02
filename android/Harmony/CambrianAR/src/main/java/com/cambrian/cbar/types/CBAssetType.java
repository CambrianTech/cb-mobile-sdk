package com.cambrian.cbar.types;

/**
 * Created by Joseph on 8/24/2017.
 */

public enum CBAssetType {
    All,
    Paint,
    Floor,
    Model;

    public static CBAssetType fromOrdinal(int n) {return values()[n];}
}