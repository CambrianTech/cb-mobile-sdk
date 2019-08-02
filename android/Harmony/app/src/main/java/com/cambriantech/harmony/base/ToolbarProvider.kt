package com.cambriantech.harmony.base

import android.support.v7.widget.Toolbar

interface ToolbarProvider {
    fun getActivityToolbar(): Toolbar
    fun resetNav()
}
