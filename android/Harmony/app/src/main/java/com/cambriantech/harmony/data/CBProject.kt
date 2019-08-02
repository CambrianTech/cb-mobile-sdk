package com.cambriantech.harmony.data

import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.util.FileUtil
import io.realm.RealmList
import io.realm.RealmObject
import io.realm.RealmResults
import io.realm.Sort
import io.realm.annotations.PrimaryKey
import timber.log.Timber
import java.io.File
import java.util.*

/**
 * Created by Joseph Sullivan on 6/15/16.
 */

open class CBProject : RealmObject() {

    @PrimaryKey
    var id: String = UUID.randomUUID().toString()
    var name: String = ""
    var created: Date = Date(Calendar.getInstance().timeInMillis)
    var modified: Date = Date(Calendar.getInstance().timeInMillis)
    var images: RealmList<CBImage> = RealmList()

    fun modified() {
        HHApp.realmProjects.executeTransaction {
            this.modified = Date(Calendar.getInstance().timeInMillis)
        }
    }

    val firstImage: File?
        get() = sortedImages.firstOrNull()?.previewOrOriginal

    val lastImage: File?
        get() = sortedImages.lastOrNull()?.previewOrOriginal

    val sortedImages: RealmResults<CBImage>
        get() = images.sort("modified", Sort.DESCENDING)

    val path: File
        get() = File(FileUtil.projectsDirectory, id)

    fun renameProject(name: String) {
        HHApp.realmProjects.executeTransaction {
            this.name = name
        }
    }

    fun delete() {
        if(this.isManaged) {
            Timber.i("deleting project: " + this.name)
            path.deleteRecursively()
            if(ProjectManager.projects.size == 1) {
                ProjectManager.createProject("New Project")
            }
            HHApp.realmProjects.executeTransaction {
                this.deleteFromRealm()
            }
        }
    }

    fun addImage(id: String?) {
        if(id == null) return
        val image = CBImage(id)
        addImage(image)
    }

    fun addImage(image: CBImage) {
        Timber.i("adding new image with ID: " + image.id)
        HHApp.realmProjects.executeTransaction {
            this.images.add(image)
        }
    }

    fun getImageForID(id: String?) : CBImage? {
        return this.images.firstOrNull { it.id == id }
    }
}
