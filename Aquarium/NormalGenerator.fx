Texture2D<float4>    txPos  : register(t0);

cbuffer cbPerCall : register(b0){
	int2 widthHeight;
	int2 NIU;
};
//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct GS_INPUT
{
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};
//--------------------------------------------------------------------------------------
// Vertex Shader for every filter
//--------------------------------------------------------------------------------------
GS_INPUT VS( )
{
    GS_INPUT output = (GS_INPUT)0;
 
    return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader for every filter
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GS(point GS_INPUT particles[1], inout TriangleStream<PS_INPUT> triStream)
{
   PS_INPUT output;
	output.Pos=float4(-1.0f,1.0f,0.0f,1.0f);
	output.Tex=float2(0.0f,0.0f);
	triStream.Append(output);

	output.Pos=float4(1.0f,1.0f,0.0f,1.0f);
	output.Tex=float2(widthHeight.x,0.0f);
	triStream.Append(output);

	output.Pos=float4(-1.0f,-1.0f,0.0f,1.0f);
	output.Tex=float2(0.0f,widthHeight.y);
	triStream.Append(output);

	output.Pos=float4(1.0f,-1.0f,0.0f,1.0f);
	output.Tex=float2(widthHeight);
	triStream.Append(output);
}

//--------------------------------------------------------------------------------------
// Pixel Shader 
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	 PS_INPUT output;

    // use the minimum of near mode and standard
    static const int minDepth = 300 << 3;

    // use the maximum of near mode and standard
    static const int maxDepth = 4000 << 3;

    // texture load location for the pixel we're on 
	int3 texCoord = int3(input.Tex,0);
	if (texCoord.x == widthHeight.x-1) texCoord.x -= 1;
	if (texCoord.y == widthHeight.y-1) texCoord.y -= 1;
	float4 point0 = txPos.Load(texCoord, 0);
	float4 point1 = txPos.Load(texCoord, int2(1, 0));
	float4 point2 = txPos.Load(texCoord, int2(0, 1));
    //if (point2.w<0.5)
       //return float4(0,0,0,-1);

	float3 u = point2.xyz-point0.xyz;
	float3 v = point1.xyz-point0.xyz;

	float3 normal = cross(u,v);

	return float4(normalize(normal),0)*0.5+float4(0.5,0.5,0.5,0);
}
