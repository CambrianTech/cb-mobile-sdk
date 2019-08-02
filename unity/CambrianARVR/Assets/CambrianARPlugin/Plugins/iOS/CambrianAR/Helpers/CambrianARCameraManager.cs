//#undef UNITY_EDITOR

using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AOT;
using Cambrian.iOS;
using UnityEngine;

public class CambrianARCameraManager : MonoBehaviour {
    
	private static CambrianARCameraManager m_instance;
    private Camera m_camera;
	private Material savedClearMaterial;

	public delegate void ARDisplayTransformUpdated(DisplayParams dp);
	public static event ARDisplayTransformUpdated ARDisplayTransformUpdatedEvent;

	private bool m_isRunning = false;
	private static CBMode m_mode;
	private static bool m_hasSentMode;

	private String m_screenshot_temp_name;
	private String m_screenshot_final_path;
	private bool m_hasScreenshot;
	private bool m_hasFirstUpdate;
	
	private static bool m_setDisplayTransform;

	private DisplayParams m_display;
	
	[DllImport("__Internal")]
	private static extern void cambrian_firstFrameReceived();

	public delegate void ARModeChanged(CBMode mode);
	public static event ARModeChanged ARModeChangedEvent;
	private static void _modeChanged(CBMode mode) {
		if (m_mode == mode && m_hasSentMode) return;
		
		m_mode = mode;
		m_hasSentMode = true;
		
		if (ARModeChangedEvent != null)
		{
			ARModeChangedEvent(mode);
		}
	}

	// Use this for initialization
	void Start () {
		m_instance = this;

		Debug.Log("Starting CambrianARCameraManager");
		
		CambrianARSessionNativeInterface.GetARSessionNativeInterface();
		
		CambrianARSessionNativeInterface.ARSessionStartedEvent += SessionStartedEvent;
		CambrianARSessionNativeInterface.ARSessionStoppedEvent += SessionStoppedEvent;
		CambrianARSessionNativeInterface.ARSessionCaptureToStillEvent += SessionCaptureEvent;
		CambrianARSessionNativeInterface.ARCaptureScreenshotEvent += SaveScreenshot;
		CambrianARSessionNativeInterface.ARImageReadyEvent += ImageReady;
        CambrianARSessionNativeInterface.ARFrameReadyEvent += FrameUpdated;


		if (CambrianARSessionNativeInterface.shouldStartMode() == 1) {
			StartVideo();
		}
		else if (CambrianARSessionNativeInterface.shouldStartMode() == 2) {
			//StartStillMode();//TODO: make a getCameraTransform in plugin c++
		}

        CambrianARSessionNativeInterface.SceneLoaded();
	}

	public void OnDestroy()
	{
		m_instance = null;
		m_setDisplayTransform = false;
		
		CambrianARSessionNativeInterface.ARSessionStartedEvent -= SessionStartedEvent;
		CambrianARSessionNativeInterface.ARSessionStoppedEvent -= SessionStoppedEvent;
		CambrianARSessionNativeInterface.ARSessionCaptureToStillEvent -= SessionCaptureEvent;
		CambrianARSessionNativeInterface.ARCaptureScreenshotEvent -= SaveScreenshot;
		CambrianARSessionNativeInterface.ARImageReadyEvent -= ImageReady;
		CambrianARSessionNativeInterface.ARFrameReadyEvent -= FrameUpdated;
	}

	private void SessionStartedEvent(CambrianARSessionState sessionState) {
		
		Debug.Log("SessionStartedEvent");

		m_setDisplayTransform = false;
		m_hasFirstUpdate = false;

		bool isVideo = (m_mode == CBMode.Video);
		if (m_isRunning && isVideo == sessionState.isVideo) return;
		
		if (sessionState.isVideo) {
			StartVideo();
		}
		else {
			StartStillMode(sessionState.worldTransform);
		}
	}
	
	private void SessionStoppedEvent(CambrianARSessionState sessionState) {
		
		m_isRunning = false;
		Debug.Log("SessionStoppedEvent");
	}

