package com.cambriantech.harmony.wheel;

import android.support.v7.widget.RecyclerView;
import android.util.SparseArray;
import android.view.View;
import android.view.ViewGroup;

import timber.log.Timber;

/**
 * Created by Joseph Sullivan on 7/11/16.
 */

public class WheelLayoutManager extends RecyclerView.LayoutManager {

    /* Fill Direction Constants */
    private static final int DIRECTION_NONE = -1;
    private static final int DIRECTION_START = 0;
    private static final int DIRECTION_END = 1;


    /* First (top-left) getPosition visible at any point */
    private int mFirstVisiblePosition;
    /* Consistent size applied to all child views */
    private int mDecoratedChildWidth;
    private int mDecoratedChildHeight;
    private int mDecoratedChildSweep = 24;

    boolean firstViewVisible;
    boolean lastViewVisible;

    private int mVisibleItemCount;

    @Override
    public void onLayoutChildren(RecyclerView.Recycler recycler, RecyclerView.State state) {
        if (getItemCount() == 0) {
            detachAndScrapAttachedViews(recycler);              //We have nothing to show for an empty data set but clear any existing views
            return;
        }
        if (getChildCount() == 0 && state.isPreLayout()) {
            return;
        }

        if (getChildCount() == 0) { //First or empty layout
            //Scrap measure one child
            View scrap = recycler.getViewForPosition(0);
            addView(scrap);
            measureChildWithMargins(scrap, 0, 0);

            /*
             * We make some assumptions in this code based on every child
             * view being the same size (i.e. a uniform grid). This allows
             * us to compute the following values up front because they
             * won't change.
             */
            mDecoratedChildWidth = getDecoratedMeasuredWidth(scrap);
            mDecoratedChildHeight = getDecoratedMeasuredHeight(scrap);

            detachAndScrapView(scrap, recycler);
        }

        updateWindow();             //Always update the visible row/column counts

        int childLeft;
        int childTop;
        float childRot;
        if (getChildCount() == 0) {         //First or empty layout
            mFirstVisiblePosition = 0;          //Reset the visible and scroll positions
            childLeft = 0;
            childTop = 0;
            childRot = -mDecoratedChildSweep * 3;
        } else if (!state.isPreLayout()
                && getVisibleChildCount() >= state.getItemCount()) {
            mFirstVisiblePosition = 0;              //Data set is too small to scroll fully, just reset getPosition
            childLeft = 0;
            childTop = 0;
            childRot = -mDecoratedChildSweep * 3;
        } else { //Adapter data set changes
            /*
             * Keep the existing initial getPosition, and save off
             * the current scrolled offset.
             */
            final View topChild = getChildAt(0);
            childLeft = 0;
            childTop = 0;
            childRot = -mDecoratedChildSweep * 3;

            /*
             * Adjust the visible getPosition if out of bounds in the
             * new layout. This occurs when the new item count in an adapter
             * is much smaller than it was before, and you are scrolled to
             * a location where no items would exist.
             */
            int maxFirstItem = getItemCount() - (mVisibleItemCount-1);
            boolean isOutOfBounds = mFirstVisiblePosition > maxFirstItem;

            if(isOutOfBounds) {
                int firstItem = mFirstVisiblePosition;
                mFirstVisiblePosition = maxFirstItem + firstItem;
                childLeft = getHorizontalSpace() - (mDecoratedChildWidth + mVisibleItemCount);
                if(mFirstVisiblePosition == 0) {
                    childLeft = Math.min(childLeft, getPaddingLeft());
                }
            }
        }
        detachAndScrapAttachedViews(recycler);                              //Clear all attached views into the recycle bin
        fillList(DIRECTION_NONE, childRot, childLeft, childTop, recycler, state);         //Fill the grid for the initial layout of views
    }


    public int getVisibleChildCount() {
        return mVisibleItemCount;
    }

    public int getFirstVisiblePosition() {
        return mFirstVisiblePosition;
    }

    public int getLastVisiblePosition() {
        if(getItemCount() > mVisibleItemCount) {
            return mFirstVisiblePosition + mVisibleItemCount - 1;
        } else {
            return getItemCount()-1;
        }
    }

