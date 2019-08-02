package com.cambriantech.harmony.util

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Matrix
import android.net.Uri
import android.provider.MediaStore
import android.util.Size
import com.bumptech.glide.Glide
import com.bumptech.glide.load.resource.drawable.GlideDrawable
import com.bumptech.glide.request.animation.GlideAnimation
import com.bumptech.glide.request.target.SimpleTarget
import com.cambriantech.harmony.base.HHApp
import timber.log.Timber
import java.io.ByteArrayOutputStream
import java.io.FileNotFoundException
import java.io.IOException
import java.io.InputStream

/**
 * Created by Joseph Sullivan on 9/12/16.
 */

object ImageUtil {


    fun loadBitmap(url: String?, loaded: (Bitmap) -> Unit) {
        Glide.with(HHApp.context)
                .load(url)
                .asBitmap()
                .into(object : SimpleTarget<Bitmap>() {
                    override fun onResourceReady(resource: Bitmap?, glideAnimation: GlideAnimation<in Bitmap>?) {
                        resource?.let {
                            loaded(it)
                        }

                    }
                })
    }

    fun loadByteArray(url: String?, loaded: (ByteArray) -> Unit) {
        Glide.with(HHApp.context)
                .load(url)
                .asBitmap()
                .toBytes()
                .into(object : SimpleTarget<ByteArray>() {
                    override fun onResourceReady(byteArray: ByteArray?, glideAnimation: GlideAnimation<in ByteArray>) {
                        byteArray?.let {
                            loaded(it)
                        }
                    }
                })
    }

    fun loadDrawable(url: String?, loaded: (GlideDrawable) -> Unit) {
        Glide.with(HHApp.context)
                .load(url)
                .into(
                    object : SimpleTarget<GlideDrawable>() {
                        override fun onResourceReady(resource: GlideDrawable?, anim: GlideAnimation<in GlideDrawable>) {
                            resource?.let {
                                loaded(resource)
                            }
                        }
                    }
                 )
    }

    fun loadImageResource(id: Int, loaded: (Bitmap) -> Unit) {
        Glide.with(HHApp.context)
                .load(id)
                .asBitmap()
                .into(object : SimpleTarget<Bitmap>() {
                    override fun onResourceReady(resource: Bitmap?, glideAnimation: GlideAnimation<in Bitmap>?) {
                        resource?.let {
                            loaded(it)
                        }
                    }
                })
    }

    fun getImageUrlWithAuthority(context: Context, uri: Uri): String? {
        var stream: InputStream? = null
        if (uri.authority != null) {
            try {
                stream = context.contentResolver.openInputStream(uri)
                val bmp = BitmapFactory.decodeStream(stream)
                return writeToTempImageAndGetPathUri(context, bmp).toString()
            } catch (e: FileNotFoundException) {
                e.printStackTrace()
            } finally {
                try {
                    stream?.close()
                } catch (e: IOException) {
                    e.printStackTrace()
                }
            }
        }
        return null
    }

    private fun writeToTempImageAndGetPathUri(inContext: Context, inImage: Bitmap): Uri {
        val bytes = ByteArrayOutputStream()
        inImage.compress(Bitmap.CompressFormat.JPEG, 100, bytes)
        val path = MediaStore.Images.Media.insertImage(inContext.contentResolver, inImage, "Title", null)
        return Uri.parse(path)
    }


    fun getScaledDimensions(width: Float, height: Float, maxSize: Float): Size {
        var scaleFactor = 1.0f
        if (width > maxSize || height > maxSize) {
            scaleFactor = if (width > height) {
                maxSize / width
            } else {
                maxSize / height
            }
        }

        val newWidth = width * scaleFactor
        val newHeight = height * scaleFactor

        return Size(newWidth.toInt(), newHeight.toInt())
    }

    fun scaleAndRotate(bitmap: Bitmap, maxSize: Int, rotation: Int): Bitmap {
        val width = bitmap.width.toFloat()
        val height = bitmap.height.toFloat()

        val dims = ImageUtil.getScaledDimensions(width, height, maxSize.toFloat())

        val newWidth = dims.width / width
        val newHeight = dims.height / height

        val matrix = Matrix()
        matrix.postScale(newWidth, newHeight)
        matrix.postRotate(rotation.toFloat())

        val diff = height - (720 * (width/dims.width))
        Timber.d("diff is " + diff)

        val bmp = Bitmap.createBitmap(bitmap, 0, (diff/2).toInt(), width.toInt(), (height - diff).toInt(), matrix, false)
        bitmap.recycle()
        return bmp
    }
}
