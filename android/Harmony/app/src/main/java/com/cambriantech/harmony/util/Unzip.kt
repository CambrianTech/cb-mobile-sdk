package com.cambriantech.harmony.util

import io.reactivex.Observable
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers
import timber.log.Timber
import java.io.*
import java.util.zip.ZipInputStream

/**
 * Created by Joseph Sullivan on 7/27/16.
 */


/**
 * Unzip logic from: http://stackoverflow.com/a/9377397/2316935
 * Unzip a zip file.  Will overwrite existing files.
 */

class Unzip(private val location: String,
            private var destination: String,
            listener: Unzip.UnzipListener) {

    interface UnzipListener {
        fun onComplete(location: String)
    }


    init {
        Observable.fromCallable<String>({ this.doUnzip() })
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe { result ->
                    Timber.d("result as path: " + result)
                    listener.onComplete(result)
                }
    }

    private fun doUnzip(): String {
        val buffer = ByteArray(BUFFER_SIZE)
        try {
            if (!destination.endsWith("/")) {
                destination += "/"
            }

            try {
                ZipInputStream(BufferedInputStream(FileInputStream(location), BUFFER_SIZE)).use { zin ->

                    while(true) {
                        val ze = zin.nextEntry ?: break

                        val path = destination + ze.name
                        val unzipFile = File(path)

                        if (ze.isDirectory) {
                            if (!unzipFile.isDirectory) {
                                unzipFile.mkdirs()
                                Timber.d("path: " + unzipFile.absolutePath)
                            }
                        } else {
                            // unzip the file
                            val out = FileOutputStream(unzipFile, false)
                            val fout = BufferedOutputStream(out, BUFFER_SIZE)
                            try {
                                while (true) {
                                    val size = zin.read(buffer, 0, BUFFER_SIZE)
                                    if(size == -1) break
                                    fout.write(buffer, 0, size)
                                }
                                zin.closeEntry()
                            } finally {
                                fout.flush()
                                fout.close()
                            }
                        }
                    }
                }
            } catch (e: Exception) {
                Timber.e(e, "Unzip exception")
            } finally {
                Timber.i("unzip complete")
                Timber.d("Unzip directory: " + destination)
            }
        } catch (e: Exception) {
            Timber.e(e, "Unzip exception")
        }

        return destination
    }

    companion object {
        private val BUFFER_SIZE = 1024 * 8
    }
}
