package com.cambriantech.harmony.screens.search

import android.graphics.Color
import android.support.v4.graphics.drawable.DrawableCompat
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.ImageView
import android.widget.TextView
import butterknife.BindView
import butterknife.ButterKnife
import com.bumptech.glide.Glide
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.data.BrandItem
import io.realm.RealmResults
import timber.log.Timber

/**
 * Created by Joseph on 3/25/2017.
 */

internal class SearchAdapter(
        private val listener: SearchAdapter.ClickListener)
    : RecyclerView.Adapter<SearchAdapter.ViewHolder>() {

    private var items: RealmResults<BrandItem>? = null


    internal class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        @BindView(R.id.category_item_container) lateinit var container: View
        @BindView(R.id.item_thumb)          lateinit var itemThumb: ImageView
        @BindView(R.id.item_name)           lateinit var itemName: TextView
        @BindView(R.id.item_id)             lateinit var itemID: TextView
        @BindView(R.id.details_button)      lateinit var buttonDetails: Button

        init {
            ButterKnife.bind(this, view)
            var moreButtonDrawable = buttonDetails.compoundDrawables[0]
            moreButtonDrawable = DrawableCompat.wrap(moreButtonDrawable)
            DrawableCompat.setTint(moreButtonDrawable, Color.WHITE)
            buttonDetails.setCompoundDrawables(moreButtonDrawable, null, null, null)
        }
    }

    interface ClickListener {
        fun onClick(v: View, position: Int)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): SearchAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.palette_detail_item, parent, false)

        val viewHolder = SearchAdapter.ViewHolder(v)
        v.setOnClickListener { view -> listener.onClick(view, viewHolder.adapterPosition) }
        return viewHolder
    }

    override fun onBindViewHolder(holder: SearchAdapter.ViewHolder, position: Int) {
        val itemThumb = holder.itemThumb
        val itemName = holder.itemName
        val itemID = holder.itemID

        val item = items?.get(position) ?: return
        val type = if(item.isPaint()) "Paint" else "Floor"

        itemThumb.setBackgroundColor(item.color)
        itemName.text = "$type Name: ${item.name}"
        itemID.text = "$type ID: ${item.storeID}"


        if(item.isFloor()) {
            Glide.with(HHApp.context)
                    .load(item.resource?.thumbnailUrl)
                    .into(itemThumb)
        }

        holder.container.setOnClickListener { view -> listener.onClick(view, holder.adapterPosition) }
    }

    override fun getItemCount(): Int {
        items?.size?.let {
            return it
        }
        return 0
    }

    fun setItems(items: RealmResults<BrandItem>?) {
        items?.addChangeListener { collection, changeSet ->  notifyDataSetChanged()}
        this.items = items
        if(items == null) {
            notifyDataSetChanged()
        }
        items?.let {
            val count:Int = it.size
            Timber.d("list size: $count")
        }
    }
}