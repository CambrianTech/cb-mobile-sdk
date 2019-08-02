//#undef UNITY_EDITOR

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine;

namespace Cambrian.iOS
{
    public class CambrianARMaskIntegrator
	{
		//Mask (floor, paint) delegates
		public delegate void CBSurfaceCreated(CBSurfaceTextureData textureInfo);
		public static event CBSurfaceCreated CBSurfaceCreatedEvent;
		private delegate void internal_CBSurfaceCreated(CBSurfaceAssetParams data);
		
		public delegate void CBSurfaceRemoved(CBSurfaceTextureData textureInfo);
		public static event CBSurfaceRemoved CBSurfaceRemovedEvent;
		private delegate void internal_CBSurfaceRemoved(CBSurfaceAssetParams data);
		
		public delegate void CBSurfaceDataUpdated(CBSurfaceAssetParams info, CBSurfaceData surfaceData);
		public static event CBSurfaceDataUpdated CBSurfaceDataUpdatedEvent;
		private delegate void internal_CBSurfaceDataUpdated(CBSurfaceAssetParams data, CBSurfaceDataExtents extents);
		
        static CambrianARMaskIntegrator s_CambrianMaskIntegrator = null;
		
        public static CambrianARMaskIntegrator GetMaskIntegrator()
		{
			if (s_CambrianMaskIntegrator == null) {
				s_CambrianMaskIntegrator = new CambrianARMaskIntegrator();
			}
			return s_CambrianMaskIntegrator;
		}
		
        public CambrianARMaskIntegrator()
		{
			m_surfaceAssets = new Dictionary<int, CBSurfaceTextureData>();
			
			CambrianARSessionNativeInterface.ARAssetRemovedEvent += assetRemoved;
			CambrianARSessionNativeInterface.ARSceneChangingEvent += sceneChanging;
			CambrianARSessionNativeInterface.ARSessionStartedEvent += sessionStarted;
			CambrianARSessionNativeInterface.ARSessionStoppedEvent += sessionStopped;
			
#if !UNITY_EDITOR
			cambrian_SetSurfaceCallbacks(
				_surface_created,
				_surface_removed,
				_surface_data_updated
			);
#endif
		}

		private void sessionStarted(CambrianARSessionState sessionstate) {
			GetMaskIntegrator();
		}
		
		private void sessionStopped(CambrianARSessionState sessionstate) {
			m_surfaceAssets = new Dictionary<int, CBSurfaceTextureData>();
			cambrian_ClearAllSurfaces();
		}
		
		private Dictionary<int, CBSurfaceTextureData> m_surfaceAssets;
		
		[DllImport("__Internal")]
		private static extern void cambrian_SetSurfaceCallbacks(
			internal_CBSurfaceCreated surfaceCreated,
			internal_CBSurfaceRemoved surfaceRemoved,
			internal_CBSurfaceDataUpdated surfaceData
		);
		
		[DllImport("__Internal")]
		private static extern void cambrian_ClearAllSurfaces();

		public static void ClearAllSurfaces() {
			cambrian_ClearAllSurfaces();
		}
		
		[DllImport("__Internal")]
        private static extern bool cambrian_GetMeshData(Int32 index, out IntPtr verts3D, out int vertLength, out IntPtr indices3D, out int indiceLength);
				
		static object generateSurfaceTextureDataLock = new object();
		
		private CBSurfaceTextureData generateSurfaceTextureData(CBSurfaceAssetParams data)
		{
			lock (generateSurfaceTextureDataLock)
			{
				if (m_surfaceAssets.ContainsKey(data.index))
				{
					return m_surfaceAssets[data.index];
				}

				CBSurfaceTextureData surface = new CBSurfaceTextureData
				{
					asset = data,
					index = data.index
				};
				
				m_surfaceAssets[data.index] = surface;
				
				Debug.Log(String.Format("Creating surface at index {0}. There are {1} surfaces.", data.index, m_surfaceAssets.Count));

				//mask
				surface.maskTexture = new Texture2D(data.maskWidth, data.maskHeight, TextureFormat.R8, false);
				surface.maskTexture.filterMode = FilterMode.Bilinear;
				surface.maskTexture.wrapMode = TextureWrapMode.Clamp;


				surface.shadowsTexture = new Texture2D(data.shadowWidth, data.shadowHeight, TextureFormat.R8, false);
				surface.shadowsTexture.filterMode = FilterMode.Trilinear;
				surface.shadowsTexture.wrapMode = TextureWrapMode.Clamp;

				return surface;
			}
		}
		
