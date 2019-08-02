package com.cambriantech.harmony.screens.home

import com.cambriantech.harmony.base.Presenter

/**
 * Created by Joseph Sullivan on 9/13/16.
 */

class HomePresenter : Presenter<HomeInterface> {

    private var homeView: HomeInterface? = null

    override fun attachView(view: HomeInterface) {
        this.homeView = view
    }

    override fun detachView() {
        this.homeView = null
    }

}
