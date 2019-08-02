package com.cambriantech.harmony.util

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.Paint
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.HHApp
import io.reactivex.Observable
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream

/**
 * Created by Joseph Sullivan on 8/25/16.
 */

class ImageStitch(private val beforePath: String,
                  private val afterPath: String,
                  private val onComplete: (result: String) -> Unit?) {

    private val width: Int
    private val height: Int

    private var outFile: File? = null

    init {
        val options = BitmapFactory.Options()
        options.inJustDecodeBounds = true

        BitmapFactory.decodeFile(beforePath, options)
        width = options.outWidth
        height = options.outHeight

        Timber.d("input width: " + width)
        Timber.d("input height: " + height)

        loadImages()
    }

    private fun loadImages() {

        var before: Bitmap? = null
        var after: Bitmap? = null
        var watermark: Bitmap? = null

        fun finishLoading() {
            safeLet(before, after, watermark) { b, a, w ->
                combine(b, a, w)
            }
        }

        ImageUtil.loadBitmap(beforePath) {
            before = it
            finishLoading()
        }


        ImageUtil.loadBitmap(afterPath) {
            after = it
            finishLoading()
        }

        ImageUtil.loadImageResource(R.mipmap.watermark) {
            watermark = it
            watermark!!.resize((width / 1.9f).toInt())
            finishLoading()
        }
    }


    private fun combine(before: Bitmap, after: Bitmap, watermark: Bitmap) {
        Observable.fromCallable {
            val result: Bitmap? = Bitmap.createBitmap(width * 2, height, Bitmap.Config.ARGB_8888)
            after.resize(maxOf(width, height))
            Timber.d("stitch width: " + result?.width)
            Timber.d("stitch height: " + result?.height)
            val canvas: Canvas? = Canvas(result)
            val paint = Paint()
            canvas?.drawBitmap(before, 0f, 0f, paint)
            canvas?.drawBitmap(after, width.toFloat(), 0f, paint)
            canvas?.drawBitmap(watermark, 16f, (height - (watermark.height + 16)).toFloat(), paint)


            try {
                val path = File(HHApp.context.getExternalFilesDir(null), "share")
                path.mkdir()
                outFile = File(path, "share.jpg")
                val fOut = FileOutputStream(outFile)
                result?.compress(Bitmap.CompressFormat.JPEG, 95, fOut)
                fOut.flush()
                fOut.close()
            } catch (e: Exception) {
                Timber.e("Failed to save shareable image")
                e.printStackTrace()
            }

            result?.recycle()
            before.recycle()
            after.recycle()
            watermark.recycle()
        }
            .subscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe {
                outFile?.let {
                    onComplete(it.toString())
                }
            }
    }
}
