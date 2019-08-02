package com.cambriantech.harmony.data

import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.base.UserSettings
import io.realm.RealmList
import io.realm.Sort
import timber.log.Timber

/**
 * Created by Joseph Sullivan on 9/13/16.
 */

object ProjectManager {

    val projectID: String
        get() = UserSettings.currentProject

    val currentProject: CBProject
        get() {
            getProjectFromID(projectID)?.let { project ->
                return project
            }
            projects.first()?.let { project ->
                setActiveProject(project.id)
                return project
            }

            return createProject("New Project")
        }

    val projects: RealmList<CBProject>
        get() {
            val list = HHApp.realmProjects.where(CBProject::class.java).findAllSorted("modified", Sort.DESCENDING)

            val projects = RealmList<CBProject>()
            projects.addAll(list)
            if (projects.isEmpty()) {
                Timber.d("no project, creating new")
                UserSettings.deleteProject()
                projects.add(createProject("New Project"))
            }
            return projects
        }

    fun getProjectFromID(id: String): CBProject? {
        return HHApp.realmProjects.where(CBProject::class.java)
                .equalTo("id", id)
                .findFirst()
    }


    fun createProject(name: String): CBProject {
        Timber.i("Creating new project")

        val project = CBProject()
        project.name = name

        UserSettings.currentProject = project.id
        Timber.v("ProjectID: " + project.id)

        //Load info into Realm
        HHApp.realmProjects.executeTransaction {
            HHApp.realmProjects.copyToRealmOrUpdate(project)
        }

        //File structure
        Timber.v("Creating project directory: " + project.path.toString())
        project.path.mkdirs()

        return project
    }

    fun setActiveProject(id: String) {
        UserSettings.currentProject = id
        Timber.i("Active project is now " + currentProject.name + " - " + currentProject.id)
    }
}
