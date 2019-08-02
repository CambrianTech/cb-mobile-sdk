package com.cambriantech.harmony.dialogs

import android.content.Context
import android.support.v7.app.AlertDialog

/**
 * Created by Joseph Sullivan on 6/21/16.
 */

class ConfirmationDialog(private val title: String,
                         private val message: String,
                         private val context: Context,
                         private val onConfirm: () -> Unit?) {


    private fun createDialog() {
        val confirmDialog = AlertDialog.Builder(context)
        confirmDialog
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton("Yes") { dialog, id -> onConfirm() }
                .setNegativeButton("No") { dialog, id -> dialog.cancel() }

        TextInputDialog.dialogStyle(confirmDialog)
    }

    private fun createSaveDialog() {
        val confirmDialog = AlertDialog.Builder(context)
        confirmDialog
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton("Discard") { dialog, id -> onConfirm() }
                .setNegativeButton("Cancel") { dialog, id -> dialog.cancel() }

        TextInputDialog.dialogStyle(confirmDialog)
    }

    init {
        createDialog()
    }
}
