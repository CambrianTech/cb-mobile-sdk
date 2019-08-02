package com.cambrian.cbar.remodeling;

/**
 * Created by joelteply on 8/1/17.
 */

public class CBRemodelingTypes {
    public enum CBUndoChange {
        Mask,
        PaintColor,
        PaintSheen;
        public static CBUndoChange fromOrdinal(int n) {return values()[n];}
    }

    public enum CBTransparency {
        Clear,
        Translucent,
        Transparent,
        SemiTransparent,
        Solid,
        UltraSolid;
        public static CBTransparency fromOrdinal(int n) {return values()[n];}
    }

    public enum CBPaintSheen {
        Flat,
        Matte,
        Eggshell,
        Satin,
        SemiGloss,
        Gloss,
        HighGloss;
        public static CBPaintSheen fromOrdinal(int n) {return values()[n];}
    }

    public enum CBFinish {
        None,
        AgedPatina,
        AntiqueLeather,
        BrightCanvas,
        Matte,
        Chalkboard,
        Whiteboard,
        Crackle,
        Frosted,
        Glitter,
        GlowInTheDark,
        Sand,
        Hammered,
        Denim,
        Metallic,
        RiverRock,
        SlipResistant,
        Suede,
        WetLook;
        public static CBFinish fromOrdinal(int n) {return values()[n];}
    }
}