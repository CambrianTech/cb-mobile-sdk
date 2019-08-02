package com.cambriantech.harmony.screens.search

import android.app.SearchManager
import android.content.Context
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.support.v7.widget.SearchView
import android.view.LayoutInflater
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.InputMethodManager
import android.widget.EditText
import butterknife.BindColor
import butterknife.BindView
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.BaseActivity
import com.cambriantech.harmony.base.HHApp
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.base.MvpView
import com.cambriantech.harmony.data.BrandItem
import com.cambriantech.harmony.util.hide
import com.cambriantech.harmony.util.onQueryTextChanged
import com.cambriantech.harmony.util.show
import com.cambriantech.harmony.view.BottomSheetDetail
import com.crashlytics.android.answers.Answers
import com.crashlytics.android.answers.SearchEvent
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.subjects.PublishSubject
import io.realm.Case
import io.realm.RealmResults
import timber.log.Timber
import java.util.concurrent.TimeUnit

/**
 * Created by Joseph on 3/20/2017.
 */

class ColorSearchView : KtController(), MvpView {

    @BindView(R.id.search_rv) lateinit var rvSearch: RecyclerView
    @BindView(R.id.progress_view) lateinit var progressView: View
    @BindView(R.id.bottom_sheet) lateinit var bottomSheet: BottomSheetDetail
    @JvmField @BindColor(R.color.colorPrimary) var color: Int = 0

    private lateinit var adapter: SearchAdapter
    private var items: RealmResults<BrandItem>? = null
    private var query: String? = null


    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_search, container, false)
    }

    override fun onAttach(view: View) {
        super.onAttach(view)

        setToolbarColor(color)
        bottomSheet.init()

        adapter = SearchAdapter(object: SearchAdapter.ClickListener {
            override fun onClick(v: View, position: Int) {
                items?.get(position)?.let {
                    updateColorSheet(it)
                }
            }
        })

        rvSearch.adapter = adapter
        val layout = LinearLayoutManager(context, LinearLayoutManager.VERTICAL, false)
        layout.isAutoMeasureEnabled = true
        rvSearch.setHasFixedSize(true)
        rvSearch.layoutManager = layout
    }

    fun handleMenu(item: MenuItem) {
        val searchView: SearchView = item.actionView as SearchView
        searchView.queryHint = "Name or ID..."
        val searchEditText: EditText = searchView.findViewById(android.support.v7.appcompat.R.id.search_src_text)
        val searchManager = context.getSystemService(Context.SEARCH_SERVICE) as SearchManager
        searchView.setSearchableInfo(searchManager.getSearchableInfo((activity as BaseActivity).componentName))

        val source = PublishSubject.create<String>()
        source
                .doOnNext { text ->
                    query = text
                    if(text.isEmpty()) clear()
                    else progressView.show()
                }
                .debounce(350, TimeUnit.MILLISECONDS, AndroidSchedulers.mainThread())
                .subscribe { text ->
                    if(text.isEmpty()) {
                        clear()
                        return@subscribe
                    }
                    getResults(text)

                    items?.let {
                        progressView.hide()
                        adapter.setItems(it)
                    }
                }
        source
                .filter { it.isNotEmpty() }
                .debounce(2000, TimeUnit.MILLISECONDS)
                .subscribe { text ->
                    Timber.d("logging search to fabric")
                    Answers.getInstance().logSearch(SearchEvent().putQuery(text))
                }

        query?.let {
            item.expandActionView()
            searchView.setQuery(it, true)
            searchView.requestFocus()
            source.onNext(it)
        }

        searchView.onQueryTextChanged { text ->
            source.onNext(text)
        }

        searchEditText.setOnFocusChangeListener { _, hasFocus ->
            if(hasFocus) bottomSheet.hide()
        }
    }


    private fun getResults(text: String) {
        items = HHApp.realmBrands.where(BrandItem::class.java)
                .contains("name", text, Case.INSENSITIVE)
                .or()
                .contains("storeID", text, Case.INSENSITIVE).findAllAsync()
    }


    private fun updateColorSheet(item: BrandItem) {
        bottomSheet.updateSheet(item)
        val focused = activity?.currentFocus
        focused?.clearFocus()
        val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as? InputMethodManager
        imm?.hideSoftInputFromWindow(focused?.windowToken, 0)
    }

    fun clear() {
        if(isAttached) {
            items = null
            adapter.setItems(items)
            progressView.visibility = View.GONE
        }
    }
}