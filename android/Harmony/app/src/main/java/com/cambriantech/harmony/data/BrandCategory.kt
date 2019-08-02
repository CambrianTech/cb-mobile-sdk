package com.cambriantech.harmony.data

import android.graphics.Color
import com.cambrian.cbar.types.CBAssetType
import com.cambriantech.harmony.util.FileUtil
import io.realm.RealmList
import io.realm.RealmObject
import io.realm.annotations.PrimaryKey
import java.io.File
import java.util.*

open class BrandCategory: RealmObject() {

    @PrimaryKey
    var id: String = UUID.randomUUID().toString()
    var parentCategory: BrandCategory? = null
    var isIndoor: Boolean = false
    var isOutdoor: Boolean = false
    var name: String = ""
    var assetPath: String = ""
    var orderIndex: Int = 0
    var displayItem: BrandItem? = null
    var items: RealmList<BrandItem>? = null
    var subCategories: RealmList<BrandCategory>? = null
    var red: Int = 0
    var green: Int = 0
    var blue: Int = 0
    var type: Int = 0

    fun hasSubcategories()  = this.subCategories?.isNotEmpty() ?: false
    fun hasItems()          = this.items?.isNotEmpty() ?: false
    fun subcategoryCount()  = this.subCategories?.count() ?: 0
    fun itemCount()         = this.items?.count() ?: 0


    var color: Int
        get() {
            val col = Color.rgb(red, green, blue)
            if (col != Color.BLACK) return col
            else
                return displayItem?.color ?: Color.GRAY
        }
        set(value) {
            this.red = Color.red(value)
            this.green = Color.green(value)
            this.blue = Color.blue(value)
        }


    fun getAssetPath(): File? {
        if(this.assetPath.isNullOrEmpty()) return null
        val dir = File(FileUtil.appDirectory.toString() + File.separator + "assets" + File.separator + this.assetPath)
        dir.list()?.firstOrNull()?.let {
            val file = File(dir, it)
            if(file.exists())
                return file
        }
        return null
    }

    fun getRootCategory(): BrandCategory {
        var cat = this
        while (cat.parentCategory != null) {
            cat.parentCategory?.let {
                cat = it
            }
        }
        return cat
    }

    fun getBrand() : BrandCategory {
        var cat = this
        while(cat.parentCategory != null) {
            cat.parentCategory?.let { parent ->
                if(parent.parentCategory == null) return cat
                cat = parent
            }
        }
        return cat
    }

    fun isBrand() = getBrand() == this

    fun type(): CBAssetType = CBAssetType.fromOrdinal(type)
    fun isPaint() = type() == CBAssetType.Paint
    fun isFloor() = type() == CBAssetType.Floor
}

