package com.cambriantech.harmony.util

import android.content.ContentUris
import android.content.Context
import android.content.res.AssetManager
import android.database.Cursor
import android.net.Uri
import android.os.Environment
import android.provider.DocumentsContract
import android.provider.MediaStore
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.data.ProjectManager
import timber.log.Timber
import java.io.*
import java.util.*


/**
 * Created by Joseph Sullivan on 9/13/16.
 */

object FileUtil {

    val appDirectory: File
        get() = HHApp.context.getExternalFilesDir(null)

    val projectsDirectory: File
        get() = File(appDirectory, "projects")

    fun newImagePath(result: (File, String) -> Unit) {
        val id = UUID.randomUUID().toString()
        val dir = File(ProjectManager.currentProject.path, id)
        //dir.mkdirs()
        val path = File(dir, "scene.jpg")
        Timber.i("new path: " + path.toString())
        result(path, id)
    }

    fun getPath(context: Context, uri: Uri): String? {
        Timber.i("getting absolute file id")

        // DocumentProvider
        if (DocumentsContract.isDocumentUri(context, uri)) {
            Timber.v("is document provider")
            // ExternalStorageProvider
            if (isExternalStorageDocument(uri)) {
                val docId = DocumentsContract.getDocumentId(uri)
                val split = docId.split(":".toRegex()).dropLastWhile(String::isEmpty).toTypedArray()
                val type = split[0]

                if ("primary".equals(type, ignoreCase = true)) {
                    return Environment.getExternalStorageDirectory().toString() + "/" + split[1]
                }
            } else if (isDownloadsDocument(uri)) {
                DocumentsContract.getDocumentId(uri)?.let {
                    val contentUri = ContentUris.withAppendedId(
                            Uri.parse("content://downloads/public_downloads"), java.lang.Long.valueOf(it))
                    return getDataColumn(context, contentUri, null, null)
                }

            } else if (isMediaDocument(uri)) {
                val docId = DocumentsContract.getDocumentId(uri)
                val split = docId.split(":".toRegex()).dropLastWhile(String::isEmpty).toTypedArray()
                val type = split[0]

                var contentUri: Uri? = null
                if ("image" == type) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI
                }

                val selection = "_id=?"
                val selectionArgs = arrayOf(split[1])

                return contentUri?.let { getDataColumn(context, it, selection, selectionArgs) }
            }
        } else if ("content".equals(uri.scheme, ignoreCase = true)) {
            Timber.v("is media store")
            return getDataColumn(context, uri, null, null)
        } else if ("file".equals(uri.scheme, ignoreCase = true)) {
            Timber.v("is file")
            return uri.path
        }
        return null
    }

    /**
     * Get the value of the data column for this Uri. This is useful for
     * MediaStore Uris, and other file-based ContentProviders.

     * @param context The context.
     * *
     * @param uri The Uri to query.
     * *
     * @param selection (Optional) Filter used in the query.
     * *
     * @param selectionArgs (Optional) Selection arguments used in the query.
     * *
     * @return The value of the _data column, which is typically a file id.
     */
    private fun getDataColumn(context: Context, uri: Uri, selection: String?,
                      selectionArgs: Array<String>?): String? {

        var cursor: Cursor? = null
        val column = "_data"
        val projection = arrayOf(column)

        try {
            cursor = context.contentResolver.query(uri, projection, selection, selectionArgs, null)
            if (cursor != null && cursor.moveToFirst()) {
                val columnIndex = cursor.getColumnIndexOrThrow(column)
                return cursor.getString(columnIndex)
            }
        } finally {
            if (cursor != null)
                cursor.close()
        }
        return null
    }

    fun copyBundledRealmFile(context: Context, inputStream: InputStream) {
        try {
            val file = File(context.filesDir, "brands")
            val outputStream = FileOutputStream(file)
            val buf = ByteArray(1024)

            while (true) {
                val len = inputStream.read(buf)
                if (len > 0) outputStream.write(buf, 0, len)
                else break
            }
            outputStream.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    // If targetLocation does not exist, it will be created.
    @Throws(IOException::class)
    private fun copyDirectory(sourceLocation: File, targetLocation: File) {

        Timber.d("path: " + sourceLocation.toString())

        if (sourceLocation.isDirectory) {
            if (!targetLocation.exists() && !targetLocation.mkdirs()) {
                throw IOException("Cannot create dir " + targetLocation.absolutePath)
            }

            val children = sourceLocation.list()
            for (i in children.indices) {
                copyDirectory(File(sourceLocation, children[i]),
                        File(targetLocation, children[i]))
            }
        } else {
            copyFile(sourceLocation, targetLocation)
        }
    }

    @Throws(IOException::class)
    fun copyFile(file: File, dest: File) {
        // make sure the directory we plan to store the recording in exists
        val directory = dest.parentFile
        if (directory != null && !directory.exists() && !directory.mkdirs()) {
            throw IOException("Cannot create dir " + directory.absolutePath)
        }

        val input = FileInputStream(file)
        val out = FileOutputStream(dest)

        // Copy the bits from instream to outstream
        val buf = ByteArray(1024)
        var len: Int = input.read(buf)
        while (len > 0) {
            out.write(buf, 0, len)
            len = input.read(buf)
        }
        input.close()
        out.close()
    }

    fun moveProjects() {
        val projects = appDirectory.listFiles()
                .filter { it.name != "assets" && it.name != "share" && it.name != "projects" }
                .listIterator()
        val projectsDir = File(appDirectory, "projects")
        projects.forEach {
            Timber.d("moving folder: " + it.toString())
            copyDirectory(it, File(projectsDir, it.name))
            it.deleteRecursively()
        }
    }


    @Throws(IOException::class)
    fun copyDirorfileFromAssetManager(assetManager: AssetManager, assetDir: String, destDir: String): String {
        val destFile = File(destDir)
        Timber.i("dir: " + assetDir)
        Timber.d("destination dir: " + destFile.toString())

        destFile.mkdirs()

        val files = assetManager.list(assetDir)
        Timber.i("num of files " + files.size)

        for (file in files) {

            val assetFilePath = assetDir + File.separator + file

            if (assetFilePath.contains(".")) {
                // It is a file
                val destFilePath = File.separator + destDir + file
                Timber.d("saving file to $destFilePath")
                copyAssetFile(assetManager, assetFilePath, destFilePath)
            } else {
                // It is a sub directory
                copyDirorfileFromAssetManager(assetManager, assetFilePath, destDir + file + File.separator)
            }

        }

        return destDir
    }


    @Throws(IOException::class)
    private fun copyAssetFile(assetManager: AssetManager, assetFilePath: String, destinationFilePath: String) {
        val input = assetManager.open(assetFilePath)
        val out = FileOutputStream(destinationFilePath)

        val buf = ByteArray(1024)
        var loop = true

        while (loop) {
            val len = input.read(buf)
            if (len > 0) out.write(buf, 0, len)
            else loop = false
        }

        input.close()
        out.close()
    }


    /**
     * @param uri The Uri to check.
     * *
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    private fun isExternalStorageDocument(uri: Uri): Boolean {
        return "com.android.externalstorage.documents" == uri.authority
    }

    /**
     * @param uri The Uri to check.
     * *
     * @return Whether the Uri authority is DownloadsProvider.
     */
    private fun isDownloadsDocument(uri: Uri): Boolean {
        return "com.android.providers.downloads.documents" == uri.authority
    }

    /**
     * @param uri The Uri to check.
     * *
     * @return Whether the Uri authority is MediaProvider.
     */
    private fun isMediaDocument(uri: Uri): Boolean {
        return "com.android.providers.media.documents" == uri.authority
    }
}
