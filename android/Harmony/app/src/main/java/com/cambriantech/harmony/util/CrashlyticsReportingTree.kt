package com.cambriantech.harmony.util

import com.crashlytics.android.Crashlytics
import timber.log.Timber


/**
 * Created by Joseph on 10/24/2017.
 */

class CrashlyticsReportingTree : Timber.Tree() {
    override fun log(priority: Int, tag: String?, message: String, t: Throwable?) {
        Crashlytics.log(priority, tag, message)

        t?.let {
            Crashlytics.logException(it)
        }
    }
}