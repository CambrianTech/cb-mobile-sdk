
using UnityEngine;
using System.Collections.Generic;
using System.Collections;
using System;
using System.Linq;
using Cambrian.iOS;
using System.Runtime.Serialization.Json;
using System.IO;
using Shaw.Data;
//using System.Collections.IEnumerable;
namespace Cambrian.Shaw
{
    public class ShawTile : MonoBehaviour
    {

        private Vector3 velocity = Vector3.zero;
        private Vector3 targetpt;
        private Quaternion targetrt;
        private Quaternion ort;

        bool ashlar;
        bool brick;
        bool herringbone;
        bool stagger;
        bool monolithic;

        public GameObject oddColumn;
        public GameObject evenColumn;
        public GameObject floor;
        private Vector3 oddcolumnpos;
        private Vector3[] oddcolumntilepos;

        private Vector3 slide1= new Vector3(.193f, 0, -.494f);
        private Vector3 currentPosition;
        private string currentMethod;

        // Use this for initialization
        void Start()
        {

            ashlar = true;
            brick = true;
            herringbone = true;
            stagger = true;
            monolithic = true;
            currentMethod = null;
            oddcolumnpos = oddColumn.transform.position;
            targetpt = oddcolumnpos;

            ort = floor.transform.rotation;
            currentPosition = floor.transform.position;
          
            var orends = floor.GetComponentsInChildren<MeshRenderer>();

            targetrt = Quaternion.identity;
            oddcolumntilepos = new Vector3[orends.Length];

            for (int i = 0; i < orends.Length; i++)
            {
                oddcolumntilepos[i] = orends[i].bounds.center;
            }

            var f = floor.transform.localRotation;
            float angf;
            Vector3 axf;
            f.ToAngleAxis(out angf, out axf);
            /// oddcolumnTranslate = true;
            //var a = 90 * (Mathf.Sign(45 - angf) - 1) / 2;
            targetrt = Quaternion.AngleAxis(90 * (Mathf.Sign(45 - angf) - 1) / 2, Vector3.up);

            PrepareTextures();
 
            CambrianARSessionNativeInterface.ARExecuteCommandEvent += HandleARExecuteCommand;
            
            CambrianARSessionNativeInterface.GetARSessionNativeInterface().NotifyReady(true);
        }

        protected void OnDestroy()
        {
            CambrianARSessionNativeInterface.ARExecuteCommandEvent -= HandleARExecuteCommand;
            
            CambrianARSessionNativeInterface.GetARSessionNativeInterface().NotifyReady(false);
        }

        void HandleARExecuteCommand(CambrianCommand command)
        {
            //Debug.Log($"Unity received command: {command.commandString} with data: {command.jsonString}");

            //MeshRenderer test;
            //test.sh
            if (command.commandString.Equals("updateMaterial")) {
                var materialCommand = JsonUtility.FromJson<UpdateMaterialCommand>(command.jsonString);

                if (materialCommand != null) {
                    //Debug.Log("Got materialCommand " + materialCommand.color.name + " with " + materialCommand.color.variations.Length + " variations");
                    if (materialCommand.color.variations.Length > 0) {
                        m_productColor = materialCommand.color;
                        m_variationIndex = 0;
                    }
                }
            }
            else if (command.commandString.Equals("updateInstallation"))
            {
                var updateInstallation = JsonUtility.FromJson<UpdateInstallationCommand>(command.jsonString);

                if (updateInstallation != null)
                {
                    //Debug.Log("Got updateInstallation " + updateInstallation.method.name);
                    UpdateInstallation(updateInstallation.method);
                }
            }
        }

        private ProductColor m_productColor = null;
        private int m_variationIndex = -1;

        List<Texture2D> m_diffuseTextures, m_normalTextures, m_roughnessTextures;
        void PrepareTextures() {

            m_productColor = null;
            m_variationIndex = -1;

            int maxVariations = 12;
            int size = 1024;
            m_diffuseTextures = new List<Texture2D>();
            m_normalTextures = new List<Texture2D>();
            m_roughnessTextures = new List<Texture2D>();

            for (int i = 0; i < maxVariations; i++)
            {
                m_diffuseTextures.Add(new Texture2D(size, size));
                m_normalTextures.Add(new Texture2D(size, size));
                m_roughnessTextures.Add(new Texture2D(size, size));
            }

            var random = new System.Random();
            var tileRenderers = floor.GetComponentsInChildren<MeshRenderer>();

            for (int i = 0; i < tileRenderers.Length; i++)
            {
                var j = random.Next(0, maxVariations - 1);
                tileRenderers[i].material.mainTexture = m_diffuseTextures[j];
                tileRenderers[i].material.SetTexture("_BumpMap", m_normalTextures[j]);
                tileRenderers[i].material.SetTexture("_Glossiness", m_roughnessTextures[j]);
            }
        }

