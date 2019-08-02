package com.cambriantech.harmony.util

import android.animation.Animator
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Matrix
import android.graphics.Paint
import android.graphics.drawable.Drawable
import android.support.v4.graphics.drawable.DrawableCompat
import android.support.v7.widget.SearchView
import android.view.MenuItem
import android.view.View
import android.view.ViewPropertyAnimator
import android.widget.ImageView


fun <T1: Any, T2: Any, R: Any> safeLet(p1: T1?, p2: T2?, block: (T1, T2)->R?): R? {
    return if (p1 != null && p2 != null) block(p1, p2) else null
}
fun <T1: Any, T2: Any, T3: Any, R: Any> safeLet(p1: T1?, p2: T2?, p3: T3?, block: (T1, T2, T3)->R?): R? {
    return if (p1 != null && p2 != null && p3 != null) block(p1, p2, p3) else null
}
fun <T1: Any, T2: Any, T3: Any, T4: Any, R: Any> safeLet(p1: T1?, p2: T2?, p3: T3?, p4: T4?, block: (T1, T2, T3, T4)->R?): R? {
    return if (p1 != null && p2 != null && p3 != null && p4 != null) block(p1, p2, p3, p4) else null
}
fun <T1: Any, T2: Any, T3: Any, T4: Any, T5: Any, R: Any> safeLet(p1: T1?, p2: T2?, p3: T3?, p4: T4?, p5: T5?, block: (T1, T2, T3, T4, T5)->R?): R? {
    return if (p1 != null && p2 != null && p3 != null && p4 != null && p5 != null) block(p1, p2, p3, p4, p5) else null
}


fun Drawable.resize(width: Int, height: Int) {
    val img = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
    val canvas = Canvas(img)
    this.draw(canvas)
    img.recycle()
}

fun MenuItem.tint(color: Int) {
    val drawable = this.icon
    drawable.mutate()
    val wrapDrawable = DrawableCompat.wrap(drawable)
    DrawableCompat.setTint(wrapDrawable, color)
}

fun ImageView.tint(color: Int) {
    var drawable = this.drawable
    drawable = DrawableCompat.wrap(drawable.mutate())
    DrawableCompat.setTint(drawable, color)
    this.setImageDrawable(drawable)
}

fun ViewPropertyAnimator.onComplete(onComplete : () -> Unit) : ViewPropertyAnimator {
    this.setListener(object : Animator.AnimatorListener {
        override fun onAnimationStart(animator: Animator) {}
        override fun onAnimationCancel(animator: Animator) {}
        override fun onAnimationRepeat(animator: Animator) {}
        override fun onAnimationEnd(animator: Animator) {
            onComplete()
        }
    })
    return this
}

fun SearchView.onQueryTextChanged(onChanged: (String) -> Unit) {
    this.setOnQueryTextListener(object: SearchView.OnQueryTextListener {
        override fun onQueryTextSubmit(query: String?): Boolean {
            return false
        }

        override fun onQueryTextChange(newText: String?): Boolean {
            newText?.let { onChanged(it) }
            return false
        }
    })
}

fun Bitmap.resize(maxSize: Int) {

    val width = this.width.toFloat()
    val height = this.height.toFloat()

    val dims = ImageUtil.getScaledDimensions(width, height, maxSize.toFloat())

    val scaledBitmap = Bitmap.createBitmap(dims.width, dims.height, Bitmap.Config.ARGB_8888)
    val scaleWidth = dims.width.toFloat() / width
    val scaleHeight = dims.height.toFloat() / height

    val matrix = Matrix()
    matrix.postScale(scaleWidth, scaleHeight)

    val canvas = Canvas(scaledBitmap)
    canvas.matrix = matrix
    canvas.drawBitmap(this, 0f, 0f, Paint(Paint.FILTER_BITMAP_FLAG))
}


fun View.isVisible() : Boolean {
    return (this.visibility == View.VISIBLE)
}

fun View.isHidden() : Boolean {
    return (this.visibility == View.INVISIBLE || this.visibility == View.GONE)
}

fun View.fadeIn() {
    Animations.fade(this, true)
}

fun View.fadeOut() {
    Animations.fade(this, false)
}

fun View.hide() {
    this.visibility = View.GONE
}

fun View.show() {
    this.visibility = View.VISIBLE
}

fun View.setInvisible() {
    this.visibility = View.INVISIBLE
}