package com.cambriantech.harmony.screens.visualizer

import android.annotation.SuppressLint
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.support.v7.graphics.drawable.DrawerArrowDrawable
import android.support.v7.view.SupportMenuInflater
import android.support.v7.view.menu.MenuBuilder
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.*
import android.view.animation.AccelerateDecelerateInterpolator
import butterknife.BindColor
import butterknife.BindDrawable
import butterknife.BindView
import butterknife.OnClick
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.HorizontalChangeHandler
import com.cambrian.cbar.sensor.representation.Quaternion
import com.cambrian.cbar.types.CBLightingType
import com.cambrian.cbar.types.CBToolMode
import com.cambriantech.harmony.PermissionsController
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.BundleBuilder
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.base.ToolbarProvider
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.data.CBAsset
import com.cambriantech.harmony.data.ProjectManager
import com.cambriantech.harmony.screens.project.ProjectImagePager
import com.cambriantech.harmony.util.DimenUtil
import com.cambriantech.harmony.util.hide
import com.cambriantech.harmony.util.setInvisible
import com.cambriantech.harmony.util.show
import com.cambriantech.harmony.view.BottomSheetDetail
import com.cambriantech.harmony.view.ProgressView
import com.cambriantech.harmony.wheel.WheelController
import io.realm.RealmList
import timber.log.Timber

/**
 * Created by Joseph Sullivan on 9/13/16.
 */

class VisualizerView(args: Bundle) : KtController(args), VisualizerInterface, ToolsView.ToolsCallback {

    companion object {
        private val KEY_PATH = "VisualizerView.path"
        private val KEY_ID  = "VisualizerView.id"
        private val KEY_QUAT = "VisualizerView.quat"
    }

    enum class Settings {
        NONE, LIGHTING
    }

    @BindView(R.id.visualizer_container)lateinit var visualizerContainer: ViewGroup
    @BindView(R.id.tools_view)          lateinit var toolsView: ToolsView
    @BindView(R.id.search_container)    lateinit var searchContainer: ViewGroup
    @BindView(R.id.wheel_container)     lateinit var wheelContainer: ViewGroup
    @BindView(R.id.bottom_container)    lateinit var bottomContainer: ViewGroup
    @BindView(R.id.progress_view)       lateinit var progressBar: ProgressView
    @BindView(R.id.rv_picker)           lateinit var rvList: RecyclerView
    @BindView(R.id.capture_controls)    lateinit var captureControls: ViewGroup
    @BindView(R.id.capture_button)      lateinit var buttonCapture: View
    @BindView(R.id.item_button)         lateinit var buttonFloatingItem: FloatingItemButton
    @BindView(R.id.wheel_hide_button)   lateinit var buttonHideWheel: View
    @BindView(R.id.bottom_sheet)        lateinit var bottomSheet: BottomSheetDetail

    @JvmField @BindColor(R.color.transparent)              var color: Int = 0
    @BindDrawable(R.drawable.translucent_toolbar) lateinit var toolbarDrawable: Drawable
    @BindDrawable(R.drawable.ic_cancel)           lateinit var iconCancel: Drawable


    private var controller: VisualizerController? = null
    private var wheelHidden = false
    private var value = CBLightingType.None

    private val quat = Quaternion()


    private var editing: Settings
        get() = controller?.editing ?: Settings.NONE
        set(value) { controller?.editing = value }

    private var selectedAsset: CBAsset? = null
        get() {
            val asset = controller?.selectedAsset
            Timber.d("selected item is ${asset?.item?.name}")
            return asset
        }