        //probably should just thread this:
        int m_whichTextureType = 0;
		void UpdateTilesTextures(List<ProductVariation> variations)
		{
            var variation = variations[m_variationIndex % variations.Count()];

            if (m_whichTextureType == 0) {
                m_diffuseTextures[m_variationIndex].LoadImage(variation.GetDiffuse());
            } else if (m_whichTextureType == 1) {
                m_normalTextures[m_variationIndex].LoadImage(variation.GetNormal());
            } else if (m_whichTextureType == 2) {
                m_roughnessTextures[m_variationIndex].LoadImage(variation.GetRoughness());
            }

            m_whichTextureType++;

            if (m_whichTextureType == 3) {
                m_variationIndex ++;
                m_whichTextureType = 0;
            }

            if (m_variationIndex >= m_diffuseTextures.Count)
            {
                m_variationIndex = -1;
            }
		}

        void UpdateInstallation(InstallationMethod method) {
            switch (method.name)
            {
                case "ashlar": Ashlar(); break;
                case "brick": Brick(); break;
                case "monolithic": Monolithic(); break;
                case "herringbone": Herringbone(); break;
                case "stagger": Stagger(); break;
            }
            currentMethod = method.name;
        }

		// Update is called once per frame
		void Update()
        {
            if (Input.GetKeyDown(KeyCode.A))
            {
                Stagger();

            }
            if (Input.GetKeyDown(KeyCode.S))
            {
                Ashlar();

            }
            if (Input.GetKeyDown(KeyCode.D))
            {
                Brick();

            }
            if (Input.GetKeyDown(KeyCode.F))
            {
                Herringbone();

            }
            if (Input.GetKeyDown(KeyCode.G))
            {
                Monolithic();

            }
            
            if (m_variationIndex >= 0) {
                //few at a time
                var variations = m_productColor.GetLoadedVariations();
                if (variations.Count > 0)
                {
                    UpdateTilesTextures(variations);
                }
            }

            var currenty = floor.transform.position.y;

            var ycolfixed = oddcolumnpos;
            ycolfixed.y = currentPosition.y;
            oddcolumnpos = ycolfixed;

            for (int i = 0; i < oddcolumntilepos.Length; i++)
            {
                var yfixed = oddcolumntilepos[i];
                yfixed.y = currenty;
                oddcolumntilepos[i] = yfixed;
            }

            if (!monolithic)
            {
                oddColumn.transform.position = Vector3.SmoothDamp(oddColumn.transform.position, oddcolumnpos, ref velocity, .2f);
         
                if (Vector3.Magnitude(oddcolumnpos - oddColumn.transform.position) < .01f)
                {
                    oddColumn.transform.position = oddcolumnpos;
                    var tiles = floor.GetComponentsInChildren<MeshRenderer>();

                    for (int i = 0; i < tiles.Length; i++)
                    {
                        var yfixed = oddcolumntilepos[i];
                        yfixed.y = currentPosition.y;
                        tiles[i].transform.position = yfixed;
                    }
                    monolithic = true;
                    currentMethod = null;
                  
                }

            }
            //if (currentMethod == "stagger" )
            // {
            if (!stagger && monolithic)
            {
                float totalerror = 0;

                var tiles = floor.GetComponentsInChildren<MeshRenderer>();
                float xcoor = oddcolumntilepos[0].x;

                int j = 0;
                int k = 0;

                for (int i = 0; i < tiles.Length; i++)
                {
                    var yfixed = tiles[i].transform.position;
                    yfixed.y = currenty;
                    tiles[i].transform.position = yfixed;
                    if (oddcolumntilepos[i].z - floor.GetComponentInParent<Transform>().position.z < 32)
                    {
                        if (currentMethod == "stagger")
                        {
                            if (Math.Abs(xcoor - oddcolumntilepos[i].x) > .01f)
                                j++;
                            if (Math.Abs(xcoor - oddcolumntilepos[i].x) > 1.5f)
                            {
                                k = 1;
                                j = 0;
                            }

                            if (k == 0)
                            {
                                if (j % 3 == 0)
                                    j = 0;

                                if (j % 3 == 1)
                                    j = 1;

                                if (j % 3 == 2)
                                    j = -1;
                            }
                            else
                            {

                                if (j % 2 == 0)
                                    j = 2;

                                if (j % 2 == 1)
                                    j = 3;
                            }

                        }

                        tiles[i].transform.position = Vector3.SmoothDamp(tiles[i].transform.position, oddcolumntilepos[i] + new Vector3(0, 0, j * .4f), ref velocity, 2 * Time.smoothDeltaTime);
                        xcoor = oddcolumntilepos[i].x;
                        var err = Vector3.Magnitude((oddcolumntilepos[i] + new Vector3(0, 0, j * .4f)) - tiles[i].transform.position);
                        totalerror += err;
                        if (err < .02f)
                        {
                            tiles[i].transform.position = oddcolumntilepos[i] + new Vector3(0, 0, j * .4f);

                        }
                    }
                    else
                    {
                        
                        tiles[i].transform.position = oddcolumntilepos[i] -new Vector3(0, 0, .4f);
                    }

                }
                //Debug.Log(totalerror + "te");
                if (totalerror < .1f)
                {
                    stagger = true;
                    //Debug.Log("stagger done");
                    switch (currentMethod)
                    {
                        case "ashlar": ashlar = false; break;
                        case "brick": brick = false; break;
                        case "monolithic": monolithic = false; break;
                        case "herringbone": herringbone = false; break;
                    }
                }


            }
           



           // Debug.Log(currentMethod + " cm " + stagger + "s" + brick + "b");
            if (!brick&& monolithic&&ashlar)
            {
                var iquat = floor.transform.rotation;
                float iquatangle;
                Vector3 iquataxis;
                float tangle;
                Vector3 taxis;
                iquat.ToAngleAxis(out iquatangle, out iquataxis);
                targetrt.ToAngleAxis(out tangle, out taxis);

                int onoff = 1;

                if (currentMethod == "brick")
                {
                    onoff = 0;
                }

                targetpt = oddcolumnpos + new Vector3(1,0,0);

                if (Mathf.Abs(iquatangle - onoff * 90) > .1f)
                {
                    floor.transform.RotateAround(floor.GetComponentInParent<BoxCollider>().bounds.center , -1 * Mathf.Sign(onoff - .5f) * Vector3.up, onoff * 360 * Time.smoothDeltaTime * (1 - iquatangle / 90) + 5 * (1 - onoff) * Time.smoothDeltaTime * (iquatangle));
                    //Debug.Log("center" + floor.transform.TransformPoint(Vector3.zero));
                }
                else
                {
                    iquatangle = onoff * 90;
                    if (currentMethod != "brick")
                    {
                        floor.transform.rotation = ort;
                        oddColumn.transform.position = oddcolumnpos;
                 
                    }
                    else
                    {
                   
                        oddColumn.transform.position = oddColumn.transform.position + new Vector3Int(1, 0, 0);
                      //  Debug.Log("beforeafter" + );
                    }

                   // var ofpo
                   // ofpos.y = floor.transform.position.y;
                   // floor.transform.position = ofpos;
                        brick = true;
                    Debug.Log("brick done");
                  
                        switch (currentMethod)
                        {
                            case "ashlar": ashlar = false; break;
                            case "herringbone": herringbone = false; break;
                            case "monolithic": monolithic = false; break;
                            case "stagger": stagger = false; break;
                        }
               

                }

            }

            if (!ashlar&& monolithic)
            {


                if (currentMethod == "ashlar")
                {
                    targetpt = oddcolumnpos + 1.1f * Vector3.forward;

                }
                else
                {
                    targetpt = oddcolumnpos;
                }
                //  targetpt.y = floor.transform.position.y;


                oddColumn.transform.position = Vector3.SmoothDamp(oddColumn.transform.position, targetpt, ref velocity, .2f);

                if (Vector3.Magnitude(targetpt - oddColumn.transform.position) < .01f)
                {
                   targetpt.y = floor.transform.position.y;
                    //targetpt.x = oddcolumnpos.x;
                   oddColumn.transform.position = targetpt;

                 ashlar = true;
                    switch (currentMethod)
                    {
                        case "herringbone": herringbone = false; break;
                        case "brick": brick = false; break;
                        case "monolithic": monolithic = false; break;
                        case "stagger": stagger = false; break;
                    }
                }


            }


            if (!herringbone&&monolithic&&brick)
            {
                
                int onoff = 0;

                if (currentMethod == "herringbone")
                {
                    onoff = 1;
                }

              
                var tiles = floor.GetComponentsInChildren<MeshRenderer>();
              
                for (int i = 0; i < tiles.Length; i++)
                {
                  
                    var iquat = tiles[i].transform.rotation;
                    float iquatangle;
                    Vector3 iquataxis;
                    iquat.ToAngleAxis(out iquatangle, out iquataxis);


                    tiles[i].transform.RotateAround(tiles[i].transform.TransformPoint(Vector3.zero), Mathf.Sign(onoff - .5f) * Mathf.Sign(i % 2 - .5f) * Vector3.up, onoff * 180 * Time.smoothDeltaTime * (1 - iquatangle / 45) + 5 * (1 - onoff) * Time.smoothDeltaTime * (iquatangle));
                    if (Mathf.Abs(iquatangle - onoff * 45) > 5f)
                    {
                        tiles[i].transform.position = Vector3.SmoothDamp(tiles[i].transform.position, oddcolumntilepos[i] + onoff * .4f * (oddcolumntilepos[i] - floor.transform.TransformPoint(Vector3.zero)), ref velocity, Time.smoothDeltaTime);
  

                    }
                    else
                    {
                        var trans = oddcolumntilepos[i] + onoff * .4f * (oddcolumntilepos[i] - floor.transform.TransformPoint(Vector3.zero)) - floor.transform.TransformPoint(Vector3.zero);
                        var pos = oddcolumntilepos[i] + onoff * (.4f * (oddcolumntilepos[i] - floor.transform.TransformPoint(Vector3.zero)) + new Vector3(Mathf.Sign(i % 2 - .5f) * slide1.x, 0, slide1.z * trans.z));
                        if (Vector3.Magnitude(pos - tiles[i].transform.position) > .01f || Mathf.Abs(iquatangle - onoff * 45) > 1)
                        {
                            tiles[i].transform.position = Vector3.SmoothDamp(tiles[i].transform.position, pos, ref velocity, .5f*Time.smoothDeltaTime);
                        }
                        else
                        {
                            var axis = Mathf.Sign(onoff - .5f) * Mathf.Sign(i % 2 - .5f) * Vector3.up;

                            var quat = new Quaternion();
                            quat = Quaternion.AngleAxis(onoff * 45, axis);

                            tiles[i].transform.localRotation = quat;
                            tiles[i].transform.position = pos;
                          
                            herringbone = true;
                   
                            switch (currentMethod)
                            {
                                case "ashlar": ashlar = false; break;
                                case "brick": brick = false; break;
                                case "monolithic": monolithic = false; break;
                                case "stagger": stagger = false; break;
                            }
                        }
                    }
                }
            }

        }

