using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace Cambrian.iOS
{
    public class CambrianARAxisRenderer : MonoBehaviour
    {
        Dictionary<string, LineRenderer> m_lineRenderers;
        
        float AxisLength = 1f;

        // Use this for initialization
        void Start() {
            CambrianARSessionNativeInterface.AROrientationNormalFoundEvent += OrientationNormalFoundEvent;
            
            m_lineRenderers = new Dictionary<string, LineRenderer>();
            //defaults
            m_lineRenderers["x-axis"] = GenerateLineRenderer(Color.red, new Vector3(1.0F, 0.0F, 0.0F), 3.0F);
            m_lineRenderers["y-axis"] = GenerateLineRenderer(Color.green, new Vector3(0.0F, 1.0F, 0.0F), 3.0F);
            m_lineRenderers["z-axis"] = GenerateLineRenderer(Color.blue, new Vector3(0.0F, 0.0F, 1.0F), 3.0F);
        }
        
        private void OnDestroy()
        {
            CambrianARSessionNativeInterface.AROrientationNormalFoundEvent -= OrientationNormalFoundEvent;
            m_lineRenderers = null;
        }
        
        private void OrientationNormalFoundEvent(Vector3 normal)
        {
            const string KEY = "orientation-normal";
            if (m_lineRenderers.ContainsKey(KEY))
            {
                SetOrientation(m_lineRenderers[KEY], normal);
            }
            else
            {
                m_lineRenderers[KEY] = GenerateLineRenderer(Color.yellow, normal);
            }
        }

        LineRenderer GenerateLineRenderer(Color color, Vector3 orientation, float width=1.0F)
        {
            GameObject container = new GameObject();
  
            container.transform.SetParent(gameObject.transform, true);
                
            LineRenderer lineRenderer = container.AddComponent<LineRenderer>();
            lineRenderer.material = new Material(Shader.Find("Mobile/Particles/Additive"));


            float alpha = 1.0f;
            Gradient gradient = new Gradient();
            gradient.SetKeys(
                new[] { new GradientColorKey(color, 0.0f), new GradientColorKey(color, 1.0f) },
                new[] { new GradientAlphaKey(alpha, 0.0f), new GradientAlphaKey(alpha, 1.0f) }
            );
            lineRenderer.colorGradient = gradient;

            lineRenderer.startWidth = width * 0.001f;
            lineRenderer.endWidth = width * 0.001f;
            lineRenderer.loop = true;
            
            SetOrientation(lineRenderer, orientation);

            return lineRenderer;
        }

        void SetOrientation(LineRenderer lineRenderer, Vector3 orientation)
        {
            lineRenderer.positionCount = 2;
            lineRenderer.SetPosition(0, new Vector3());
            lineRenderer.SetPosition(1, orientation);
        }

        

    }
}