	void FrameUpdated(CBCameraFrame cam) {

        if (!m_setDisplayTransform) {
            _updateDisplayMatrix(cam.displayTransform, cam.videoParams.yWidth, cam.videoParams.yHeight);
        }

		var cameraTransform = TransformExtensions.FromCBMatrix(cam.cameraTransform);

		m_camera.transform.localPosition = cameraTransform.ExtractPosition();
		m_camera.transform.localRotation = cameraTransform.ExtractRotation();

		m_camera.projectionMatrix = TransformExtensions.FromCBMatrix(cam.projectionMatrix);
		
		if (!m_hasFirstUpdate) {
			m_hasFirstUpdate = true;
			cambrian_firstFrameReceived();
		}

	}

	void _updateDisplayMatrix(CBMatrix4x4 trans, int width, int height) {
		
		m_setDisplayTransform = true;
		
		m_display = new DisplayParams();
		m_display.displayTransform = TransformExtensions.FromCBMatrix(trans);

		m_display.width = width;
		m_display.height = height;
		
		if (ARDisplayTransformUpdatedEvent != null)
		{
			ARDisplayTransformUpdatedEvent(m_display);
		}
	}
	
	private void ImageReady(CambrianARImage image) {
		_modeChanged(CBMode.Still);
	}
	
	private void SessionCaptureEvent() {
		
		Debug.Log("SessionCaptureEvent");

		_modeChanged(CBMode.Capture);
	}
	
	private void SaveScreenshot(CambrianScreenshot screenshot) {
		m_screenshot_temp_name = "screenshot.png";
		
		string tempPath = System.IO.Path.Combine(Application.persistentDataPath, m_screenshot_temp_name);
		if (System.IO.File.Exists(tempPath)) {
			System.IO.File.Delete(tempPath);
		}

		//CB.Log("Saving screenshot to {0}", m_screenshot_temp_name);
		ScreenCapture.CaptureScreenshot(m_screenshot_temp_name);
		
		m_screenshot_final_path = screenshot.path;
		m_hasScreenshot = true;
	}

	void StartVideo() {
		//Debug.Log("JOEL: Starting in Video Mode");
		
		m_isRunning = true;
		_modeChanged(CBMode.Video);
		
		Application.targetFrameRate = 60;

		if (m_camera == null) {
			m_camera = Camera.main;
		}
	}

    void StartStillMode(Matrix4x4 worldTransform) {
		m_isRunning = true;

        m_camera.transform.localPosition = worldTransform.inverse.ExtractPosition();
        m_camera.transform.localRotation = worldTransform.inverse.rotation;

		_modeChanged(CBMode.Still);
	}
	
	public void SetCamera(Camera newCamera)
	{
		if (m_camera != null) {
			CambrianARVideo oldARVideo = m_camera.gameObject.GetComponent<CambrianARVideo> ();
			if (oldARVideo != null) {
				savedClearMaterial = oldARVideo.m_ClearMaterial;
				Destroy (oldARVideo);
			}
		}
		SetupNewCamera (newCamera);
	}

	private void SetupNewCamera(Camera newCamera)
	{
		m_camera = newCamera;

        if (m_camera != null) {
	        CambrianARVideo unityARVideo = m_camera.gameObject.GetComponent<CambrianARVideo> ();
            if (unityARVideo != null) {
                savedClearMaterial = unityARVideo.m_ClearMaterial;
                Destroy (unityARVideo);
            }
            unityARVideo = m_camera.gameObject.AddComponent<CambrianARVideo> ();
            unityARVideo.m_ClearMaterial = savedClearMaterial;
        }
	}

	void Update () {

		if (m_hasScreenshot) {
			//CB.Log("Searching for screenshot {0}", m_screenshot_temp_name);
			string tempPath = System.IO.Path.Combine(Application.persistentDataPath, m_screenshot_temp_name);
			if (System.IO.File.Exists(tempPath)) {
				m_hasScreenshot = false;

				if (System.IO.File.Exists(m_screenshot_final_path)) {
					System.IO.File.Delete(m_screenshot_final_path);
				}
				
				System.IO.File.Move(tempPath, m_screenshot_final_path);

				CambrianARSessionNativeInterface.ScreenshotSaved();
				
				//CB.Log("Moved screenshot to {0}", m_screenshot_final_path);
				m_screenshot_temp_name = String.Empty;
				m_screenshot_final_path = String.Empty;
			}
		}
	}

}