        public void Ashlar()
        {
            currentPosition = floor.transform.position;
          
            Debug.Log("ashlar");
            switch (currentMethod)
            {
                case "herringbone": herringbone = false; currentMethod = "ashlar"; break;
                case "brick": brick = false; currentMethod = "ashlar"; break;
                case "monolithic": monolithic = false; currentMethod = "ashlar"; break;
                case "stagger": stagger = false; currentMethod = "ashlar"; break;
                case "ashlar":  ashlar = false; currentMethod = null; break;
                case null: oddcolumnpos = oddColumn.transform.position;ashlar = false; currentMethod = "ashlar"; break;
            }
        }



        public void Herringbone()
        {
            currentPosition = floor.transform.position;

            Debug.Log("herringbone");
            switch (currentMethod)
            {
                case "herringbone": herringbone = false; currentMethod = null; break;
                case "brick": brick = false; currentMethod = "herringbone"; break;
                case "monolithic": monolithic = false; currentMethod = "herringbone"; break;
                case "stagger": stagger = false; currentMethod = "herringbone"; break;
                case "ashlar": ashlar = false; currentMethod = "herringbone"; break;
                case null: herringbone = false; currentMethod = "herringbone"; break;
            }
           

        }

        public void Stagger()
        {
            currentPosition = floor.transform.position;
          
            Debug.Log("stagger");
            switch (currentMethod)
            {
                case "herringbone": herringbone = false; currentMethod = "stagger"; break;
                case "brick": brick = false; currentMethod = "stagger"; break;
                case "monolithic": monolithic = false; currentMethod = "stagger"; break;
                case "stagger": stagger = false; currentMethod = null; break;
                case "ashlar": ashlar = false; currentMethod = "stagger"; break;
                case null: stagger = false; currentMethod = "stagger"; break;
            }

        }




        void Monolithic()
        {

            Debug.Log("monolithic");
            switch (currentMethod)
            {
                case "herringbone": herringbone = false; currentMethod = "monolithic"; break;
                case "brick": brick = false; currentMethod = "monolithic"; break;
                case "monolithic": currentMethod = null; break;
                case "stagger": stagger = false; currentMethod = "monolithic"; break;
                case "ashlar": ashlar = false; currentMethod = "monolithic"; break;
                    //case null: monolithic = false; currentMethod = "monolithic"; break;
            }


        }

        public void Brick()
        {
            Debug.Log("brick");
          
                switch (currentMethod)
                {
                    case "herringbone": herringbone = false; currentMethod = "brick"; break;
                    case "brick": brick = false; currentMethod = null; break;
                    case "monolithic": monolithic = false; currentMethod = "brick"; break;
                    case "stagger": stagger = false; currentMethod = "brick"; break;
                    case "ashlar": ashlar = false; currentMethod = "brick"; break;
                    case null: brick = false; currentMethod = "brick"; break;
                }


        }



    }
}