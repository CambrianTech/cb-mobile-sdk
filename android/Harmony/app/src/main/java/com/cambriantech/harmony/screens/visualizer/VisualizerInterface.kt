package com.cambriantech.harmony.screens.visualizer


import com.cambrian.cbar.types.CBToolMode
import com.cambriantech.harmony.base.MvpView
import com.cambriantech.harmony.data.CBAsset
import io.realm.RealmList

/**
 * Created by Joseph Sullivan on 9/13/16.
 */

interface VisualizerInterface : MvpView {

    fun initAssets(assets: RealmList<CBAsset>)
    fun initStillMode()
    fun updateAssets(asset: CBAsset?)
    fun updateFloatingButton(asset: CBAsset?)
    fun setToolMode(mode: CBToolMode)
    fun showProgress()
    fun hideProgress()
    fun saveComplete()
    fun enableUndoButton(enable: Boolean)
}

