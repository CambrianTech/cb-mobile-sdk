using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine;
using UnityEngine.UI;

namespace Cambrian.iOS
{
	public class CambrianARDrawingRenderer : CambrianRenderer {
		
		private Texture2D DrawingTexture;
		private RawImage img;

		private static CambrianARDrawingRenderer m_instance;
		
		[DllImport("__Internal")]
		private static extern void cambrian_SetDrawingCallbacks(
			internal_ARDrawingUpdated drawingUpdated
		);
	
		private delegate void internal_ARDrawingUpdated(DrawingParams drawingParams);
		
		[MonoPInvokeCallback(typeof(internal_ARDrawingUpdated))]
		static void _drawing_updated(DrawingParams drawingParams) {			
			//CB.Log("Drawing was updated");
			Color color = new Color(
				drawingParams.colorRed, 
				drawingParams.colorGreen, 
				drawingParams.colorBlue, 
				drawingParams.colorAlpha);
			
			m_instance.img.material.SetColor("m_color", color);
		}
		
		[DllImport("__Internal")]
		private static extern IntPtr cambrian_GenerateDrawingTexture(Int32 width, Int32 height);


		protected override Material getMaterial() {
			return img.material;
		}

		protected override CBAssetType getAssetType() {
			return CBAssetType.All;
		}

		public new void Start () {
			base.Start();
			
			m_instance = this;
						
			img = GetComponent<RawImage>();

#if !UNITY_EDITOR
			cambrian_SetDrawingCallbacks(_drawing_updated);
#endif
		}

		public new void OnDestroy() {
			m_instance = null;
			base.OnDestroy();
		}
		
		protected override void DisplayParametersChanged(DisplayParams image) {
			generateDrawingTexture(image.width, image.height);
		}

		private void generateDrawingTexture(int width, int height) {
			
			IntPtr drawingMask = cambrian_GenerateDrawingTexture(width, height);
		    
			DrawingTexture = Texture2D.CreateExternalTexture(Screen.currentResolution.width, Screen.currentResolution.height,
				TextureFormat.R8, false, false, drawingMask);
			DrawingTexture.filterMode = FilterMode.Bilinear;
			DrawingTexture.wrapMode = TextureWrapMode.Repeat;

			img.texture = DrawingTexture;
		}
	}
}

