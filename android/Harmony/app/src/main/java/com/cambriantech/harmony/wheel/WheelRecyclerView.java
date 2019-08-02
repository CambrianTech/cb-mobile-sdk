package com.cambriantech.harmony.wheel;

import android.content.Context;
import android.support.annotation.Nullable;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;

/**
 * Created by Joseph Sullivan on 7/15/16.
 */

public class WheelRecyclerView extends RecyclerView {

    public WheelRecyclerView(Context context) {
        super(context);
    }

    public WheelRecyclerView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs, 0);
    }

    public WheelRecyclerView(Context context, @Nullable AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public boolean fling(int velocityX, int velocityY) {
        velocityX *= 1.0f; // (between 0 for no fling, and 1 for normal fling, or more for faster fling).

        return super.fling(velocityX, velocityY);
    }
}
