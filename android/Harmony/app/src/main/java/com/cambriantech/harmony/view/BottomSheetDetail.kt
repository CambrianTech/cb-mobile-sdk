package com.cambriantech.harmony.view

import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.content.res.Configuration
import android.graphics.Color
import android.graphics.drawable.Drawable
import android.net.Uri
import android.support.constraint.ConstraintLayout
import android.support.design.widget.BottomSheetBehavior
import android.util.AttributeSet
import android.view.LayoutInflater
import android.view.View
import android.widget.Button
import android.widget.ImageView
import android.widget.TextView
import butterknife.BindDrawable
import butterknife.BindView
import butterknife.ButterKnife
import butterknife.OnClick
import com.bumptech.glide.Glide
import com.cambriantech.harmony.BuildConfig
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.data.Favorites
import com.cambriantech.harmony.util.ColorUtil
import com.cambriantech.harmony.util.DimenUtil
import com.cambriantech.harmony.util.hide
import com.cambriantech.harmony.util.show
import com.crashlytics.android.answers.AddToCartEvent
import com.crashlytics.android.answers.Answers
import timber.log.Timber
import javax.annotation.Nullable


/**
 * Created by Joseph Sullivan on 7/28/16.
 */

class BottomSheetDetail : ConstraintLayout {

    private lateinit var bottomSheetBehavior: BottomSheetBehavior<*>

    @BindView(R.id.item_preview)            lateinit var previewView: ImageView
    @BindView(R.id.item_name)               lateinit var itemNameView: TextView
    @BindView(R.id.paint_details)           lateinit var itemDetailsView: TextView
    @BindView(R.id.button_add_favorite)     lateinit var buttonFavorite: Button
    @BindView(R.id.buy_now_button)          lateinit var buttonBuy: Button
    @BindView(R.id.prestige_info_icon)      lateinit var infoIcon: View
    @BindView(R.id.prestige_info)           lateinit var infoText: TextView

    @BindDrawable(R.drawable.ic_delete)     lateinit var drawableDelete: Drawable
    @BindDrawable(R.drawable.ic_add) lateinit var drawableAdd: Drawable

    @JvmField
    @Nullable
    @BindView(R.id.brand_logo)
    var logoView: ImageView? = null

    lateinit private var item: BrandItem

    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)
    constructor(context: Context, attrs: AttributeSet, defStyle: Int) : super(context, attrs, defStyle)


    fun init() {
        val inflater = LayoutInflater.from(context)
        inflater.inflate(R.layout.bottom_sheet_detail, this, true)
        bottomSheetBehavior = BottomSheetBehavior.from(this)
        bottomSheetBehavior.peekHeight = 0
        hide()
        ButterKnife.bind(this)
    }


    @SuppressLint("SetTextI18n")
    fun updateSheet(item: BrandItem) {
        this.item = item
        Timber.d("updating sheet with " + item.name)

        itemNameView.text = item.name
        val brand = item.getBrand()
        itemDetailsView.text = "Brand: ${brand.name}\n" +
                               "Name: ${item.name}\n" +
                               "Product ID: ${item.storeID}"

        handlePrestige()
        handlePreview()
        unhide()

        val path = brand.getAssetPath()
        Timber.i("path: $path")
        logoView?.let { view ->
            Glide.with(context)
                    .load(path)
                    .into(view)
        }
    }


    private fun handlePreview() {
        previewView.setBackgroundColor(item.color)

        val params = previewView.layoutParams

        if(item.isFloor()) {
            Timber.d("is floor")
            params.height = DimenUtil.dpToPx(context, 136)
            previewView.layoutParams = params
            item.resource?.diffuseUrl?.let { url ->
                Glide.with(context)
                        .load(url)
                        .dontAnimate()
                        .centerCrop()
                        .into(previewView)
            }
            buttonBuy.hide()
        } else {
            previewView.setImageResource(android.R.color.transparent)
            params.height = DimenUtil.dpToPx(context, 72)
            previewView.layoutParams = params
            buttonBuy.show()
        }

    }


    private fun handlePrestige() {
        if (isPrestige(item) || !item.isPaint()) {
            infoIcon.hide()
            infoText.hide()
        } else {
            infoIcon.show()
            infoText.show()
            if(resources.configuration.orientation != Configuration.ORIENTATION_LANDSCAPE) {
                val text = context.getString(R.string.explain_prestige_amazon)
                text.plus(" ${item.name}")
                infoText.text = text
            }
        }
    }


    private fun unhide() {
        updateFavoriteButton()
        bottomSheetBehavior.state = BottomSheetBehavior.STATE_EXPANDED
    }

    private fun toggleFavorite() {
        if(Favorites.hasItem(item))
            Favorites.remove(item)
        else
            Favorites.add(item)
    }

    private fun updateFavoriteButton() {
        if(Favorites.hasItem(item)) {
            buttonFavorite.text = context.getString(R.string.favorites_remove)
            ColorUtil.setDrawableLeft(buttonFavorite, drawableDelete, Color.WHITE)
        } else {
            buttonFavorite.text = context.getString(R.string.favorites_add)
            ColorUtil.setDrawableLeft(buttonFavorite, drawableAdd, Color.WHITE)
        }
    }

    fun handleBack(): Boolean {
        if (bottomSheetBehavior.state == BottomSheetBehavior.STATE_EXPANDED) {
            hide()
            return true
        }
        return false
    }

    private fun isPrestige(item: BrandItem): Boolean {
        val category = item.getBrand()
        return category.name.contains("Prestige")
    }


    private fun sendToAmazon(item: BrandItem) {
        val baseURL = "https://www.amazon.com/gp/product/"
        val asin = item.storeLink
        val code = "?ie=UTF8"
        var tag = "&tag="
        when (BuildConfig.FLAVOR) {
            "prestige" -> tag += "prestige-android-20"
            "harmony" -> tag += "cambrian-android-20"
        }
        val url = baseURL + asin + code + tag
        Timber.i("URL: " + url)
        val i = Intent(Intent.ACTION_VIEW)
        i.data = Uri.parse(url)
        context?.startActivity(i)
        //https://www.amazon.com/gp/product/B00QCEO9QE?ie=UTF8&tag=prestige-android-20
    }


    /****************************
     *     Click Listeners      *
     ****************************/

    @OnClick(R.id.button_add_favorite) fun pressedFavorite() {
        toggleFavorite()
        updateFavoriteButton()
    }

    @OnClick(R.id.buy_now_button) fun pressedBuy() {
        Answers.getInstance().logAddToCart(AddToCartEvent()
                                           .putItemName(item.name)
                                           .putItemId(item.storeID)
                                           .putCustomAttribute("Brand", item.getRootCategory().name))
        sendToAmazon(item)
    }

    @OnClick(R.id.hide_sheet_button)
    fun hide() {
        bottomSheetBehavior.state = BottomSheetBehavior.STATE_COLLAPSED
    }
}
