package com.cambriantech.harmony.base

import android.content.Context
import android.content.SharedPreferences

/**
 * Created by Joseph Sullivan on 6/27/16.
 */

object UserSettings {

    lateinit private var settings: SharedPreferences

    private val PROJECT_STRING = "currentProject"
    private val TUTORIAL_STRING = "hasSeenTutorial"
    private val ASSETS_STRING = "hasAssets"
    private val NEW_DIR_STRING = "newProjectDir"

    fun initialize(context: Context) {
        settings = context.getSharedPreferences("HOME_HARMONY", Context.MODE_PRIVATE)
    }

    var currentProject: String
        get() = this.settings.getString(PROJECT_STRING, null)
        set(id) = this.settings.edit().putString(PROJECT_STRING, id).apply()

    fun deleteProject() {
        this.settings.edit().remove(PROJECT_STRING).apply()
    }

    fun hasAssets(): Boolean {
        return this.settings.contains(ASSETS_STRING)
    }

    fun setHasAssets(value: Boolean) {
        this.settings.edit().putBoolean(ASSETS_STRING, value).apply()
    }

    fun hasProject(): Boolean {
        return this.settings.contains(PROJECT_STRING)
    }

    fun hasSeenTutorial(): Boolean {
        return this.settings.getBoolean(TUTORIAL_STRING, false)
    }

    fun setHasSeenTutorial(value: Boolean) {
        this.settings.edit().putBoolean(TUTORIAL_STRING, value).apply()
    }

    fun wipeSettings() {
        this.settings.edit().remove(PROJECT_STRING).apply()
    }

    fun hasNewProjectDir(): Boolean {
        return this.settings.getBoolean(NEW_DIR_STRING, false)
    }

    fun setHasNewProjectDir() {
        this.settings.edit().putBoolean(NEW_DIR_STRING, true).apply()
    }
}