Shader "Cambrian/DrawingShader"
{
	Properties
	{
		_MainTex ("Texture", 2D) = "black" {}
	}
	SubShader
	{
		Tags {"Queue"="Transparent" "RenderType"="Transparent" }
		LOD 100
		
		ZWrite Off
        Blend SrcAlpha OneMinusSrcAlpha

		Pass
		{
			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag
			
			#include "UnityCG.cginc"
			#include "CambrianCommon.cginc"

			struct Vertex
			{
				float4 position : POSITION;
				float2 texcoord : TEXCOORD0;
			};

			struct TexCoordInOut
			{
				float4 position : SV_POSITION;
				float2 texcoord : TEXCOORD0;
			};

			TexCoordInOut vert (Vertex vertex)
			{
				TexCoordInOut o;
				o.position = UnityObjectToClipPos(vertex.position); 

				float texX = vertex.texcoord.x;
				float texY = vertex.texcoord.y;
				
				o.texcoord.x = (m_displayTransform[0].x * texX + m_displayTransform[1].x * (texY) + m_displayTransform[2].x);
 			 	o.texcoord.y = (m_displayTransform[0].y * texX + m_displayTransform[1].y * (texY) + (m_displayTransform[2].y));
	            
				return o;
			}

			sampler2D _MainTex;
			float4 _MainTex_ST;
			float4 m_color;
			
			fixed4 frag (TexCoordInOut i) : SV_Target
			{
				// sample the texture
                float2 texcoord = i.texcoord;
                float2 minCoord = (m_isStillMode == 1.0) ? float2(0.0, -1.0) : float2(-1.0, 0.0);
                float2 maxCoord = (m_isStillMode == 1.0) ? float2(1.0, 0.0) : float2(0.0, 1.0);
                
                fixed4 baseColor;
                if (texcoord.x > maxCoord.x || texcoord.x < minCoord.x || texcoord.y > maxCoord.y || texcoord.y < minCoord.y) {
                    baseColor = fixed4(0.0, 0.0, 0.0, 0.0);
                } else {
                    baseColor = m_color;
                    baseColor.a = tex2D(_MainTex, texcoord).r;
                }
 
                return baseColor;
			}
			
			ENDCG
		}
	}
}
