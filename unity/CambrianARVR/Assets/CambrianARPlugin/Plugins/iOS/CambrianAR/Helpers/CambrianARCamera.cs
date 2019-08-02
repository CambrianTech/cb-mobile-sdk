using System;
using UnityEngine;
using UnityEngine.UI;

namespace Cambrian.iOS {
	[RequireComponent(typeof(Camera))]
	public class CambrianARCamera : MonoBehaviour {

		private Camera attachedCamera;
		private float currentNearZ;
		private float currentFarZ;

        public delegate void CameraPostRender();
        public static event CameraPostRender CameraPostRenderEvent;

		// Use this for initialization
		void Start() {
			attachedCamera = GetComponent<Camera>();
			UpdateCameraClipPlanes();
			
			//UnityARSessionNativeInterface.ARFrameUpdatedEvent += frameUpdated;	
			//CambrianARSessionNativeInterface.ARFrameReadyEvent += frameReady;
			
			//CB.Log("Starting cambrian camera");
		}

		void UpdateCameraClipPlanes() {
			currentNearZ = attachedCamera.nearClipPlane;
			currentFarZ = attachedCamera.farClipPlane;
			//UnityARSessionNativeInterface.GetARSessionNativeInterface().SetCameraClipPlanes(currentNearZ, currentFarZ);
		}

		// Update is called once per frame
		void Update() {
			if (currentNearZ != attachedCamera.nearClipPlane || currentFarZ != attachedCamera.farClipPlane) {
				UpdateCameraClipPlanes();
			}
		}

        protected void OnPostRender()
        {
            if (CameraPostRenderEvent != null) {
                CameraPostRenderEvent();
            }
        }
 
	}

}