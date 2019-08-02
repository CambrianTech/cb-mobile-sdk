using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine;

namespace Cambrian.iOS
{
	public abstract class CambrianRenderer : MonoBehaviour {
		
		private bool m_hasInitializedCamera;
		private CBMode m_mode;
		
		protected DisplayParams m_display;

		protected CambrianRenderer() {
			
		}
		
		protected abstract Material getMaterial();
		protected abstract CBAssetType getAssetType();
		
		public void Start()
		{
			CambrianARCameraManager.ARDisplayTransformUpdatedEvent += _updateDisplayMatrix;
			CambrianARCameraManager.ARModeChangedEvent += _modeChanged;
		}

		public void OnDestroy()
		{			
			CambrianARCameraManager.ARDisplayTransformUpdatedEvent -= _updateDisplayMatrix;
			CambrianARCameraManager.ARModeChangedEvent -= _modeChanged;
		}

		void _modeChanged(CBMode mode) {
			m_mode = mode;
			getMaterial().SetInt("m_isStillMode", m_mode == CBMode.Still ? 1 : 0);
		}

		protected virtual void DisplayParametersChanged(DisplayParams image) {
			
		}

		void _updateDisplayMatrix(DisplayParams dp) {
			m_display = dp;
			
			getMaterial().SetMatrix("m_displayTransform", m_display.displayTransform);
			
			DisplayParametersChanged(m_display);
		}
	}
}

