/*
 * Copyright (C) 2014 The Android Open Source Project
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

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Range;
import android.util.Size;
import android.util.SizeF;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.WindowManager;

import java.util.List;

import timber.log.Timber;

import static java.lang.Math.atan;

/**
 * Simple interface for operating the camera, with major camera operations
 * all performed on a background handler thread.
 */
class CameraOps {

    private static final long CAMERA_CLOSE_TIMEOUT = 1000; // ms

    private final CameraInfo cameraInfo;
    private CameraDevice mCameraDevice;
    private CameraCaptureSession mCameraSession;
    private List<Surface> mSurfaces;

    private Range desiredFPSRange = null;

    private final ConditionVariable mCloseWaiter = new ConditionVariable();

    private HandlerThread mCameraThread;
    private Handler mCameraHandler;

    private final ErrorDisplayer errorDisplayer;
    private final CameraReadyListener mReadyListener;
    private final Handler mReadyHandler;
    private boolean isRunning = false;

    /**
     * Create a new camera ops thread.
     *
     * @param errorDisplayer listener for displaying errorDisplayer messages
     * @param readyListener  listener for notifying when camera is ready for requests
     * @param readyHandler   the handler for calling readyListener methods on
     */
    CameraOps(CameraInfo manager, ErrorDisplayer errorDisplayer,
              CameraReadyListener readyListener, Handler readyHandler) {
        mCameraThread = new HandlerThread("CameraOpsThread");
        mCameraThread.start();

        if (manager == null
            || errorDisplayer == null
            || readyListener == null
            || readyHandler == null) {
            throw new IllegalArgumentException("Need valid displayer, listener, handler");
        }

        cameraInfo = manager;
        this.errorDisplayer = errorDisplayer;
        mReadyListener = readyListener;
        mReadyHandler = readyHandler;
    }

    void setupCamera() {
        if(cameraInfo.cameraMeetsCriteria()) {
            openCamera(cameraInfo.getBackCameraId());

            //set 60fps if possible
            Range<Integer>[] fpsRanges = cameraInfo.getFpsRanges();

            assert fpsRanges != null;
            for (Range fpsRange : fpsRanges) {
                if (fpsRange.getUpper().equals(60)) {
                    //Set that range
                    desiredFPSRange = fpsRange;
                    break;
                }
            }
        } else {
            throw new RuntimeException("no cameras fit necessary parameters");
        }
    }


    boolean checkPermissions(Context context) {
        int permissionState = ActivityCompat.checkSelfPermission(context, Manifest.permission.CAMERA);

        // Check if the Camera permission is already available.
        if (permissionState != PackageManager.PERMISSION_GRANTED) {
            Timber.e("CAMERA permission has NOT been granted.");
            return false;
        } else {
            //Timber.i("CAMERA permission has already been granted.");
            return true;
        }
    }

    /**
     * Open the camera that is passed in
     * Displays a dialog if it cannot open a camera.
     */
    private void openCamera(final String cameraId) {
        mCameraHandler = new Handler(mCameraThread.getLooper());

        mCameraHandler.post(new Runnable() {
            @SuppressWarnings("MissingPermission")
            public void run() {
                if (mCameraDevice != null) {
                    throw new IllegalStateException("Camera already open");
                }
                try {
                    cameraInfo.getCameraManager().openCamera(cameraId, mCameraDeviceListener, mCameraHandler);
                    isRunning = true;
                } catch (CameraAccessException e) {
                    String errorMessage = errorDisplayer.getErrorString(e);
                    errorDisplayer.showErrorDialog(errorMessage);
                }
            }
        });
    }

    Size getOutputSize() {
        final int MAX_WIDTH = 1280;
        final float TARGET_ASPECT = 16.f / 9.f;
        final float ASPECT_TOLERANCE = 0.1f;

        StreamConfigurationMap configs = cameraInfo.getConfigs();
        if (configs == null) {
            throw new RuntimeException("Cannot get available picture/preview sizes.");
        }
        Size[] outputSizes = configs.getOutputSizes(SurfaceHolder.class);

        Size outputSize = outputSizes[0];
        float outputAspect = (float) outputSize.getWidth() / outputSize.getHeight();
        for (Size candidateSize : outputSizes) {
            if (candidateSize.getWidth() > MAX_WIDTH) continue;
            float candidateAspect = (float) candidateSize.getWidth() / candidateSize.getHeight();
            boolean goodCandidateAspect =
                    Math.abs(candidateAspect - TARGET_ASPECT) < ASPECT_TOLERANCE;
            boolean goodOutputAspect =
                    Math.abs(outputAspect - TARGET_ASPECT) < ASPECT_TOLERANCE;
            if ((goodCandidateAspect && !goodOutputAspect) ||
                candidateSize.getWidth() > outputSize.getWidth()) {
                outputSize = candidateSize;
                outputAspect = candidateAspect;
            }
        }
        //Timber.v("Resolution chosen: " + outputSize);
        return outputSize;
    }

    float getHFOV() {
        SizeF sensorSize = cameraInfo.getSensorSize();
        float[] focalLengths = cameraInfo.getFocalLengths();

        if (focalLengths != null && focalLengths.length > 0) {
            return (float) (2.0f * atan(sensorSize.getWidth() / (2.0f * focalLengths[0])));
        }

        return 1.1f;
    }

