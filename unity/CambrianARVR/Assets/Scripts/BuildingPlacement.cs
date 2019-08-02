using System.Collections;
using System.Collections.Generic;
using Cambrian.iOS;
using Shaw.Data;
using UnityEngine;

public class BuildingPlacement : MonoBehaviour {

    public Transform m_HitTransform;
    private float m_elevation;
    private Vector3 m_worldOffset;
    const float EPSILON = 0.00001F;

	// Use this for initialization
	void Start () {
        m_elevation = -1.5F;

        CambrianARSessionNativeInterface.ARExecuteCommandEvent += HandleARExecuteCommand;
	    CambrianARSessionNativeInterface.ARPlaneAnchorAddedEvent += PlaneAnchorAddedEvent;
	}

    public void OnDestroy()
    {
        CambrianARSessionNativeInterface.ARExecuteCommandEvent -= HandleARExecuteCommand;
        CambrianARSessionNativeInterface.ARPlaneAnchorAddedEvent -= PlaneAnchorAddedEvent;
    }

    void PlaneAnchorAddedEvent(CBPlaneAnchor anchorData) {
        
        var position = TransformExtensions.FromCBMatrix(anchorData.transform).ExtractPosition();
        
        var elevation = position.y;

        if (elevation > -2 && (elevation < m_elevation || System.Math.Abs(m_elevation) < EPSILON)) {
            m_elevation = elevation;
            UpdatePosition();
        }
    }

    void HandleARExecuteCommand(CambrianCommand command)
    {
        if (command.commandString.Equals("updateLocation"))
        {
            var updateLocation = JsonUtility.FromJson<UpdateLocationCommand>(command.jsonString);

            if (updateLocation != null)
            {
                m_worldOffset = updateLocation.location.position;
                UpdatePosition();
            }
        }
    }
	
	void UpdatePosition() {
        m_HitTransform.position = new Vector3(m_worldOffset.x, 
                                              m_elevation + m_worldOffset.y - 0.16F, 
                                              m_worldOffset.z);
	}
}
