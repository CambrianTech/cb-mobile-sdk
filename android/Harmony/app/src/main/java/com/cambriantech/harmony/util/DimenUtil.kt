package com.cambriantech.harmony.util

import android.content.Context

/**
 * Created by joseph on 9/9/16.
 */

object DimenUtil {

    fun getScreenWidthDP(context: Context): Float {
        val displayMetrics = context.resources.displayMetrics
        val screenDpWidth = displayMetrics.widthPixels / displayMetrics.density
        return screenDpWidth
    }

    fun getScreenWidthPixel(context: Context): Int {
        val displayMetrics = context.resources.displayMetrics
        val screenDpWidth = displayMetrics.widthPixels / displayMetrics.density
        val screenPxWidth = Math.round(screenDpWidth * displayMetrics.density)
        return screenPxWidth
    }

    fun dpToPx(context: Context, dp: Int): Int {
        val screenDPWidth = getScreenWidthDP(context)
        val screenPxWidth = getScreenWidthPixel(context)
        val dpPxWidth = Math.round(screenPxWidth / screenDPWidth)
        return dp * dpPxWidth
    }

    fun getDensity(context: Context) : Float {
        return context.resources.displayMetrics.density
    }
}
