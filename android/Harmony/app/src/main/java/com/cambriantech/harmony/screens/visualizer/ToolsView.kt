package com.cambriantech.harmony.screens.visualizer

import android.content.Context
import android.graphics.Color
import android.graphics.drawable.Drawable
import android.support.constraint.ConstraintLayout
import android.support.v4.graphics.drawable.DrawableCompat
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.util.AttributeSet
import android.view.LayoutInflater
import android.view.View
import android.view.animation.AccelerateDecelerateInterpolator
import android.view.animation.OvershootInterpolator
import android.widget.ImageView
import butterknife.BindDrawable
import butterknife.BindView
import butterknife.ButterKnife
import butterknife.OnClick
import com.cambrian.cbar.types.CBToolMode
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.data.CBAsset
import com.cambriantech.harmony.data.Favorites
import com.cambriantech.harmony.dialogs.ConfirmationDialog
import com.cambriantech.harmony.util.*
import io.realm.RealmList

/**
 * Created by Joseph on 12/15/2017.
 */

class ToolsView : ConstraintLayout {

    interface ToolsCallback {
        fun assetSelected(asset: CBAsset)
        fun assetRemoved(asset: CBAsset)
        fun assetAdded()
        fun itemSelected(item: BrandItem)
        fun undoPressed()
        fun setToolMode(mode: CBToolMode)
        fun hideWheel()
        fun isLive(): Boolean
    }

    @BindView(R.id.button_more)         lateinit var buttonMore: ImageView
    @BindView(R.id.button_assets)       lateinit var buttonAssets: ImageView
    @BindView(R.id.button_favorites)    lateinit var buttonFavorites: ImageView
    @BindView(R.id.button_tools)        lateinit var buttonTools: ImageView
    @BindView(R.id.button_undo)         lateinit var buttonUndo: ImageView
    @BindView(R.id.button_fill)         lateinit var buttonFill: ImageView
    @BindView(R.id.button_brush)        lateinit var buttonBrush: ImageView
    @BindView(R.id.button_eraser)       lateinit var buttonEraser: ImageView
    @BindView(R.id.menu_tools)          lateinit var menuTools: View
    @BindView(R.id.rv_assets)           lateinit var rvAssets: RecyclerView
    @BindView(R.id.rv_favorites)        lateinit var rvFavorites: RecyclerView