    constructor(path: String,
                id: String,
                quat: FloatArray? = null)
            : this(BundleBuilder(Bundle())
            .putString(KEY_PATH, path)
            .putString(KEY_ID, id)
            .putFloatArray(KEY_QUAT, quat)
            .build()) {

        controller = VisualizerController()
    }

    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_visualizer, container, false)
    }


    override fun onAttach(view: View) {
        super.onAttach(view)

        setTitle("")
        setToolbarColor(0x77000000)

        activity?.window?.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                                   WindowManager.LayoutParams.FLAG_FULLSCREEN)

        toolbar?.apply {
            background = toolbarDrawable
            animate()
                    .translationY((-DimenUtil.dpToPx(context, 25)).toFloat())
                    .setDuration(250)
                    .setStartDelay(150)?.start()
        }

        toolsView.callback = this
        setHasOptionsMenu(true)

        controller?.let { presenter ->
            presenter.attachView(this)

            if (!presenter.hasScene) {
                quat.set(args.getFloatArray(KEY_QUAT) ?: FloatArray(4, {0.0f}))
                Timber.d("valid quaternion %b", quat.isValid)
                presenter.init(visualizerContainer,
                               args.getString(KEY_PATH),
                               args.getString(KEY_ID),
                                quat)
            } else {
                Timber.i("Reattaching")
                presenter.reattach(visualizerContainer)
            }


            if(editing == Settings.LIGHTING) {
                editing = Settings.NONE
            }
        }

        initLighting()
        bottomSheet.init()
        initWheel()
    }

    override fun onDetach(view: View) {

        activity?.window?.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)
        toolbar?.run {
            animate()
                .translationY(0f)
                .setDuration(250).start()
        }

        setConfirmToolbar(false)

        Timber.d("detaching presenter")
        controller?.detachView()
        super.onDetach(view)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.toolbar_visualizer, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (item.itemId == R.id.paint_done) {
            controller?.save()
            return super.onOptionsItemSelected(item)
        }
        if (item.itemId == R.id.confirm) {
            when (editing) {
                Settings.LIGHTING -> openList(false)
                Settings.NONE -> return super.onOptionsItemSelected(item)
            }
            editing = Settings.NONE
            setConfirmToolbar(false)
            toolsView.hide(false)
        }
        return super.onOptionsItemSelected(item)
    }


    @SuppressLint("RestrictedApi")
    private fun initLighting() {
        val menu = MenuBuilder(context)
        val inflater = SupportMenuInflater(context)

        rvList.show()
        rvList.translationY = -1f

        controller?.lighting?.let { value = it }
        inflater.inflate(R.menu.dropdown_lighting, menu)

        val adapter = DropdownAdapter(menu, true, onClick = { position ->
            controller?.lighting = CBLightingType.fromOrdinal(position)
        })
        rvList.setHasFixedSize(true)
        rvList.adapter = adapter
        rvList.layoutManager = LinearLayoutManager(context, LinearLayoutManager.HORIZONTAL, false)
    }


    override fun initAssets(assets: RealmList<CBAsset>) {
        toolsView.initAssets(assets)
    }


    // Init wheel
    private fun initWheel() {
        if(wheelContainer.childCount > 1) return
        val paintOnly = controller?.hasWorldTransform()?.not() ?: true
        WheelController(context, wheelContainer, false,
                        itemSelected = { item ->
                            controller?.itemSelected(item)
                        })
    }

    override fun handleBack(): Boolean {
        if (bottomSheet.handleBack()) return true

        if(editing != Settings.NONE) {
            editing = Settings.NONE
            toolsView.hide(false)
            openList(false)
            setConfirmToolbar(false)
            return true
        }

        return false
    }


    /****************************
     *                          *
     *     Click Listeners      *
     *                          *
     ****************************/




    @OnClick(R.id.item_button) fun pressedColor() {
        selectedAsset?.let {
            bottomSheet.updateSheet(it.item)
        }
    }

    @OnClick(R.id.wheel_hide_button) fun pressedWheelToggle() {
        openWheel()
    }

    @OnClick(R.id.capture_button) fun pressedCapture() {
        controller?.capture()
        controller?.toolMode = CBToolMode.None
        buttonCapture.setInvisible()
        captureControls.show()
        hideAll()
    }

    @OnClick(R.id.capture_cancel) fun pressedCancel() {
        controller?.enableLive()
        buttonCapture.show()
        captureControls.hide()
        bottomContainer.show()
        toolsView.hide(false)
    }

    @OnClick(R.id.capture_confirm) fun pressedConfirm() {
        captureControls.hide()
        bottomContainer.show()
        controller?.toolMode = CBToolMode.Fill
        toolsView.hide(false)
    }

    @OnClick(R.id.lighting_button) fun pressedLighting() {
        editing = Settings.LIGHTING
        toolsView.hide(true)
        openList(true)
        openWheel(false)
        setConfirmToolbar(true)
    }


    private fun hideAll() {
        toolsView.hide(true)
        openList(false)
        openWheel(false)
        bottomContainer.hide()
    }

    override fun initStillMode() {
        buttonCapture.setInvisible()
    }

    override fun setToolMode(mode: CBToolMode) {
        toolsView.setToolMode(mode)
    }




    /****************************
     *                          *
     *     Event Listeners      *
     *                          *
     ****************************/



    //Tool callbacks
    override fun assetSelected(asset: CBAsset) {
        controller?.assetSelected(asset)
    }

    override fun assetRemoved(asset: CBAsset) {
        controller?.removeAsset(asset)
    }

    override fun assetAdded() {
        controller?.assetAdded()
    }

    override fun itemSelected(item: BrandItem) {
        controller?.itemSelected(item)
    }

    override fun undoPressed() {
        controller?.undo()
    }

    override fun hideWheel() {
        openWheel(false)
    }


    override fun isLive(): Boolean {
        return controller?.isLive ?: false
    }

    override fun updateFloatingButton(asset: CBAsset?) {
        buttonFloatingItem.update(asset)
    }

    override fun showProgress() {
        progressBar.showProgress()
    }

    override fun hideProgress() {
        progressBar.hideProgress()
    }

    override fun saveComplete() {
        /*
        Manage backstack when leaving the painter. By default, when clicking done  a new instance of project overview is launched.
        This means if the user presses "back" after pressing "done", the visualizer would reopen which is not natural or expected behavior.
        This checks through the backstack to see if the project overview has already been opened, and if so it culls the backstack to that point
         */

        val size = router.backstackSize
        for (i in size - 1 downTo 4) {
            if (router.backstack[i].controller() is ProjectImagePager) {
                //Timber.d("instance at index " + i)
                //Timber.d("popping to project view")
                router.popCurrentController()
                return
            }
        }
        //Timber.i("launching new instance of project view")
        val backstack = router.backstack
        if (router.backstack[backstack.size - 1].controller() is (PermissionsController)) {
            backstack.removeAt(backstack.size - 1)
        }
        backstack.add(backstack.size - 1, RouterTransaction.with(ProjectImagePager(ProjectManager.currentProject.id))
                .pushChangeHandler(HorizontalChangeHandler(150))
                .popChangeHandler(HorizontalChangeHandler(150)))
        router.setBackstack(backstack, HorizontalChangeHandler(100))
        router.popCurrentController()

        //Timber.d("finished altering backstack")
        hideProgress()
    }



    /****************************
     *                          *
     *        Animations        *
     *                          *
     ****************************/




    private fun openList(open: Boolean) {
        rvList.show()
        val destination = if (open) -rvList.height.toFloat() else -1f

        rvList.animate()
                .translationY(destination)
                .setInterpolator(AccelerateDecelerateInterpolator()).start()
    }

    override fun updateAssets(asset: CBAsset?) {
        toolsView.updateAssets(asset)
    }

    override fun enableUndoButton(enable: Boolean) {
        toolsView.enableUndoButton(enable)
    }

    private fun openWheel(open: Boolean? = null) {
        var rotation = 0f //initial "open" values
        var translation = 0f
        if (open == !wheelHidden) return
        if (open != true && !wheelHidden) { //set values to close
            rotation = 180f
            translation = (bottomContainer.height - buttonHideWheel.height).toFloat()
        }
        bottomContainer.animate()
                .translationY(translation)
                .setInterpolator(AccelerateDecelerateInterpolator())
                .setDuration(180).start()
        buttonHideWheel.animate()
                .rotation(rotation)
                .setInterpolator(AccelerateDecelerateInterpolator())
                .setDuration(180)
                .setStartDelay(250).start()
        wheelHidden = !wheelHidden
    }

    private fun setConfirmToolbar(enabled: Boolean) {
        toolbar?.run {
            if (!enabled) {
                navigationIcon = DrawerArrowDrawable(context)
                if(menu.size() > 1) {
                    menu.getItem(0).isVisible = true
                    menu.getItem(1).isVisible = false
                }
                (activity as ToolbarProvider).resetNav()
            } else {
                navigationIcon = iconCancel
                if(menu.size() > 1) {
                    menu.getItem(0).isVisible = false
                    menu.getItem(1).isVisible = true
                }
                setNavigationOnClickListener({
                    cancel()
                    toolsView.hide(false)
                })
            }
        }
    }

    private fun cancel() {
        when (editing) {
            Settings.LIGHTING -> {
                //controller?.lighting = value
                openList(false) }
            Settings.NONE -> return
        }
        editing = Settings.NONE
        setConfirmToolbar(false)
        toolsView.hide(false)
    }
}
