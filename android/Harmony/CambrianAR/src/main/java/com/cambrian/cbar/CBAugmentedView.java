package com.cambrian.cbar;

import android.content.Context;
import android.graphics.Matrix;
import android.graphics.PointF;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.cambrian.cbar.remodeling.CBRemodelingScene;
import com.cambrian.cbar.remodeling.CBRemodelingTypes;
import com.cambrian.cbar.sensor.representation.Quaternion;
import com.cambrian.cbar.types.CBToolMode;

import java.io.File;
import java.util.ArrayList;

import timber.log.Timber;


public abstract class CBAugmentedView
        extends SurfaceView
        implements ICBJavaObject,
        SurfaceHolder.Callback,
        CBFrameListener,
        RotationGestureDetector.OnRotationGestureListener {

    static {
        System.loadLibrary("CambrianAR");
    }

    private CBGLRenderer renderer;
    private CBAugmentedViewListener m_listener;
    private RotationGestureDetector rotationDetector;

    private long m_coreObject;
    private CBRemodelingScene m_scene;


    public interface CBAugmentedViewListener {
        void historyChanged(String assetID, CBRemodelingTypes.CBUndoChange change, boolean forward);
        void saveComplete();
    }


    public CBAugmentedView(Context context) {
        super(context);
        init();
    }

    public CBAugmentedView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }


    private void historyChanged(String assetID, int change, boolean forward) {
        m_listener.historyChanged(assetID, CBRemodelingTypes.CBUndoChange.fromOrdinal(change), forward);
    }

    public void finalize() {
        destroyNativeInstance( this.m_coreObject );

        try {
            super.finalize();
        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }

    @Override
    public long getCoreObject() {
        return m_coreObject;
    }

    public void setListener(CBAugmentedViewListener listener) {
        m_listener = listener;
    }

    private void init() {
        m_coreObject = createNativeInstance();
        getHolder().addCallback(this);

        if (isInEditMode()) {
            setBackgroundColor(0x009ef3);
            return;
        }

        if (getContext() instanceof CBAugmentedViewListener) {
            setListener((CBAugmentedViewListener) getContext());
        }

        rotationDetector = new RotationGestureDetector(this);
    }


    final ArrayList<PointF> history = new ArrayList<>();
    @Override
    public boolean onTouchEvent(MotionEvent event) {

        rotationDetector.onTouchEvent(event);
        if(rotationDetector.getDidRotate()) {
            history.clear();
            return true;
        }

        PointF point = getPointerCoords(event);
        history.add(point);

        if (event.getAction() == MotionEvent.ACTION_UP) {
            long duration = event.getEventTime() - event.getDownTime();
            if(duration < 400) {
                //calculate distance traveled during touch event
                float distance = 0;
                for(int i = 1; i < history.size(); i++) {
                    float xDist = Math.abs(history.get(i).x - history.get(i-1).x);
                    float yDist = Math.abs(history.get(i).y - history.get(i-1).y);
                    distance += (xDist + yDist);
                }
                history.clear();
                //if distance is negligable, perform a tap event instead of "moved", which causes brush action
                if(distance < 0.03f) {
                    touchedAt(point.x, point.y, 0); //4 = TouchStepTapped
                    return true;
                }
            }
        }
        if(getToolMode() == CBToolMode.Paintbrush || getToolMode() == CBToolMode.Eraser) {
            touchedAt(point.x, point.y, event.getAction());
        }

        return true;
    }

    @Override
    public void onRotation(RotationGestureDetector rotationDetector) {
        float delta = rotationDetector.getAngleDelta();
        float radians = delta * (float)(Math.PI / 180);
        rotatedBy(radians);
        //Timber.d("radians is: " + radians);
        //Timber.d("rotation is: " + angle);
        //Timber.d("angle delta is: " + delta);
    }

    protected final PointF getPointerCoords(MotionEvent e) {
        final int index = e.getActionIndex();
        return getTranslatedCoords(this, e.getX(index), e.getY(index), true);
    }

    protected final PointF getTranslatedCoords(View view, float x, float y, boolean doTranslation) {
        //Log.d("Touch", "getTranslatedCoords " + x + ", " + y);
        final float[] coords = new float[] { x - view.getPaddingLeft(), y - view.getPaddingTop() };

        Matrix matrix = new Matrix();
        view.getMatrix().invert(matrix);
        if (doTranslation) {
            matrix.postTranslate(view.getScrollX(), view.getScrollY());
        }
        matrix.mapPoints(coords);
        final PointF point = new PointF(coords[0], coords[1]);

        int drawableW = getMeasuredWidth();
        int drawableH = getMeasuredHeight();

        if (drawableW > 0 && drawableH > 0) {
            point.x = point.x / drawableW;
            point.y = point.y / drawableH;
        } else {
            point.x = point.y = 0.f;
        }

        return point;
    }


    @Override
    public void onFrameAdded(boolean isLive) {
        if(isLive) {
            setWorldTransform(renderer.getOrientation());
        }
    }

    public void setWorldTransform(Quaternion quat) {
        float[] transform = new float[4];
        transform[0] = -quat.getW();
        transform[1] = quat.getX();
        transform[2] = quat.getY();
        transform[3] = quat.getZ();

        setWorldTransform(transform);
    }


    /**
     *  Current scene
     */
    public CBRemodelingScene getScene() {
        return m_scene;
    }
    public void setScene(CBRemodelingScene scene) {
        setCoreScene(scene.getCoreObject());
        m_scene = scene;
        if(renderer != null) {
            renderer.mDisplayHandler.post(new Runnable() {
                @Override
                public void run() {
                    renderer.start();
                }
            });
        }
    }

    /**
     * current tool mode
     */
    public CBToolMode getToolMode() {
        return CBToolMode.fromOrdinal(_getToolMode());
    }
    public void setToolMode(CBToolMode toolMode) {
        _setToolMode(toolMode.ordinal());
    }

    /**
     * Whether the view is live or still mode
     */
    public boolean getIsLive() {
        return renderer != null && renderer.isLive;
    }

    public void setIsLive(boolean isLive) {
        if(renderer != null) {
            renderer.isLive = isLive;
        }
    }

    /**
     * Capture the current state in live mode into still
     */
    public void captureToStill() {
        _captureToStill();

        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                renderer.stopCamera();
            }
        };
        new Handler().postDelayed(runnable, 150);
    }

    /**
     * Start/stop augmented reality
     */
    public void startRunning() {
        _startRunning();
    }
    public void stopRunning() {
        _stopRunning();
    }


    public void startCamera() {
        renderer.startCamera();
        _pause(false);
    }


    /*************************
     *
     *  SURFACE METHODS
     *
     *************************/
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        renderer.prepareViewport(width, height);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        try {
            renderer = new CBGLRenderer(this);
            renderer.setFrameListener(this);
            if(m_scene != null) {
                renderer.start();
            }
        } catch (RuntimeException e){
            Timber.d("caught camera exception at surface");
            Timber.e(e);
        }

    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        if (renderer != null) {
            Timber.d("DESTROYING SURFACE");
            renderer.requestStop();
            stopRunning();
        }
    }


    /************
     *  SAVING
     ***********/
    public void saveToDirectory(final File path, final boolean doCompression) {
        if(renderer.mDisplayHandler != null) {
            Timber.i("saving image");
            renderer.mDisplayHandler.post(new Runnable() {
                @Override
                public void run() {
                    if(m_scene != null) {
                        m_scene.saveToDirectory(path, doCompression);
                        saveComplete();
                    }
                }
            });
        } else {
            Timber.e("can't save, display thread is dead");
        }
    }

    private void saveComplete() {
        Handler mainThreadHandler = new Handler(getContext().getMainLooper());
        mainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                m_listener.saveComplete();
            }
        });
    }


    protected abstract long createNativeInstance();
    private native void destroyNativeInstance( long p_native_ptr );
    private native void _pause(boolean pause);

    public native int getUndoSize();
    public native int getMaxUndoSize();
    public native void setMaxUndoSize(int size);
    public native int getRedoSize();
    public native int getMaxRedoSize();
    public native void setMaxRedoSize(int size);
    public native void undo();
    public native void redo();

    public native void clearAll();
    public native void _captureToStill();
    private native void _startRunning();
    private native void _stopRunning();
    private native void setCoreScene(long coreScenePtr);
    private native int _getToolMode();
    private native void _setToolMode(int toolMode);
    private native void touchedAt(float normalizedX, float normalizedY, int step);
    private native void rotatedBy(float amount);
    //private native void rotateGesture(float amount, float x, float y, int step);
    private native void setWorldTransform(float[] quaternion);
    public native float[] getWorldTransform();
}



