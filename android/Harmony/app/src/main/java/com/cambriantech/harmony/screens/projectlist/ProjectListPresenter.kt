package com.cambriantech.harmony.screens.projectlist

import com.cambriantech.harmony.base.Presenter

/**
 * Created by Joseph Sullivan on 9/15/16.
 */

class ProjectListPresenter : Presenter<ProjectListInterface> {

    private var projectListView: ProjectListInterface? = null

    override fun attachView(view: ProjectListInterface) {
        this.projectListView = view
        initList()
    }

    override fun detachView() {
        this.projectListView = null
    }

    private fun initList() {

    }
}
