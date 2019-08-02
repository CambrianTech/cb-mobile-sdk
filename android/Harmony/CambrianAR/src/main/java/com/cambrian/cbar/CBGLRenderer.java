/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.cambrian.cbar;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.SensorManager;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CaptureRequest;
import android.media.Image;
import android.media.ImageReader;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Size;
import android.view.Surface;

import com.cambrian.cbar.sensor.orientationProvider.ImprovedOrientationSensor2Provider;
import com.cambrian.cbar.sensor.orientationProvider.OrientationProvider;
import com.cambrian.cbar.sensor.representation.Quaternion;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import timber.log.Timber;

import static com.cambrian.cbar.GlUtil.checkGlError;

/**
 * Provides drawing instructions for a GLSurfaceView object. This class
 * must override the OpenGL ES drawing lifecycle methods:
 * <ul>
 * <li>{@link android.opengl.GLSurfaceView.Renderer#onSurfaceCreated}</li>
 * <li>{@link android.opengl.GLSurfaceView.Renderer#onDrawFrame}</li>
 * <li>{@link android.opengl.GLSurfaceView.Renderer#onSurfaceChanged}</li>
 * </ul>
 */
public class CBGLRenderer extends Thread implements
        ICBJavaObject,
        CameraOps.ErrorDisplayer,
        CameraOps.CameraReadyListener {

    private static final float MS_PER_FRAME = 1000f / 60f;

    static {
        System.loadLibrary("CambrianAR");
    }

    Handler mDisplayHandler;
    boolean isLive = true;
    private SurfaceTexture previewTexture;
    private Surface previewSurface;
    private Size previewSize;
    private int textureID = 0;
    private CBAugmentedView m_sv;
    private long coreRenderer;
    private boolean isRunning = true;
    private boolean sizeChanged = false;
    private float hFOV = 1.1f;
    private int m_frameWidth, m_frameHeight;
    private int m_frameRotation;
    private int m_outputWidth, m_outputHeight;
    private ImageReader mImageReader;
    private CameraOps cameraOps;


    private OrientationProvider orientationProvider;
    private Quaternion orientation = new Quaternion();
    private CBFrameListener frameListener;
    private final ImageReader.OnImageAvailableListener mOnImageAvailableListener
            = new ImageReader.OnImageAvailableListener() {

        @Override
        public synchronized void onImageAvailable(final ImageReader reader) {
            try {
                orientationProvider.getQuaternion(orientation);
                if(frameListener != null) {
                    frameListener.onFrameAdded(isLive);
                }

                previewTexture.updateTexImage();

                Image image = reader.acquireLatestImage();
                if (image == null) return;

                ByteBuffer buffer0 = image.getPlanes()[0].getBuffer();
                ByteBuffer buffer1 = image.getPlanes()[1].getBuffer();
                ByteBuffer buffer2 = image.getPlanes()[2].getBuffer();

                int buffer0_size = buffer0.remaining();
                int buffer1_size = buffer1.remaining();
                int buffer2_size = buffer2.remaining();

                final byte[] buffer0_byte = new byte[buffer0_size];
                final byte[] buffer1_byte = new byte[buffer1_size];
                final byte[] buffer2_byte = new byte[buffer2_size];

                buffer0.get(buffer0_byte, 0, buffer0_size);
                buffer1.get(buffer1_byte, 0, buffer1_size);
                buffer2.get(buffer2_byte, 0, buffer2_size);

                image.close();

                int rot = cameraOps.getRotation(m_sv.getContext());
                addFrame(textureID, rot, buffer0_byte, buffer1_byte, buffer2_byte);

            } catch (RuntimeException ignore) {

            }
        }
    };

    private HandlerThread mDisplayThread;
    private Handler mUiHandler;

    public CBGLRenderer(CBAugmentedView view) {
        m_sv = view;
        coreRenderer = createNativeInstance(view);

        mUiHandler = new Handler(Looper.getMainLooper());
    }


    @Override
    public long getCoreObject() {
        return coreRenderer;
    }
    Quaternion getOrientation() {
        return orientation;
    }

    public void setFrameListener(CBFrameListener listener) {
        this.frameListener = listener;
    }

    //https://stackoverflow.com/questions/29003414/render-camera-preview-on-a-texture-with-target-gl-texture-2d
    //https://stackoverflow.com/questions/22456884/how-to-render-androids-yuv-nv21-camera-image-on-the-background-in-libgdx-with-o


    public void finalize() {
        destroyNativeInstance(this.coreRenderer);

        try {
            super.finalize();
        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }

    public void run() {

        startBackgroundThread();

        if(isLive) {
            findAndOpenCamera();
        } else {
            configureImageSurfaces();
        }

        while (isRunning) {
            long start = System.currentTimeMillis();

            if (sizeChanged && m_frameWidth > 0) {
                sizeChanged = false;
            }

            if (!isLive) {
                mDisplayHandler.post(new Runnable() {
                    public void run() {
                        /*
                         * isRunning needs to be checked within the thread to avoid synchronization issues
                         * - if it is not checked within the thread there will be calls to stillRender() while
                         * the thread is attempting to join, causing a lock which freezes the entire application
                         * for 2-5 seconds
                         */
                        if(isRunning) {
                            stillRender();
                        }
                    }
                });
            }

            long duration = System.currentTimeMillis() - start;
            float remaining = MS_PER_FRAME - duration;
            if (remaining > 0) {
                try {
                    sleep((long) (2 * remaining / 3));
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        // Wait until camera is closed to ensure the next application can open it
        if (cameraOps != null) {
            cameraOps.closeCameraAndWait();
            cameraOps = null;
        }

        stopBackgroundThread();
    }

    /**
     * Starts a background thread and its {@link Handler}.
     */
    private void startBackgroundThread() {
        mDisplayThread = new HandlerThread("CameraBackground");
        mDisplayThread.setPriority(MAX_PRIORITY);
        mDisplayThread.start();
        mDisplayHandler = new Handler(mDisplayThread.getLooper());
    }

    private void stopBackgroundThread() {
        mDisplayThread.quitSafely();
        try {
            mDisplayThread.join();
            mDisplayThread = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    void prepareViewport(int outputWidth, int outputHeight) {
        m_outputWidth = outputWidth;
        m_outputHeight = outputHeight;
        sizeChanged = true;
        SensorManager manager = (SensorManager) m_sv.getContext().getSystemService(Context.SENSOR_SERVICE);
        orientationProvider = new ImprovedOrientationSensor2Provider(manager);
        orientationProvider.start();

    }

    void requestStop() {
        isRunning = false;
        orientationProvider.stop();
        try {
            join();
        } catch (InterruptedException e) {
            Timber.e(e, "Failed to stop GL thread");
        }
    }

    /**
     * Attempt to initialize the camera.
     */
    private void initializeCamera() {

        CameraInfo cameraInfo = new CameraInfo(m_sv.getContext());
        if (cameraInfo.cameraMeetsCriteria()) {
            cameraOps = new CameraOps(cameraInfo,
                                      this, /*errorDisplayer*/
                                      this, /*readyListener*/
                                      mUiHandler); /*readyHandler*/

        } else {
            Timber.e("Couldn't initialize the camera");
        }
    }


    private void findAndOpenCamera() {
        initializeCamera();

        if (cameraOps != null) {
            if(!cameraOps.checkPermissions(m_sv.getContext())) return;
            cameraOps.setupCamera();

            configureCameraSurfaces();
        }
    }


    void startCamera() {
        isRunning = true;
        isLive = true;
        findAndOpenCamera();
    }

    void stopCamera() {
        isLive = false;
        if(cameraOps != null) {
            cameraOps.closeCameraAndWait();
        }
    }


    /**
     * Configure the surfaceview and RS processing.
     */
    private void configureImageSurfaces() {
        CBAugmentedScene scene = m_sv.getScene();
        Size size = scene.getFrameSize();
        m_frameWidth = size.getWidth();
        m_frameHeight = size.getHeight();
        m_frameRotation = 0; //always pass 0 as the rotation for still images to get correct scaling

        previewSize = new Size(m_frameWidth, m_frameHeight);

        mDisplayHandler.post(new Runnable() {
            public void run() {
                _prepareViewport(m_sv.getHolder().getSurface(),
                        m_frameWidth, m_frameHeight,
                        m_outputWidth, m_outputHeight,
                        hFOV, m_frameRotation);
            }
        });
    }


    /**
     * Configure the surfaceview and RS processing.
     */
    private void configureCameraSurfaces() {
        // Find a good size for output - largest 16:9 aspect ratio that's less than 720p
        Size outputSize = cameraOps.getOutputSize();

        m_frameWidth = outputSize.getWidth();
        m_frameHeight = outputSize.getHeight();
        m_frameRotation = cameraOps.getRotation(m_sv.getContext());

        previewSize = new Size(m_frameWidth, m_frameHeight);

        mDisplayHandler.post(new Runnable() {
            public void run() {
                _prepareViewport(m_sv.getHolder().getSurface(),
                        m_frameWidth, m_frameHeight,
                        m_outputWidth, m_outputHeight,
                        cameraOps.getHFOV(), m_frameRotation);

                setupCameraTexture();
            }
        });

        mImageReader = ImageReader.newInstance(m_frameWidth, m_frameHeight, ImageFormat.YUV_420_888, 2);
        mImageReader.setOnImageAvailableListener(mOnImageAvailableListener, mDisplayHandler);
    }

    /**
     * Remember that Android's camera api returns camera texture not as {@link GLES20#GL_TEXTURE_2D}
     * but rather as {@link GLES11Ext#GL_TEXTURE_EXTERNAL_OES}, which we bind here
     */
    private void setupCameraTexture() {

        int[] texturesIds = new int[1];

        GLES20.glGenTextures(1, texturesIds, 0);
        textureID = texturesIds[0];

        //set texture[0] to camera texture
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureID);
        checkGlError("Texture bind");

        previewTexture = new SurfaceTexture(textureID);
        previewSurface = new Surface(previewTexture);

        double scale = 2.0f;
        previewTexture.setDefaultBufferSize((int)(scale * previewSize.getWidth()),
                (int)(scale * previewSize.getHeight()));

        List<Surface> cameraOutputSurfaces = new ArrayList<>();
        cameraOutputSurfaces.add(mImageReader.getSurface());
        cameraOutputSurfaces.add(new Surface(previewTexture));
        if(cameraOps != null) {
            cameraOps.setSurfaces(cameraOutputSurfaces);
        } else {
            Timber.e(new RuntimeException(), "Could not set surfaces, cameraOps is null");
        }

    }

    @Override
    public void onCameraReady(CaptureRequest.Builder previewBuilder) {
        previewBuilder.addTarget(mImageReader.getSurface());
        previewBuilder.addTarget(previewSurface);
        CaptureRequest previewRequest = previewBuilder.build();

        cameraOps.setRepeatingRequest(previewRequest, null, mUiHandler);
    }


    @Override public void showErrorDialog(String errorMessage) { Timber.e("error from camera ops: " + errorMessage); }
    @Override public String getErrorString(CameraAccessException e) {
        return null;
    }

    private native long createNativeInstance(CBAugmentedView view);
    private native void destroyNativeInstance(long p_native_ptr);

    private native void stillRender();
    private native void addFrame(int textureID, int rotation, byte[] dataY, byte[] dataU, byte[] dataV);

    private native void destroyViewport();
    private native void _prepareViewport(Surface surface,
                                         int frameWidth, int frameHeight,
                                         int outputWidth, int outputHeight,
                                         float deviceFOV, int frameRotation);
}
