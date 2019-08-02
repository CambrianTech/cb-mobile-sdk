package com.cambriantech.harmony.view

import android.content.Context
import android.util.Size
import android.view.ViewGroup
import com.cambriantech.harmony.util.ColorUtil
import com.cambriantech.harmony.util.DimenUtil
import timber.log.Timber

/**
 * Created by Joseph on 10/19/2017.
 */
class ColorView : RoundButtonBordered {

    private var size: Int

    constructor(context: Context, size: Int, container: ViewGroup) : super(context) {
        this.size = DimenUtil.dpToPx(context, size)

        Timber.d("view size is ${this.size}")
        this.layoutParams = ViewGroup.LayoutParams(this.size, this.size)
    }


    fun scaleUp() {
        val size = Size(width, height)
        val params = this.layoutParams
        params.width = this.width * 2
        params.height = this.height * 2
        this.layoutParams = params
        this.x = x + size.width
        this.y = y + size.height
        invalidate()
    }

    fun scaleDown() {
        val size = Size(width, height)
        val params = this.layoutParams
        params.width = this.width / 2
        params.height = this.height / 2
        this.layoutParams = params
        this.x = x + size.width / 2
        this.y = y + size.height / 2
        invalidate()
    }

    override fun setColor(color: Int) {
        this.isSelected = !ColorUtil.isBright(color)
        super.setColor(color)
    }
}