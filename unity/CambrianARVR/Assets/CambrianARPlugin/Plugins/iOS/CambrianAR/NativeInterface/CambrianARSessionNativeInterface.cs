//#undef UNITY_EDITOR

using UnityEngine;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine.SceneManagement;
using System.Threading.Tasks;
using Debug = UnityEngine.Debug;

//original inspiration https://www.youtube.com/watch?v=uBWThlohEn4

namespace Cambrian.iOS {
	
	public class CambrianARSessionNativeInterface {
        void Handleinternal_ARExecuteCommand(CambrianCommand command)
        {
        }


		static CambrianARSessionNativeInterface s_CambrianARSessionNativeInterface = null;
		
		//session delegates
		public delegate void ARSessionStarted(CambrianARSessionState sessionState);
		public static event ARSessionStarted ARSessionStartedEvent;
		private delegate void internal_ARSessionStarted(CambrianARSessionState assetData);
		
		public delegate void ARSessionStopped(CambrianARSessionState sessionState);
		public static event ARSessionStopped ARSessionStoppedEvent;
		private delegate void internal_ARSessionStopped(CambrianARSessionState assetData);
		
		public delegate void ARSceneChanging(CambrianARSessionState sessionState);
		public static event ARSceneChanging ARSceneChangingEvent;
		private delegate void internal_ARSceneChanging(CambrianARSessionState assetData);
		
		public delegate void ARFrameReady(CBCameraFrame frame);
		public static event ARFrameReady ARFrameReadyEvent;
		private delegate void internal_ARFrameReady(CBCameraFrame frame);
		
		public delegate void ARImageReady(CambrianARImage image);
		public static event ARImageReady ARImageReadyEvent;
		private delegate void internal_ARImageReady(CambrianARImage image);
		
		public delegate void ARSessionCaptureToStill();
		public static event ARSessionCaptureToStill ARSessionCaptureToStillEvent;
		private delegate void internal_ARSessionCaptureToStill();
		
		public delegate void ARCaptureScreenshot(CambrianScreenshot screenshot);
		public static event ARCaptureScreenshot ARCaptureScreenshotEvent;
		private delegate void internal_ARCaptureScreenshot(CambrianScreenshot screenshot);

		[DllImport("__Internal")]
		private static extern Matrix4x4 cambrian_ScreenshotSaved();

		internal static void ScreenshotSaved()
		{
			cambrian_ScreenshotSaved();
		}

        public delegate void ARExecuteCommand(CambrianCommand command);
        public static event ARExecuteCommand ARExecuteCommandEvent;
        private delegate void internal_ARExecuteCommand(CambrianCommand command);
		
		//anchor callbacks
		
		public delegate void AROrientationNormalFound(Vector3 normal);
		public static event AROrientationNormalFound AROrientationNormalFoundEvent;
		private delegate void internal_AROrientationNormalFound(Vector3 normal);
		
		public delegate void ARPlaneAnchorAdded(CBPlaneAnchor anchorData);
		public static event ARPlaneAnchorAdded ARPlaneAnchorAddedEvent;
		private delegate void internal_ARPlaneAnchorAdded(CBPlaneAnchor anchorData);

		public delegate void ARPlaneAnchorUpdated(CBPlaneAnchor anchorData);
		public static event ARPlaneAnchorUpdated ARPlaneAnchorUpdatedEvent;
		private delegate void internal_ARPlaneAnchorUpdated(CBPlaneAnchor anchorData);

		public delegate void ARPlaneAnchorRemoved(CBPlaneAnchor anchorData);
		public static event ARPlaneAnchorRemoved ARPlaneAnchorRemovedEvent;
		private delegate void internal_ARPlaneAnchorRemoved(CBPlaneAnchor anchorData);

		// User Anchors
		public delegate void ARUserAnchorAdded(CBUserAnchor anchorData);
		public static event ARUserAnchorAdded ARUserAnchorAddedEvent;
		private delegate void internal_ARUserAnchorAdded(CBUserAnchor anchorData);

		public delegate void ARUserAnchorUpdated(CBUserAnchor anchorData);
		public static event ARUserAnchorUpdated ARUserAnchorUpdatedEvent;
		private delegate void internal_ARUserAnchorUpdated(CBUserAnchor anchorData);

