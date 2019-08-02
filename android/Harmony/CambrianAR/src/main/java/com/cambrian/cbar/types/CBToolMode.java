package com.cambrian.cbar.types;

/**
 * Created by Joseph on 8/24/2017.
 */

public enum CBToolMode {
    None,
    Select,
    Move,
    Fill,
    Unfill,
    Paintbrush,
    Eraser,
    FindColor;

    public static CBToolMode fromOrdinal(int n) {return values()[n];}
}