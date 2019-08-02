package com.cambriantech.harmony.view

import android.content.Context
import android.graphics.Canvas
import android.graphics.CornerPathEffect
import android.graphics.Paint
import android.graphics.Path
import android.graphics.drawable.Drawable
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import com.cambriantech.harmony.R
import com.cambriantech.harmony.util.DimenUtil
import com.cambriantech.harmony.util.ImageUtil
import com.cambriantech.harmony.util.resize
import java.io.File

class BeforeAfterSlider : View {

    enum class TouchMode {
        ALL,
        DELIMITER
    }

    private var paint: Paint = Paint()
    private var touchMode: BeforeAfterSlider.TouchMode? = null
    private var delimiterPosition: Int = 0
    private var materialMargin: Int = 0
    private var isMoving: Boolean = false
    private var delimiterColor: Int = 0
    private var delimiterWidth: Int = 0
    private var arrowColor: Int = 0
    private var arrowVisible: Boolean = true
    private var arrowWidth: Int = 0
    private var arrowHeight: Int = 0
    private var arrowStrokeWidth: Int = 0
    private var arrowFill: Boolean = false
    private var arrowMargin: Int = 0
    private var drawableLeft: Drawable? = null
    private var drawableRight: Drawable? = null
    private var arrowLeft: Path? = null
    private var arrowRight: Path? = null
    private var cornerPathEffect: CornerPathEffect? = null
    private var imageOffsetX = 0
    private var imageOffsetY = 0

    constructor(context: Context)
            : super(context)