		public delegate void ARUserAnchorRemoved(CBUserAnchor anchorData);
		public static event ARUserAnchorRemoved ARUserAnchorRemovedEvent;
		private delegate void internal_ARUserAnchorRemoved(CBUserAnchor anchorData);
		
		//asset delegates
		public delegate void ARAssetAdded(CambrianARAsset assetData);
		public static event ARAssetAdded ARAssetAddedEvent;
		private delegate void internal_ARAssetAdded(CambrianARAsset assetData);
	
		public delegate void ARAssetRemoved(CambrianARAsset assetData);
		public static event ARAssetRemoved ARAssetRemovedEvent;
		private delegate void internal_ARAssetRemoved(CambrianARAsset assetData);
		
		public delegate void ARAssetSelected(CambrianARAsset assetData);
		public static event ARAssetSelected ARAssetSelectedEvent;
		private delegate void internal_ARAssetSelected(CambrianARAsset assetData);
		
		private delegate Vector3 internal_ViewportToWorldPoint(Vector3 point2D);
		private delegate Vector3 internal_WorldToViewportPoint(Vector3 point3D);
		
		[DllImport("__Internal")]
		private static extern void cambrian_NotifyReady(bool isReady);

		public void NotifyReady(bool isReady)
		{
			cambrian_NotifyReady(isReady);
		}
		
		[DllImport("__Internal")]
		private static extern int cambrian_HitTest(CBPoint2D point, CBHitTestResultType types);
		
		[DllImport("__Internal")]
		private static extern CBHitTestResult cambrian_GetLastHitTestResult(int index);

        [DllImport("__Internal")]
        private static extern IntPtr cambrian_getSceneName();

		[DllImport("__Internal")]
		private static extern float cambrian_SegmentationAverageSeconds();

		public float getSegmentationAverageSeconds() {
			return cambrian_SegmentationAverageSeconds();
		}

        internal static string getSceneName() {
            IntPtr sceneName = cambrian_getSceneName();
            return Marshal.PtrToStringAuto(sceneName);
        }
		
		[DllImport("__Internal")]
		private static extern void cambrian_TouchEvent(
			int touchPhase, float screenX, float screenY
		);
		
		public static void touchEvent(int phase, Vector3 position) {
			cambrian_TouchEvent(phase, position.x, 1.0F - position.y);
		}

		[DllImport("__Internal")]
		private static extern void cambrian_StillRender();

		public static void stillRender() {
			cambrian_StillRender();
		}

		#if !UNITY_EDITOR
		private IntPtr m_NativeARSession;
		#endif
		
		[DllImport("__Internal")]
		private static extern IntPtr cambrian_GetNativeARSession(
			internal_ARSessionStarted sessionStarted,
			internal_ARSessionStopped sessionStopped,
			internal_ARSceneChanging sceneChanging,
			internal_ARFrameReady frameReady,
			internal_ARImageReady imageReady,
			internal_ARSessionCaptureToStill captureToStill,
			internal_ARCaptureScreenshot screenshot,
            internal_ARExecuteCommand command
		);

		[DllImport("__Internal")]
		private static extern void cambrian_SetAnchorCallbacks(
			internal_AROrientationNormalFound orientationChanged,
			internal_ARPlaneAnchorAdded planeAnchorAdded,
			internal_ARPlaneAnchorUpdated planeAnchorUpdated,
			internal_ARPlaneAnchorRemoved planeAnchorRemoved,
			internal_ARUserAnchorAdded userAnchorAdded,
			internal_ARUserAnchorUpdated userAnchorUpdated,
			internal_ARUserAnchorRemoved userAnchorRemoved
		);
		
		[DllImport("__Internal")]
		private static extern CBTextureHandles cambrian_GetVideoTextureHandles();

		public CBTextureHandles GetVideoTextureHandles() {
			return cambrian_GetVideoTextureHandles();
		}
		
		[DllImport("__Internal")]
		private static extern Matrix4x4 cambrian_Get2DAffineTransform(ref Vector3 pointsBefore, ref Vector3 pointsAfter);

		public static Matrix4x4 Get2DAffineTransform(Vector3[] pointsBefore, Vector3[] pointsAfter)
		{
			Matrix4x4 result = cambrian_Get2DAffineTransform(ref pointsBefore[0], ref pointsAfter[0]);

			return result;
		}

		[DllImport("__Internal")]
		private static extern int cambrian_shouldStartMode();

		public static int shouldStartMode() {
			return cambrian_shouldStartMode();
		}

