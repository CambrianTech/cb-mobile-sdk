package com.cambriantech.harmony.util

import android.animation.ObjectAnimator
import android.animation.ValueAnimator
import android.view.View
import android.view.animation.AccelerateInterpolator
import android.view.animation.OvershootInterpolator
import butterknife.ButterKnife
import timber.log.Timber

/**
 * Created by Joseph on 5/11/2017.
 */

object Animations {

    val bounce: ButterKnife.Action<View> = ButterKnife.Action { view, index ->
        val animX = ObjectAnimator.ofFloat(view, "scaleX", 1.0f, 1.07f)
        animX.repeatMode = ValueAnimator.REVERSE
        animX.duration = 60
        animX.repeatCount = 1
        animX.start()

        val animY = ObjectAnimator.ofFloat(view, "scaleY", 1.0f, 1.07f)
        animY.repeatMode = ValueAnimator.REVERSE
        animY.duration = 60
        animY.repeatCount = 1
        animY.start()
    }


    fun fade(view: View, fadeIn: Boolean = true) {
        val alpha = if(fadeIn) 1f else 0f
        if(fadeIn) {
            Timber.d("fading in")
            view.alpha = 0f
            view.show()
        }
        view.animate()
                .alpha(alpha)
                .setDuration(150)
                .onComplete {
                    if(!fadeIn) {
                        view.hide()
                    }
                }.start()
    }


    fun show(showing: Boolean? = null, view: View, vert: Boolean, onComplete: (Boolean) -> Unit) {
        if(showing == true && view.isVisible()) return
        if(showing == false && view.isHidden()) return
        val show = showing ?: (view.isHidden())

        val translation =
        if(vert)
            if(show) 0f else ((-view.height / 4).toFloat())
        else
            if(show) 0f else ((view.width / 2).toFloat())

        val scale = if(show) 1f else 0f

        var duration = 50
        if(show) {
            view.show()
            view.scaleX = 0f
            view.scaleY = 0f
            view.alpha = 0f
            duration = 170
            if(vert)
                view.translationY = (-view.height).toFloat()
            else
                view.translationX = (view.width / 2).toFloat()
        }
        var translationX = 0f
        var translationY = 0f
        if(vert)
            translationY = translation
        else
            translationX = translation
        view.animate()
                //.alpha(scale)
                .scaleX(scale)
                .scaleY(scale)
                .translationX(translationX)
                .translationY(translationY)
                .setDuration(170)
                .setInterpolator(OvershootInterpolator())
                .onComplete {
                    onComplete(show)
                    if(!show)
                        view.visibility = View.INVISIBLE
                }.start()


        view.animate()
                .alpha(scale)
                .setDuration(duration.toLong())
                .setInterpolator(AccelerateInterpolator()).start()

    }
}
