package com.cambriantech.harmony.base

import android.content.Intent
import android.os.Bundle
import android.support.v7.app.AppCompatActivity

/**
 * Created by joseph on 10/19/16.
 */

class SplashActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val intent = Intent(this, BaseActivity::class.java)
        startActivity(intent)
        finish()
    }
}