    constructor(context: Context, attrs: AttributeSet?)
            : super(context, attrs) {
        initAttrs(attrs)
    }

    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int)
            : super(context, attrs, defStyleAttr) {
        initAttrs(attrs)
    }

    init {
        touchMode = TouchMode.ALL
        isMoving = false
        paint = Paint()
        arrowLeft = Path()
        arrowRight = Path()
        materialMargin = DimenUtil.dpToPx(context, 16)
    }

    private fun initAttrs(attrs: AttributeSet?) {
        if (attrs != null) {
            val a = this.context.theme.obtainStyledAttributes(attrs, R.styleable.BeforeAfterSlider, 0, 0)

            val arrowCornerRadius: Int
            try {
                drawableLeft = a.getDrawable(R.styleable.BeforeAfterSlider_drawableLeft)
                drawableRight = a.getDrawable(R.styleable.BeforeAfterSlider_drawableRight)
                delimiterColor = a.getColor(R.styleable.BeforeAfterSlider_delimiterColor, -1)
                delimiterWidth = a.getDimensionPixelSize(R.styleable.BeforeAfterSlider_delimiterWidth, 3)
                arrowColor = a.getColor(R.styleable.BeforeAfterSlider_arrowColor, -1)
                arrowVisible = a.getBoolean(R.styleable.BeforeAfterSlider_arrowVisibility, true)
                arrowWidth = a.getDimensionPixelSize(R.styleable.BeforeAfterSlider_arrowWidth, DimenUtil.dpToPx(context, 12))
                arrowHeight = a.getDimensionPixelSize(R.styleable.BeforeAfterSlider_arrowHeight, DimenUtil.dpToPx(context, 10))
                arrowMargin = a.getDimensionPixelSize(R.styleable.BeforeAfterSlider_arrowMargin, DimenUtil.dpToPx(context, 5))
                arrowStrokeWidth = a.getDimensionPixelSize(R.styleable.BeforeAfterSlider_arrowStrokeWidth, 5)
                arrowFill = a.getBoolean(R.styleable.BeforeAfterSlider_arrowFill, true)
                arrowCornerRadius = a.getDimensionPixelSize(R.styleable.BeforeAfterSlider_arrowCornerRadius, 0)
                touchMode = if (a.getInteger(R.styleable.BeforeAfterSlider_touchMode, 0) == 0)
                    TouchMode.ALL
                else
                    TouchMode.DELIMITER
            } finally {
                a.recycle()
            }

            cornerPathEffect = CornerPathEffect(arrowCornerRadius.toFloat())
        }

    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        val width = MeasureSpec.getSize(widthMeasureSpec)
        val height = MeasureSpec.getSize(heightMeasureSpec)
        delimiterPosition = width / 2

        setMeasuredDimension(width, height)

        drawableLeft?.let  { scaleDrawable(it, width, height) }
        drawableRight?.let { scaleDrawable(it, width, height) }
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val x = event.x
        if (arrowVisible) {
            redrawArrows()
        }

        when (event.action) {
            0 -> {
                if (touchMode == TouchMode.DELIMITER) {
                    if (x > (delimiterPosition + 200) || x < (delimiterPosition - 200)) {
                        return false
                    }

                    parent.requestDisallowInterceptTouchEvent(true)
                }
                isMoving = true
            }
            2 -> isMoving = true
            1, 3 -> {
                isMoving = false
                if (touchMode == TouchMode.DELIMITER) {
                    parent.requestDisallowInterceptTouchEvent(false)
                }
            }
        }

        delimiterPosition = (x / 1.0f).toInt()
        invalidate()
        return true
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if (delimiterPosition > 0) {
            drawableLeft?.let { drawable ->
                if (width - delimiterPosition < 0) {
                    delimiterPosition = width
                }


                val right = drawable.intrinsicWidth
                val bottom = drawable.intrinsicHeight


                drawable.setBounds(-imageOffsetX, -imageOffsetY, right - imageOffsetX, bottom - imageOffsetY)
                drawable.draw(canvas)
            }
        }

        paint.color = delimiterColor
        paint.strokeWidth = delimiterWidth.toFloat()
        paint.style = Paint.Style.STROKE
        canvas.drawLine(delimiterPosition.toFloat(), 0.0f, delimiterPosition.toFloat(), height.toFloat(), paint)
        if (arrowVisible && !isMoving) {
            redrawArrows()
            paint.color = arrowColor
            paint.style = if (arrowFill) Paint.Style.FILL else Paint.Style.STROKE
            paint.strokeWidth = arrowStrokeWidth.toFloat()
            paint.pathEffect = cornerPathEffect
            paint.isAntiAlias = true
            canvas.drawPath(arrowLeft!!, paint)
            this.paint.pathEffect = null
        }

        if (width - delimiterPosition > 0) {
            drawableRight?.let { drawable ->
                if (delimiterPosition < 0) {
                    delimiterPosition = 0
                }

                val right = drawable.intrinsicWidth - imageOffsetX
                val bottom = drawable.intrinsicHeight - imageOffsetY


                drawable.setBounds(-imageOffsetX, -imageOffsetY, right, bottom)
                canvas.clipRect(delimiterPosition + delimiterWidth / 2, 0, width, height)
                drawable.draw(canvas)
            }
        }

        if (arrowVisible && !isMoving) {
            paint.color = arrowColor
            paint.style = if (arrowFill) Paint.Style.FILL else Paint.Style.STROKE
            paint.strokeWidth = arrowStrokeWidth.toFloat()
            paint.pathEffect = cornerPathEffect
            paint.isAntiAlias = true
            canvas.drawPath(arrowRight!!, paint)
            paint.pathEffect = null
        }
    }

    private fun recreateArrowLeft() {
        arrowLeft?.apply {
            rewind()
            moveTo((delimiterPosition - delimiterWidth / 2 - arrowMargin - arrowWidth).toFloat(), (height / 2).toFloat())
            lineTo((delimiterPosition - delimiterWidth / 2 - arrowMargin).toFloat(), (height / 2 - arrowHeight / 2).toFloat())
            lineTo((delimiterPosition - delimiterWidth / 2 - arrowMargin).toFloat(), (height / 2 + arrowHeight / 2).toFloat())
            close()
        }
    }

    private fun recreateArrowRight() {
        arrowRight?.apply {
            rewind()
            moveTo((delimiterPosition + delimiterWidth / 2 + arrowMargin + arrowWidth).toFloat(), (height / 2).toFloat())
            lineTo((delimiterPosition + delimiterWidth / 2 + arrowMargin).toFloat(), (height / 2 - arrowHeight / 2).toFloat())
            lineTo((delimiterPosition + delimiterWidth / 2 + arrowMargin).toFloat(), (height / 2 + arrowHeight / 2).toFloat())
            close()
        }
    }

    fun setDrawableLeft(path: File) {
        ImageUtil.loadDrawable(path.toString()) { drawable ->
            scaleDrawable(drawable)
            drawableLeft = drawable
            if(drawableRight != null) invalidate()
        }
    }

    fun setDrawableRight(path: File) {
        ImageUtil.loadDrawable(path.toString()) { drawable ->
            scaleDrawable(drawable)
            drawableRight = drawable
            if(drawableLeft != null) invalidate()
        }
    }

    fun scaleDrawable(drawable: Drawable, viewX: Int? = null, viewY: Int? = null) {
        val viewWidth: Int = viewX ?: width
        val viewHeight: Int = viewY ?: height
        if(viewWidth <= 0 || viewHeight <= 0) return

        val imageWidth = drawable.intrinsicWidth
        val imageHeight = drawable.intrinsicHeight

        //val imageAspect = imageWidth.toFloat() / imageHeight.toFloat()
        val viewAspect = viewWidth.toFloat() / viewHeight.toFloat()


        //Timber.d("view width is $viewWidth")
        //Timber.d("view height is $viewHeight")
        //Timber.d("imagewidth is $imageWidth")
        //Timber.d("imageheight is $imageHeight")
        //Timber.d("imageAspect is $imageAspect")
        //Timber.d("viewAspect is $viewAspect")

        var scale: Float = 1f
        if(viewAspect < 1f) {
            scale = (minOf(imageWidth, imageHeight).toFloat() / minOf(viewWidth, viewHeight).toFloat())
            imageOffsetX = (((imageWidth * scale) - viewWidth) / 2).toInt()
            imageOffsetY = (((imageHeight * scale) - viewHeight) / 2).toInt()
        }
        //Timber.d("scale is $scale")

        //Timber.d("x offset is $imageOffsetX")
        //Timber.d("y offset is $imageOffsetY")

        drawable.resize((imageWidth * scale).toInt(), (imageHeight * scale).toInt())
    }

    fun isLandscape(): Boolean {
        return width > height
    }

    fun redrawArrows() {
        recreateArrowLeft()
        recreateArrowRight()
    }

    fun setRightText(text: String) {
        invalidate()
    }

    fun setLeftText(text: String) {
        invalidate()
    }
}
