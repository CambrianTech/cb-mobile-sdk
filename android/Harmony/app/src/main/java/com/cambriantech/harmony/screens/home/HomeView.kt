package com.cambriantech.harmony.screens.home

import android.app.ProgressDialog
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.FrameLayout
import butterknife.BindColor
import butterknife.BindView
import butterknife.OnClick
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.HorizontalChangeHandler
import com.cambrian.cbar.CameraInfo
import com.cambriantech.harmony.GalleryController
import com.cambriantech.harmony.PermissionsController
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.data.ProjectManager
import com.cambriantech.harmony.screens.colorfinder.ColorFinderController
import com.cambriantech.harmony.screens.explorecolor.CategoryView
import com.cambriantech.harmony.screens.photo.TakePhotoController
import com.cambriantech.harmony.screens.project.ProjectImagePager
import com.cambriantech.harmony.screens.visualizer.VisualizerView


class HomeView : KtController(), HomeInterface {


    @BindView(R.id.launch_project)      lateinit var buttonProject:View
    @BindView(R.id.launch_visualizer)   lateinit var buttonVisualizer: Button
    @BindView(R.id.launch_photo_loader) lateinit var buttonLoad:View
    @BindView(R.id.launch_color_finder) lateinit var buttonColorFinder:View

    @JvmField
    @BindColor(R.color.transparent) var color: Int = 0


    private var presenter: HomePresenter? = null
    lateinit private var overlay: ViewGroup
    private var canDoVideo = true


    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_home, container, false)
    }


    override fun onAttach(view: View) {
        super.onAttach(view)

        setTitle("")
        setToolbarColor(color)

        presenter = HomePresenter()
        presenter?.attachView(this)
        setMenuAsBack(false)

        overlay = FrameLayout(context)

        val camInfo = CameraInfo(context)
        canDoVideo = camInfo.cameraMeetsCriteria()
        //canDoVideo = false
        if(!canDoVideo) {
            buttonVisualizer.text = "Take Photo"
        }

        if(ProjectManager.currentProject.images.isEmpty()) {
            buttonProject.visibility = View.GONE
        }

        val progressDialog = ProgressDialog(context)
        progressDialog.setTitle("Downloading data...")
        progressDialog.setMessage("This shouldn't take long")
    }

    override fun onDetach(view: View) {
        super.onDetach(view)
        presenter?.detachView()
    }

    @OnClick(R.id.launch_visualizer) fun visualizer() {
        getPerm(PermissionsController.REQUEST_CAMERA) {
            if(canDoVideo) {
                launch(VisualizerView("", ""))
            } else {
                launch(TakePhotoController())
            }
        }
    }

    @OnClick(R.id.launch_project) fun project() {
        launch(ProjectImagePager(ProjectManager.currentProject.id))
    }

    @OnClick(R.id.launch_explore) fun explore() {
        launch(CategoryView())
    }

    @OnClick(R.id.launch_color_finder) fun finder() {
        getPerm(PermissionsController.REQUEST_CAMERA) {
            launch(ColorFinderController())
        }
    }

    @OnClick(R.id.launch_photo_loader) fun gallery() {
        getPerm(PermissionsController.REQUEST_FILES,
                PermissionsController.REQUEST_CAMERA) {
            launchHeadless(GalleryController())
        }
    }

    private fun getPerm(vararg perm: Int, onSuccess: () -> Unit) {
        val controller = PermissionsController(
                perm,
                onComplete = { success ->
                    if(success) {
                        onSuccess()
                    }
                })
        launchHeadless(controller)
    }

    private fun launchHeadless(controller: KtController) {
        if (childRouters.size > 0) {
            removeChildRouter(childRouters[0])
        }
        getChildRouter(overlay, null)
                .setPopsLastView(true)
                .setRoot(RouterTransaction.with(controller))
    }


    private fun launch(controller: KtController) {
        router.pushController(RouterTransaction.with(controller)
                                      .pushChangeHandler(HorizontalChangeHandler(100))
                                      .popChangeHandler(HorizontalChangeHandler(100)))
    }
}