    private var assetAdapter: AssetAdapter? = null
    private var favoritesOpen = false
    private var menuOpen = true
    var callback: ToolsCallback? = null


    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)

    init {
        val inflater = context.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        inflater.inflate(R.layout.tools_view, this, true)
        ButterKnife.bind(this)
        initFavorites()
    }


    fun initAssets(assets: RealmList<CBAsset>) {
        assetAdapter = AssetAdapter(assets, true,
                                    onClick = { asset ->
                                        callback?.assetSelected(asset)
                                    },
                                    onLongClick = { asset ->
                                        val title = "Delete Item"
                                        val message = "Are you sure?"
                                        ConfirmationDialog(title, message, context, {
                                            callback?.assetRemoved(asset)
                                        })
                                    },
                                    addClicked = {
                                        callback?.assetAdded()
                                    })
        val lm = LinearLayoutManager(context, LinearLayoutManager.HORIZONTAL, false)
        lm.reverseLayout = true

        rvAssets.setHasFixedSize(true)
        rvAssets.adapter = assetAdapter
        rvAssets.layoutManager = lm
    }

    private fun initFavorites() {
        val favorites = Favorites.favorites
        val adapter =
                FavoritesAdapter(favorites,
                                 onClick = { favorite ->
                                     callback?.itemSelected(favorite)
                                 },
                                 onLongClick = { favorite ->
                                     val title = "Remove From Favorites"
                                     val message = "Are you sure?"
                                     ConfirmationDialog(title, message, context, {
                                         Favorites.remove(favorite)
                                     })
                                 })
        val lm = LinearLayoutManager(context, LinearLayoutManager.HORIZONTAL, false)
        lm.reverseLayout = true

        rvFavorites.setHasFixedSize(true)
        rvFavorites.adapter = adapter
        rvFavorites.layoutManager = lm

        if(favorites.isEmpty()) Animations.show(false, buttonFavorites, true) {}


        toggleFavorites(favorites)
        favorites.addChangeListener { favs ->
            adapter.notifyDataSetChanged()
            toggleFavorites(favs)
        }
    }


    private fun toggleFavorites(favorites: RealmList<BrandItem>) {
        if(favorites.isEmpty()) {
            rvFavorites.hide()
            buttonFavorites.hide()
        } else {
            if(menuOpen) {
                if (favoritesOpen) rvFavorites.show()
                buttonFavorites.show()
            }
        }
    }


    /****************************
     *                          *
     *     Click Listeners      *
     *                          *
     ****************************/


    @OnClick(R.id.button_more) fun pressedMore() {
        openTools(false)
        openAssets(false)
        openFavorites(false)
        expand(!menuOpen)
    }

    @OnClick(R.id.button_assets) fun pressedAssets() {
        openAssets()
        openFavorites(false)
        openTools(false)
    }

    @OnClick(R.id.button_favorites) fun pressedFavorites() {
        openFavorites(!favoritesOpen)
        openAssets(false)
        openTools(false)
    }

    @OnClick(R.id.button_tools) fun pressedTools() {
        openTools()
        openAssets(false)
        openFavorites(false)
    }

    @OnClick(R.id.button_undo) fun pressedUndo() {
        callback?.undoPressed()
    }



    @BindDrawable(R.drawable.ic_eraser) lateinit var eraserIcon: Drawable
    @BindDrawable(R.drawable.ic_brush) lateinit var brushIcon: Drawable
    @BindDrawable(R.drawable.ic_bucket) lateinit var fillIcon: Drawable

    @OnClick(R.id.button_fill, R.id.button_brush, R.id.button_eraser)
    fun pressedTool(view: View) {
        callback?.hideWheel()
        when(view.id) {
            R.id.button_brush -> callback?.setToolMode(CBToolMode.Paintbrush)
            R.id.button_eraser -> callback?.setToolMode(CBToolMode.Eraser)
            R.id.button_fill -> callback?.setToolMode(CBToolMode.Fill)
        }
    }


    fun setToolMode(mode: CBToolMode) {
        fun getTint(match: Boolean) = if(match) Color.WHITE else Color.DKGRAY

        fun tintButtons(index: Int) {
            buttonFill.tint(getTint(index == 0))
            buttonBrush.tint(getTint(index == 1))
            buttonEraser.tint(getTint(index == 2))
        }

        when(mode) {
            CBToolMode.Fill -> {
                tintButtons(0)
                buttonTools.setImageDrawable(fillIcon)
            }
            CBToolMode.Paintbrush -> {
                tintButtons(1)
                buttonTools.setImageDrawable(brushIcon)
            }
            CBToolMode.Eraser -> {
                tintButtons(2)
                buttonTools.setImageDrawable(eraserIcon)
            }
        }
    }


    fun updateAssets(asset: CBAsset?) {
        assetAdapter?.setSelected(asset)
    }

    fun enableUndoButton(enable: Boolean) {
        val color = if(enable) Color.WHITE else Color.DKGRAY
        DrawableCompat.setTint(buttonUndo.drawable, color)
    }

    fun hide(hide: Boolean) {
        val amount: Float
        if (this.translationY == 0f && hide) {
            amount = (buttonMore.left + (this.right)).toFloat()
            openAssets(false)
            openFavorites(false)
            openTools(false)
            expand(false)
        } else {
            amount = 0f
        }
        this.animate()
                .translationX(amount)
                .setDuration(180)
                .setInterpolator(OvershootInterpolator(1f)).start()
    }


    private fun expand(show: Boolean) {
        menuOpen = show
        Animations.show(show, buttonAssets, true) {}
        if(show && Favorites.favorites.isNotEmpty()) {
            Animations.show(show, buttonFavorites, true) {}
        } else if (!show) {
            Animations.show(show, buttonFavorites, true) {}
        }
        Animations.show(show, buttonTools, true) {}
        //Animations.show(show, buttonUndo, true) {}
        val rotate = if(show) 0f else 135f
        buttonMore.animate()
                .rotation(rotate)
                .setInterpolator(AccelerateDecelerateInterpolator())
                .start()
    }



    private fun openAssets(show: Boolean? = null) {
        Animations.show(show, rvAssets, false) {}
    }

    private fun openFavorites(show: Boolean? = null) {
        Animations.show(show, rvFavorites, false) { open ->
            favoritesOpen = open
        }
    }

    private fun openTools(show: Boolean? = null) {
        if(menuTools.isHidden()) {
            if(callback?.isLive() == true) {
                buttonBrush.hide()
                buttonEraser.hide()
            } else {
                buttonBrush.show()
                buttonEraser.show()
            }
        }
        Animations.show(show, menuTools, false) {}
    }

}