package com.cambriantech.harmony.screens.explorecolor

import android.os.Bundle
import android.support.v4.view.MenuItemCompat
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.*
import butterknife.BindColor
import butterknife.BindView
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.FadeChangeHandler
import com.bluelinelabs.conductor.changehandler.HorizontalChangeHandler
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.BundleBuilder
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.base.MvpView
import com.cambriantech.harmony.data.BrandCategory
import com.cambriantech.harmony.screens.search.ColorSearchView
import io.realm.RealmList
import timber.log.Timber

class CategoryView : KtController, MvpView {

    @BindView(R.id.category_rv)      lateinit var rvCategories: RecyclerView
    @BindView(R.id.search_container) lateinit var searchContainer: ViewGroup
    @JvmField @BindColor(R.color.colorPrimary) var color: Int = 0

    private var categories = RealmList<BrandCategory>()
    private var name = "Explore Color"

    constructor() : super()
    constructor(args: Bundle) : super(args)
    constructor(categories: RealmList<BrandCategory>, title: String) : this(BundleBuilder(Bundle())
            .build()) {

        this.categories = categories
        this.name = title
    }

    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_color_categories, container, false)
    }

    override fun onAttach(view: View) {
        super.onAttach(view)

        setTitle(name)
        setToolbarColor(color)

        setHasOptionsMenu(true)
        initCategories()

        if(getChildRouter(searchContainer).hasRootController())
            Timber.d("has search view")
    }


    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.toolbar_explore_color, menu)
        val searchItem = menu.findItem(R.id.item_search)

        if(getChildRouter(searchContainer).hasRootController()) {
            Timber.d("has root controller")
            val searchView = getChildRouter(searchContainer).backstack[0].controller() as ColorSearchView?
            searchView?.handleMenu(searchItem)
        }

        MenuItemCompat.setOnActionExpandListener(searchItem, object: MenuItemCompat.OnActionExpandListener {
            override fun onMenuItemActionExpand(item: MenuItem): Boolean {
                Timber.d("clicked on search view")
                if(!getChildRouter(searchContainer).hasRootController()) {
                    Timber.d("opening search view")
                    val searchView = ColorSearchView()
                    getChildRouter(searchContainer).setRoot(RouterTransaction.with(searchView)
                            .pushChangeHandler(FadeChangeHandler(250))
                            .popChangeHandler(FadeChangeHandler(250)))
                    searchView.handleMenu(item)
                }
                return true
            }

            override fun onMenuItemActionCollapse(item: MenuItem?): Boolean {
                if (getChildRouter(searchContainer).hasRootController()) {
                    removeChildRouter(getChildRouter(searchContainer))
                }
                return true
            }
        })
    }


    private fun initCategories() {

        if (categories.isEmpty()) {
            categories = RealmList()

            categories.addAll(HHApp.realmBrands.where(BrandCategory::class.java)
                    .isNull("parentCategory")
                    .findAll())
        }

        val adapter = CategoryAdapter(categories) { position ->
            categories[position]?.let { category ->
                //check if it has child categories
                if (category.hasSubcategories() && (category.subCategories?.get(0)?.hasItems() == false)) {
                    router.pushController(RouterTransaction.with(CategoryView(category.subCategories!!, category.name))
                                                  .pushChangeHandler(HorizontalChangeHandler(100))
                                                  .popChangeHandler(HorizontalChangeHandler(100)))
                } else if (category.subCategories?.get(0)?.hasItems() == true) {
                    router.pushController(RouterTransaction.with(ExploreViewPager(category))
                                                  .pushChangeHandler(HorizontalChangeHandler(100))
                                                  .popChangeHandler(HorizontalChangeHandler(100)))
                }
            }
        }

        rvCategories.adapter = adapter
        rvCategories.setHasFixedSize(true)
        rvCategories.layoutManager = LinearLayoutManager(activity)

    }

    override fun handleBack(): Boolean {
        if (getChildRouter(searchContainer).hasRootController()) {
            Timber.d("popping search view")
            removeChildRouter(getChildRouter(searchContainer))
            return true
        }
        return false
    }
}
