package com.cambriantech.harmony.screens.visualizer

import android.content.Context
import android.graphics.*
import android.graphics.drawable.Drawable
import android.support.v4.content.res.ResourcesCompat
import android.support.v4.graphics.drawable.DrawableCompat
import android.util.AttributeSet
import android.widget.ImageView
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.CBAsset
import com.cambriantech.harmony.util.ColorUtil
import com.cambriantech.harmony.util.ImageUtil
import com.cambriantech.harmony.util.show
import timber.log.Timber

/**
 * Created by Joseph on 3/13/2017.
 */

class FloatingItemButton : ImageView {


    private val white = Color.WHITE
    private val black = Color.BLACK
    private var color = 0xffd2c8b6.toInt()

    private val path = Path()
    private val paint = Paint()
    private var image: Bitmap? = null
    private var icon: Drawable? = null


    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        paint.isAntiAlias = true


        val size = (width * 0.5).toFloat()
        if(image != null) {

            paint.shader = BitmapShader(image, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP)

            path.reset()
            path.addCircle(width/2f, width/2f, size, Path.Direction.CW)

            canvas.drawPath(path, paint)

        } else {
            paint.reset()
            paint.color = color
            canvas.drawCircle(width/2f, width/2f, size, paint)
        }

        if(icon == null) {
            icon = ResourcesCompat.getDrawable(context.resources, R.drawable.ic_info, null)
        }
        icon?.let { icon ->
            val start = (width * 0.2f).toInt()
            val end = (width * 0.8f).toInt()
            icon.setBounds(start, start, end, end)
            val isBright = ColorUtil.isBright(color)
            DrawableCompat.setTint(icon, if(isBright) black else white)
            icon.draw(canvas)
        }

    }

    fun update(asset: CBAsset?) {
        asset?.item?.color?.let {
            setColor(it)
            show()
        }
        asset?.item?.resource?.thumbnailUrl?.let {
            setImage(it)
            show()
        }
        if(asset?.item?.resource?.thumbnailUrl == null) {
            Timber.w("clearing image of asset %s", asset?.item?.name)
            image = null
            invalidate()
        }
    }

    private fun setColor(color: Int) {
        this.color = color
        invalidate()
    }

    private fun setImage(path: String) {
        ImageUtil.loadBitmap(path) {
            image = it
            invalidate()
        }
    }
}
