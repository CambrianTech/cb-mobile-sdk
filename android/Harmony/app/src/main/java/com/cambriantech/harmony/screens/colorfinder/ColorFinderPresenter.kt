package com.cambriantech.harmony.screens.colorfinder

import com.cambriantech.harmony.base.Presenter

/**
 * Created by Joseph on 10/19/2017.
 */

class ColorFinderPresenter : Presenter<ColorFinderInterface> {

    var view: ColorFinderInterface? = null

    override fun attachView(view: ColorFinderInterface) {
        this.view = view
    }

    override fun detachView() {
        this.view = null
    }
}
