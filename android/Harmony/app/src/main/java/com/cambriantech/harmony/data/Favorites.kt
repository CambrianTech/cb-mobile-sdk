package com.cambriantech.harmony.data

import com.cambriantech.harmony.base.HHApp
import io.realm.RealmList
import timber.log.Timber

/**
 * Created by Joseph on 10/13/2017.
 */
object Favorites {

    val favorites: RealmList<BrandItem>
        get() {
            val fav = HHApp.realmProjects.where(FavoritesContainer::class.java).contains("id", "favorites").findFirst()
            if(fav == null) {
                //Timber.d("creating new favorites")
                val favs = FavoritesContainer()
                HHApp.realmProjects.executeTransaction {
                    HHApp.realmProjects.copyToRealmOrUpdate(favs)
                }
                return favs.favorites
            } else {
                //Timber.d("attaching existing favorites")
                return fav.favorites
            }
        }

    //for some reason I have to access the list here and add/remove items
    //or else it will crash in the visualizer when accessed. I have no idea why
    fun initialize() {
        Timber.d("listing favorites")
        val favs = Favorites.favorites
        val paint = CBAsset.randomPaint().item
        HHApp.realmProjects.executeTransaction {
            favs.add(paint)
        }
        favs.forEach({ Timber.d("name " + it.name) })
        HHApp.realmProjects.executeTransaction {
            favs.remove(paint)
        }
    }


    fun add(item: BrandItem) {
        HHApp.realmProjects.executeTransaction {
            Timber.d("adding item with id: " + item.id)
            favorites.add(item)
        }
    }

    fun remove(item: BrandItem) {
        HHApp.realmProjects.executeTransaction {
            favorites.remove(item)
        }
    }

    fun hasItem(item: BrandItem): Boolean {
        val result = favorites.where().equalTo("id", item.id).findFirst()
        return (result != null)
    }

    fun getCategory() : BrandCategory {
        val category = BrandCategory()
        category.name = "Favorites"
        category.id = "favorites"
        category.items = favorites
        return category
    }
}