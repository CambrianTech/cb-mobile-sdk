using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace Cambrian.iOS
{
    public class CambrianARAmbientLight : MonoBehaviour
    {

        private Light l;

        public void Start()
        {
            l = GetComponent<Light>();
            CambrianARSessionNativeInterface.ARFrameReadyEvent += UpdateLightEstimation;
        }
        
        void OnDestroy() {
            CambrianARSessionNativeInterface.ARFrameReadyEvent -= UpdateLightEstimation;
        }

        void UpdateLightEstimation(CBCameraFrame frame)
        {
            if (frame.lightData.lightingType == CBLightDataType.LightEstimate) {
                // Convert ARKit intensity to Unity intensity
                // ARKit ambient intensity ranges 0-2000
                // Unity ambient intensity ranges 0-8 (for over-bright lights)
                // float newai = frame.lightData.lightEstimate.ambientIntensity;
                // l.intensity = Math.Min((newai + 100.0F) / 8000.0f, 250.0f);

                l.colorTemperature = frame.lightData.lightEstimate.ambientColorTemperature;
            }
        }

        
    }
}
