package com.cambriantech.harmony.data

import io.realm.RealmList
import io.realm.RealmObject
import io.realm.annotations.PrimaryKey

/**
 * Created by Joseph on 10/13/2017.
 */
open class FavoritesContainer : RealmObject() {

    @PrimaryKey
    var id: String = "favorites"
    var favorites: RealmList<BrandItem> = RealmList()
}