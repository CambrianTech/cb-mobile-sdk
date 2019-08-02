using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace Cambrian.iOS
{
    public class SceneLoader : MonoBehaviour
    {

        // Use this for initialization
        void Start()
        {
            string sceneName = CambrianARSessionNativeInterface.getSceneName();
            Debug.Log("SessionStarted: " + sceneName);

            SceneManager.LoadScene(sceneName, LoadSceneMode.Single);
        }

    }
}