		[DllImport("__Internal")]
		private static extern void cambrian_SetAssetCallbacks(
			internal_ARAssetAdded assetAdded,
			internal_ARAssetRemoved assetRemoved,
			internal_ARAssetRemoved assetSelected
		);

		[DllImport("__Internal")]
		internal static extern CBMatrix4x4 cambrian_AddUserAnchorWithTransform(CBMatrix4x4 transform);

		CBMatrix4x4 AddUserAnchorWithTransform(CBMatrix4x4 transform) {
			return cambrian_AddUserAnchorWithTransform(transform);
		}

		public static CambrianARSessionNativeInterface GetARSessionNativeInterface()
		{
			if (s_CambrianARSessionNativeInterface == null) {
				Debug.Log("CambrianARSessionNativeInterface init");
				s_CambrianARSessionNativeInterface = new CambrianARSessionNativeInterface ();
			}
			return s_CambrianARSessionNativeInterface;
		}

		public CambrianARSessionNativeInterface()
		{
			#if !UNITY_EDITOR
			m_NativeARSession = cambrian_GetNativeARSession(
				_sessionStarted,
				_sessionStopped,
				_sceneChanging,
				_frameReady,
				_imageReady,
				_captureToStill,
                _screenshot,
                _executeCommand
			);
	
			cambrian_SetAnchorCallbacks(
				_orientationNormalFound,
				_plane_anchor_added,
				_plane_anchor_updated,
				_plane_anchor_removed,
				_user_anchor_added,
				_user_anchor_updated,
				_user_anchor_removed
			);
	
			cambrian_SetAssetCallbacks(
				_asset_added,
				_asset_removed,
				_asset_selected
			);

			CambrianARMaskIntegrator.GetMaskIntegrator();

			#endif
		}

        #region Session Events


		[MonoPInvokeCallback(typeof(internal_ARSessionStarted))]
		static void _sessionStarted(CambrianARSessionState sessionState)
		{
            if (m_isPaused)
            {
                Debug.Log("RESUMING UNITY");
                ResumeSession(sessionState);
            }
            else
            {
                Debug.Log("STARTING UNITY");
                if (ARSessionStartedEvent != null)
                {
                    ARSessionStartedEvent(sessionState);
                }
            }
		}
		
		[MonoPInvokeCallback(typeof(internal_ARSessionStopped))]
		static void _sessionStopped(CambrianARSessionState sessionState)
		{
			Debug.Log("STOPPING UNITY");
			
			if (ARSessionStoppedEvent != null)
			{
				ARSessionStoppedEvent(sessionState);
			}

			//UnityARSessionNativeInterface.GetARSessionNativeInterface().Pause();
            m_isPaused = true;

			//SceneManager.LoadScene("CambrianARPlugin/Scenes/SceneLoader", LoadSceneMode.Single);
		}

        private static bool m_isPaused = false;
        private static CambrianARSessionState m_savedSessionState;

		private static void ResumeSession(CambrianARSessionState sessionState)
		{
			m_savedSessionState = sessionState;

			string sceneName = CambrianARSessionNativeInterface.getSceneName();

			var lastScene = SceneManager.GetActiveScene();
			var nextScene = SceneManager.GetSceneByName(sceneName);

			if (nextScene.isLoaded) {
				SceneManager.SetActiveScene(nextScene);
				Debug.Log("Loading existing scene " + sceneName);
			} else {
				SceneManager.LoadScene(sceneName, LoadSceneMode.Single);
			}

			//UnityARSessionNativeInterface.GetARSessionNativeInterface();

			var building = GameObject.FindWithTag("Building");

			if (building != null && building.activeInHierarchy) {
				//Debug.Log("JOEL: Getting rid of Building");
				building.SetActive(false);
			}

			if (!lastScene.isLoaded) return;
 
			foreach (var gameObject in lastScene.GetRootGameObjects()) {
				gameObject.SetActive(false);
			}
		}

        public static void SceneLoaded()
        {
            if (ARSessionStartedEvent != null && m_isPaused)
            {
                ARSessionStartedEvent(m_savedSessionState);
            }
            m_isPaused = false;
        }
		
