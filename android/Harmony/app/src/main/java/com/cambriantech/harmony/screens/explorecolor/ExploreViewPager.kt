package com.cambriantech.harmony.screens.explorecolor

import android.os.Bundle
import android.support.design.widget.TabLayout
import android.support.v4.view.ViewPager
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import butterknife.BindColor
import butterknife.BindView
import com.bluelinelabs.conductor.Controller
import com.bluelinelabs.conductor.Router
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.support.RouterPagerAdapter
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.BundleBuilder
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.data.BrandCategory
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.view.BottomSheetDetail
import io.realm.RealmList
import timber.log.Timber

/**
 * Created by Joseph Sullivan on 9/14/16.
 */

class ExploreViewPager(args: Bundle) : KtController(args), GridView.SelectionInterface {

    @BindView(R.id.tab_layout)          lateinit var tabLayout: TabLayout
    @BindView(R.id.view_pager)          lateinit var viewPager: ViewPager
    @BindView(R.id.bottom_sheet)  lateinit var bottomSheet: BottomSheetDetail

    @JvmField @BindColor(R.color.colorPrimary) var color: Int = 0

    private lateinit var collection: BrandCategory
    private lateinit var categories: RealmList<BrandCategory>

    constructor(collection: BrandCategory) : this(BundleBuilder(Bundle())
            .build()) {
        this.collection = collection
    }

    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_explore_color, container, false)
    }

    override fun onAttach(view: View) {
        super.onAttach(view)

        setTitle(collection.name)
        setToolbarColor(color)


        categories = collection.subCategories ?: RealmList(collection)

        val pagerAdapter = object : RouterPagerAdapter(this) {
            override fun configureRouter(router: Router, position: Int) {
                if (!router.hasRootController()) {
                    router.setRoot(RouterTransaction.with(createPage(categories[position]!!)))
                } else {
                    val controller = router.backstack[0].controller() as GridView
                    setListener(controller)
                }
            }

            override fun getPageTitle(position: Int): CharSequence {
                return categories[position]!!.name
            }

            override fun getCount(): Int {
                return categories.size
            }
        }

        Timber.d("attaching")

        viewPager.adapter = pagerAdapter
        tabLayout.setupWithViewPager(viewPager)
        bottomSheet.init()
    }

    fun setListener(controller: GridView) {
        controller.setListener(this)
    }


    override fun handleBack(): Boolean {
        Timber.d("pressed back")
        return bottomSheet.handleBack()
    }

    private fun createPage(category: BrandCategory): Controller {
        Timber.d("category: " + category.name)
        val tab = GridView(category)
        tab.setListener(this)
        tab.instanceId
        Timber.d("instance id = $tab.instanceID")
        return tab
    }

    override fun onDestroyView(view: View) {
        viewPager.adapter = null
        super.onDestroyView(view)
    }

    override fun itemSelected(item: BrandItem) {
        Timber.i("item selected")
        bottomSheet.updateSheet(item)
    }
}
