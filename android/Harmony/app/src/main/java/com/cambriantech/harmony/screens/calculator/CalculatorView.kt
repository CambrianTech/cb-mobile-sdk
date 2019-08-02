package com.cambriantech.harmony.screens.calculator

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.RadioGroup
import android.widget.TextView
import butterknife.BindColor
import butterknife.BindView
import butterknife.OnClick
import butterknife.OnTextChanged
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.KtController

/**
 * Created by Joseph Sullivan on 9/21/16.
 */

class CalculatorView : KtController(), CalculatorInterface {

    private var presenter = CalculatorPresenter()

    @BindView(R.id.coats_radio_group)   lateinit var radioCoats: RadioGroup
    @BindView(R.id.type_radio_group)    lateinit var radioType: RadioGroup
    @BindView(R.id.sqr_feet_total_num)  lateinit var resultArea: TextView
    @BindView(R.id.paint_can_total_num) lateinit var resultCans: TextView
    @BindView(R.id.door_num)            lateinit var doorNum: TextView
    @BindView(R.id.window_num)          lateinit var windowNum: TextView
    @JvmField @BindColor(R.color.colorPrimary)   var color: Int = 0


    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_calculator, container, false)
    }

    override fun onAttach(view: View) {
        super.onAttach(view)

        setTitle("Calculator")
        setToolbarColor(color)

        presenter = CalculatorPresenter()
        presenter.attachView(this)

        radioCoats.setOnCheckedChangeListener { radioGroup, i ->
            val coat = radioCoats.indexOfChild(view.findViewById(radioCoats.checkedRadioButtonId)) + 1
            presenter.coats = coat
        }

        radioType.setOnCheckedChangeListener { radioGroup, i ->
            val type = radioType.indexOfChild(view.findViewById(radioType.checkedRadioButtonId)) + 1
            presenter.type = type
        }
    }

    override fun onDetach(view: View) {
        presenter.detachView()
    }

    @OnTextChanged(R.id.width_input)
    fun widthChanged(charSequence: CharSequence) {
        presenter.setWidth(charSequence.toString())
    }

    @OnTextChanged(R.id.height_input)
    fun heightChanged(charSequence: CharSequence) {
        presenter.setHeight(charSequence.toString())
    }

    @OnClick(R.id.door_remove, R.id.door_add, R.id.window_remove, R.id.window_add)
    fun change(view: View) {
        val id = view.id
        if (id == R.id.door_add) presenter.doors = 1
        if (id == R.id.door_remove) presenter.doors = -1
        if (id == R.id.window_add) presenter.windows = 1
        if (id == R.id.window_remove) presenter.windows = -1
    }


    override fun setDoorsAndWindows(doors: String, windows: String) {
        doorNum.text = doors
        windowNum.text = windows
    }

    override fun setResults(area: String, cans: String) {
        resultArea.text = area
        resultCans.text = cans
    }
}
