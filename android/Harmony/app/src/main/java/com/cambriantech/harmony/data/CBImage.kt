package com.cambriantech.harmony.data

import com.cambriantech.harmony.base.HHApp
import io.realm.RealmList
import io.realm.RealmObject
import io.realm.annotations.PrimaryKey
import timber.log.Timber
import java.io.File
import java.util.*

/**
 * Created by Joseph Sullivan on 6/15/16.
 */

open class CBImage(): RealmObject() {

    @PrimaryKey
    var id: String = UUID.randomUUID().toString()
    var project: CBProject? = ProjectManager.currentProject
    var name: String = ""
    var created: Date = Date(Calendar.getInstance().timeInMillis)
    var modified: Date = Date(Calendar.getInstance().timeInMillis)
    var assets: RealmList<CBAsset> = RealmList()

    constructor(imageID: String) : this() {
        this.id = imageID
    }

    fun addToRealm() {
        HHApp.realmProjects.executeTransaction {
            it.copyToRealmOrUpdate(this)
        }
    }

    fun modified() {
        val time = Date(Calendar.getInstance().timeInMillis)
        this.modified = time
        project?.modified = time
    }

    val rootPath: File
        get() {
            val project = this.project ?: ProjectManager.currentProject
            val dir = File(project.path, id)
            if(!dir.exists()) dir.mkdirs()
            return dir
        }

    val previewPath: File
        get() = File(rootPath, "preview.jpg")

    val originalPath: File
        get() = File(rootPath, "scene.jpg")

    val previewOrOriginal: File
        get() = if (previewPath.exists()) previewPath else originalPath

    fun delete() {
        Timber.d("deleting image: " + this.name)
        rootPath.deleteRecursively()
        if (isManaged)
            HHApp.realmProjects.executeTransaction { this.deleteFromRealm() }
    }

    fun itemIsInAssets(item: BrandItem): Boolean {
        assets.filter { it.item.id == item.id }
                .map {
                    Timber.v("item match found in assets: " + item.name)
                    return true }
        Timber.v(item.name + " is not in assets")
        return false
    }

    fun assetForID(id: String) : CBAsset? {
        return assets.firstOrNull { it.id == id }
    }

    fun getFloor() : CBAsset? {
        return assets.firstOrNull { it.item.isFloor() }
    }

    fun hasFloor() : Boolean {
        assets.forEach { if(it.item.isFloor()) return true }
        return false
    }

    fun hasPaint() : Boolean {
        assets.forEach { if(it.item.isPaint()) return true }
        return false
    }
}