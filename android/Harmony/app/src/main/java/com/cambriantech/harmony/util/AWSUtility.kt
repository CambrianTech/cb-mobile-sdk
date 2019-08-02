package com.cambriantech.harmony.util

import android.content.Context
import com.amazonaws.auth.CognitoCachingCredentialsProvider
import com.amazonaws.regions.Region
import com.amazonaws.regions.Regions
import com.amazonaws.services.s3.AmazonS3Client
import com.cambriantech.harmony.data.CBRemoteResource
import kotlinx.coroutines.experimental.CommonPool
import kotlinx.coroutines.experimental.android.UI
import kotlinx.coroutines.experimental.async


object AWSUtility {

    private lateinit var s3: AmazonS3Client
    private var resources = mutableListOf<CBRemoteResource>()

    fun initialize(context: Context) = async(UI) {
        async(CommonPool) {
            val creds = CognitoCachingCredentialsProvider(
                    context.applicationContext,
                    "us-east-1:1db8ca1e-3634-46fd-b7a0-f29a86ec8cd0",
                    Regions.US_EAST_1)

            s3 = AmazonS3Client(creds)
            s3.setRegion(Region.getRegion(Regions.US_EAST_1))

            getFlooring()
        }.await()
    }

    private fun getFlooring() = async(UI) {
        async(CommonPool) {

            val bucket = "cambrian-dbassets"
            val objectListing = s3.listObjects(bucket)

            do {
                val summaries = objectListing.objectSummaries
                //Timber.d("list size is " + summaries.size)

                for(summary in summaries) {

                    val url = s3.getResourceUrl(bucket, summary.key)
                    val id = CBRemoteResource.idFromKey(summary.key)
                    val match = resources.firstOrNull { it.id == id }

                    if(match == null)
                        resources.add(CBRemoteResource(url, summary))
                    else
                        match.addElement(summary)
                }

            } while (objectListing.isTruncated)

            //Timber.d("found ${resources.size} resources")
        }.await()
    }


    fun getResourceForId(id: String) : CBRemoteResource? {

        val match = resources.firstOrNull { it.id == id }
        //Timber.d("match is " + match?.id)
        return match
    }
}