    private void updateWindow() {
        mVisibleItemCount = (int) Math.floor(180 / mDecoratedChildSweep);
        //if(getHorizontalSpace() % mDecoratedChildWidth > 0) {
        //    mVisibleItemCount++;
        //}
    }



    @Override
    public void onAdapterChanged(RecyclerView.Adapter oldAdapter, RecyclerView.Adapter newAdapter) {
        //Completely scrap the existing layout
        removeAllViews();
    }

    private void fillList(int direction, RecyclerView.Recycler recycler, RecyclerView.State state) {
        fillList(direction, 0, 0, 0, recycler, state);
    }

    private void fillList(int direction, float emptyRot, int emptyLeft, int emptyTop,
                          RecyclerView.Recycler recycler,
                          RecyclerView.State state) {
        if (mFirstVisiblePosition < 0) mFirstVisiblePosition = 0;
        if (mFirstVisiblePosition >= getItemCount()) mFirstVisiblePosition = (getItemCount() - 1);

        /*
         * First, we will detach all existing views from the layout.
         * detachView() is a lightweight operation that we can use to
         * quickly reorder views without a full add/remove.
         */
        SparseArray<View> viewCache = new SparseArray<View>(getChildCount());
        int startLeftOffset = emptyLeft;
        int startTopOffset = emptyTop;
        float startRotOffset = emptyRot;
        if (getChildCount() != 0) {
            final View topView = getChildAt(0);
            startLeftOffset = getDecoratedLeft(topView);
            startRotOffset = topView.getRotation();
            switch (direction) {
                case DIRECTION_START:
                    startLeftOffset -= mDecoratedChildWidth;
                    startRotOffset -= mDecoratedChildSweep;
                    break;
                case DIRECTION_END:
                    startLeftOffset += mDecoratedChildWidth;
                    startRotOffset += mDecoratedChildSweep;
                    break;
            }

            //Cache all views by their existing getPosition, before updating counts
            for (int i=0; i < getChildCount(); i++) {
                int position = positionOfIndex(i);
                final View child = getChildAt(i);
                viewCache.put(position, child);
            }

            //Temporarily detach all views.
            // Views we still need will be added back at the proper index.
            for (int i=0; i < viewCache.size(); i++) {
                detachView(viewCache.valueAt(i));
            }
        }

        /*
         * Next, we advance the visible getPosition based on the fill direction.
         * DIRECTION_NONE doesn't advance the getPosition in any direction.
         */
        switch (direction) {
            case DIRECTION_START:
                mFirstVisiblePosition--;
                break;
            case DIRECTION_END:
                mFirstVisiblePosition++;
                break;
        }

        /*
         * Next, we supply the list of items that are deemed visible.
         * If these items were previously there, they will simply be
         * re-attached. New views that must be created are obtained
         * from the Recycler and added.
         */
        int leftOffset = startLeftOffset;
        float rotOffset = startRotOffset;
        int topOffset = startTopOffset;

        for (int i = 0; i < getVisibleChildCount(); i++) {
            int nextPosition = positionOfIndex(i);

            if (nextPosition < 0 || nextPosition >= state.getItemCount()) {
                //Item space beyond the data set, don't attempt to add a view
                continue;
            }

            //Layout this getPosition
            View view = viewCache.get(nextPosition);
            if (view == null) {
                /*
                 * The Recycler will give us either a newly constructed view,
                 * or a recycled view it has on-hand. In either case, the
                 * view will already be fully bound to the data by the
                 * adapter for us.
                 */
                view = recycler.getViewForPosition(nextPosition);
                addView(view);

                Arc arc = (Arc) view;

                /*
                 * It is prudent to measure/layout each new view we
                 * receive from the Recycler. We don't have to do
                 * this for views we are just re-arranging.
                 */
                measureChildWithMargins(arc, 0, 0);

                layoutDecorated(arc, 0, 0,
                        mDecoratedChildWidth,
                        mDecoratedChildHeight);

                arc.setRotation(rotOffset);
                arc.setTranslationX((getWidth()/2) - (arc.getMeasuredWidth()/2));

            } else {
                //Re-attach the cached view at its new index
                attachView(view);
                viewCache.remove(nextPosition);
            }

            if(i % mVisibleItemCount == (mVisibleItemCount -1)) {
                rotOffset = startRotOffset;
            } else {
                rotOffset += mDecoratedChildSweep;
            }
        }

        /*
         * Finally, we ask the Recycler to scrap and store any views
         * that we did not re-attach. These are views that are not currently
         * necessary because they are no longer visible.
         */
        for (int i=0; i < viewCache.size(); i++) {
            final View removingView = viewCache.valueAt(i);
            recycler.recycleView(removingView);
        }
    }