    /**
     * Close the camera and wait for the close callback to be called in the camera thread.
     * Times out after @{value CAMERA_CLOSE_TIMEOUT} ms.
     */
    void closeCameraAndWait() {
        if(isRunning) {
            Timber.d("Closing camera");
        } else {
            Timber.e("Camera already closed");
            return;
        }

        mCloseWaiter.close();
        mCameraHandler.post(mCloseCameraRunnable);
        boolean closed = mCloseWaiter.block(CAMERA_CLOSE_TIMEOUT);
        if (!closed) {
            Timber.e("Timeout closing camera");
        } else {
            isRunning = false;
            Timber.i("Successfully closed camera");
        }
    }

    private Runnable mCloseCameraRunnable = new Runnable() {
        public void run() {
            if (mCameraDevice != null) {
                mCameraDevice.close();
            }
            mCameraDevice = null;
            mCameraSession = null;
            mSurfaces = null;
        }
    };

    /**
     * Set the output Surfaces, and finish configuration if otherwise ready.
     */
    void setSurfaces(final List<Surface> surfaces) {
        mCameraHandler.post(new Runnable() {
            public void run() {
                mSurfaces = surfaces;
                startCameraSession();
            }
        });
    }


    private CaptureRequest.Builder createCaptureRequest(int template) throws CameraAccessException {
        CameraDevice device = mCameraDevice;
        if (device == null) {
            throw new IllegalStateException("Can't get requests when no camera is open");
        }
        return device.createCaptureRequest(template);
    }


    void setRepeatingRequest(final CaptureRequest request,
                                    final CameraCaptureSession.CaptureCallback listener,
                                    final Handler handler) {
        mCameraHandler.post(new Runnable() {
            public void run() {
                try {
                    mCameraSession.setRepeatingRequest(request, listener, handler);
                } catch (CameraAccessException e) {
                    String errorMessage = errorDisplayer.getErrorString(e);
                    errorDisplayer.showErrorDialog(errorMessage);
                }
            }
        });
    }


    private void startCameraSession() {
        // Wait until both the camera device is open and the SurfaceView is ready
        if (mCameraDevice == null || mSurfaces == null) return;

        try {
            mCameraDevice.createCaptureSession(
                    mSurfaces, mCameraSessionListener, mCameraHandler);
        } catch (CameraAccessException e) {
            String errorMessage = errorDisplayer.getErrorString(e);
            errorDisplayer.showErrorDialog(errorMessage);
            mCameraDevice.close();
            mCameraDevice = null;
        }
    }

    int getRotation(Context context) {
        int rotation = 270;
        int displayRotation = getDeviceRotation(context);

        int orientation = cameraInfo.getOrientation();
        //int facing = cameraInfo.get(CameraCharacteristics.LENS_FACING);


//        if (facing == CameraCharacteristics.LENS_FACING_FRONT) {
//            rotation = (orientation + displayRotation) % 360;
//            rotation = (360 - rotation) % 360;  // compensate the mirror
//            Timber.e("camera is facing front");
//        } else {  // back-facing
            rotation = (orientation - displayRotation + 360) % 360;
        //}

        return rotation;
    }

    private int getDeviceRotation(Context context) {
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        return 90 * (int) wm.getDefaultDisplay().getRotation();
    }


    /**
     * Main listener for camera session events
     * Invoked on mCameraThread
     */
    private CameraCaptureSession.StateCallback mCameraSessionListener =
            new CameraCaptureSession.StateCallback() {

                @Override
                public void onConfigured(@NonNull CameraCaptureSession session) {
                    mCameraSession = session;
                    mReadyHandler.post(new Runnable() {
                        public void run() {
                            // This can happen when the screen is turned off and turned back on.
                            if (mCameraDevice == null) return;
                            onCameraReady();
                        }
                    });

                }

                @Override
                public void onConfigureFailed(@NonNull CameraCaptureSession session) {
                    errorDisplayer.showErrorDialog("Unable to configure the capture session");
                    mCameraDevice.close();
                    mCameraDevice = null;
                }
            };

    private void onCameraReady() {
        CaptureRequest.Builder previewBuilder;

        try {
            previewBuilder = createCaptureRequest(CameraDevice.TEMPLATE_ZERO_SHUTTER_LAG);
            previewBuilder.set(CaptureRequest.CONTROL_MODE, CaptureRequest.CONTROL_MODE_USE_SCENE_MODE);
            previewBuilder.set(CaptureRequest.CONTROL_SCENE_MODE, CaptureRequest.CONTROL_SCENE_MODE_HIGH_SPEED_VIDEO);

            if (desiredFPSRange != null) {
                previewBuilder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, desiredFPSRange);
            }

            mReadyListener.onCameraReady(previewBuilder);

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Main listener for camera device events.
     * Invoked on mCameraThread
     */
    private CameraDevice.StateCallback mCameraDeviceListener = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            mCameraDevice = camera;
            startCameraSession();
        }

        @Override
        public void onClosed(@NonNull CameraDevice camera) {
            mCloseWaiter.open();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            errorDisplayer.showErrorDialog("The camera device has been disconnected.");
            camera.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            CameraOps.this.errorDisplayer.showErrorDialog("The camera encountered an errorDisplayer:" + error);
            camera.close();
            mCameraDevice = null;
        }

    };

    interface CameraReadyListener {
        void onCameraReady(CaptureRequest.Builder previewBuilder);
    }

    /**
     * Simple listener for displaying errorDisplayer messages
     */
    interface ErrorDisplayer {
        void showErrorDialog(String errorMessage);
        String getErrorString(CameraAccessException e);
    }
}
