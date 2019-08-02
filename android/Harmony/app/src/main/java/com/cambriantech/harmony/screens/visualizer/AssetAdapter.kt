package com.cambriantech.harmony.screens.visualizer

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageButton
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.CBAsset
import com.cambriantech.harmony.view.RoundButtonBordered
import io.realm.RealmList

/**
 * Created by Joseph Sullivan on 5/5/16.
 */
class AssetAdapter(private val assets: RealmList<CBAsset>,
                   private val canAdd: Boolean,
                   private val onClick: (CBAsset) -> Unit?,
                   private val onLongClick: (CBAsset) -> Unit,
                   private val addClicked: () -> Unit)
        : RecyclerView.Adapter<RecyclerView.ViewHolder>() {

    private var selectedItem = -1
    private val maxSize = 4

    private val VH_COLOR = 0
    private val VH_ADD = 1

    class ColorViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val colorItem: RoundButtonBordered = view.findViewById(R.id.color_item)
    }

    class AddViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val colorItem: ImageButton = view.findViewById(R.id.button_add)
    }

    override fun getItemViewType(position: Int): Int {
        if(canAdd
           && assets.size < maxSize
           && position == assets.size) {
            return VH_ADD
        } else {
            return VH_COLOR
        }
    }

    // Usually involves inflating a layout from XML and returning the holder
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
        //Timber.d("viewType is: $viewType")
        if(viewType == VH_COLOR) {
            val view = LayoutInflater.from(parent.context).inflate(R.layout.asset_item, parent, false)

            val viewHolder = ColorViewHolder(view)

            view.setOnClickListener {
                notifyItemChanged(selectedItem)
                selectedItem = viewHolder.adapterPosition
                notifyItemChanged(selectedItem)
                onClick(assets[viewHolder.adapterPosition]!!)
            }
            view.setOnLongClickListener {
                onLongClick(assets[viewHolder.adapterPosition]!!)
                true
            }
            return viewHolder
        } else {
            val view = LayoutInflater.from(parent.context).inflate(R.layout.asset_add_item, parent, false)

            val viewHolder = AddViewHolder(view)

            view.setOnClickListener {
                addClicked()
            }
            return viewHolder
        }

    }

    // Involves populating data into the item through holder
    override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
        // Get the data model based on getPosition
        if(holder is ColorViewHolder && position < assets.size) {
            val asset = assets[position]!!.item

            val color = asset.color
            holder.colorItem.isSelected = selectedItem == position
            holder.colorItem.setColor(color)
            asset.resource?.thumbnailUrl?.let {
                holder.colorItem.setImage(it)
            }

        }
    }

    fun setSelected(asset: CBAsset?) {
        selectedItem = assets.indexOf(asset)
        //Timber.d("selected index: $selectedItem")
        notifyDataSetChanged()
    }

    // Return the total count of items
    override fun getItemCount(): Int {
        if(canAdd) {
            if(assets.size < maxSize) {
                val count = assets.size + 1
                return count
            }
        }
        return assets.size
    }
}
