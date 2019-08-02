package com.cambriantech.harmony.screens.visualizer

import android.graphics.Color
import android.support.v4.graphics.drawable.DrawableCompat
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.Menu
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import butterknife.BindView
import butterknife.ButterKnife
import com.cambriantech.harmony.R

/**
 * Created by Joseph Sullivan on 7/21/16.
 */

internal class DropdownAdapter(val menu: Menu,
                               val selectable: Boolean,
                               val onClick: (Int) -> Unit)
    : RecyclerView.Adapter<DropdownAdapter.ViewHolder>() {

    private var selectedItem = 0

    internal class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        @BindView(R.id.dropdown_generic_text)       lateinit var name: TextView
        @BindView(R.id.dropdown_generic_icon)       lateinit var icon: ImageView
        @BindView(R.id.dropdown_generic_container)  lateinit var container: View

        init {
            ButterKnife.bind(this, view)
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): DropdownAdapter.ViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.dropdown_item, parent, false)

        // Return a new holder instance
        val viewHolder = ViewHolder(view)
        view.setOnClickListener { v ->
            notifyItemChanged(selectedItem)
            selectedItem = viewHolder.adapterPosition
            notifyItemChanged(selectedItem)
            onClick(viewHolder.adapterPosition)
        }

        return viewHolder
    }

    override fun onBindViewHolder(holder: DropdownAdapter.ViewHolder, position: Int) {
        val icon = holder.icon
        val name = holder.name
        val container = holder.container

        name.text = menu.getItem(position).title
        icon.setImageDrawable(menu.getItem(position).icon)

        container.isActivated = selectedItem == position

        if (selectable) {
            var iconTinted = icon.drawable
            iconTinted = DrawableCompat.wrap(iconTinted)
            if (container.isActivated) {
                name.setTextColor(Color.WHITE)
                DrawableCompat.setTint(iconTinted, Color.WHITE)
            } else {
                name.setTextColor(Color.GRAY)
                DrawableCompat.setTint(iconTinted, Color.GRAY)
            }
            icon.setImageDrawable(iconTinted)
        }
    }

    override fun getItemCount(): Int {
        return menu.size()
    }
}
