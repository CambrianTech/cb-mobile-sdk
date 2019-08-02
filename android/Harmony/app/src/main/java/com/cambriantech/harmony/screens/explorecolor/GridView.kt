package com.cambriantech.harmony.screens.explorecolor

import android.os.Bundle
import android.support.v7.widget.GridLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import butterknife.BindView
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.BundleBuilder
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.base.MvpView
import com.cambriantech.harmony.data.BrandCategory
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.util.DimenUtil
import timber.log.Timber


/**
 * Created by Joseph Sullivan on 9/14/16.
 */

class GridView(args: Bundle) : KtController(args), MvpView {

    private var listener: SelectionInterface? = null


    interface SelectionInterface {
        fun itemSelected(item: BrandItem)
    }

    @BindView(R.id.color_subcollection) lateinit var rvItems: RecyclerView

    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_color_tab, container, false)
    }

    companion object {
        private val CATEGORY_ID = "ColorTab.category"
    }

    constructor(category: BrandCategory) : this(BundleBuilder(Bundle())
            .putString(CATEGORY_ID, category.id)
            .build())

    fun setListener(listener: SelectionInterface) {
        this.listener = listener
    }

    override fun onViewBound(view: View) {
        super.onViewBound(view)

        val id = args.getString(CATEGORY_ID)
        val category = HHApp.realmBrands.where(BrandCategory::class.java).equalTo("id", id).findFirst()
        if(category == null) {
            Timber.e("CATEGORY NULL")
            router.popCurrentController()
        }


        val items = category?.items ?: return

        val columns = if(category.isFloor()) 3 else 7
        val spacing = DimenUtil.dpToPx(context, 2) // in pixels
        val padding = DimenUtil.dpToPx(context, 16)
        val screenWidth = DimenUtil.getScreenWidthPixel(context)
        val viewWidth = screenWidth - (padding * 2 + spacing * (columns - 1))
        val size = viewWidth / columns

        val decorator = GridSpacingItemDecoration(columns, spacing, false)

        val adapter = GridAdapter(items, size, size, onClick = { position ->
            val item = items[position]
            listener?.itemSelected(item!!)
        })

        rvItems.layoutManager = GridLayoutManager(activity, columns)
        rvItems.setHasFixedSize(true)
        rvItems.isNestedScrollingEnabled = false
        rvItems.adapter = adapter
        rvItems.addItemDecoration(decorator)
    }
}
