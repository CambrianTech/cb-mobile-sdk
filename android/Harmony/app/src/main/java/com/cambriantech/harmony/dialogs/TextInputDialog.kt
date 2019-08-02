package com.cambriantech.harmony.dialogs

import android.content.Context
import android.graphics.Color
import android.support.v7.app.AlertDialog
import android.view.ViewGroup
import android.view.WindowManager
import android.view.inputmethod.InputMethodManager
import android.widget.EditText
import com.cambriantech.harmony.R
import com.cambriantech.harmony.util.DimenUtil


/**
 * Created by Joseph Sullivan on 6/21/16.
 */

class TextInputDialog(private val title: String,
                      private val message: String,
                      private val context: Context,
                      private val onSubmit: (String) -> Unit?) {

    init {
        createDialog()
    }

    private fun createDialog() {
        // Title text input alert dialog
        val userInput = EditText(context)
        userInput.setTextColor(Color.BLACK)
        val padding = DimenUtil.dpToPx(context, 16)
        //userInput.setPadding(padding, padding, padding, padding);

        val inputAlert = AlertDialog.Builder(context, R.style.AppTheme)

        inputAlert
                .setTitle(title)
                .setMessage(message)
                .setView(userInput)
                .setPositiveButton("Submit") { dialog, which ->
                    val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
                    imm.hideSoftInputFromWindow(userInput.windowToken, 0)

                    val input = userInput.text.toString()
                    onSubmit(input)
                }
                .setNegativeButton("Cancel") { dialog, which ->
                    val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
                    imm.hideSoftInputFromWindow(userInput.windowToken, 0)

                    dialog.dismiss()
                }

        dialogStyle(inputAlert)

        val p = userInput.layoutParams as ViewGroup.MarginLayoutParams
        p.marginStart = padding
        p.marginEnd = padding
        userInput.requestLayout()
        userInput.requestFocus()

        val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0)
    }

    companion object {
        fun dialogStyle(inputAlert: AlertDialog.Builder) {

            val alertDialog = inputAlert.create()
            alertDialog.window.let {
                val lp = it.attributes
                lp.dimAmount = 0.7f
                lp.y = -200
                it.attributes = lp
                it.addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND)
                it.setLayout(WindowManager.LayoutParams.WRAP_CONTENT, WindowManager.LayoutParams.WRAP_CONTENT)
                alertDialog.show()
            }
        }
    }

}
