package com.cambriantech.harmony.util

import android.graphics.Bitmap
import io.reactivex.Observable
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream



class SaveBitmap(image: Bitmap,
                 dest: File,
                 private val onComplete: () -> Unit) {

    init {
        save(image, dest)
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe { dir ->
                    Timber.d("saved image to: " + dir)
                    onComplete()
                }
    }


    private fun save(image: Bitmap, dest: File): Observable<File> {
        val dir = File(dest.parent)
        if (!dir.exists())
            dir.mkdirs()
        val fOut = FileOutputStream(dest)

        image.compress(Bitmap.CompressFormat.JPEG, 95, fOut)
        fOut.flush()
        fOut.close()
        return Observable.just(dest)
    }
}