		[DllImport("__Internal")]
		private static extern bool cambrian_GetSurfaceMaskData(Int32 index, out IntPtr verts3D, out int length);
		
		[DllImport("__Internal")]
		private static extern bool cambrian_GetSurfaceShadowData(Int32 index, out IntPtr verts3D, out int length);
		
		CBSurfaceTextureData removeSurface(int index) {
			
			if (m_surfaceAssets.ContainsKey(index)) {
				CBSurfaceTextureData mask = m_surfaceAssets[index];
				m_surfaceAssets.Remove(index);
				return mask;
			}
			
			return null;
		}

		CBMeshData getMeshData(Int32 index) {
	        
            IntPtr ptrResultVerts3D = IntPtr.Zero;
	        IntPtr ptrResultIndices3D = IntPtr.Zero;

            int resultVertLength, resultIndicesLength;
            bool success = cambrian_GetMeshData(index, out ptrResultVerts3D, out resultVertLength, out ptrResultIndices3D, out resultIndicesLength);
			
			CBMeshData data = new CBMeshData();
			
			if (success) {
                // Load the results into a managed array.
                int totalPoints = resultVertLength / 3;

                float[] resultVertices3D = new float[resultVertLength];
                Marshal.Copy (ptrResultVerts3D, resultVertices3D, 0, resultVertLength); 
                Marshal.FreeCoTaskMem(ptrResultVerts3D);

                data.vertices = new Vector3[totalPoints];

                for (int i = 0; i < totalPoints; i++) {

                    int j = i * 3;

                    data.vertices[i].x = resultVertices3D[j];
                    data.vertices[i].y = resultVertices3D[j + 1];
                    data.vertices[i].z = resultVertices3D[j + 2];
				}
				
				int[] resultIndices3D = new int[resultIndicesLength];
				Marshal.Copy (ptrResultIndices3D, resultIndices3D, 0, resultIndicesLength); 
				Marshal.FreeCoTaskMem(ptrResultIndices3D);
				
				data.indices = resultIndices3D;
			}

			return data;
		}

		void assetRemoved(CambrianARAsset asset) {
			removeSurface(asset.index);
		}
		
		void sceneChanging(CambrianARSessionState sessionState) {
			//cambrian_ClearAllSurfaces();
		}
		
		#region Mask Events
			
		[MonoPInvokeCallback(typeof(internal_CBSurfaceCreated))]
		static void _surface_created(CBSurfaceAssetParams data)
		{
			if (s_CambrianMaskIntegrator == null) return;
			
			var surface = s_CambrianMaskIntegrator.generateSurfaceTextureData(data);
			
			if (CBSurfaceCreatedEvent != null) {
				CBSurfaceCreatedEvent(surface);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_CBSurfaceRemoved))]
		static void _surface_removed(CBSurfaceAssetParams data)
		{
			if (s_CambrianMaskIntegrator == null) return;
			var surface = s_CambrianMaskIntegrator?.removeSurface(data.index);
			
			if (CBSurfaceRemovedEvent != null) {
				CBSurfaceRemovedEvent(surface);
			}
		}
		
		[MonoPInvokeCallback(typeof(internal_CBSurfaceDataUpdated))]
		static void _surface_data_updated(CBSurfaceAssetParams info, CBSurfaceDataExtents extents) {

			if (CBSurfaceDataUpdatedEvent == null) return;
			
			if (s_CambrianMaskIntegrator == null) {
				Debug.Log("ERROR: NO MASK INTEGRATOR!");
				return;
			};

			if (!s_CambrianMaskIntegrator.m_surfaceAssets.ContainsKey(info.index))
			{
				_surface_created(info);
			}
			
			var surface = s_CambrianMaskIntegrator.m_surfaceAssets[info.index];
			
			//get mesh data
			CBSurfaceData data = new CBSurfaceData(extents);
			data.mesh = s_CambrianMaskIntegrator.getMeshData(surface.index);

			//get mask data
			{
				int length = 0;

				IntPtr ptrResult = IntPtr.Zero;
				cambrian_GetSurfaceMaskData(surface.index, out ptrResult, out length);

				//get mask data
				data.maskBytes = new byte[length];
				Marshal.Copy(ptrResult, data.maskBytes, 0, length);
			}

			//get shadow data
			{
				int length = 0;

				IntPtr ptrResult = IntPtr.Zero;
				cambrian_GetSurfaceShadowData(surface.index, out ptrResult, out length);

				//get mask data
				data.shadowBytes = new byte[length];
				Marshal.Copy(ptrResult, data.shadowBytes, 0, length);
			}
				
			CBSurfaceDataUpdatedEvent(info, data);
		}

		
		#endregion
	}
}

