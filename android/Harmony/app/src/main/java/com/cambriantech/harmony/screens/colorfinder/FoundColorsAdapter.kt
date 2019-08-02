package com.cambriantech.harmony.screens.colorfinder

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.util.DimenUtil

/**
 * Created by Joseph on 6/16/2017.
 */
internal class FoundColorsAdapter(private var items: MutableList<ColorMatch>,
                                  private val onClick: (Int) -> Unit?)
    : RecyclerView.Adapter<FoundColorsAdapter.ViewHolder>() {


    internal class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val assetTile: View = view.findViewById(R.id.asset_tile)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): FoundColorsAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.project_asset_item, parent, false)

        val viewHolder = ViewHolder(v)
        v.setOnClickListener { onClick(viewHolder.adapterPosition) }
        return viewHolder
    }

    override fun onBindViewHolder(holder: FoundColorsAdapter.ViewHolder, position: Int) {
        val tile = holder.assetTile


        val width  = DimenUtil.getScreenWidthPixel(HHApp.context) / itemCount
        val params = tile.layoutParams //ViewGroup.LayoutParams(width, 240)
        params.width = width
        tile.layoutParams = params
        tile.requestLayout()

        tile.setBackgroundColor(items[position].color)

        tile.setOnClickListener{ onClick(holder.adapterPosition) }
    }

    fun setItems(items: MutableList<ColorMatch>) {
        this.items = items
        notifyDataSetChanged()
    }

    override fun getItemCount() = items.size
}