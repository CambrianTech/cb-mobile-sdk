package com.cambriantech.harmony.view

import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.OrientationHelper
import android.support.v7.widget.PagerSnapHelper
import android.view.View

/**
 * Created by Joseph on 10/10/2017.
 */
class LockablePager() : PagerSnapHelper() {

    private var verticalHelper: OrientationHelper? = null
    private var horizontalHelper: OrientationHelper? = null

    fun findSnapView(layoutManager: LockableLLM): View? {
        if (layoutManager.orientation == LinearLayoutManager.VERTICAL) {
            return findCenterView(layoutManager, getVerticalHelper(layoutManager))
        } else if (layoutManager.orientation == LinearLayoutManager.HORIZONTAL) {
            return findCenterView(layoutManager, getHorizontalHelper(layoutManager))
        }
        return null
    }


    private fun findCenterView(lm: LockableLLM,
                               helper: OrientationHelper): View? {
        val childCount = lm.childCount
        if (childCount == 0) {
            return null
        }

        var closestChild: View? = null
        val center: Int
        center = if (lm.clipToPadding) {
            helper.startAfterPadding + helper.totalSpace / 2
        } else {
            helper.end / 2
        }
        var absClosest = Integer.MAX_VALUE

        for (i in 0 until childCount) {
            val child = lm.getChildAt(i)
            val childCenter = helper.getDecoratedStart(child) + helper.getDecoratedMeasurement(child) / 2
            val absDistance = Math.abs(childCenter - center)

            /** if child center is closer than previous closest, set it as closest   */
            if (absDistance < absClosest) {
                absClosest = absDistance
                closestChild = child
            }
        }
        return closestChild
    }


    private fun getVerticalHelper(lm: LockableLLM): OrientationHelper {
        return verticalHelper ?: OrientationHelper.createVerticalHelper(lm)
    }

    private fun getHorizontalHelper(lm: LockableLLM): OrientationHelper {
        return horizontalHelper ?: OrientationHelper.createHorizontalHelper(lm)
    }
}