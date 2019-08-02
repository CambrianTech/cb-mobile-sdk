package com.cambrian.cbar;

import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.util.Range;
import android.util.SizeF;

import java.util.Objects;

import timber.log.Timber;

import static android.content.Context.CAMERA_SERVICE;

/**
 * Created by joseph on 12/28/2017.
 */

public class CameraInfo {

    private CameraManager cameraManager;
    private CameraCharacteristics backCameraCharacteristics;
    private String backCameraId;

    public CameraInfo(Context context) {
        cameraManager = (CameraManager) context.getSystemService(CAMERA_SERVICE);

        try {
            if(cameraManager == null) return;
            String[] cameraIds = cameraManager.getCameraIdList();
            if(cameraIds.length == 0) {
                throw new RuntimeException("Cannot access any available cameras");
            }
            for (String id : cameraIds) {
                CameraCharacteristics info = cameraManager.getCameraCharacteristics(id);
                Integer facing = info.get(CameraCharacteristics.LENS_FACING);
                if(Objects.equals(facing, CameraCharacteristics.LENS_FACING_BACK)) {
                    Timber.d("has back camera");
                    backCameraCharacteristics = info;
                    backCameraId = id;
                    break;
                }
            }
            if(backCameraId == null) {
                Timber.e("couldn't find back camera");
            }
        } catch (CameraAccessException e) {
            Timber.e(e);
        }
    }

    public boolean cameraMeetsCriteria() {
        Timber.v("has full level " + hasFullLevel());
        Timber.v("has manual control " + hasManualControl());
        Timber.v("has latency control " + hasLatencyControl());
        return (hasBackCamera()
                && (hasFullLevel()
                || (hasManualControl()
                && hasLatencyControl())));
    }

    CameraManager getCameraManager() {
        return cameraManager;
    }

    private boolean hasBackCamera() {
        return backCameraCharacteristics != null;
    }

    private boolean hasFullLevel() {
        Integer level = getBackCamera().get(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL);
        return Objects.equals(level, CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_FULL);
    }

    private boolean hasManualControl() {
        return hasCapability(getCapabilities(), CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES_MANUAL_SENSOR);
    }

    private boolean hasLatencyControl() {
        Integer syncLatency = getBackCamera().get(CameraCharacteristics.SYNC_MAX_LATENCY);
        return Objects.equals(syncLatency, CameraCharacteristics.SYNC_MAX_LATENCY_PER_FRAME_CONTROL);
    }

    CameraCharacteristics getBackCamera() {
        return backCameraCharacteristics;
    }

    String getBackCameraId() {
        return backCameraId;
    }

    @SuppressWarnings("ConstantConditions")
    private Integer getCameraLevel() {
        return backCameraCharacteristics.get(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL);
    }

    private int[] getCapabilities() {
        return getBackCamera().get(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES);
    }

    StreamConfigurationMap getConfigs() {
        return getBackCamera().get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
    }

    SizeF getSensorSize() {
        return getBackCamera().get(CameraCharacteristics.SENSOR_INFO_PHYSICAL_SIZE);
    }

    float[] getFocalLengths() {
        return getBackCamera().get(CameraCharacteristics.LENS_INFO_AVAILABLE_FOCAL_LENGTHS);
    }

    int getOrientation() {
        return getBackCamera().get(CameraCharacteristics.SENSOR_ORIENTATION);
    }

    Range<Integer>[] getFpsRanges() {
        return getBackCamera().get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
    }

    private boolean hasCapability(int[] capabilities, int capability) {
        for (int c : capabilities) {
            if (c == capability) return true;
        }
        return false;
    }
}
