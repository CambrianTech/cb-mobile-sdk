package com.cambriantech.harmony.screens.projectlist

import android.content.Context
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import com.bumptech.glide.Glide
import com.bumptech.glide.load.engine.DiskCacheStrategy
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.CBProject
import io.realm.RealmList


/**
 * Created by Joseph Sullivan on 5/25/16.
 */

internal class ProjectListAdapter(private val context: Context,
                                  var projects: RealmList<CBProject>,
                                  private val onClick: (Int) -> Unit?,
                                  private val deleteClicked: (Int) -> Unit?,
                                  private val renameClicked: (Int) -> Unit?)
    : RecyclerView.Adapter<ProjectListAdapter.ViewHolder>() {


    internal class ViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        var title: TextView = itemView.findViewById(R.id.project_title)
        var thumb: ImageView = itemView.findViewById(R.id.project_thumbnail)
        var buttonDelete: View = itemView.findViewById(R.id.button_delete)
        var buttonRename: View = itemView.findViewById(R.id.button_rename)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ProjectListAdapter.ViewHolder {
        val v = LayoutInflater.from(parent.context).inflate(R.layout.project_list_item, parent, false)

        // Return a new holder instance
        val viewHolder = ViewHolder(v)
        v.setOnClickListener { onClick(viewHolder.adapterPosition) }

        viewHolder.buttonDelete.setOnClickListener {
            deleteClicked(viewHolder.adapterPosition)
        }
        viewHolder.buttonRename.setOnClickListener {
            renameClicked(viewHolder.adapterPosition)
        }
        return viewHolder
    }

    override fun onBindViewHolder(holder: ProjectListAdapter.ViewHolder, position: Int) {

        val project = projects[position]
        val firstImage = project!!.firstImage

        Glide.with(context)
                .load(firstImage)
                .fallback(R.drawable.ic_photo)
                .crossFade(200)
                .centerCrop()
                .diskCacheStrategy(DiskCacheStrategy.ALL)
                .into(holder.thumb)

        holder.title.text = project.name
    }

    // Return the total count of items
    override fun getItemCount(): Int {
        return projects.size
    }
}
