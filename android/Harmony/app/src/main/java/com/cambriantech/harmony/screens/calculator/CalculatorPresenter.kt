package com.cambriantech.harmony.screens.calculator

import com.cambriantech.harmony.base.Presenter

import timber.log.Timber

/**
 * Created by Joseph Sullivan on 9/21/16.
 */

class CalculatorPresenter : Presenter<CalculatorInterface> {

    companion object {
        private val EXTERIOR_PAINT_AREA = 400f
        private val INTERIOR_PAINT_AREA = 250f
        private val WINDOW_SIZE = 15
        private val DOOR_SIZE = 20
    }

    private var calculatorView: CalculatorInterface? = null
    private var width = 0
    private var height = 0

    internal var doors = 0
        set(value) {
            field += value
            if (doors < 0) field = 0
            if (doors > 100) field = 100
            calculatorView?.setDoorsAndWindows(doors.toString(), windows.toString())
            calculate()
        }
    internal var windows = 0
        set(value) {
            field += value
            if (windows < 0) field = 0
            if (windows > 100) field = 100
            calculatorView?.setDoorsAndWindows(doors.toString(), windows.toString())
            calculate()
        }
    internal var coats = 1
        set(value) {
            Timber.i("number of coats: $value")
            field = value
            calculate()
        }
    internal var type = 0
        set(value) {
            field = value-1
            Timber.i("type: $field")
            calculate()
        }


    override fun attachView(view: CalculatorInterface) {
        this.calculatorView = view
    }

    override fun detachView() {
        this.calculatorView = null
    }

    fun setWidth(widthStr: String) {
        if (widthStr.isBlank()) return
        width = widthStr.toInt()
        calculate()
    }

    fun setHeight(heightStr: String) {
        if (heightStr.isBlank()) return
        height = heightStr.toInt()
        calculate()
    }

    private fun calculate() {
        var area = width * height
        area -= doors * DOOR_SIZE
        area -= windows * WINDOW_SIZE
        if (area < 0) area = 0

        val cans: Float
        if (type == 0)
            cans = area * coats / INTERIOR_PAINT_AREA
        else
            cans = area * coats / EXTERIOR_PAINT_AREA


        if (calculatorView != null) {
            calculatorView?.setResults(area.toString(), Math.ceil(cans.toDouble()).toInt().toString())
        }

    }
}
