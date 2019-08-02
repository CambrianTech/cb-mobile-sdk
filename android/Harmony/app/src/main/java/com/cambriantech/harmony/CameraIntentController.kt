package com.cambriantech.harmony

import android.app.Activity
import android.content.Intent
import android.provider.MediaStore
import android.support.v4.content.FileProvider
import android.view.View
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.FadeChangeHandler
import com.cambriantech.harmony.screens.visualizer.VisualizerView
import com.cambriantech.harmony.util.FileUtil
import timber.log.Timber
import java.io.File

/**
 * Created by Joseph on 12/29/2017.
 */
class CameraIntentController : BlankController() {

    companion object {
        const val REQUEST_CODE = 1
    }


    lateinit var id: String
    lateinit var path: File
    private val provider = BuildConfig.APPLICATION_ID + ".provider"

    override fun onViewBound(view: View) {
        super.onViewBound(view)

        launchCameraIntent()
    }

    private fun launchCameraIntent() {
        FileUtil.newImagePath { path, id ->
            this.path = path
            this.id = id
        }
        val takePictureIntent = Intent(MediaStore.ACTION_IMAGE_CAPTURE)

        Timber.d("fileprovider: " + provider)
        val photoUri = FileProvider.getUriForFile(context, provider, path)
        takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoUri)

        //grant permision for app with package "packegeName", eg. before starting other app via intent
        context.grantUriPermission("com.google.android.GoogleCamera", photoUri, Intent.FLAG_GRANT_WRITE_URI_PERMISSION or Intent.FLAG_GRANT_READ_URI_PERMISSION)
        Timber.d("starting camera intent")

        startActivityForResult(takePictureIntent, REQUEST_CODE)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode == Activity.RESULT_OK) {
            if (requestCode == REQUEST_CODE) {
                if(path.exists()) {
                    Timber.d("image exists. path: $path, id: $id")
                    launchVisualizer(path, id)
                    return
                } else {
                    //image failed
                    Timber.w("image doesn't exist")
                    if (path.parentFile?.exists() == true) {
                        path.parentFile?.deleteRecursively()
                        Timber.i("deleting path: $path")
                    }
                    router.popCurrentController()
                }
            }
        } else router.popCurrentController()
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