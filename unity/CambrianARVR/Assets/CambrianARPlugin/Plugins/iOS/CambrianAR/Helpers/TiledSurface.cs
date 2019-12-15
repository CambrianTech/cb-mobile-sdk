
using System.Collections.Generic;
using UnityEngine;
using System.Collections;
using System;
using System.Collections.Concurrent;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using Shaw.Data;
using Unity.Collections;
using UnityEngine.Rendering;
using Object = System.Object;

namespace Cambrian.iOS
{
    public class TiledSurface : MonoBehaviour
    {
        /// <summary>
        /// Prefab for the floor tile when generating floors.
        /// </summary>
        public GameObject MaskedTilePrefab;

        public int MaxVariations = 5;

        private TiledGrid.TiledGridType _tileType = TiledGrid.TiledGridType.Monolithic;
        private Vector2 _tileSize = new Vector2(0.4572F, 1.8288F); //18x72in, avg size
        private Texture2D _shadowsYTexture;

        private CBSurfaceTextureData _surfaceTextureInfo;  
        private Texture2D _maskTextureY;

        private List<PBRTexture> _textures;

        /// <summary>
        /// Queue that executes its actions in the Update() function on
        /// the Unity thread.
        /// </summary>
        private ConcurrentQueue<Action> _mainQueue;

        private TiledGrid _surface;


        private QueueThread _bgQueueThread;
       

        // Use this for initialization
        void Start()
        {
            Reset();
                        
            _bgQueueThread.Start();

            CambrianARSessionNativeInterface.ARSessionStoppedEvent += CambrianArSessionNativeInterfaceOnArSessionStoppedEvent;         
            CambrianARMaskIntegrator.CBSurfaceCreatedEvent += CBSurfaceCreated;
            CambrianARMaskIntegrator.CBSurfaceDataUpdatedEvent += CBSurfaceDataUpdated;
            CambrianARSessionNativeInterface.ARExecuteCommandEvent += HandleARExecuteCommand;
            
            CambrianARSessionNativeInterface.GetARSessionNativeInterface().NotifyReady(true);
        }

        private void OnDestroy() {

            Reset();
            
            _bgQueueThread.Stop();

            CambrianARSessionNativeInterface.ARSessionStoppedEvent -= CambrianArSessionNativeInterfaceOnArSessionStoppedEvent;
            CambrianARMaskIntegrator.CBSurfaceCreatedEvent -= CBSurfaceCreated;
            CambrianARMaskIntegrator.CBSurfaceDataUpdatedEvent -= CBSurfaceDataUpdated;
            CambrianARSessionNativeInterface.ARExecuteCommandEvent -= HandleARExecuteCommand;
            
            CambrianARSessionNativeInterface.GetARSessionNativeInterface().NotifyReady(false);
         }
        

        private void Reset()
        {

            //Texture2D must be created and destroyed on main thread
            if (_textures != null)
            {
                //memory cleanup
                foreach (var tex in _textures)
                {
                    tex.Destroy();
                }
            }
            _textures = new List<PBRTexture>();
            for (int i = 0; i < MaxVariations; i++)
            {
                _textures.Add(new PBRTexture());
            }
            
            _mainQueue = new ConcurrentQueue<Action>();
            _surface = gameObject.AddComponent<TiledGrid>();
            gameObject.GetComponent<TiledGrid>().transform.SetParent(gameObject.transform, false);

            _bgQueueThread = new QueueThread();
            
            GetComponent<MeshFilter>().mesh = new Mesh();
            GetComponent<MeshFilter>().sharedMesh = new Mesh();//empty to start
        }
        
        private void CambrianArSessionNativeInterfaceOnArSessionStoppedEvent(CambrianARSessionState sessionstate) {

            Reset();
        }
        
        private void CBSurfaceCreated(CBSurfaceTextureData textureInfo) {
            _surfaceTextureInfo = textureInfo;
        }
        
        private void CBSurfaceDataUpdated(CBSurfaceAssetParams info, CBSurfaceData surfaceData)
        {
            if (_surfaceTextureInfo == null) return;
            
            //m_queue.Enqueue();
            _mainQueue.Enqueue(() =>
            {            
                transform.position = new Vector3(transform.position.x, surfaceData.extents.center.y, transform.position.z);
         
                if (surfaceData.maskBytes.Length > 0)
                {
                    UpdateMesh(surfaceData);
                }                
            });
        }

        private void Update()
        {                
            Action action;
            if (_mainQueue.TryDequeue(out action))
            {
                action();
            }
        }

        private Material GetMaterial()
        {
            return GetComponent<Renderer>().material;
        }
        
        private Vector2 Rotate2d(Vector2 inPoint, double theta)
        {
            Vector2 outPoint;
            //CW rotation
            outPoint.x = (float) Math.Cos(theta) * inPoint.x - (float) Math.Sin(theta) * inPoint.y;
            outPoint.y = (float) Math.Sin(theta) * inPoint.x + (float) Math.Cos(theta) * inPoint.y;
            return outPoint;
        }
        
