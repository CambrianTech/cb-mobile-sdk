#undef UNITY_EDITOR

using System;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Rendering;

namespace Cambrian.iOS {

    public class CambrianARVideo : MonoBehaviour
    {
        public Material m_ClearMaterial;

	    public static CambrianARVideo singleton;

        private CommandBuffer m_VideoCommandBuffer;
        private Texture2D _videoTextureY;
        private Texture2D _videoTextureCbCr;
		private Matrix4x4 _displayTransform;
	    
		private bool bCommandBufferInitialized;

		public void Start()
		{
			CambrianARSessionNativeInterface.ARFrameReadyEvent += UpdateFrame;
			bCommandBufferInitialized = false;

			singleton = this;
		}

		void UpdateFrame(CBCameraFrame cam)
		{
			_displayTransform = new Matrix4x4();
			_displayTransform.SetColumn(0, cam.displayTransform.column0);
			_displayTransform.SetColumn(1, cam.displayTransform.column1);
			_displayTransform.SetColumn(2, cam.displayTransform.column2);
			_displayTransform.SetColumn(3, cam.displayTransform.column3);		
		}

		void InitializeCommandBuffer()
		{
			m_VideoCommandBuffer = new CommandBuffer(); 
			m_VideoCommandBuffer.Blit(null, BuiltinRenderTextureType.CurrentActive, m_ClearMaterial);
			GetComponent<Camera>().AddCommandBuffer(CameraEvent.BeforeForwardOpaque, m_VideoCommandBuffer);
			bCommandBufferInitialized = true;

		}

		void OnDestroy()
		{
			GetComponent<Camera>().RemoveCommandBuffer(CameraEvent.BeforeForwardOpaque, m_VideoCommandBuffer);
			CambrianARSessionNativeInterface.ARFrameReadyEvent -= UpdateFrame;
			bCommandBufferInitialized = false;
		}

#if !UNITY_EDITOR

        public void OnPreRender()
        {
	        CBTextureHandles handles = CambrianARSessionNativeInterface.GetARSessionNativeInterface().GetVideoTextureHandles();
            if (handles.textureY == System.IntPtr.Zero || handles.textureCbCr == System.IntPtr.Zero)
            {
                return;
            }

            if (!bCommandBufferInitialized) {
                InitializeCommandBuffer ();
            }

            Resolution currentResolution = Screen.currentResolution;

            // Texture Y
            if (_videoTextureY == null) {
              _videoTextureY = Texture2D.CreateExternalTexture(currentResolution.width, currentResolution.height,
                  TextureFormat.R8, false, false, (System.IntPtr)handles.textureY);
              _videoTextureY.filterMode = FilterMode.Bilinear;
              _videoTextureY.wrapMode = TextureWrapMode.Repeat;
              m_ClearMaterial.SetTexture("_textureY", _videoTextureY);
            }

            // Texture CbCr
            if (_videoTextureCbCr == null) {
              _videoTextureCbCr = Texture2D.CreateExternalTexture(currentResolution.width, currentResolution.height,
                  TextureFormat.RG16, false, false, (System.IntPtr)handles.textureCbCr);
              _videoTextureCbCr.filterMode = FilterMode.Bilinear;
              _videoTextureCbCr.wrapMode = TextureWrapMode.Repeat;
              m_ClearMaterial.SetTexture("_textureCbCr", _videoTextureCbCr);
            }

            _videoTextureY.UpdateExternalTexture(handles.textureY);
            _videoTextureCbCr.UpdateExternalTexture(handles.textureCbCr);
	        
			m_ClearMaterial.SetMatrix("_DisplayTransform", _displayTransform);
	        
        }
#else

		public void SetYTexure(Texture2D YTex)
		{
			_videoTextureY = YTex;
		}
	    
		public void SetUVTexure(Texture2D UVTex)
		{
			_videoTextureCbCr = UVTex;
		}

		public void OnPreRender()
		{

			if (!bCommandBufferInitialized) {
				InitializeCommandBuffer ();
			}

			m_ClearMaterial.SetTexture("_textureY", _videoTextureY);
			m_ClearMaterial.SetTexture("_textureCbCr", _videoTextureCbCr);

			m_ClearMaterial.SetMatrix("_DisplayTransform", _displayTransform);
		}
 
#endif
	    public Texture2D GetYTexure()
	    {
		    return _videoTextureY;
	    }
    }
}
