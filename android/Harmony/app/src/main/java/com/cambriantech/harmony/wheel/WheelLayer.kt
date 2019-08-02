package com.cambriantech.harmony.wheel

import android.content.Context
import android.os.Handler
import android.view.ViewGroup
import com.cambriantech.harmony.data.BrandCategory
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.util.DimenUtil
import io.realm.RealmList
import timber.log.Timber


/**
 * Created by Joseph Sullivan on 8/2/16.
 */

class WheelLayer(private val context: Context,
                 val parent: ViewGroup,
                 private val wheel: WheelRecyclerView,
                 private val onClick: (item: BrandItem?, category: BrandCategory?) -> Unit) {

    private var categories: RealmList<BrandCategory>? = null
    private var items: RealmList<BrandItem>? = null
    private var adapter: WheelAdapter? = null
    private var lm: WheelLayoutManager? = null

    private var previousPos = 0

    fun setCategories(categories: RealmList<BrandCategory>) {
        clearItems()
        this.categories = categories
        init()
    }

    fun setItems(items: RealmList<BrandItem>) {
        clearItems()
        this.items = items
        init()
    }

    fun clearItems() {
        this.categories = null
        this.items = null
    }

    fun init() {
        lm = WheelLayoutManager()
        wheel.layoutManager = lm
        //if (categories != null) Timber.d("creating wheel layer with " + categories!!.size + " categories")
        //if (items != null) Timber.d("creating wheel layer with " + items!!.size + " items")

        adapter = WheelAdapter(categories, items) { position ->
            if (adapter?.selectionEnabled == true) {
                val holder = wheel.findViewHolderForAdapterPosition(previousPos) as WheelAdapter.ViewHolder?
                if (previousPos != position) {
                    //Timber.v("reset previous item")
                    holder?.arcItem?.isSelected = false
                }

                previousPos = position
            }
            categories?.get(position)?.let {
                onClick(null, it)
            }
            items?.get(position)?.let {
                onClick(it, null)
            }
        }

        wheel.setHasFixedSize(true)
        wheel.adapter = adapter
    }

    fun scroll(delay: Int) {
        Handler().postDelayed({
            var numItems = 0
            if (categories != null) numItems = categories!!.size
            if (items != null) numItems = items!!.size

            Timber.v("size: " + numItems)


            val dp = DimenUtil.getDensity(context)
            when {
                numItems < 5 -> {
                    val amount = 360 * dp * -(numItems - 1)
                    Timber.d("Amount: " + amount)
                    //has to be run twice because there's apparently some limit to how much it can scroll at once (????)
                    wheel.scrollBy(amount.toInt() / 2, 0)
                    wheel.scrollBy(amount.toInt() / 2, 0)
                }
                numItems == 6 -> {
                    val amount = Math.round(9.5f * dp * -numItems)
                    Timber.v("scrolling by: " + amount)
                    wheel.scrollBy(amount, 0)
                }
                else -> lm?.scrollToPosition(numItems / 2 - 3)
            }
        }, delay.toLong())
    }

    fun setSelectionEnabled(bool: Boolean) {
        adapter!!.selectionEnabled = bool
    }
}
