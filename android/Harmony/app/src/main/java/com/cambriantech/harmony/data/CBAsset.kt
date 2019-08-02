package com.cambriantech.harmony.data

import com.cambrian.cbar.CBAugmentedAsset
import com.cambrian.cbar.remodeling.CBRemodelingFloor
import com.cambrian.cbar.remodeling.CBRemodelingPaint
import com.cambrian.cbar.types.CBAssetType
import com.cambriantech.harmony.base.HHApp
import io.realm.RealmObject
import io.realm.annotations.PrimaryKey
import java.util.*


open class CBAsset() : RealmObject() {

    @PrimaryKey
    var id: String = UUID.randomUUID().toString()
    var created: Date = Date(Calendar.getInstance().timeInMillis)
    var modified: Date = Date(Calendar.getInstance().timeInMillis)


    private var _item: BrandItem? = null
    var item: BrandItem
        get() = _item!!
        set(value) { _item = value }

    constructor(item: BrandItem) : this() {
        this.item = item
    }

    fun modified() {
        this.modified = Date(Calendar.getInstance().timeInMillis)
    }

    val coreAsset: CBAugmentedAsset
        get() {
            val asset: CBAugmentedAsset
            if(item.isFloor()) {
                asset = CBRemodelingFloor(this.id)
            } else {
                asset = CBRemodelingPaint(this.id)
                asset.color = item.color
            }
            return asset
        }

    companion object {
        fun randomPaint() : CBAsset {
            val query = HHApp.realmProjects.where(BrandItem::class.java)
            query.equalTo("type", CBAssetType.Paint.ordinal)
            val list = query.findAll()
            val random = Math.floor(Math.random() * query.count()).toInt()
            return CBAsset(list.get(random)!!)
        }

        fun randomFloor() : CBAsset {
            val query = HHApp.realmProjects.where(BrandItem::class.java)
            query.equalTo("type", CBAssetType.Floor.ordinal)
            val list = query.findAll()
            val random = Math.floor(Math.random() * query.count()).toInt()
            return CBAsset(list.get(random)!!)
        }
    }
}