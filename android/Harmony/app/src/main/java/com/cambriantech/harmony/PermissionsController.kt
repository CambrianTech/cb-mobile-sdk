package com.cambriantech.harmony

import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.support.v4.content.ContextCompat
import android.view.View
import com.cambriantech.harmony.base.BundleBuilder
import timber.log.Timber

/**
 * Created by Joseph on 11/28/2017.
 */

class PermissionsController(args: Bundle) : BlankController(args) {

    private var onComplete: ((success: Boolean) -> Unit)? = null


    constructor(perms: IntArray,
                onComplete: (success: Boolean) -> Unit)
            : this(BundleBuilder(Bundle())
                           .putIntArray(PERMS_CODE, perms)
                           .build()) {
        this.onComplete = onComplete
    }

    companion object {
        private val PERMS_CODE = "PermissionsController.code"
        val REQUEST_CAMERA = 0
        val REQUEST_FILES = 1
    }

    lateinit private var codes: IntArray


    override fun onViewBound(view: View) {
        super.onViewBound(view)

        codes = args.getIntArray(PERMS_CODE)

        val perms = Array(codes.size, {i ->  ""})
        for (i in codes.indices) {
            when (codes[i]) {
                REQUEST_CAMERA  -> perms[i] = android.Manifest.permission.CAMERA
                REQUEST_FILES   -> perms[i] = android.Manifest.permission.WRITE_EXTERNAL_STORAGE
            }
        }

        if(Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            for (perm in perms) {
                val check = ContextCompat.checkSelfPermission(context, perm)
                if (check != PackageManager.PERMISSION_GRANTED) {
                    complete(false)
                    return
                }
            }
        }

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            requestPermissions(perms, 0)
        }
    }


    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        var hasPermission = true
        for (result in grantResults) {
            if (PackageManager.PERMISSION_DENIED == result) {
                Timber.w("permission denied")
                hasPermission = false
            }
        }
        if(hasPermission) Timber.v("Permissions granted")
        onComplete?.invoke(hasPermission)
        router.popToRoot()
    }

    private fun complete(hasPermission: Boolean) {
        onComplete?.invoke(hasPermission)
        router.popToRoot()
    }
}