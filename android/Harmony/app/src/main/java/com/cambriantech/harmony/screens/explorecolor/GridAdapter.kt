package com.cambriantech.harmony.screens.explorecolor

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import com.bumptech.glide.Glide
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.HHApp.Companion.context
import com.cambriantech.harmony.data.BrandItem
import io.realm.RealmList

/**
 * Created by Joseph Sullivan on 5/5/16.
 */
internal class GridAdapter(private val items: RealmList<BrandItem>,
                           private val width: Int,
                           private val height: Int,
                           private val onClick: (Int) -> Unit?)
        : RecyclerView.Adapter<GridAdapter.ViewHolder>() {


    internal class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val thumbView:ImageView = view.findViewById(R.id.thumb_item)
    }

    // Usually involves inflating a layout from XML and returning the holder
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): GridAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.thumb_item, parent, false)

        val viewHolder = ViewHolder(v)
        v.setOnClickListener { onClick(viewHolder.adapterPosition) }
        return viewHolder
    }

    // Involves populating data into the item through holder
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        // Get the data model based on getPosition
        val thumb = holder.thumbView
        val dimens = ViewGroup.LayoutParams(width, height)
        val item = items[position]!!

        thumb.layoutParams = dimens
        thumb.setBackgroundColor(item.color)

        // thumbnail loading
        if(item.isFloor()) {
            val url = item.resource?.thumbnailUrl ?: return

            Glide.with(context).load(url).into(thumb)
        }
    }

    // Return the total count of items
    override fun getItemCount(): Int {
        return items.size
    }
}
