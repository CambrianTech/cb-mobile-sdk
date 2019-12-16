using UnityEngine;

namespace Cambrian.iOS
{
    public class PBRTexture : Object
    {
        public Texture2D diffuse = new Texture2D(2, 2);
        public Texture2D normals = new Texture2D(2, 2);
        public Texture2D roughness = new Texture2D(2, 2);

        public void Destroy()
        {
            Destroy(diffuse);
            Destroy(normals);
            Destroy(roughness);
        }
    }
}