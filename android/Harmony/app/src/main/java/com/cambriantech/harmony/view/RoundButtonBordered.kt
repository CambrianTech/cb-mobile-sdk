package com.cambriantech.harmony.view

import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.widget.ImageView
import com.cambriantech.harmony.util.ImageUtil

/**
 * Created by Joseph on 5/9/2017.
 */

open class RoundButtonBordered : ImageView {

    private val white = 0x77ffffff.toInt()
    private val black = 0x77000000.toInt()
    private var color = 0xffd2c8b6.toInt()

    private val path = Path()
    private val paint = Paint()
    private val borderPaint = Paint()
    private var image: Bitmap? = null


    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        paint.isAntiAlias = true
        borderPaint.isAntiAlias = true

        val width = width.toFloat()

        val borderSize = (width * 0.5).toFloat()
        borderPaint.color = if (isSelected) white else black
        canvas.drawCircle(width/2, width/2, borderSize, borderPaint)

        val centerSize = (width * 0.42).toFloat()
        if(image != null) {

            val shader = BitmapShader(image, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP)
            paint.shader = shader

            path.reset()
            path.addCircle(width/2, width/2, centerSize, Path.Direction.CW)

            canvas.drawPath(path, paint)

        } else {
            paint.color = color
            canvas.drawCircle(width/2, width/2, centerSize, paint)
        }
    }

    open fun setColor(color: Int) {
        this.color = color
        invalidate()
    }

    fun setImage(path: String) {
        ImageUtil.loadBitmap(path) {
            image = it
            invalidate()
        }
    }
}