        /// <summary>
        /// Converts a Vector4 to bounds assuming the x, y is the center and z, w is the size.
        /// </summary>
        private static Bounds BoundsFromVector4(Vector4 v)
        {
            var size = new Vector3(v.z, 1f, v.w);
            var center = new Vector3(v.x, -0.5f, v.y) + size / 2;
            return new Bounds(center, size);
        }

        /// <summary>
        /// Converts bounds to a Vector4 assuming the x, y is the minimum point and z, w is the size.
        /// </summary>
        private static Vector4 Vector4FromBounds(Bounds b)
        {
            return new Vector4(b.min.x, b.min.z, b.size.x, b.size.z);
        }

        private Bounds _gridBounds;
        
        private void UpdateMesh(CBSurfaceData surfaceData)
        {
            bool hasMaskData = false;
            
            float c = 1.0f;
            float s = 0.0f;
            float theta = 0.0f;
            
            if (surfaceData.extents.rotation2D.magnitude > 0.1f)
            {
                //rotate surface uvs
                Vector2 forward = new Vector2(0.0f,1.0f);
                c = Vector2.Dot(surfaceData.extents.rotation2D, forward);
                theta = (float) Math.Acos(c);
                s = (float) Math.Sin(theta);

                transform.rotation = Quaternion.AngleAxis(theta * Mathf.Rad2Deg, Vector3.up);
            }
            
            if (_surfaceTextureInfo.maskTexture != null && surfaceData.maskBytes.Length > 0)
            {
                _surfaceTextureInfo.maskTexture.LoadRawTextureData(surfaceData.maskBytes);
                _surfaceTextureInfo.maskTexture.Apply();
                hasMaskData = true;
            }
            
            bool hasShadowData = false;
            if (_surfaceTextureInfo.shadowsTexture != null && surfaceData.shadowBytes.Length > 0)
            {
                _surfaceTextureInfo.shadowsTexture.LoadRawTextureData(surfaceData.shadowBytes);
                _surfaceTextureInfo.shadowsTexture.Apply();
                hasShadowData = true;
            }
            
            //TODO: should use theta rotation instead
            _gridBounds = BoundsFromVector4(surfaceData.extents.maskExtents3D);
            var maxSize = Math.Sqrt(_gridBounds.size.x * _gridBounds.size.x + _gridBounds.size.z * _gridBounds.size.z);
            _gridBounds.size = new Vector3((float)maxSize, 0.0f, (float)maxSize);
            
            UpdateGridLayout();
      
            _surface.UpdateSurface(
                hasMaskData ? _surfaceTextureInfo.maskTexture : null, 
                surfaceData.extents.maskExtents3D, 
                hasShadowData ? _surfaceTextureInfo.shadowsTexture : null, 
                surfaceData.extents.shadowsExtents3D);
        }

        void UpdateGridLayout()
        {            
            _surface.UpdateGrid(MaskedTilePrefab, _gridBounds, _tileSize, _tileType);
        }
      
        void HandleARExecuteCommand(CambrianCommand command)
        {
            Debug.Log($"UNITY received command: {command.commandString} with data: {command.jsonString}");

            //MeshRenderer test;
            //test.sh
            if (command.commandString.Equals("updateMaterial"))
            {
                var materialCommand = JsonUtility.FromJson<UpdateMaterialCommand>(command.jsonString);

                if (materialCommand != null && materialCommand.color != null)
                {
                    _tileSize = materialCommand.product.sizeMeters();
                    var variations = materialCommand.color.GetLoadedVariations();
                    
                    _surface.NumVariations = Math.Min(variations.Count, MaxVariations);
                    
                    Debug.Log($"Got {variations.Count} variations");
                    //break it down into manageable chunks
                    
                    for (var i=0; i<_surface.NumVariations; i++)
                    {          
                        var index = i;
                    
                        _mainQueue.Enqueue(() =>
                        {
                            var variation = variations[index];
                            
                            //must be on main thread
                            _textures[index].diffuse.LoadImage(variation.GetDiffuse());
                            if (variation.IsPBR())
                            {
                                _textures[index].normals.LoadImage(variation.GetNormal());
                                _textures[index].roughness.LoadImage(variation.GetRoughness());
                            }
                            variation.Destroy();
                            
                            _surface.UpdateMaterials(_textures[index], index);
                        });
                    }
                    
                    _mainQueue.Enqueue(UpdateGridLayout);
                }
            } 
            else if (command.commandString.Equals("updateInstallation"))
            {
                var installationCommand = JsonUtility.FromJson<UpdateInstallationCommand>(command.jsonString);

                var existing = _tileType;
                switch (installationCommand.method.name)
                {
                    case "ashlar": _tileType = TiledGrid.TiledGridType.Ashlar; break;
                    case "brick": _tileType = TiledGrid.TiledGridType.Brick; break;
                    case "monolithic": _tileType = TiledGrid.TiledGridType.Monolithic; break;
                    case "herringbone": _tileType = TiledGrid.TiledGridType.Herringbone; break;
                    case "stagger": _tileType = TiledGrid.TiledGridType.Stagger; break;
                }

                if (existing != _tileType)
                {
                    _mainQueue.Enqueue(UpdateGridLayout);
                }
            }

        }
      
    }
}




