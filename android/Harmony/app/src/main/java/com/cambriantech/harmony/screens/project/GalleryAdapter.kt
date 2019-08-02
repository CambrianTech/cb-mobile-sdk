package com.cambriantech.harmony.screens.project

import android.content.Context
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import butterknife.BindView
import butterknife.ButterKnife
import com.bumptech.glide.Glide
import com.bumptech.glide.signature.StringSignature
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.CBImage
import io.realm.RealmResults
import timber.log.Timber

internal class GalleryAdapter(private val context:Context,
                              private val images:RealmResults<CBImage>,
                              private val onClick: (Int) -> Unit?)
    :RecyclerView.Adapter<GalleryAdapter.ViewHolder>() {


    internal class ViewHolder(view:View):RecyclerView.ViewHolder(view) {
        @BindView(R.id.project_gallery_item)  lateinit var galleryImage:ImageView

        init { ButterKnife.bind(this, view) }
    }


    override fun onCreateViewHolder(parent:ViewGroup, viewType:Int):GalleryAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.project_gallery_item, parent, false)

        val viewHolder = ViewHolder(v)
        v.setOnClickListener {
            onClick(viewHolder.adapterPosition)
        }
        return viewHolder
    }

    override fun onBindViewHolder(holder:GalleryAdapter.ViewHolder, position:Int) {
        val image = images[position]!!
        Glide.with(context)
            .load(image.previewOrOriginal)
            .signature(StringSignature(image.modified.toString()))
            .crossFade()
            .override(400, 400)
            .into((holder).galleryImage)
}

    override fun getItemCount() = images.size

    internal fun onItemRemove(position:Int) {
        Timber.i("refreshing gallery")
        notifyItemRemoved(position)
        notifyItemRangeChanged(position, itemCount)
    }
}
