package com.cambriantech.harmony.base

/**
 * Created by joseph on 9/9/16.
 */

interface Presenter<V> {
    fun attachView(view: V)
    fun detachView()
}
