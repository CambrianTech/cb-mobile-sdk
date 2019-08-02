package com.cambriantech.harmony.data

import com.amazonaws.services.s3.model.S3ObjectSummary
import com.cambriantech.harmony.util.ImageUtil

/**
 * Created by Joseph on 10/30/2017.
 */
data class CBRemoteResource(private var url: String, private val summary: S3ObjectSummary) {

    companion object {
        private const val FILENAME_DIFFUSE = "Base_Color.jpg"
        private const val FILENAME_NORMAL = "Normal.jpg"
        private const val FILENAME_ROUGHNESS = "Roughness.jpg"
        private const val FILENAME_THUMBNAIL = "Thumbnail.jpg"

        fun idFromKey(key: String) : String {
            return key.substringAfter("_")
                    .substringBefore("/")
        }
    }


    var diffuseUrl: String? = null
    var normalUrl: String? = null
    var roughnessUrl: String? = null
    var thumbnailUrl: String? = null

    val id = idFromKey(summary.key)
    private val baseURL = url
            .substringBeforeLast("/")
            .plus("/")


    init {
        addElement(summary)
    }

    fun addElement(summary: S3ObjectSummary) {
        //Timber.d("element: " + summary.key)
        when(summary.key.substringAfterLast("/")) {
            FILENAME_DIFFUSE    -> diffuseUrl   = baseURL + FILENAME_DIFFUSE
            FILENAME_NORMAL     -> normalUrl    = baseURL + FILENAME_NORMAL
            FILENAME_ROUGHNESS  -> roughnessUrl = baseURL + FILENAME_ROUGHNESS
            FILENAME_THUMBNAIL  -> thumbnailUrl = baseURL + FILENAME_THUMBNAIL
        }
    }


    fun loadFlooring(output: (ByteArray, ByteArray, ByteArray) -> Unit) {
        var diffuse: ByteArray? = null
        var normal: ByteArray? = null
        var roughness: ByteArray? = null

        fun finish() {
            if(diffuse != null && normal != null && roughness != null) {
                output(diffuse!!, normal!!, roughness!!)
            }
        }

        ImageUtil.loadByteArray(diffuseUrl) {
            diffuse = it
            finish()
        }
        ImageUtil.loadByteArray(normalUrl) {
            normal = it
            finish()
        }
        ImageUtil.loadByteArray(roughnessUrl) {
            roughness = it
            finish()
        }
    }
}