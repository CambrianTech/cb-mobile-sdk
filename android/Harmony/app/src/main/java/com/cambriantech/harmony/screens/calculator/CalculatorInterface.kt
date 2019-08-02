package com.cambriantech.harmony.screens.calculator

import com.cambriantech.harmony.base.MvpView

/**
 * Created by joseph on 9/21/16.
 */

interface CalculatorInterface : MvpView {
    fun setDoorsAndWindows(doors: String, windows: String)
    fun setResults(area: String, cans: String)
}
