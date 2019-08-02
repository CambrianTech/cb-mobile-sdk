package com.cambriantech.harmony.screens.project

import android.app.Activity
import android.app.ProgressDialog
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.support.v4.content.FileProvider
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.view.animation.AccelerateDecelerateInterpolator
import butterknife.BindDrawable
import butterknife.BindView
import butterknife.OnClick
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.HorizontalChangeHandler
import com.cambriantech.harmony.BuildConfig
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.BundleBuilder
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.base.MvpView
import com.cambriantech.harmony.data.CBImage
import com.cambriantech.harmony.data.CBProject
import com.cambriantech.harmony.data.ProjectManager
import com.cambriantech.harmony.dialogs.ConfirmationDialog
import com.cambriantech.harmony.screens.visualizer.VisualizerView
import com.cambriantech.harmony.util.DimenUtil
import com.cambriantech.harmony.util.ImageStitch
import com.cambriantech.harmony.view.BottomSheetDetail
import com.cambriantech.harmony.view.LockableLLM
import com.cambriantech.harmony.view.LockablePager
import com.cambriantech.harmony.view.LockedLLM
import com.crashlytics.android.answers.Answers
import com.crashlytics.android.answers.ShareEvent
import timber.log.Timber
import java.io.File

class ProjectImagePager(args: Bundle) : KtController(args), MvpView {

    @BindView(R.id.image_rv)            lateinit var rvImages: RecyclerView
    @BindView(R.id.button_group)        lateinit var buttonGroup: ViewGroup
    @BindView(R.id.asset_rv)            lateinit var rvAssets: RecyclerView
    @BindView(R.id.bottom_sheet)        lateinit var bottomSheet: BottomSheetDetail
    @BindView(R.id.empty_warning)       lateinit var emptyTextView: View
    @BindView(R.id.go_back)             lateinit var buttonBack: View

    @BindDrawable(R.drawable.translucent_toolbar) lateinit var toolbarDrawable: Drawable

    lateinit private var project: CBProject
    private var progressDialog: ProgressDialog? = null
    private var shareFile: File? = null
    private val provider = BuildConfig.APPLICATION_ID + ".provider"

    private val pagerHelper = LockablePager()
    private lateinit var layout: LockableLLM
    private lateinit var adapter: ImageAdapter
    private var pos: Int = 0
    private var uiVisible = true
    private var assetsVisible = false
    private var buttonsVisible = true

    private val duration = 250L

    private var comparing = false

    companion object {
        private val KEY_ID = "id"
    }

    constructor(id: String) : this(BundleBuilder(Bundle())
            .putString(KEY_ID, id)
            .build())

    init {
        val id = getArgs().getString(KEY_ID)
        val result = ProjectManager.getProjectFromID(id)
        if(result != null) project = result
        else router.popCurrentController()
    }

    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View =
            inflater.inflate(R.layout.controller_project_pager, container, false)

    override fun onAttach(view: View) {
        super.onAttach(view)

        setTitle(project.name)
        toolbar?.background = toolbarDrawable

        bottomSheet.init()

        adapter = ImageAdapter(
                images = project.sortedImages,
                onImageClick = { toggleUI() })
        layout = LockableLLM(context, LinearLayoutManager.HORIZONTAL, false)

        if(project.sortedImages.isEmpty()) {
            noImages()
        }


        rvImages.layoutManager = layout
        rvImages.adapter = adapter
        pagerHelper.attachToRecyclerView(rvImages)

        val lllm = LockedLLM(context, LinearLayoutManager.HORIZONTAL, false)
        rvAssets.layoutManager = lllm

        rvImages.scrollToPosition(pos)

        progressDialog = ProgressDialog(context)
        progressDialog?.setTitle("Preparing image...")
        progressDialog?.setMessage("This shouldn't take long")
    }


    private fun shareImage(path: File) {
        shareFile = path
        val uri = FileProvider.getUriForFile(context, provider, shareFile!!)
        val intent = Intent(Intent.ACTION_SEND)
        intent.type = "image/jpeg"
        intent.putExtra(Intent.EXTRA_STREAM, uri)
        intent.flags = Intent.FLAG_GRANT_READ_URI_PERMISSION
        progressDialog?.dismiss()
        Answers.getInstance().logShare(ShareEvent())

        val resInfoList = context.packageManager?.queryIntentActivities(intent, PackageManager.MATCH_DEFAULT_ONLY)
        resInfoList?.forEach {
            val packageName = it.activityInfo.packageName
            context.grantUriPermission(packageName, uri, Intent.FLAG_GRANT_WRITE_URI_PERMISSION or Intent.FLAG_GRANT_READ_URI_PERMISSION)
        }

        if(intent.resolveActivity(context.packageManager) != null) {
            startActivity(Intent.createChooser(intent, "Share Image"))
        }
    }

