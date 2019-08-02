package com.cambriantech.harmony.view

import android.content.Context
import android.util.AttributeSet
import android.view.LayoutInflater
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.util.DimenUtil
import com.cambriantech.harmony.util.hide
import com.cambriantech.harmony.util.onComplete
import com.cambriantech.harmony.util.show

/**
 * Created by Joseph on 12/13/2017.
 */

class ProgressView : RoundedContainer {

    private var animating = false

    override var radius: Float? = DimenUtil.dpToPx(HHApp.context, 16).toFloat()

    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)


    init {
        val inflater = context.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        inflater.inflate(R.layout.progress_view, this, true)
    }


    fun showProgress() {
        animating = true
        this.show()
        this.animate()
                .alpha(1f)
                .setDuration(100L)
                .onComplete { animating = false }
                .start()
    }

    fun hideProgress() {
        this.animate()
                .alpha(0f)
                .setDuration(100L)
                .onComplete { if(!animating) { this.hide() } }
                .start()
    }
}