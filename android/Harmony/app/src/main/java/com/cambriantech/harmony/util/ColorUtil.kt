package com.cambriantech.harmony.util

import android.graphics.Color
import android.graphics.drawable.Drawable
import android.support.v4.graphics.drawable.DrawableCompat
import android.widget.Button
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.data.BrandItem
import io.realm.RealmResults

/**
 * Created by Joseph Sulliva on 9/14/16.
 */

object ColorUtil {

    fun tintDrawableLeft(button: Button, color: Int) {
        var drawable: Drawable = button.compoundDrawables[0]
        drawable = DrawableCompat.wrap(drawable.mutate())
        DrawableCompat.setTint(drawable, color)
        button.setCompoundDrawables(drawable, null, null, null)

    }

    fun setDrawableLeft(button: Button, drawableIn: Drawable, color: Int) {
        var drawable = drawableIn
        drawable.mutate()
        drawable = DrawableCompat.wrap(drawable)
        DrawableCompat.setTint(drawable, color)
        button.setCompoundDrawablesWithIntrinsicBounds(drawable, null, null, null)
    }

    fun colorDistance(colorA: Int, colorB: Int): Int {
        val colA = intArrayOf(Color.red(colorA),
                            Color.green(colorA),
                            Color.blue(colorA))
        val colB = intArrayOf(Color.red(colorB),
                            Color.green(colorB),
                            Color.blue(colorB))

        //Euclidean distance
        val distR = 2 * Math.pow((colB[0] - colA[0]).toDouble(), 2.0)
        val distG = 4 * Math.pow((colB[1] - colA[1]).toDouble(), 2.0)
        val distB = 3 * Math.pow((colB[2] - colA[2]).toDouble(), 2.0)

        return Math.sqrt(distR + distB + distG).toInt()
    }

    fun getCloseColors(color: Int): RealmResults<BrandItem> {
        val red = Color.red(color)
        val green = Color.green(color)
        val blue = Color.blue(color)

        val range = 15
        val query = HHApp.realmBrands.where(BrandItem::class.java)
        query.between("red", red - range, red + range)
        query.between("green", green - range, green + range)
        query.between("blue", blue - range, blue + range)
        return query.findAll()
    }

    fun getClosestMatch(color: Int): BrandItem? {
        val results = getCloseColors(color)
        var bestDistance = Integer.MAX_VALUE
        var bestMatch: BrandItem? = null

        results.map { result ->
            val distance = ColorUtil.colorDistance(color, result.color)
            if (distance < bestDistance) {
                bestDistance = distance
                bestMatch = result
            }
        }
        return bestMatch
    }

    fun isBright(color: Int) : Boolean {
        val rgb = intArrayOf(Color.red(color), Color.green(color), Color.blue(color))

        val brightness = Math.sqrt(rgb[0] * rgb[0] * .241 + rgb[1] * rgb[1] * .691 + rgb[2] * rgb[2] * .068).toInt()

        return brightness >= 180
    }
}