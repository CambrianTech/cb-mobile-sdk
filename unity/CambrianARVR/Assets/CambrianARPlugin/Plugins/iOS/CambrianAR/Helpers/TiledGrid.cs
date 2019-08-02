using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace Cambrian.iOS
{
    public class TiledGrid : MonoBehaviour
    {
        public enum TiledGridType
        {
            Monolithic,
            Ashlar,
            Stagger,
            Herringbone,
            Brick
        }
        
        private List<GameObject> _tiles;
        private Dictionary<int, PBRTexture> _materials;
        private readonly object _materialsLock = new object();

        protected void GetGridRange(TiledGridType type, Bounds bounds, Vector2 tileSize, out int gridStartX, out int gridStartY, out int gridEndX, out int gridEndY)
        {
            switch (type)
            {
                case TiledGridType.Brick:
                    gridStartX = Mathf.FloorToInt(bounds.min.x / tileSize.y);
                    gridEndX = Mathf.CeilToInt(bounds.max.x / tileSize.y);
            
                    gridStartY = Mathf.FloorToInt(bounds.min.z / tileSize.x);
                    gridEndY = Mathf.CeilToInt(bounds.max.z / tileSize.x);
                    break;
                
                case TiledGridType.Ashlar:
                case TiledGridType.Stagger:
                case TiledGridType.Herringbone:
                case TiledGridType.Monolithic:

                default:
                    gridStartX = Mathf.FloorToInt(bounds.min.x / tileSize.x);
                    gridEndX = Mathf.CeilToInt(bounds.max.x / tileSize.x);
            
                    gridStartY = Mathf.FloorToInt(bounds.min.z / tileSize.y);
                    gridEndY = Mathf.CeilToInt(bounds.max.z / tileSize.y);
                    break;
            }
            
        }

        protected void GetPositionAndRotation(TiledGridType type, int x, int y, Vector2 tileSize, out Vector2 position, out float angle)
        {
            switch (type)
            {

                case TiledGridType.Ashlar:
                {
                    var yOffset = x % 2 == 0 ? 0.0f : -tileSize.y / 2.0f;
                    position = new Vector2(x * tileSize.x, y * tileSize.y + yOffset);
                    angle = 0.0f;
                    break;
                }
                case TiledGridType.Stagger:
                {
                    var yOffset = 0.0f;
                    switch (Math.Abs(x) % 5)
                    {
                        case 1:
                            yOffset = -tileSize.y / 5.0f;
                            break;
                        case 2:
                            yOffset = -2.0f * tileSize.y / 5.0f;
                            break;
                        case 3:
                            yOffset = -3.0f * tileSize.y / 5.0f;
                            break;
                        case 4:
                            yOffset = tileSize.y / 5.0f;
                            break;
                        default:
                            yOffset = 0.0f;
                            break;
                    }
                    position = new Vector2(x * tileSize.x, y * tileSize.y + yOffset);
                    angle = 0.0f;
                    break;
                }
                case TiledGridType.Brick:
                {
                    var xOffset = y % 2 == 0 ? 0.0f : tileSize.y / 2.0f;
                    position = new Vector2(x * tileSize.y + xOffset, y * tileSize.x);
                    angle = 90.0f;
                    break;
                }
                
                case TiledGridType.Herringbone:
                case TiledGridType.Monolithic:

                default:
                {
                    position = new Vector2(x * tileSize.x, y * tileSize.y);
                    angle = 0.0f;
                    break;
                }
            }
            
            
        }

        public void UpdateGrid(GameObject prefab, Bounds bounds, Vector2 tileSize, TiledGridType type)
        {
            var existingTiles = GetTiles();
            int gridStartX, gridEndX, gridStartY, gridEndY;

            GetGridRange(type, bounds, tileSize, out  gridStartX, out  gridStartY, out  gridEndX, out gridEndY);

            //Debug.Log($"Tile size:({tileSize}), Grid with bounds ({bounds.min.x}, {bounds.min.z}, {bounds.max.x}, {bounds.max.z}) goes from ({gridStartX},{gridStartY}) => ({gridEndX},{gridEndY})");

            var tileIndex = 0;
            for (var y = gridStartY; y < gridEndY; y++)
            {
                for (var x = gridStartX; x < gridEndX; x++, tileIndex++)
                {
                    Vector2 positionXY;
                    float angle;
                    GetPositionAndRotation(type, x, y, tileSize, out positionXY, out angle);

                    var rotation = Quaternion.Euler(0, angle, 0);
                    
                    GameObject tile;
                    if (tileIndex < existingTiles.Count)
                    {
                        tile = existingTiles[tileIndex];
                        tile.transform.SetPositionAndRotation(new Vector3(positionXY.x, tile.transform.position.y, positionXY.y), rotation);
                    }
                    else
                    {
                        tile = GenerateTile(prefab, new Vector3(positionXY.x, 0.0f, positionXY.y), rotation);
                    }
                    
                    //Debug.Log($"Would generate tile at {position}");
                    
                    tile.transform.localScale = new Vector3(tileSize.x, 1.0f, tileSize.y);
                }
            }

            //remove unused
            RemoveUnused(tileIndex);
        }

        protected List<GameObject> GetTiles()
        {
            if (_tiles == null)
            {
                _tiles = new List<GameObject>();
            }

            return _tiles;
        }

        protected void RemoveUnused(int startIndex)
        {
            if (startIndex >= _tiles.Count) return;
            
            var toRemove = _tiles.GetRange(startIndex, _tiles.Count - startIndex);
            foreach (var tile in toRemove)
            {
                tile.SetActive(false);
            }
            
            _tiles = _tiles.GetRange(0, startIndex);
        }

        protected GameObject GenerateTile(GameObject prefab, Vector3 position, Quaternion rotation)
        {
            var tiles = GetTiles();
            
            var tile = Instantiate(prefab, position, rotation);
            tile.transform.SetParent(gameObject.transform, false);

            lock (_materialsLock)
            {
                if (_materials != null && _materials.Count > 0)
                {
                    UpdateTileTexture(tile, _materials.Values.First());
                }
            }
            
            //Debug.Log($"Generating tile at {position}");
            tiles.Add(tile);
            
            return tile;
        }

        public void UpdateSurface(Texture2D maskTexture2D, Vector4 maskBounds, Texture2D shadowTexture2D, Vector4 shadowBounds)
        {
            foreach (var tile in GetTiles())
            {
                var rend = tile.GetComponentsInChildren<Renderer>(true)[0];

                if (maskTexture2D)
                {
                    rend.material.SetTexture("_MaskY", maskTexture2D);
                    rend.material.SetVector("_MaskBounds", maskBounds);   
                }
                
                if (shadowTexture2D)
                {
                    rend.material.SetTexture("_ShadowsY", shadowTexture2D);
                    rend.material.SetVector("_ShadowBounds", shadowBounds);   
                }
            }
            
        }

        public int NumVariations { get; set; } = 1;

        public void UpdateMaterials(PBRTexture texture, int index=0)
        {
            PBRTexture existing = null;
            lock (_materialsLock)
            {
                if (_materials == null)
                {
                    _materials = new Dictionary<int, PBRTexture>();
                }
                
                if (_materials.ContainsKey(index))
                {
                    existing = _materials[index];
                }
                _materials[index] = texture;
            }

            var tiles = GetTiles();
            for (var i = index; i < tiles.Count; i += NumVariations)
            {
                UpdateTileTexture(tiles[i], texture);
            }

            if (existing != null)
            {
                existing.Destroy();
            }
        }

        private void UpdateTileTexture(GameObject tile, PBRTexture texture)
        {
            var oRend = tile.GetComponentsInChildren<Renderer>(true)[0];
                
            oRend.material.mainTexture = texture.diffuse;
            oRend.material.SetTexture("_BumpMap", texture.normals);
            oRend.material.SetTexture("_Roughness", texture.roughness);
        }
    }
}