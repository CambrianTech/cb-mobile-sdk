package com.cambriantech.harmony.base

import android.animation.ObjectAnimator
import android.os.Bundle
import android.support.design.widget.NavigationView
import android.support.v4.view.GravityCompat
import android.support.v4.widget.DrawerLayout
import android.support.v7.app.ActionBarDrawerToggle
import android.support.v7.app.AppCompatActivity
import android.support.v7.graphics.drawable.DrawerArrowDrawable
import android.support.v7.widget.Toolbar
import android.view.*
import butterknife.BindView
import butterknife.ButterKnife
import com.bluelinelabs.conductor.Conductor
import com.bluelinelabs.conductor.Controller
import com.bluelinelabs.conductor.Router
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.HorizontalChangeHandler
import com.cambriantech.harmony.R
import com.cambriantech.harmony.screens.calculator.CalculatorView
import com.cambriantech.harmony.screens.colorfinder.ColorFinderController
import com.cambriantech.harmony.screens.explorecolor.CategoryView
import com.cambriantech.harmony.screens.home.HomeView
import com.cambriantech.harmony.screens.projectlist.ProjectListView

class BaseActivity : AppCompatActivity(), NavigationView.OnNavigationItemSelectedListener, ToolbarProvider, ToolbarToggle {

    @BindView(R.id.toolbar)             lateinit var toolbar: Toolbar
    @BindView(R.id.drawer_layout)       lateinit var drawer: DrawerLayout
    @BindView(R.id.nav_view)            lateinit var navigationView: NavigationView
    @BindView(R.id.controller_container) lateinit var container: ViewGroup

    private var header: View? = null
    private var router: Router? = null
    private var menuIsBack = false
    private var drawerArrow: DrawerArrowDrawable? = null
    private var navClick: View.OnClickListener? = null


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_base)

        ButterKnife.bind(this)

        setSupportActionBar(toolbar)
        drawerArrow = DrawerArrowDrawable(this)

        toolbar.navigationIcon = drawerArrow
        toolbar.setPadding(0, statusBarHeight, 0, 0)

        val toggle = ActionBarDrawerToggle(this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close)
        toggle.isDrawerIndicatorEnabled = false
        toggle.syncState()

        navClick = View.OnClickListener {
            if (menuIsBack)
                router?.popCurrentController()
            else
                drawer.openDrawer(GravityCompat.START)
        }
        toggle.toolbarNavigationClickListener = navClick

        router = Conductor.attachRouter(this, container, savedInstanceState)
        if (!router?.hasRootController()!!) {
            router?.setRoot(RouterTransaction.with(HomeView()))
        }

        navigationView.setNavigationItemSelectedListener(this)
        header = navigationView.getHeaderView(0)
        header?.setOnClickListener {
            drawer.closeDrawer(Gravity.LEFT)
            router?.let {
                if(it.backstack[it.backstackSize - 1]?.controller() !is HomeView) {
                    launch(HomeView())
                }
            }

        }

    }

    override fun onBackPressed() {
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START)
        } else if (!router?.handleBack()!!) {
            super.onBackPressed()
        }
    }


    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.main, menu)
        return true
    }

    override fun onNavigationItemSelected(item: MenuItem): Boolean {
        val id = item.itemId

        when (id) {
            R.id.action_home -> {
                router?.setRoot(RouterTransaction.with(HomeView()))
                router?.popToRoot()
            }
            /*
            R.id.action_camera -> {
                router?.backstack!![0].controller().let {
                    if (it.childRouters.size > 0) {
                        it.removeChildRouter(it.childRouters[0])
                    }
                    it.getChildRouter(FrameLayout(this), null)
                            .setPopsLastView(true)
                            .setRoot(RouterTransaction.with(PermissionsView(PermissionsView.CAMERA_REQUEST_CODE)))
                }
            }
            */
            R.id.action_color ->        launch(CategoryView())
            R.id.action_project_list -> launch(ProjectListView())
            R.id.action_calculator ->   launch(CalculatorView())
            R.id.action_finder ->       launch(ColorFinderController())
        }

        drawer.closeDrawer(GravityCompat.START)
        return true
    }

    private fun launch(controller: Controller) {
        router?.pushController(RouterTransaction.with(controller)
                .pushChangeHandler(HorizontalChangeHandler(150))
                .popChangeHandler(HorizontalChangeHandler(150)))
    }


    val statusBarHeight: Int
        get() {
            val resourceId = resources.getIdentifier("status_bar_height", "dimen", "android")
            if (resourceId > 0)
                return resources.getDimensionPixelSize(resourceId)
            else
                return 0
        }

    override fun setMenuAsBack(bool: Boolean) {
        if (menuIsBack == bool) return
        ObjectAnimator.ofFloat(drawerArrow, "progress", if (bool) 1f else 0f).start()
        menuIsBack = bool
    }

    override fun getActivityToolbar(): Toolbar {
        return toolbar
    }

    override fun resetNav() {
        if (navClick != null) {
            toolbar.setNavigationOnClickListener(navClick)
        }
    }
}
