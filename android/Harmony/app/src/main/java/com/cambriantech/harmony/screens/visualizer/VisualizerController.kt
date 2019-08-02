package com.cambriantech.harmony.screens.visualizer

import android.support.design.widget.CoordinatorLayout
import android.view.View
import android.view.ViewGroup
import com.cambrian.cbar.remodeling.*
import com.cambrian.cbar.sensor.representation.Quaternion
import com.cambrian.cbar.types.CBLightingType
import com.cambrian.cbar.types.CBToolMode
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.base.Presenter
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.data.CBAsset
import com.cambriantech.harmony.data.CBImage
import com.cambriantech.harmony.data.ProjectManager
import timber.log.Timber
import java.lang.RuntimeException
import java.util.*

/**
 * Created by Joseph Sullivan on 9/13/16.
 */

internal class VisualizerController
    : Presenter<VisualizerInterface>,
        CBRemodelingView.CBRemodelingViewListener {

    private var visualizerView: VisualizerInterface? = null
    private var augmentedView: CBRemodelingView? = null

    private var wasLive = false
    private var exiting = false

    private lateinit var image: CBImage
    var selectedAsset: CBAsset? = null
        set(value) {
            field = value
            visualizerView?.updateFloatingButton(value)
            visualizerView?.updateAssets(value)
        }

    var hasScene = false
    private lateinit var quat: Quaternion
    var editing = VisualizerView.Settings.NONE

    override fun attachView(view: VisualizerInterface) {
        Timber.d("has scene: " + hasScene)
        this.visualizerView = view
        visualizerView?.updateFloatingButton(selectedAsset)
    }

    override fun detachView() {
        augmentedView?.let {

            wasLive = it.isLive
            Timber.i("is live: $wasLive")
            if(!it.isLive && !exiting) {
                Timber.i("saving for rotation")
                it.saveToDirectory(image.rootPath, false)
            }
            hasScene = !it.isLive
            Timber.i("has scene: $hasScene")
            it.stopRunning()
            (it.parent as ViewGroup).removeView(it)

            this.augmentedView = null
            this.visualizerView = null
        }

    }

    fun init(container: ViewGroup, path: String, id: String, quat: Quaternion) {
        Timber.d("initializing visualizer presenter")
        initView(container)

        this.quat = quat

        if(id.isEmpty() && path.isEmpty()) {
            initLive()
        } else if (id.isNotEmpty() && path.isEmpty()) {
            image = ProjectManager.currentProject.getImageForID(id)!!
            initWithScene(id)
        } else if(path.isNotEmpty() && id.isNotEmpty()) {
            initWithImage(path, id)
        }

        visualizerView?.initAssets(image.assets) ?: Timber.w("Visualizer View is null")
    }

    private fun initView(container: ViewGroup) {
        try {
            augmentedView = CBRemodelingView(container.context)
            augmentedView?.layoutParams = CoordinatorLayout.LayoutParams(CoordinatorLayout.LayoutParams.MATCH_PARENT, CoordinatorLayout.LayoutParams.MATCH_PARENT)
            container.addView(augmentedView, 0)
            augmentedView?.setListener(this)
            augmentedView?.visibility = View.VISIBLE
        } catch (e: RuntimeException) {
            Timber.e(e)
        }
    }

    private fun initLive() {
        Timber.d("creating live visualizer")
        if(!wasLive) {
            image = CBImage(UUID.randomUUID().toString())
        }
        Timber.d("new image id is: " + image.id)
        augmentedView?.isLive = true
        augmentedView?.scene = CBRemodelingScene()
        image.assets.forEach { augmentedView?.scene?.assets?.put(it.coreAsset.assetID, it.coreAsset) }
        if (image.assets.isNotEmpty()) { assetSelected(image.assets.first()!!) }
        toolMode = CBToolMode.Fill
    }

    private fun initWithImage(path: String, id: String) {
        Timber.d("create scene with new image")
        image = CBImage(id)
        visualizerView?.initStillMode()

        augmentedView?.isLive = false
        augmentedView?.scene = CBRemodelingScene(image.id, path)

        Timber.d("created new scene from image with id ${image.id}")
        Timber.d("scene is at ${image.rootPath}")

        hasScene = true
        toolMode = CBToolMode.Fill

        if(quat.isValid) {
            Timber.d("setting world transform to quat: " + quat.toString())
            augmentedView?.setWorldTransform(quat)
            Timber.d("has world transform: ${hasWorldTransform()}")
        }
    }

    private fun initWithScene(id: String) {
        Timber.d("load existing scene with path: " + image.rootPath)

        visualizerView?.initStillMode()
        augmentedView?.isLive = false
        augmentedView?.scene = CBRemodelingScene(image.rootPath)

        //Timber.i("Updating core assets")
        image.assets.forEach { cbasset ->
            val asset = augmentedView?.scene?.getAsset(cbasset.id)
            when(asset) {
                is CBRemodelingFloor -> setFlooring(cbasset)
                is CBRemodelingPaint -> asset.color = cbasset.item.color
                else -> Timber.w("no core asset match")
            }
        }
        selectedAsset = image.assets.firstOrNull { it.id == augmentedView?.scene?.selectedAssetID }

        toolMode = CBToolMode.Fill
        hasScene = true
    }


    fun reattach(container: ViewGroup) {
        Timber.i("reattaching")

        initView(container)

        initWithScene(image.id)

        visualizerView?.initAssets(image.assets) ?: Timber.w("Visualizer View is null")
        visualizerView?.updateFloatingButton(selectedAsset)
    }

    fun capture() {
        augmentedView?.captureToStill()
        toolMode = CBToolMode.Fill
    }

    fun enableLive() {
        augmentedView?.startCamera()
    }

    fun itemSelected(item: BrandItem) {
        if (image.assets.isEmpty()) {
            appendAsset(CBAsset(item))
            updateAsset(item)
            return
        }
        if(item.isFloor() && image.hasFloor()) {
            assetSelected(image.getFloor()!!)
            updateAsset(item)
            return
        } else if (item.isFloor() && !image.hasFloor()) {
            appendAsset(CBAsset(item))
            updateAsset(item)
            return
        }
        if(item.isPaint() && selectedAsset?.item?.isFloor() == true) {
            appendAsset(CBAsset(item))
            updateAsset(item)
            return
        }
        updateAsset(item)
    }

    fun assetAdded() {
        appendPaint()
    }

    fun appendFloor() {
        appendAsset(CBAsset.randomFloor())
    }

    fun appendPaint() {
        appendAsset(CBAsset.randomPaint())
    }

    private fun appendAsset(asset: CBAsset) {
        if (augmentedView?.scene?.canAppendAsset() == true) {
            augmentedView?.scene?.assets?.put(asset.coreAsset.assetID, asset.coreAsset)

            HHApp.realmProjects.executeTransaction {
                image.assets.add(asset)
            }
            assetSelected(asset)
        }
    }

    fun removeAsset(asset: CBAsset) {
        augmentedView?.scene?.removeAsset(asset.id)
        image.assets.remove(asset)
        val selected = image.assets.firstOrNull { it.id == augmentedView?.scene?.selectedAssetID }
        selectedAsset = selected
    }

    fun assetSelected(asset: CBAsset) {
        selectedAsset = asset
        augmentedView?.scene?.selectedAssetID = asset.id
    }

    // Set color of currently active layer
    private fun updateAsset(item: BrandItem) {
        selectedAsset?.let { asset ->
            HHApp.realmProjects.executeTransaction {
                asset.item = item
            }
            assetSelected(asset)
            augmentedView?.scene?.selectedAsset = asset.coreAsset
            val coreAsset = augmentedView?.scene?.selectedAsset
            when (coreAsset) {
                is CBRemodelingPaint -> coreAsset.color = item.color
                is CBRemodelingFloor -> setFlooring(asset)
                else -> {}
            }
        }
    }

    private fun setFlooring(asset: CBAsset) {
        if(quat.isValid) {
            Timber.d("valid quat")
            if(!isLive) {
                Timber.d("is still")
            }
        }
        visualizerView?.showProgress()
        asset.item.resource?.loadFlooring { diffuse, normal, roughness ->
            val floor = augmentedView?.scene?.getAsset(asset.id)
            //val floor = asset.coreAsset
            if(floor is CBRemodelingFloor) {
                floor.setScale(asset.item.scale)
                floor.setDiffuse(diffuse)
                floor.setNormalRoughness(normal,roughness)
                visualizerView?.hideProgress()
            }
        }
    }

    fun undo() {
        augmentedView?.undo()
    }

    override fun historyChanged(assetID: String?, change: CBRemodelingTypes.CBUndoChange?, forward: Boolean) {
        Timber.d("history changed")
        Timber.d("id: $assetID, change: ${change.toString()}, forward: $forward")
        val size = augmentedView?.undoSize ?: 0
        visualizerView?.enableUndoButton(size > 0)
    }


    var toolMode: CBToolMode
        get() = augmentedView?.toolMode ?: CBToolMode.None
        set(mode) {
            Timber.d("setting tool mode to " + mode.toString())
            augmentedView?.toolMode = mode
            visualizerView?.setToolMode(mode)
        }

    val isLive: Boolean
        get() = augmentedView?.isLive ?: false


    var lighting: CBLightingType
        get() = augmentedView?.scene?.lightingAdjustment ?: CBLightingType.None
        set(type) { augmentedView?.scene?.lightingAdjustment = type }


    fun hasWorldTransform() = augmentedView?.scene?.hasWorldTransform() ?: false


    fun save() {
        visualizerView?.showProgress()
        Timber.i("Saving image to " + image.rootPath)
        augmentedView?.saveToDirectory(image.rootPath, false)
    }

    override fun saveComplete() {
        Timber.i("finished saving at ${image.rootPath}")
        HHApp.realmProjects.executeTransaction {
            val project = ProjectManager.currentProject
            if(!project.images.contains(image)) {
                project.images.add(image)
            }
            image.modified()
        }
        exiting = true
        visualizerView?.saveComplete()
    }
}
