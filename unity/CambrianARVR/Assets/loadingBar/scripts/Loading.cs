using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using System.Collections.Generic;

namespace Cambrian.iOS
{
    public class Loading : MonoBehaviour
    {
        private RectTransform rectComponent;
        private Image imageComp;
        public Image imageComp1;
        public Text text;
        private bool up;
        public bool m_ready=false;
        public bool m_loaded=false;

        public float rotateSpeed = 200f;
        public float openSpeed = .005f;
        public float closeSpeed = .01f;

        public void Start()
        {
            rectComponent = GetComponent<RectTransform>();
            imageComp = rectComponent.GetComponent<Image>();
            up = true;
            m_ready = false;
            m_loaded = false;
            
            Reset();
            CambrianARMaskIntegrator.CBSurfaceDataUpdatedEvent += CBSurfaceDataProvided;
        }

        private void OnDestroy()
        {
            CambrianARMaskIntegrator.CBSurfaceDataUpdatedEvent -= CBSurfaceDataProvided;
            Reset();
        }

        private void Reset()
        {
            m_ready = false;
            text.text = "Scanning Space";
            imageComp.CrossFadeAlpha(1, 0.0f, false);
            imageComp1.CrossFadeAlpha(1, 0.0f, false);
            text.CrossFadeAlpha(1, 0.0f, false);
        }

        private void CBSurfaceDataProvided(CBSurfaceAssetParams info, CBSurfaceData data)
        {
            m_ready = true;
        }

        private void Update()
        {
           // if (Input.GetMouseButtonDown(0))
           //     Reset();
            if (m_loaded) return;

            rectComponent.Rotate(0f, 0f, rotateSpeed * Time.deltaTime);
            changeSize();
            if (m_ready)
            {
                m_loaded = true;
                Debug.Log("Fading out 'Ready for AR'");
                text.text = "Ready for AR";
                imageComp.CrossFadeAlpha(0, 1.0f, false);
                imageComp1.CrossFadeAlpha(0, 1.0f, false);
                text.CrossFadeAlpha(0, 2.0f, false);
            }
        }

        private void changeSize()
        {
            float currentSize = imageComp.fillAmount;

            if (currentSize < .30f && up)
            {
                imageComp.fillAmount += openSpeed;
            }
            else if (currentSize >= .30f && up)
            {
                up = false;
            }
            else if (currentSize >= .02f && !up)
            {
                imageComp.fillAmount -= closeSpeed;
            }
            else if (currentSize < .02f && !up)
            {
                up = true;
            }
        }
    }
}