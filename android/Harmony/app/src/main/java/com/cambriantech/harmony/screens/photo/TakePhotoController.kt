package com.cambriantech.harmony.screens.photo

import android.content.Context
import android.hardware.SensorManager
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageButton
import android.widget.ImageView
import butterknife.BindView
import butterknife.OnClick
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.FadeChangeHandler
import com.bumptech.glide.Glide
import com.cambrian.cbar.sensor.orientationProvider.ImprovedOrientationSensor2Provider
import com.cambrian.cbar.sensor.orientationProvider.OrientationProvider
import com.cambrian.cbar.sensor.representation.Quaternion
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.screens.visualizer.VisualizerView
import com.cambriantech.harmony.util.*
import com.cambriantech.harmony.view.ProgressView
import com.cambriantech.harmony.view.RoundImageButton
import io.fotoapparat.Fotoapparat
import io.fotoapparat.configuration.CameraConfiguration
import io.fotoapparat.log.Logger
import io.fotoapparat.selector.*
import io.fotoapparat.view.CameraView
import timber.log.Timber
import java.io.File


class TakePhotoController : KtController() {

    @BindView(R.id.camera_view)     lateinit var cameraView: CameraView
    @BindView(R.id.image_preview)   lateinit var imagePreview: ImageView
    @BindView(R.id.capture_button)  lateinit var buttonCapture: ImageButton
    @BindView(R.id.progress_view)   lateinit var progressView: ProgressView
    @BindView(R.id.capture_confirm) lateinit var buttonConfirm: RoundImageButton
    @BindView(R.id.capture_cancel)  lateinit var buttonCancel: RoundImageButton

    private lateinit var id: String
    private lateinit var path: File
    private lateinit var fotoapparat: Fotoapparat

    lateinit private var orientationProvider: OrientationProvider
    private val quat = Quaternion()

    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_camera, container, false)
    }

    private class TimberLogger: Logger {
        override fun log(message: String) {
            Timber.tag("Fotoapparat").d(message)
        }
    }

    override fun onAttach(view: View) {
        super.onAttach(view)

        val configuration = CameraConfiguration(
                previewResolution = firstAvailable(
                        wideRatio(highestResolution()),
                        standardRatio(highestResolution())
                ),
                previewFpsRange = highestFps(),
                flashMode = off(),
                focusMode = continuousFocusPicture()
        )

        fotoapparat = Fotoapparat(
                context = context,
                view = cameraView,
                logger = TimberLogger(),
                lensPosition = back(),
                cameraConfiguration = configuration,
                cameraErrorCallback = { Timber.e("Camera error: $it") }
        )

        fotoapparat.start()

        val sensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
        orientationProvider = ImprovedOrientationSensor2Provider(sensorManager)
        orientationProvider.start()

        FileUtil.newImagePath { path, id ->
            this.path = path
            this.id = id
        }
    }

    @OnClick(R.id.capture_button) fun clickedCapture() {
        orientationProvider.getQuaternion(quat)
        buttonCapture.hide()
        progressView.showProgress()
        val photoResult = fotoapparat
                .autoFocus()
                .takePicture()

        var rotation = 0
        photoResult.toBitmap().whenAvailable { photo ->
            rotation -= photo?.rotationDegrees ?: 0
            photo?.bitmap?.let { bmp ->
                val bitmap = ImageUtil.scaleAndRotate(bmp, 1280, rotation)
                SaveBitmap(bitmap, path) {

                    cameraView.fadeOut()
                    imagePreview.fadeIn()
                    progressView.hideProgress()
                    buttonCancel.fadeIn()
                    buttonConfirm.fadeIn()

                    Glide.with(context)
                        .load(path)
                        .centerCrop()
                        .crossFade(75)
                        .into(imagePreview)
                }
            }
        }

    }

    @OnClick(R.id.capture_cancel) fun clickedCancel() {
        imagePreview.fadeOut()
        cameraView.fadeIn()
        buttonCapture.fadeIn()
        buttonConfirm.hide()
        buttonCancel.hide()

        path.parentFile.deleteRecursively()
    }

    @OnClick(R.id.capture_confirm) fun clickedConfirm() {
        router.pushController(
                RouterTransaction.with(VisualizerView(path.toString(), id, quat.array()))
                        .pushChangeHandler(FadeChangeHandler(100))
                        .popChangeHandler(FadeChangeHandler(100)))
    }

    override fun onDetach(view: View) {
        super.onDetach(view)
        fotoapparat.stop()
        orientationProvider.stop()
    }
}