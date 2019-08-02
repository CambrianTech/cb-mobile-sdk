package com.cambriantech.harmony.screens.projectlist

import com.cambriantech.harmony.base.MvpView
import com.cambriantech.harmony.data.CBProject
import io.realm.RealmList

/**
 * Created by joseph on 9/15/16.
 */

interface ProjectListInterface : MvpView {
    fun initList(projects: RealmList<CBProject>)
}