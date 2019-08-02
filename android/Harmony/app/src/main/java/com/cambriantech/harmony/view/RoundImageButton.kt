package com.cambriantech.harmony.view

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.drawable.ColorDrawable
import android.support.v7.widget.AppCompatImageView
import android.util.AttributeSet

/**
 * Created by Joseph on 5/9/2017.
 */

class RoundImageButton : AppCompatImageView {

    private val white = 0x77ffffff.toInt()
    private val black = 0x77000000.toInt()
    private var color = 0xffd2c8b6.toInt()

    private val paint = Paint()


    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)

    init {
        color = (background as ColorDrawable).color
        paint.color = color
        setBackgroundColor(Color.TRANSPARENT)
    }

    override fun onDraw(canvas: Canvas) {


        paint.isAntiAlias = true

        val width = width.toFloat()

        val size = (width * 0.5).toFloat()
        paint.color = color
        canvas.drawCircle(width/2, width/2, size, paint)

        val matrix = imageMatrix
        val scale = 0.5f
        val offset = width * 0.18f
        matrix.preScale(scale, scale, offset, offset)
        imageMatrix = matrix

        super.onDraw(canvas)
    }

    fun setColor(color: Int) {
        this.color = color
        invalidate()
    }
}