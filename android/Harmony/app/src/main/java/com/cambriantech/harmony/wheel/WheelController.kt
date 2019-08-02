package com.cambriantech.harmony.wheel

import android.content.Context
import android.os.Handler
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.ViewTreeObserver
import android.view.animation.DecelerateInterpolator
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.data.BrandCategory
import com.cambriantech.harmony.data.BrandItem
import io.realm.RealmList
import timber.log.Timber
import java.util.*

class WheelController(private val context: Context,
                      private val container: ViewGroup,
                      private val paintOnly: Boolean,
                      private val itemSelected: (BrandItem) -> Unit) {

    lateinit private var backdrop: Arc
    private var topLayer = 0
    private val layers = ArrayList<WheelLayer>()

    init {
        wheelCategories?.let {
            setCategories(it)
        }
    }

    private val wheelCategories: RealmList<BrandCategory>?
        get() {
            if(paintOnly) {
                val wheel = HHApp.realmProjects.where(BrandCategory::class.java)
                        .equalTo("name", "Paint")
                        .findFirst()
                return wheel!!.subCategories
            } else {
                val wheel = HHApp.realmProjects.where(BrandCategory::class.java)
                        .equalTo("id", "wheel")
                        .findFirst()
                return wheel!!.subCategories
            }
        }

    private fun setCategories(categories: RealmList<BrandCategory>) {
        clearWheel()
        addLayer()
        layers[0].setCategories(categories)
        layers[0].setSelectionEnabled(true)
        init()
    }


    private fun clearWheel() {
        for (i in layers.size downTo 1) {
            Timber.i("deleting layer " + i)
            layers.removeAt(i)
            container.removeViewAt(container.childCount - 1)
        }
    }

    private fun init() {
        Handler().postDelayed({
            //Timber.v("opening wheel")
            updateWheel()
            backdrop.visibility = View.VISIBLE
            layers[0].scroll(200)
        }, 600)

        backdrop = container.findViewById(R.id.wheel_layer_background)
        backdrop.sweep = 180f
        backdrop.color = 0xaaffffff.toInt()
        backdrop.multiplier = 0.55f
    }

    private fun itemClick(cat: BrandCategory, layerClicked: Int) {
        var category = cat
        //if top layer
        //Timber.v("click on layer " + layerClicked)

        if (layerClicked < topLayer) {
            Timber.i("stepping up a layer")
            topLayer--
            updateWheel()
            return
        }

        if (layerClicked == layers.size - 1)
            addLayer()
        else
            topLayer++

        val layer = layers[topLayer]

        if (category.hasSubcategories()) {          //if has subCategories
            if (category.subcategoryCount() == 1) {     //if there's only one category, step into it
                category = category.subCategories?.get(0)!!
                if (category.hasSubcategories())       //if this category has subCategories, load them
                    layer.setCategories(category.subCategories!!)
                else //else, load the items
                    layer.setItems(category.items!!)
            } else
                layers[topLayer].setCategories(category.subCategories!!)
        } else
            layers[topLayer].setItems(category.items!!)
        layers[topLayer].scroll(15)
        updateWheel()
    }

    private fun updateWheel() {
        val upper = container.height * 0.01f
        val lower = container.height * 0.62f
        val hidden = container.height.toFloat()

        for (i in layers.indices) {
            when (i) {
                topLayer - 1 -> animateTo(layers[i], lower)
                topLayer -> animateTo(layers[i], upper)
                else -> animateTo(layers[i], hidden)
            }
            layers[i].setSelectionEnabled(i == topLayer)
        }
    }


    private fun animateTo(layer: WheelLayer, height: Float) {
        val v = layer.parent

        v.visibility = View.VISIBLE
        val animate = v.animate()
        animate.y(height)
                .setInterpolator(DecelerateInterpolator())
                .setDuration(300)
                .start()
        animate.setUpdateListener {
            val pos = layers.indices
                    .map { layers[it].parent.translationY }
                    .min()
                    ?: java.lang.Float.MAX_VALUE

            backdrop.translationY = pos
        }
    }

    private fun addLayer() {
        val level = layers.size
        //Timber.v("creating layer " + level)

        val layerContainer = LayoutInflater.from(context).inflate(R.layout.wheel_layer, container, false) as ViewGroup
        val rv = layerContainer.findViewById<WheelRecyclerView>(R.id.wheel_layer)

        val layer = WheelLayer(context, layerContainer, rv) { item, category ->
            if (category != null) {
                itemClick(category, level)
            }
            if (item != null) {
                itemSelected(item)
            }
        }

        rv.viewTreeObserver.addOnGlobalLayoutListener(object : ViewTreeObserver.OnGlobalLayoutListener {
            override fun onGlobalLayout() {
                rv.viewTreeObserver.removeOnGlobalLayoutListener(this)
                //set layer position when layout is complete
                layerContainer.translationY = layerContainer.height.toFloat()
            }
        })

        container.addView(layerContainer, 1)

        //set initial pos to out-of-sight so animations work better
        layerContainer.translationY = container.height.toFloat()
        layerContainer.visibility = View.VISIBLE
        layers.add(layer)
        topLayer = layers.size - 1
    }

}
