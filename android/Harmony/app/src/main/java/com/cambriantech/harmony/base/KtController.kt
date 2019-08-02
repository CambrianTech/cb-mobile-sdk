package com.cambriantech.harmony.base

import android.content.Context
import android.graphics.drawable.ColorDrawable
import android.os.Bundle
import android.support.v7.widget.Toolbar
import com.bluelinelabs.conductor.ControllerChangeHandler
import com.bluelinelabs.conductor.ControllerChangeType

/**
 * Created by Joseph on 3/14/2017.
 */

abstract class KtController : BindingController, MvpView {

    constructor() : super()
    constructor(args: Bundle) : super(args)

    override val context: Context
        get() = activity!!

    // Note: This is just a quick demo of how an ActionBar *can* be accessed, not necessarily how it *should*
    // be accessed. In a production app, this would use Dagger instead.
    protected val toolbar: Toolbar?
        get() {
            val toolbarProvider = activity as ToolbarProvider?
            return toolbarProvider?.getActivityToolbar()
        }

    override fun onChangeStarted(changeHandler: ControllerChangeHandler, changeType: ControllerChangeType) {
        super.onChangeStarted(changeHandler, changeType)
        setOptionsMenuHidden(!changeType.isEnter)
    }

    protected fun setMenuAsBack(bool: Boolean) {
        val actionBarProvider = activity as ToolbarToggle?
        actionBarProvider?.setMenuAsBack(bool)
    }

    protected fun setTitle(text: String) {
        toolbar?.title = text
    }

    protected fun setToolbarColor(color: Int) {
        toolbar?.background = ColorDrawable(color)
    }
}
