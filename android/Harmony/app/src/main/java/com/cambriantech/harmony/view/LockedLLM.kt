package com.cambriantech.harmony.view

import android.content.Context
import android.support.v7.widget.LinearLayoutManager

class LockedLLM(context: Context?, orientation: Int, reverseLayout: Boolean)
    : LinearLayoutManager(context, orientation, reverseLayout) {

    override fun canScrollHorizontally(): Boolean {
        return false
    }
}