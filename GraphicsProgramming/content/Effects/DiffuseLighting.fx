#include "include\\Common.fxh"

/************* Resources *************/

cbuffer CBufferPerFrame
{
    float4 AmbientColor : AMBIENT <
		string UIName =  "Ambient Light";
		string UIWidget = "Color";
	> = { 1.0f, 1.0f, 1.0f, 0.0f };

    float4 LightColor : COLOR <
		string Object = "LightColor0";
		string UIName =  "Light Color";
		string UIWidget = "Color";
	> = { 1.0f, 1.0f, 1.0f, 1.0f };

    float3 LightDirection : DIRECTION <
		string Object = "DirectionalLight0";
		string UIName =  "Light Direction";
		string Space = "World";
	> = { 0.0f, 0.0f, -1.0f };
}

cbuffer CBufferPerObject
{
    float4x4 WorldViewProjection : WORLDVIEWPROJECTION < string UIWidget="None"; >;
    float4x4 World : WORLD < string UIWidget="None"; >;
}

Texture2D ColorTexture <
    string ResourceName = "default_color.dds";
    string UIName =  "Color Texture";
    string ResourceType = "2D";
>;

SamplerState ColorSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

/************* Data Structures *************/

struct VS_INPUT
{
    float4 ObjectPosition : POSITION;
    float2 TextureCoordinate : TEXCOORD;
    float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 TextureCoordinate : TEXCOORD0;
    float3 LightDirection : TEXCOORD1;
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
//add in the vertex shader code here
    VS_OUTPUT Out = (VS_OUTPUT) 0;
    Out.Position = mul(IN.ObjectPosition, WorldViewProjection);
    Out.TextureCoordinate = get_corrected_texture_coordinate(IN.TextureCoordinate);
    Out.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);
    Out.LightDirection = normalize(-LightDirection);
    
    return Out;
	

}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN) : SV_Target
{
	//add in the pixel shader code here
    float4 Out = (float4) 0;
    float3 normal = normalize(IN.Normal);
    float3 lightDir = normalize(IN.LightDirection);
    float n_dot_1 = dot(lightDir, normal);
    float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
    float3 ambient = AmbientColor.rgb * AmbientColor.a * color.rgb;
    float3 diffuse = (float3) 0;
    if (n_dot_1 > 0)
    {
        diffuse = LightColor * LightColor.a * n_dot_1 * color.rgb;
    }
    Out.rgb = ambient + diffuse;
    Out.a = color.a;
    return Out;
}

/************* Techniques *************/

technique11 main11
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, pixel_shader()));
    }
}