    /*
     * You must override this method if you would like to support external calls
     * to shift the view to a given adapter getPosition. In our implementation, this
     * is the same as doing a fresh layout with the given getPosition as the top-left
     * (or first visible), so we simply set that value and trigger onLayoutChildren()
     */
    @Override
    public void scrollToPosition(int position) {
        if (position >= getItemCount()) {
            Timber.e("Cannot scroll to "+position+", item count is "+getItemCount());
            return;
        }

        mFirstVisiblePosition = position;
        //removeAllViews();
        requestLayout();
    }


    @Override
    public boolean canScrollHorizontally() {
        return true;
    }
    @Override
    public boolean canScrollVertically() {
        return false;
    }


    /*
     * This method describes how far RecyclerView thinks the contents should scroll horizontally.
     * You are responsible for verifying edge boundaries, and determining if this scroll
     * event somehow requires that new views be added or old views get recycled.
     */
    @Override
    public int scrollHorizontallyBy(int dx, RecyclerView.Recycler recycler, RecyclerView.State state) {
        if (getChildCount() == 0) {
            return 0;
        }

        int delta;
        firstViewVisible = (mFirstVisiblePosition <= 0);
        lastViewVisible = (mFirstVisiblePosition + mVisibleItemCount) >= getItemCount();

        delta = -dx;

        rotateChildren(delta);

        float firstRot = getChildAt(0).getRotation();

        if (dx > 0) {
            if (firstRot < 270 && firstRot > 180 && !lastViewVisible) {
                fillList(DIRECTION_END, recycler, state);
            } else if (!lastViewVisible) {
                fillList(DIRECTION_NONE, recycler, state);
            }
        } else {
            if (firstRot > 290 && !firstViewVisible) {
                fillList(DIRECTION_START, recycler, state);
            } else if (!firstViewVisible) {
                fillList(DIRECTION_NONE, recycler, state);
            }
        }

        /*
         * Return value determines if a boundary has been reached
         * (for edge effects and flings). If returned value does not
         * match original delta (passed in), RecyclerView will draw
         * an edge effect.
         */
        return -delta;
    }


    public void rotateChildren(float dx) {

        final int childCount = getChildCount();
        float radius = getChildAt(0).getPivotY();

        float atan = (float) Math.atan2(dx, radius) * 50f;


        float firstRot = getChildAt(0).getRotation();
        float lastRot = getChildAt(childCount-1).getRotation();

        if(firstViewVisible && atan > 0) {
            if((firstRot + atan) > 360) {
                //stop overscroll
                atan = 360-firstRot;
            }
        }

        if(lastViewVisible && atan < 0) {
            if((lastRot + atan) < 0) {
                //stop overscroll
                atan = -lastRot;
            }
        }

        for (int i = 0; i < childCount; i++) {
            Arc arc = (Arc) getChildAt(i);

            float newRot = arc.getRotation() + (atan);

            while(newRot > 360) newRot -= 360;
            while(newRot < 0)   newRot += 360;

            arc.setRotation(newRot);
        }
    }

    /*
     * This is a helper method used by RecyclerView to determine
     * if a specific child view can be returned.
     */
    @Override
    public View findViewByPosition(int position) {
        for (int i=0; i < getChildCount(); i++) {
            if (positionOfIndex(i) == position) {
                return getChildAt(i);
            }
        }
        return null;
    }

    public int positionOfIndex(int childIndex) {
        return mFirstVisiblePosition + childIndex;
    }

    private int getHorizontalSpace() {
        return getWidth() - getPaddingRight() - getPaddingLeft();
    }

    /*
     * Even without extending LayoutParams, we must override this method
     * to provide the default layout parameters that each child view
     * will receive when added.
     */
    @Override
    public RecyclerView.LayoutParams generateDefaultLayoutParams() {
        return new RecyclerView.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
    }

}
