package com.cambriantech.harmony.screens.colorfinder

import android.graphics.Point
import android.graphics.PointF
import android.graphics.drawable.Drawable
import android.support.design.widget.CoordinatorLayout
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import butterknife.BindColor
import butterknife.BindDrawable
import butterknife.BindView
import butterknife.OnClick
import com.cambrian.cbar.remodeling.CBColorFinderView
import com.cambrian.cbar.remodeling.CBRemodelingScene
import com.cambrian.cbar.types.CBToolMode
import com.cambriantech.harmony.R
import com.cambriantech.harmony.base.KtController
import com.cambriantech.harmony.util.ColorUtil
import com.cambriantech.harmony.util.hide
import com.cambriantech.harmony.util.show
import com.cambriantech.harmony.view.BottomSheetDetail
import com.cambriantech.harmony.view.ColorView
import com.cambriantech.harmony.view.LockedLLM
import com.cambriantech.harmony.view.ProgressView

/**
 * Created by Joseph on 10/19/2017.
 */


class ColorFinderController : KtController(), ColorFinderInterface, CBColorFinderView.CBColorFinderViewListener {

    @BindView(R.id.color_finder_container)          lateinit var container: ViewGroup
    @BindView(R.id.rv_colors)                       lateinit var rvColors: RecyclerView
    @BindView(R.id.button_capture)                  lateinit var buttonCapture: View
    @BindView(R.id.button_video)                    lateinit var buttonVideo: ImageView
    @BindView(R.id.bottom_sheet)                    lateinit var bottomSheet: BottomSheetDetail
    @BindView(R.id.progress_view)                   lateinit var progressView: ProgressView
    @BindDrawable(R.drawable.translucent_toolbar)   lateinit var toolbarDrawable: Drawable
    @BindDrawable(R.drawable.ic_video)              lateinit var videoIcon: Drawable
    @JvmField @BindColor(R.color.bar_transparency)           var transparent: Int = 0

    lateinit private var presenter: ColorFinderPresenter
    lateinit private var colorViews: MutableList<ColorView>
    private var colorFinder: CBColorFinderView? = null

    private var items: MutableList<ColorMatch> = mutableListOf()
    lateinit private var adapter: FoundColorsAdapter


    override fun inflateView(inflater: LayoutInflater, container: ViewGroup): View {
        return inflater.inflate(R.layout.controller_color_finder, container, false)
    }

    override fun onAttach(view: View) {
        super.onAttach(view)

        setTitle("Color Finder")
        toolbar?.background = toolbarDrawable

        presenter = ColorFinderPresenter()
        presenter.attachView(this)

        colorFinder = CBColorFinderView(context)
        colorFinder?.let { finder ->

            finder.layoutParams = CoordinatorLayout.LayoutParams(CoordinatorLayout.LayoutParams.MATCH_PARENT, CoordinatorLayout.LayoutParams.MATCH_PARENT)
            container.addView(colorFinder, 0)
            finder.setListener(this)
            finder.visibility = View.VISIBLE

            colorViews = mutableListOf()
            for (i in 0..4) {
                val cv = ColorView(context, 40, container)
                cv.hide()
                cv.tag = i.toString()
                container.addView(cv)
                colorViews.add(cv)

                val touchListener = View.OnTouchListener {
                    view , motionEvent ->
                    val v = view as ColorView
                    val tag = v.tag.toString().toInt()
                    val x = motionEvent.rawX - (v.width / 2)
                    val y = motionEvent.rawY - (v.height / 2)
                    val color = finder.getColorAtPoint(PointF(x, y))
                    when(motionEvent.action) {
                        MotionEvent.ACTION_DOWN -> v.scaleUp()
                        MotionEvent.ACTION_MOVE -> {
                            v.x = x - v.layoutParams.width / 3
                            v.y = y - v.layoutParams.height / 3
                            v.setColor(color)
                        }
                        MotionEvent.ACTION_UP -> {
                            v.scaleDown()
                            val item = ColorUtil.getClosestMatch(color)
                            items[tag] = ColorMatch(item, color)
                            adapter.setItems(items)
                        }
                    }
                    true
                }
                cv.setOnTouchListener(touchListener)
            }

            finder.isLive = true
            finder.scene = CBRemodelingScene()
            finder.toolMode = CBToolMode.FindColor
            finder.setListener(this)
        }

        progressView.showProgress()

        initializeAdapter()
        bottomSheet.init()
    }

    override fun onDetach(view: View) {
        presenter.detachView()
        colorFinder?.stopRunning()
        container.removeView(colorFinder)
        colorFinder = null
        colorViews.forEach { container.removeView(it) }
    }

    @OnClick(R.id.button_capture) fun pressedCapture() {
        colorFinder?.captureToStill()
        buttonCapture.hide()
        buttonVideo.show()
    }

    @OnClick(R.id.button_video) fun pressedVideo() {
        colorFinder?.startCamera()
        buttonVideo.hide()
        buttonCapture.show()
    }


    override fun colorsFound(colors: MutableList<CBColorFinderView.CBColorResult>) {
        if (!this.isAttached) return
        progressView.hideProgress()
        items.clear()
        val positions = mutableListOf<Point>()
        for(i in colors.indices) {
            ColorUtil.getClosestMatch(colors[i].color)?.let {
                items.add(ColorMatch(it, colors[i].color))
                positions.add(colors[i].position)
            }
        }
        adapter.setItems(items)
        for(i in items.indices) {
            colorViews[i].setColor(items[i].color)
            colorViews[i].visibility = View.VISIBLE
            colorViews[i].animate()
                    .translationX(positions[i].x.toFloat())
                    .translationY(positions[i].y.toFloat())
                    .start()
        }
        for(i in items.size..colors.lastIndex) {
            if(i < 0) return
            //hide views that have no color
            colorViews[i].visibility = View.GONE
        }
    }

    private fun initializeAdapter() {
        adapter = FoundColorsAdapter(mutableListOf(), { pos ->
            items[pos].item?.let { item ->
                bottomSheet.updateSheet(item)
            }
        })

        rvColors.layoutManager = LockedLLM(context, LinearLayoutManager.HORIZONTAL, false)
        rvColors.adapter = adapter
    }
}