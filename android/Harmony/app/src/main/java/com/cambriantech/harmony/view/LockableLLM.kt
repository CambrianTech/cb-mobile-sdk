package com.cambriantech.harmony.view

import android.content.Context
import android.support.v7.widget.LinearLayoutManager
import timber.log.Timber

class LockableLLM(context: Context?, orientation: Int, reverseLayout: Boolean)
    : LinearLayoutManager(context, orientation, reverseLayout) {

    var canScroll = true

    override fun canScrollHorizontally(): Boolean {
        return canScroll
    }
}