package com.cambriantech.harmony.screens.projectlist

import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import butterknife.BindColor
import butterknife.BindView
import butterknife.OnClick
import com.bluelinelabs.conductor.RouterTransaction
import com.bluelinelabs.conductor.changehandler.HorizontalChangeHandler
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.base.MvpView
import com.cambriantech.harmony.data.CBProject
import com.cambriantech.harmony.data.ProjectManager
import com.cambriantech.harmony.dialogs.ConfirmationDialog
import com.cambriantech.harmony.dialogs.TextInputDialog
import com.cambriantech.harmony.screens.project.ProjectImagePager
import io.realm.RealmList
import timber.log.Timber

class ProjectListView : KtController(), MvpView {


    private val projects: RealmList<CBProject>
        get() = ProjectManager.projects
    private var listAdapter: ProjectListAdapter? = null

    @BindView(R.id.tiles) lateinit var rvList: RecyclerView
    @JvmField @BindColor(R.color.colorPrimary) var color: Int = 0

    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_project_list, container, false)
    }

    override fun onAttach(view: View) {
        super.onAttach(view)

        Timber.d("attaching project list view")
        setTitle("My Projects")
        setToolbarColor(color)

        initList()
    }

    private fun updateProjects() {
        listAdapter?.projects = projects
        listAdapter?.notifyItemInserted(0)
        rvList.layoutManager.scrollToPosition(0)
    }

    private fun initList() {
        rvList.layoutManager = LinearLayoutManager(context, LinearLayoutManager.VERTICAL, false)
        listAdapter = ProjectListAdapter(context, projects,
                onClick = { pos ->
                    openProject(pos)
                },
                deleteClicked = { pos ->
                    deleteProject(pos)
                },
                renameClicked = { pos ->
                    renameProject(pos)
                })
        rvList.adapter = listAdapter
        rvList.setHasFixedSize(true)
    }

    private fun openProject(position: Int) {
        val project = projects[position]!!
        router.pushController(RouterTransaction.with(ProjectImagePager(project.id))
                .pushChangeHandler(HorizontalChangeHandler(100))
                .popChangeHandler(HorizontalChangeHandler(100)))
    }

    private fun deleteProject(position: Int) {
        val title = "Delete Project"
        val deleteMessage = "Are you sure?"
        ConfirmationDialog(title, deleteMessage, context, {
            projects[position]!!.delete()
            listAdapter?.projects = projects
            listAdapter?.notifyDataSetChanged()
        })
    }

    private fun renameProject(position: Int) {
        val title = "Rename Project"
        TextInputDialog(title, "", context) { name ->
            projects[position]!!.renameProject(name)
            listAdapter?.notifyItemChanged(position)
        }
    }

    @OnClick(R.id.button_add_project)
    fun addProject() {
        val title = "Create New Project"
        val message = "Enter Name For New Project"
        TextInputDialog(title, message, context, onSubmit = { name ->
            ProjectManager.createProject(name)
            updateProjects()
        })
    }
}
