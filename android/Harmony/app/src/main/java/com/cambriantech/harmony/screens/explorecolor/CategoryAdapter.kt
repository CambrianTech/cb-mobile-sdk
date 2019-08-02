package com.cambriantech.harmony.screens.explorecolor

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import butterknife.BindView
import butterknife.ButterKnife
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.BrandCategory
import com.cambriantech.harmony.view.NameView
import io.realm.RealmList
import timber.log.Timber

/**
 * Created by Joseph Sullivan on 7/15/16.
 */

internal class CategoryAdapter(private val categories: RealmList<BrandCategory>,
                               private val onClick: (pos: Int) -> Unit)
                : RecyclerView.Adapter<CategoryAdapter.ViewHolder>() {


    internal class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        @BindView(R.id.item_name)   lateinit var title: TextView
        @BindView(R.id.brand_image) lateinit var icon: NameView

        init { ButterKnife.bind(this, view) }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): CategoryAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.paint_category_item, parent, false)

        val viewHolder = ViewHolder(v)

        v.setOnClickListener { onClick(viewHolder.adapterPosition) }
        return viewHolder
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val title = holder.title
        val image = holder.icon

        val category = categories.get(position)!!

        val name: String
        val color: Int
        if(category.parentCategory == null) {
            name = category.name
            color = category.color
        } else {
            name = category.getBrand().name
            color = category.getBrand().color
        }

        Timber.v("name: $name")

        image.itemName = name
        image.color = color

        title.text = category.name
    }

    // Return the total count of items
    override fun getItemCount(): Int {
        return categories.size
    }
}
