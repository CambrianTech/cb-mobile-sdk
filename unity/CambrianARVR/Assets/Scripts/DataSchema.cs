using UnityEngine;
using System.IO;
using System;
using System.Collections.Generic;

namespace Shaw.Data
{
    class Utility {
        public static byte[] LoadImage(string filePath)
        {
            if (File.Exists(filePath))
            {
                return File.ReadAllBytes(filePath);
            }
            Debug.Log($"{filePath} is an invalid image");
            return new byte[] {};
        }
    }

    [Serializable]
    public class Product
    {
        public string name;
        public float width;
        public float height;

        public Vector2 sizeMeters()
        {
            return new Vector2(0.0254f * width, 0.0254f * height);
        }
    }

    [Serializable]
    public class ProductColor
    {
        public string name;
        public ProductVariation[] variations;

        public List<ProductVariation> GetLoadedVariations()
        {
            var variationList = new List<ProductVariation>();
            foreach (var variation in variations)
            {
                if (!variation.IsEmpty())
                {
                    variationList.Add(variation);
                }
            }
            return variationList;
        }
    }

    [Serializable]
    public class ProductVariation
    {
        public string name;
        public string diffusePath;
        public string normalsPath;
        public string roughnessPath;

        public bool LoadAll()
        {
            GetDiffuse();
            if (IsPBR())
            {
                GetNormal();
                GetRoughness();
                return _diffuse != null && _diffuse.Length > 0 &&
                   _normal != null && _normal.Length > 0 &&
                   _roughness != null && _roughness.Length > 0;
            }

            return _diffuse != null && _diffuse.Length > 0;
        }

        public bool IsPBR()
        {
            return normalsPath.Length > 0;
        }

        private byte[] _diffuse = {};
        public byte[] GetDiffuse() {
            if (_diffuse.Length == 0 && diffusePath.Length > 0) {
                _diffuse = Utility.LoadImage(diffusePath);
            }
            return _diffuse;
        }

        private byte[] _normal = {};
        public byte[] GetNormal()
        {
            if (_normal.Length == 0 && normalsPath.Length > 0)
            {
                _normal = Utility.LoadImage(normalsPath);
            }
            return _normal;
        }

        private byte[] _roughness = {};
        public byte[] GetRoughness()
        {
            if (_roughness.Length == 0 && roughnessPath.Length > 0)
            {
                _roughness = Utility.LoadImage(roughnessPath);
            }
            return _roughness;
        }

        public bool IsEmpty()
        {
            if (IsPBR())
            {
                return !(File.Exists(diffusePath) && File.Exists(normalsPath) && File.Exists(roughnessPath));
            }

            return !File.Exists(diffusePath);
        }

        public void Destroy()
        {
            _diffuse = new byte[] {};
            _normal = new byte[] {};
            _roughness = new byte[] {};
        }
    }

    [Serializable]
    public class InstallationMethod
    {
        public string name;
    }

    [Serializable]
    public class SceneLocation
    {
        public string name;
        public float[] worldPosition;
        public Vector3 position {
            get {
                Vector3 trans = new Vector3();
                for (int i = 0; i < worldPosition.Length; i++) {
                    trans[i] = worldPosition[i];
                }
                return trans;
            }
        }
    }

    [Serializable]
    public class UpdateMaterialCommand
    {
        public Product product;
        public ProductColor color;
    }

    [Serializable]
    public class UpdateInstallationCommand
    {
        public Product product;
        public InstallationMethod method;
    }

    [Serializable]
    public class UpdateLocationCommand
    {
        public SceneLocation location;
    }
}