package com.cambriantech.harmony.view

import android.content.Context
import android.graphics.*
import android.text.Layout
import android.text.StaticLayout
import android.text.TextPaint
import android.util.AttributeSet
import android.view.View
import com.cambriantech.harmony.util.DimenUtil

/**
 * Created by Joseph on 11/2/2017.
 */
class NameView : View {

    private val p = Path()
    private val paint = Paint()
    private val textPaint = TextPaint()
    var itemName = ""
    var color = 0xffd2c8b6.toInt()
    var showText = true
    var image: Bitmap? = null


    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)


    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        setBackgroundColor(color)

        paint.reset()
        paint.isAntiAlias = true
        paint.color = color
        paint.style = Paint.Style.FILL

        // scale and render image
        if (image != null) {
            val shader = BitmapShader(image, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP)
            shader.setLocalMatrix(matrix)
            paint.shader = shader
        }

        canvas.drawPaint(paint)


        textPaint.isAntiAlias = true
        textPaint.textSize = DimenUtil.dpToPx(context, 12).toFloat()
        textPaint.color = Color.WHITE

        val textLayout = StaticLayout(itemName,
                                      textPaint,
                                      (width * 0.8f).toInt(),
                                      Layout.Alignment.ALIGN_CENTER,
                                      1.0f,
                                      0.0f,
                                      false)

        canvas.save()
        // calculate x and y position where your text will be placed

        val textX = (width * 0.1f).toInt()
        val textY = ((canvas.height / 2 - (paint.descent() + paint.ascent()) / 2) * 1f).toInt() - textLayout.height / 2

        canvas.translate(textX.toFloat(), textY.toFloat())
        textLayout.draw(canvas)
        canvas.restore()
    }
}