package com.cambrian.cbar.remodeling;

import android.content.Context;
import android.util.AttributeSet;

import com.cambrian.cbar.CBAugmentedView;


/**
 * Created by joelteply on 8/1/17.
 */

public class CBRemodelingView extends CBAugmentedView {

    public interface CBRemodelingViewListener extends CBAugmentedViewListener {

    }

    /**
     * Creates an Augmented View for incorporation into a view
     * @param context
     */
    public CBRemodelingView(Context context) {
        super(context);
        java.util.HashMap<String, String> map = new java.util.HashMap<String, String>();
        map.put("", "");

        //this.setScene(new CBRemodelingScene());
    }

    /**
     * Creates an Augmented View for incorporation into a view
     * @param context
     * @param attrs
     */
    public CBRemodelingView(Context context, AttributeSet attrs) {
        super(context, attrs);
        //this.setScene(new CBRemodelingScene());
    }

    @Override
    protected native long createNativeInstance();

    public void setListener(CBRemodelingViewListener listener) {
        super.setListener(listener);
    }


}
