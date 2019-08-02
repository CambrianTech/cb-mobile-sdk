package com.cambrian.cbar.types;

/**
 * Created by Joseph on 8/24/2017.
 */

public enum CBLightingType {
    None,
    Incandescent,
    Fluorescent,
    LEDWhite,
    LEDWarm,
    DaylightMorning,
    Daylight,
    DaylightEvening,
    DaylightOvercast;

    public static CBLightingType fromOrdinal(int n) {return values()[n];}
}