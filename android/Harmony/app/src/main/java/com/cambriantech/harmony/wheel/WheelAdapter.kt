package com.cambriantech.harmony.wheel


import android.graphics.Color
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.BrandCategory
import com.cambriantech.harmony.data.BrandItem
import io.realm.RealmList

/**
 * Created by Joseph Sullivan on 7/6/16.
 */

class WheelAdapter(private val categories: RealmList<BrandCategory>?,
                   private val items: RealmList<BrandItem>?,
                   private val onClick: (position: Int) -> Unit)
        : RecyclerView.Adapter<WheelAdapter.ViewHolder>() {


    private val multiplier = 1.9f
    var selectionEnabled = true

    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        var arcItem: Arc = view.findViewById(R.id.arc)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): WheelAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.wheel_item, parent, false) as Arc
        val viewHolder = ViewHolder(v)

        v.onClick {
            if (selectionEnabled) {
                viewHolder.arcItem.isSelected = true
            }
            onClick(viewHolder.adapterPosition)
        }
        return viewHolder
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val arcItem = holder.arcItem

        var name: String
        var path: String?
        categories?.get(position)?.let { category ->
            val color = category.displayItem?.color ?: Color.GRAY

            name = category.name
            arcItem.setParams(color, name, multiplier)

            path = category.getAssetPath()?.toString()
                   ?: category.displayItem?.resource?.thumbnailUrl

            if(!category.isBrand()) {
                arcItem.setImagePath(path)
            }
        }

        items?.get(position)?.let { item ->
            arcItem.setParams(item.color, item.name, multiplier)
            path = item.resource?.thumbnailUrl
            arcItem.setImagePath(path)
        }

        arcItem.showText = true
        //arcItem.setImagePath(path)
    }

    override fun getItemCount(): Int {
        return if (categories != null)
            categories.size
        else
            items!!.size
    }
}