    private fun noImages() {
        rvImages.visibility = View.INVISIBLE
        buttonGroup.visibility = View.INVISIBLE

        emptyTextView.visibility = View.VISIBLE
        buttonBack.visibility = View.VISIBLE
    }


    private fun toggleUI() {
        val toolbarAmount = if(uiVisible) -DimenUtil.dpToPx(context, 76) else 0
        toggleButtons()

        if(uiVisible) {
            activity?.window?.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                                       WindowManager.LayoutParams.FLAG_FULLSCREEN)
        } else {
            activity?.window?.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)
        }

        toolbar?.apply {
            animate()
                    .translationY(toolbarAmount.toFloat())
                    .setDuration(duration)
                    .start()
        }
        uiVisible = !uiVisible
    }

    private fun toggleButtons() {
        Timber.d("toggling buttons")
        val amount = if(buttonsVisible) buttonGroup.height else -buttonGroup.height
        Timber.d("amount " + amount)
        buttonGroup.animate()
                .translationYBy(amount.toFloat())
                .setInterpolator(AccelerateDecelerateInterpolator())
                .setDuration(duration)
                .start()
        buttonsVisible = !buttonsVisible
    }

    @OnClick(R.id.go_back) fun goBack() {
        router.popCurrentController()
    }

    @OnClick(R.id.info_button)
    fun toggleAssets() {
        if(assetsVisible) {
            rvAssets.visibility = View.GONE
            layout.canScroll = true
        } else {
            undoToggles()
            val assets = currentImage().assets
            val assetAdapter = ImageAssetAdapter(assets) { pos ->
                assets[pos]?.item?.let {
                    bottomSheet.updateSheet(it)
                }
            }
            rvAssets.adapter = assetAdapter
            rvAssets.visibility = View.VISIBLE
            layout.canScroll = false
        }
        assetsVisible = !assetsVisible
    }

    @OnClick(R.id.swap_button)
    fun swap() {
        if(assetsVisible) toggleAssets()
        comparing = !comparing
        //Timber.d("swapping, locked: $comparing")
        adapter.comparing = comparing
        layout.canScroll = !comparing
        adapter.notifyItemChanged(getPosition())
        toggleUI()
    }



    @OnClick(R.id.edit_button)
    fun startVisualizer() {
        pos = getPosition()
        val id = currentImage().id
        Timber.d("starting visualizer...")
        router.pushController(RouterTransaction.with(VisualizerView("", id))
                .pushChangeHandler(HorizontalChangeHandler(100))
                .popChangeHandler(HorizontalChangeHandler(100)))
    }

    @OnClick(R.id.delete_button)
    fun deleteClicked() {
        ConfirmationDialog("Delete Image",
                           "Are you sure?",
                           context,
                           onConfirm = {
            val pos = getPosition()
            project.sortedImages[pos]!!.delete()
            rvAssets.adapter = null
            if(assetsVisible) toggleAssets()
            adapter.notifyItemRemoved(pos)
            if(project.images.isEmpty()) noImages()
        })
    }

    @OnClick(R.id.share_button)
    fun shareClicked() {
        val image = currentImage()
        val before = image.originalPath
        val after = image.previewOrOriginal
        if (before.exists() && after.exists()) {
            progressDialog?.show()
            ImageStitch(before.toString(), after.toString(), { result ->
                shareImage(File(result))
            })
        } else {
            Timber.i("image hasn't been modified, sharing single image")
            Timber.v("id: " + image.originalPath)
            shareImage(image.originalPath)
        }
    }

    private fun getPosition() : Int {
        val tag = pagerHelper.findSnapView(layout)?.tag as String?
        return project.sortedImages.indexOf(project.getImageForID(tag))
    }

    private fun currentImage() : CBImage = project.sortedImages[getPosition()]!!

    private fun undoToggles() : Boolean {
        if(bottomSheet.handleBack()) return true
        if(assetsVisible) {
            toggleAssets()
            return true
        }
        if(comparing) {
            swap()
            return true
        }
        if(!uiVisible) {
            toggleUI()
            return true
        }
        return false
    }

    override fun handleBack(): Boolean = undoToggles()


    private fun revokeFileReadPermission() {
        shareFile?.let { file ->
            val uri = FileProvider.getUriForFile(context, provider, file)
            context.revokeUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
        }

    }

    override fun onActivityResumed(activity: Activity) {
        super.onActivityResumed(activity)
        revokeFileReadPermission()
    }

    override fun onActivityStopped(activity: Activity) {
        super.onActivityStopped(activity)
        revokeFileReadPermission()
    }
}

