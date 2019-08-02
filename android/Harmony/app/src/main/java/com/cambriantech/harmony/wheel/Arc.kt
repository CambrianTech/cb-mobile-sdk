package com.cambriantech.harmony.wheel

import android.content.Context
import android.graphics.*
import android.text.Layout
import android.text.StaticLayout
import android.text.TextPaint
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import com.cambriantech.harmony.util.ColorUtil
import com.cambriantech.harmony.util.DimenUtil
import com.cambriantech.harmony.util.ImageUtil


/**
 * Created by Joseph Sullivan on 7/6/16.
 */

class Arc : View {

    private val p = Path()
    private val paint = Paint()
    private val arc1 = RectF()
    private val arc2 = RectF()
    private val region = Region()
    private val textPaint = TextPaint()
    private var selected: Boolean = false

    private var itemName = ""

    internal var multiplier: Float = 0f
    internal var showText = true
    internal var color = 0xffd2c8b6.toInt()
    internal var sweep = 24f
    internal var image: Bitmap? = null

    private var onClick: (() -> Unit)? = null

    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        // do measurements
        val width = width.toFloat()
        val height = height.toFloat()
        val innerRadius: Float
        val thickness = height * 0.85f

        val currentAngle = 270 - sweep / 2

        val radius = width * multiplier

        innerRadius = radius - thickness

        val midX = width / 2
        val midY = radius

        pivotX = midX
        pivotY = midY


        //setup initial
        paint.reset()
        paint.isAntiAlias = true
        paint.color = color
        paint.strokeWidth = 35f
        paint.style = Paint.Style.FILL

        //this is the background arc, it remains constant
        p.reset()
        arc1.set(midX - radius, midY - radius, midX + radius, midY + radius)
        arc2.set(midX - innerRadius, midY - innerRadius, midX + innerRadius, midY + innerRadius)
        p.arcTo(arc1, currentAngle, sweep)
        p.arcTo(arc2, currentAngle + sweep, -sweep)
        p.close()


        // scale and render image
        if (image != null) {
            val density = DimenUtil.getDensity(context)

            val scale = .45f * density
            val x = 28f
            val y = -5f * density

            val matrix = Matrix()
            matrix.postTranslate(x, y)
            matrix.postScale(scale, scale)

            val shader = BitmapShader(image, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP)
            shader.setLocalMatrix(matrix)
            paint.shader = shader
        }

        region.set(0, 0, width.toInt(), height.toInt())
        canvas.drawPath(p, paint)


        if (selected) {
            val strokeWidth = 4
            val newSweep = sweep - 0.3f
            val newAngle = 270 - newSweep / 2
            val newThickness = thickness - strokeWidth / 2
            val newInnerRadius = radius - newThickness
            paint.reset()
            paint.isAntiAlias = true
            paint.color = blackOrWhite()
            paint.strokeWidth = strokeWidth.toFloat()
            paint.style = Paint.Style.STROKE

            p.reset()
            arc1.set(midX - radius + strokeWidth / 2,
                    midY - radius + strokeWidth / 2,
                    midX + radius - strokeWidth / 2,
                    midY + radius - strokeWidth / 2)
            arc2.set(midX - newInnerRadius + strokeWidth / 2,
                    midY - newInnerRadius + strokeWidth / 2,
                    midX + newInnerRadius - strokeWidth / 2,
                    midY + newInnerRadius - strokeWidth / 2)
            p.arcTo(arc1, newAngle, newSweep)
            p.arcTo(arc2, newAngle + newSweep, -newSweep)
            p.close()

            canvas.drawPath(p, paint)
        }

        if (showText) {
            textPaint.isAntiAlias = true
            textPaint.textSize = DimenUtil.dpToPx(context, 14).toFloat()
            textPaint.color = blackOrWhite()

            val textLayout = StaticLayout(itemName,
                                          textPaint,
                                          Math.round(canvas.width * 0.6f),
                                          Layout.Alignment.ALIGN_CENTER,
                                          1.0f,
                                          0.0f,
                                          false)

            canvas.save()
            // calculate x and y position where your text will be placed

            val textX = Math.round(width * 0.2f)
            val textY = ((canvas.height / 2 - (paint.descent() + paint.ascent()) / 2) * 0.8f).toInt() - textLayout.height / 2

            canvas.translate(textX.toFloat(), textY.toFloat())
            textLayout.draw(canvas)
            canvas.restore()
        }
    }

    private fun blackOrWhite() : Int {
        return if(ColorUtil.isBright(color)) Color.BLACK else Color.WHITE
    }

    override fun setSelected(selected: Boolean) {
        this.selected = selected
        invalidate()
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val state = event.action
        val touchX = event.x.toInt()
        val touchY = event.y.toInt()

        val r = Region()
        r.setPath(p, region)

        when (state) {
            MotionEvent.ACTION_UP ->
                if (r.contains(touchX, touchY)) {
                    onClick?.invoke()
                }
            MotionEvent.ACTION_SCROLL -> return false
        }
        return true
    }

    fun setParams(color: Int, name: String, multiplier: Float) {
        this.color = color
        this.itemName = name
        this.multiplier = multiplier
        invalidate()
    }

    fun setImagePath(path: String?) {
        ImageUtil.loadBitmap(path) {
            image = it
            invalidate()
        }
    }

    fun setSweep(sweep: Float) {
        this.sweep = sweep
        invalidate()
    }

    fun setColor(color: Int) {
        this.color = color
    }

    fun onClick(onClick: () -> Unit) {
        this.onClick = onClick
    }
}