		[MonoPInvokeCallback(typeof(internal_ARSceneChanging))]
		static void _sceneChanging(CambrianARSessionState sessionState)
		{
			if (ARSceneChangingEvent != null)
			{
				ARSceneChangingEvent(sessionState);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_ARFrameReady))]
		static void _frameReady(CBCameraFrame frame)
		{
			if (ARFrameReadyEvent != null)
			{
				ARFrameReadyEvent(frame);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_ARImageReady))]
		static void _imageReady(CambrianARImage frame)
		{
			if (ARImageReadyEvent != null)
			{
				ARImageReadyEvent(frame);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_ARSessionCaptureToStill))]
		static void _captureToStill()
		{
			if (ARSessionCaptureToStillEvent != null)
			{
				ARSessionCaptureToStillEvent();
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_ARCaptureScreenshot))]
		static void _screenshot(CambrianScreenshot screenshot)
		{
			if (ARCaptureScreenshotEvent != null)
			{
				ARCaptureScreenshotEvent(screenshot);
			}
		}

        [MonoPInvokeCallback(typeof(internal_ARExecuteCommand))]
        static void _executeCommand(CambrianCommand command)
        {
            if (ARExecuteCommandEvent != null)
            {
                ARExecuteCommandEvent(command);
            }
        }
		
		#endregion
		
		#region Anchor Events
		
		[MonoPInvokeCallback(typeof(internal_AROrientationNormalFound))]
		static void _orientationNormalFound(Vector3 normal)
		{
			if (AROrientationNormalFoundEvent != null)
			{
				AROrientationNormalFoundEvent(normal);
			}
		}
		
		
		[MonoPInvokeCallback(typeof(internal_ARPlaneAnchorAdded))]
		static void _plane_anchor_added(CBPlaneAnchor anchorData)
		{
			if (ARPlaneAnchorAddedEvent != null)
			{
				ARPlaneAnchorAddedEvent(anchorData);
			}
		}

		[MonoPInvokeCallback(typeof(internal_ARPlaneAnchorUpdated))]
		static void _plane_anchor_updated(CBPlaneAnchor anchorData)
		{
			if (ARPlaneAnchorUpdatedEvent != null)
			{
				ARPlaneAnchorUpdatedEvent(anchorData);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_ARPlaneAnchorRemoved))]
		static void _plane_anchor_removed(CBPlaneAnchor anchorData) {
			if (ARPlaneAnchorRemovedEvent != null)
			{
				ARPlaneAnchorRemovedEvent(anchorData);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_ARUserAnchorAdded))]
		static void _user_anchor_added(CBUserAnchor anchorData)
		{
			if (ARUserAnchorAddedEvent != null)
			{
				ARUserAnchorAddedEvent(anchorData);
			}
		}

		[MonoPInvokeCallback(typeof(internal_ARUserAnchorUpdated))]
		static void _user_anchor_updated(CBUserAnchor anchorData)
		{
			if (ARUserAnchorUpdatedEvent != null)
			{
				ARUserAnchorUpdatedEvent(anchorData);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_ARUserAnchorRemoved))]
		static void _user_anchor_removed(CBUserAnchor anchorData) {
			if (ARUserAnchorRemovedEvent != null)
			{
				ARUserAnchorRemovedEvent(anchorData);
			}
		}
		
		#endregion

		#region Asset Events
		[MonoPInvokeCallback(typeof(internal_ARAssetAdded))]
		static void _asset_added(CambrianARAsset assetData)
		{
			if (ARAssetAddedEvent != null)
			{
				ARAssetAddedEvent(assetData);
			}
		}

		[MonoPInvokeCallback(typeof(internal_ARAssetRemoved))]
		static void _asset_removed(CambrianARAsset assetData)
		{
			if (ARAssetRemovedEvent != null)
			{
				ARAssetRemovedEvent(assetData);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_ARAssetSelected))]
		static void _asset_selected(CambrianARAsset assetData) {
			ARAssetSelectedEvent?.Invoke(assetData);
		}
		#endregion
		
		#region Other Events
		private readonly object hitLock = new object();
		public List<CBHitTestResult> HitTest(CBPoint2D point2D, CBHitTestResultType resultType) {

			lock (hitLock) {
				int count = cambrian_HitTest(point2D, resultType);

				List<CBHitTestResult> results = new List<CBHitTestResult>();

				for (int i = 0; i < count; i++) {
					CBHitTestResult result = cambrian_GetLastHitTestResult(i);
					results.Add(result);
				}

				return results;
			}
		}


		#endregion
	}
}
