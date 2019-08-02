package com.cambriantech.harmony.screens.project

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import com.bumptech.glide.Glide
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.data.CBAsset
import com.cambriantech.harmony.util.DimenUtil
import io.realm.RealmList

/**
 * Created by Joseph on 6/16/2017.
 */
internal class ImageAssetAdapter(private val assets: RealmList<CBAsset>,
                                 private val onClick: (Int) -> Unit?)
    : RecyclerView.Adapter<ImageAssetAdapter.ViewHolder>() {


    internal class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val assetTile: ViewGroup = view.findViewById(R.id.asset_tile)
        val assetImage: ImageView = view.findViewById(R.id.tile_image)
        val assetName: TextView = view.findViewById(R.id.tile_name)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ImageAssetAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.project_asset_item, parent, false)

        val viewHolder = ViewHolder(v)
        v.setOnClickListener { onClick(viewHolder.adapterPosition) }
        return viewHolder
    }

    override fun onBindViewHolder(holder: ImageAssetAdapter.ViewHolder, position: Int) {
        val tile = holder.assetTile
        val imageView = holder.assetImage
        val textView = holder.assetName

        val width  = DimenUtil.getScreenWidthPixel(HHApp.context) / itemCount
        val params = tile.layoutParams //ViewGroup.LayoutParams(width, 240)
        params.width = width
        tile.layoutParams = params
        tile.requestLayout()

        val asset = assets[position]!!.item

        imageView.setBackgroundColor(asset.color)
        textView.text = asset.name


        if(asset.isFloor()) {
            Glide.with(tile.context)
                    .load(asset.resource?.diffuseUrl)
                    .centerCrop()
                    .into(imageView)
        }

        tile.setOnClickListener{ onClick(holder.adapterPosition) }
    }

    override fun getItemCount() = assets.size
}