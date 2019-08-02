package com.cambriantech.harmony

import android.app.Activity
import android.content.Intent
import android.view.View
import android.view.ViewGroup
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.FadeChangeHandler
import com.cambriantech.harmony.data.ProjectManager
import com.cambriantech.harmony.screens.visualizer.VisualizerView
import com.cambriantech.harmony.util.FileUtil
import com.cambriantech.harmony.util.SaveImage
import timber.log.Timber
import java.io.File
import java.util.*

/**
 * Created by Joseph on 11/28/2017.
 */

class GalleryController : BlankController() {

    lateinit private var overlay: ViewGroup


    override fun onViewBound(view: View) {
        super.onViewBound(view)

        launchGallery()
    }

    private fun launchGallery() {
        val intent = Intent()
        intent.type = "image/*"
        intent.action = Intent.ACTION_GET_CONTENT
        intent.putExtra(Intent.EXTRA_LOCAL_ONLY, true)
        intent.flags = Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION or Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
        startActivityForResult(Intent.createChooser(intent, "Select Picture"), 0)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode == Activity.RESULT_OK) {
            val uri = data?.data ?: return
            val path = FileUtil.getPath(context, uri) ?: return
            FileUtil.newImagePath { dest, id ->
                SaveImage(File(path), dest) {
                    launchVisualizer(dest, id)
                }
            }
        } else {
            router.popCurrentController()
        }
    }


    private fun launchVisualizer(path: File, id: String) {
        Timber.d("starting visualizer")
        router.popToRoot()
        parentController?.router?.pushController(
                RouterTransaction.with(VisualizerView(path.toString(), id))
                        .pushChangeHandler(FadeChangeHandler(100))
                        .popChangeHandler(FadeChangeHandler(100)))
    }
}