package com.cambrian.cbar.remodeling;

import android.content.Context;
import android.graphics.Point;
import android.graphics.PointF;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.cambrian.cbar.CBAugmentedView;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by joelteply on 8/1/17.
 */

public class CBColorFinderView extends CBAugmentedView {

    CBColorFinderViewListener listener;

    public class CBColorResult {
        public int color;
        public Point position;
    }

    public interface CBColorFinderViewListener {
        void colorsFound(@NonNull List<CBColorResult> colors);
    }

    public CBColorFinderView(Context context) {
        super(context);
    }
    public CBColorFinderView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void colorsFound(int[] colors, float[] x, float[] y) {
        final List<CBColorResult> results = new ArrayList<>();
        for(int i = 0; i < colors.length; i++) {
            CBColorResult color = new CBColorResult();
            color.color = colors[i];
            color.position = denormalizePosition(x[i], y[i]);
            results.add(color);
            //Timber.d("color is " + color.color + ", position is " + color.position.x + ", " + color.position.y);
        }

        if (results.size() <= 0) return;

        if(listener != null) {
            Handler mainThreadHandler = new Handler(getContext().getMainLooper());
            mainThreadHandler.post(new Runnable() {
                @Override
                public void run() {
                    listener.colorsFound(results);
                }
            });
        }
    }

    private Point denormalizePosition(float x, float y) {
        int posx = (int) (x * this.getMeasuredWidth());
        int posy = (int) (y * this.getMeasuredHeight());
        return new Point(posx, posy);
    }

    public int getColorAtPoint(PointF point) {
        point = getTranslatedCoords(this, point.x, point.y, true);
        return _getColorAtPoint(point.x, point.y);
    }

    private native int _getColorAtPoint(float x, float y);
    public void setListener(CBColorFinderViewListener listener) {
        this.listener = listener;
    }

    @Override
    protected native long createNativeInstance();
}
