package com.cambriantech.harmony.screens.project

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.animation.AccelerateDecelerateInterpolator
import android.widget.ImageView
import com.bumptech.glide.Glide
import com.bumptech.glide.signature.StringSignature
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.HHApp.Companion.context
import com.cambriantech.harmony.data.CBImage
import com.cambriantech.harmony.view.BeforeAfterSlider
import io.realm.RealmResults

internal class ImageAdapter(private val images: RealmResults<CBImage>,
                            private val onImageClick: () -> Unit)
    : RecyclerView.Adapter<ImageAdapter.ViewHolder>() {

    var comparing = false

    internal class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val imageView: ImageView = view.findViewById(R.id.project_image)
        val imageViewSlider: BeforeAfterSlider = view.findViewById(R.id.project_image_slider)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ImageAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.project_image, parent, false)

        return ViewHolder(v)
    }

    override fun onBindViewHolder(holder: ImageAdapter.ViewHolder, position: Int) {
        val imageView = holder.imageView
        val imageViewSlider = holder.imageViewSlider
        val image = images[holder.adapterPosition]!!
        holder.itemView.tag = image.id


        if(comparing) {
            imageViewSlider.setDrawableLeft(image.originalPath)
            imageViewSlider.setDrawableRight(image.previewOrOriginal)


            imageViewSlider.visibility = View.VISIBLE
            imageViewSlider.redrawArrows()
            imageView.visibility = View.INVISIBLE
        } else {
            imageView.visibility = View.VISIBLE
            imageViewSlider.visibility = View.INVISIBLE

            imageView.setOnClickListener {
                onImageClick()
            }

            Glide.with(context)
                    .load(image.previewOrOriginal)
                    .signature(StringSignature(image.modified.toString()))
                    .crossFade(200)
                    .fitCenter()
                    .into(imageView)
        }
    }

    private fun hideAssets(view: View, hide: Boolean, duration: Int = 0) {
        val amount = if(hide) view.height else -view.height
        view.animate()
                .translationYBy(amount.toFloat())
                .setDuration(duration.toLong())
                .setInterpolator(AccelerateDecelerateInterpolator())
                .start()
    }

    override fun getItemCount() = images.size
}