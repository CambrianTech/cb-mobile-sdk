package com.cambriantech.harmony.data


import android.graphics.Color
import com.cambrian.cbar.types.CBAssetType
import com.cambriantech.harmony.util.AWSUtility
import io.realm.RealmObject
import io.realm.annotations.Ignore
import io.realm.annotations.PrimaryKey
import java.util.*

open class BrandItem: RealmObject() {

    @PrimaryKey
    var id: String = UUID.randomUUID().toString()
    var parentCategory: BrandCategory? = BrandCategory()
    var name: String = ""
    var storeID: String = ""
    var storeLink: String = ""
    var orderIndex: Long = 0
    var assetPath: String = ""
    var info: String? = null
    var scale: Float = 0.toFloat()
    var reflectivity: Float = 0.toFloat()
    var red: Int = 0
    var green: Int = 0
    var blue: Int = 0
    var opacity: Int = 0
    var type: Int = 0

    @Ignore
    var resource: CBRemoteResource? = null
        get() {
            if(field == null) {
                field = AWSUtility.getResourceForId(id)
            }
            return field
        }


    var color: Int
        get() = Color.rgb(red, green, blue)
        set(color) {
            this.red = Color.red(color)
            this.green = Color.green(color)
            this.blue = Color.blue(color)
        }


    fun getRootCategory(): BrandCategory {
        return parentCategory!!.getRootCategory()
    }

    fun getBrand() : BrandCategory {
        return parentCategory!!.getBrand()
    }

    fun type(): CBAssetType = CBAssetType.fromOrdinal(type)
    fun isPaint() = type() == CBAssetType.Paint
    fun isFloor() = type() == CBAssetType.Floor
}
