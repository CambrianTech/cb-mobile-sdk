package com.cambriantech.harmony.view

import android.content.Context
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.RectF
import android.util.AttributeSet
import android.widget.LinearLayout
import com.cambriantech.harmony.R
import timber.log.Timber

/**
 * Created by Joseph on 6/7/2017.
 */
open class RoundedContainer : LinearLayout {

    private val paint = Paint()
    open var radius: Float? = null

    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs) {
        val att = context.theme.obtainStyledAttributes(attrs, R.styleable.RoundedContainer, 0, 0)
        Timber.d("got attributes")

        try {
            radius = att.getDimensionPixelSize(R.styleable.RoundedContainer_cornerRadius, 0).toFloat()
            Timber.d("radius is " + radius)
        } finally {
            att.recycle()
        }
    }

    init {
        this.setWillNotDraw(false)
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        paint.isAntiAlias = true

        val width = width.toFloat()
        val height = height.toFloat()

        //get value from attrs, if there isn't one, use width/2
        var rad = radius ?: 0f
        if(rad == 0f) rad = width/2f

        paint.color = 0x80000000.toInt()
        val rect = RectF(0f, 0f, width, height)
        canvas.drawRoundRect(rect, rad, rad, paint)
    }
}