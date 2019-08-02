package com.cambriantech.harmony.base

import android.app.Application
import android.content.Context
import android.support.v7.app.AppCompatDelegate
import com.cambrian.cbar.CBLicensing
import com.cambrian.cbar.remodeling.CBRemodelingConfiguration
import com.cambriantech.harmony.BuildConfig
import com.cambriantech.harmony.R
import com.cambriantech.harmony.data.BrandCategory
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.data.Favorites
import com.cambriantech.harmony.data.ProjectManager
import com.cambriantech.harmony.util.AWSUtility
import com.cambriantech.harmony.util.CrashlyticsReportingTree
import com.cambriantech.harmony.util.FileUtil
import com.crashlytics.android.Crashlytics
import io.fabric.sdk.android.Fabric
import io.realm.Realm
import io.realm.RealmConfiguration
import io.realm.RealmList
import io.realm.annotations.RealmModule
import timber.log.Timber
import java.io.File
import java.io.IOException


/**
   * Created by Joseph Sullivan on 8/15/16.
  */

class HHApp :Application() {

    companion object {
        lateinit var context: Context
        lateinit var realmProjects: Realm
        lateinit var realmBrands: Realm

        init {
            AppCompatDelegate.setCompatVectorFromResourcesEnabled(true)
        }
    }

    override fun onCreate() {
        super.onCreate()
        context = applicationContext


        if(BuildConfig.DEBUG) {
            Timber.plant(Timber.DebugTree())
        } else {
            Fabric.with(this, Crashlytics())
            Timber.plant(CrashlyticsReportingTree())
        }

        // for future reference
        //createNativeInstance(context.getAssets(), "CBAssets", "dbassets", directory.getAbsolutePath());
        val config = CBRemodelingConfiguration("f81b040d45bc43c688326e13b9877904", context)
        config.setPrimaryAssetPath("dbassets")
        config.setSecondaryAssetPath("dbassets")
        config.initialize()
        CBLicensing.isEnabled()


        Realm.init(context)

        FileUtil.copyBundledRealmFile(context, context.resources.openRawResource(R.raw.paints_floors))

        val realmConfig = RealmConfiguration.Builder()
                .deleteRealmIfMigrationNeeded()
                .build()
        Realm.setDefaultConfiguration(realmConfig)

        val brands = RealmConfiguration.Builder()
                .name("brands")
                .schemaVersion(0)
                .modules(BrandModule())
                //.migration(Migration())
                .build()

        realmProjects = Realm.getDefaultInstance()
        realmBrands = Realm.getInstance(brands)

        //create project if none exists
        UserSettings.initialize(this)

        //UserSettings.getInstance().deleteProject();
        if (UserSettings.hasProject()) {
            ProjectManager.setActiveProject(ProjectManager.projectID)
        } else {
            val project = ProjectManager.createProject("My First Project")
            ProjectManager.setActiveProject(project.id)
        }

        if (!UserSettings.hasAssets()) {
            Timber.w("copying assets directory")
            try {
                FileUtil.copyDirorfileFromAssetManager(assets,
                        "categories",
                        FileUtil.appDirectory.toString() + File.separator.toString() + "assets/categories/")
            } catch (e: IOException) {
                e.printStackTrace()
            }
            UserSettings.setHasAssets(true)
        }

        if(!UserSettings.hasNewProjectDir()) {
            FileUtil.moveProjects()
            UserSettings.setHasNewProjectDir()
        }




        //check if wheel has been made already
        var wheel: BrandCategory? = realmProjects.where(BrandCategory::class.java).contains("id", "wheel").findFirst()
        if (wheel?.id == "wheel") {
            Timber.d("wheel already created")
        } else {
            Timber.d("wheel doesn't exist, creating category")
            val results = realmBrands.where<BrandCategory>(BrandCategory::class.java)
                    .isNull("parentCategory")
                    .findAll()
            val wheelCategories = RealmList<BrandCategory>()
            wheelCategories.addAll(results)

            if (wheelCategories.size > 0) {
                realmProjects.beginTransaction()
                wheel = BrandCategory()
                wheel.id = "wheel"
                wheel.name = ""
                wheel.subCategories = wheelCategories

                realmProjects.copyToRealmOrUpdate(wheel)
                realmProjects.commitTransaction()
            } else {
                Timber.e("can't find wheel category!")
            }
        }

        //init favorites
        Favorites.initialize()

        AWSUtility.initialize(context)
    }

    @RealmModule(classes = arrayOf(BrandCategory::class, BrandItem::class))
    class BrandModule
}