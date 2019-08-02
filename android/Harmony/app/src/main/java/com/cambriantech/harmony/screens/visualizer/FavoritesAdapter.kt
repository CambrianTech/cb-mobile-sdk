package com.cambriantech.harmony.screens.visualizer

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.view.RoundButtonBordered
import io.realm.RealmList

/**
 * Created by Joseph Sullivan on 5/5/16.
 */
class FavoritesAdapter(private val favorites: RealmList<BrandItem>,
                       private val onClick: (BrandItem) -> Unit?,
                       private val onLongClick: (BrandItem) -> Unit)
        : RecyclerView.Adapter<FavoritesAdapter.ViewHolder>() {


    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val colorItem: RoundButtonBordered = view.findViewById(R.id.color_item)
    }

    // Usually involves inflating a layout from XML and returning the holder
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): FavoritesAdapter.ViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.asset_item, parent, false)

        val viewHolder = ViewHolder(view)

        view.setOnClickListener {
            onClick(favorites[viewHolder.adapterPosition]!!)
        }
        view.setOnLongClickListener {
            onLongClick(favorites[viewHolder.adapterPosition]!!)
            true
        }
        return viewHolder
    }

    // Involves populating data into the item through holder
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        // Get the data model based on getPosition
        val color = favorites[position]!!.color
        holder.colorItem.isSelected = false
        holder.colorItem.setColor(color)
    }


    // Return the total count of items
    override fun getItemCount(): Int {
        return favorites.size
    }
}