// Upgrade NOTE: replaced '_Object2World' with 'unity_ObjectToWorld'

// Unity built-in shader source. Copyright (c) 2016 Unity Technologies. MIT license (see license.txt)

// Simplified Bumped Specular shader. Differences from regular Bumped Specular one:
// - no Main Color nor Specular Color
// - specular lighting directions are approximated per vertex
// - writes zero to alpha channel
// - Normalmap uses Tiling/Offset of the Base texture
// - no Deferred Lighting support
// - no Lightmap support
// - supports ONLY 1 directional light. Other lights are completely ignored.

Shader "Cambrian Shadow Surface (One Light)" {
Properties {
    [PowerSlider(5.0)] _Shininess ("Shininess", Range (0.03, 1)) = 0.078125
    _MainTex ("Base (RGB) Gloss (A)", 2D) = "white" {}
    [NoScaleOffset] _BumpMap ("Normalmap", 2D) = "bump" {}
    
    [MaterialToggle] _isPBR("isPBR", Float) = 0
    
    _Roughness("Roughness", Range(0.0, 1.0)) = 0.5
    _RoughnessMap("Roughness Map", 2D) = "white" {}
    
    _Shadows("Shadow Contribution", Range(0.0, 1.0)) = 0.5
    _Highlights("Glossiness", Range(0.0, 1.0)) = 0.5
    
    [HideInInspector] _ShadowsY("Shadows and Highlights", 2D) = "gray" {}
    [HideInInspector] _MaskY("Occlusion Mask", 2D) = "white" {}
}
SubShader {
    Tags { "RenderType" = "Opaque" }
    LOD 250
    Cull off

CGPROGRAM
#pragma target 3.0
#pragma surface surf StandardSpecular fullforwardshadows alpha:blend vertex:vert

inline fixed4 LightingMobileBlinnPhong (SurfaceOutput s, fixed3 lightDir, fixed3 halfDir, fixed atten)
{
    fixed diff = max (0, dot (s.Normal, lightDir));
    fixed nh = max (0, dot (s.Normal, halfDir));
    fixed spec = pow (nh, s.Specular*128) * s.Gloss;

    fixed4 c;
    c.rgb = (s.Albedo * _LightColor0.rgb * diff + _LightColor0.rgb * spec) * atten;
    UNITY_OPAQUE_ALPHA(c.a);
    return c;
}

float4x4 _screenToVideoTransform;
                    
sampler2D _MainTex;
sampler2D _BumpMap;
half _Shininess;

sampler2D _RoughnessMap;
half _Roughness;

sampler2D _ShadowsY;
half _Shadows;
half _Highlights;

// World-bounds of the mask and shadows described
// by its minimum point (xy) and size (zw).
float4 _MaskBounds;
float4 _ShadowBounds;

sampler2D _MaskY;

float _isPBR;

struct Input {
    float2 uv_MainTex;
    float2 maskCoords;
    float2 shadowCoords;
};

void vert (inout appdata_full v, out Input o) 
{
    UNITY_INITIALIZE_OUTPUT(Input,o);

    o.maskCoords = (mul(unity_ObjectToWorld, v.vertex).xz - _MaskBounds.xy) / _MaskBounds.zw;
    o.shadowCoords = (mul(unity_ObjectToWorld, v.vertex).xz - _ShadowBounds.xy) / _ShadowBounds.zw;
}

void surf (Input IN, inout SurfaceOutputStandardSpecular o) {

    fixed4 tex = tex2D(_MainTex, IN.uv_MainTex);
    float maskValue = tex2D(_MaskY, IN.maskCoords).r;
    
    o.Alpha = maskValue;
    
    if (_isPBR == 1.0) {
        //float roughness = _Roughness * tex2D(_RoughnessMap, IN.uv_MainTex).r;
        float shadowsHighlights = tex2D(_ShadowsY, IN.shadowCoords).r;
        shadowsHighlights = shadowsHighlights * 2.0 - 0.2;
        o.Albedo = lerp(tex.rgb, tex.rgb * shadowsHighlights, _Shadows);
        o.Normal = UnpackNormal (tex2D(_BumpMap, IN.uv_MainTex));
    } else {
        o.Albedo = tex.rgb;
        //o.Normal = float3(0.5,0.5,1.0);
    }
}
ENDCG
}

FallBack "Mobile/VertexLit"
}
