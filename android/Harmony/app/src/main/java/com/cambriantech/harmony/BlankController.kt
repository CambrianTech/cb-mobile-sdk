package com.cambriantech.harmony

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.cambriantech.harmony.base.KtController

/**
 * Created by Joseph on 11/28/2017.
 */

open class BlankController : KtController {

    constructor() : super()
    constructor(args: Bundle) : super(args)

    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_blank, container, false)
    }

    override fun onViewBound(view: View) {
        super.onViewBound(view)

        setTitle("")
        setToolbarColor(0x00000000)
    }
}