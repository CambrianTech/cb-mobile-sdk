package com.cambriantech.harmony.util

import io.reactivex.Observable
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers
import timber.log.Timber
import java.io.File

/**
 * Created by Joseph Sullivan on 9/15/16.
 */

class SaveImage(image: File,
                dest: File,
                private val onComplete: () -> Unit) {

    init {
        Timber.d("image: " + image)
        Timber.d("destination: " + dest)
        save(image, dest)
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe { dir ->
                    Timber.d("saved image to: " + dir)
                    onComplete()
                }
    }


    private fun save(image: File, dest: File): Observable<File> {
        FileUtil.copyFile(image, dest)
        return Observable.just(dest)
    }
}
