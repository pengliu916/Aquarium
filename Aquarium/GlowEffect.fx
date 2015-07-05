Texture2D<float4>    inputTex  : register(t0);
Texture2D<float4>    downSampledTex  : register(t1);
Texture2D<float4>    H_glowTex  : register(t2);
Texture2D<float4>    HV_glowTex  : register(t3);

SamplerState samGeneral : register(s0);

#define NUMWT 9
static float Gauss[NUMWT] = {0.93, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};
#define WT_NORMALIZE (1.0/(1.0+2.0*(0.93 + 0.8 + 0.7 + 0.6 + 0.5 + 0.4 + 0.3 + 0.2 + 0.1)))
//static float Gauss[NUMWT] = { 0.98, 0.9, 0.8, 0.7, 0.4, 0.1, 0.05, 0.02, 0.001 };
//#define WT_NORMALIZE (1.0/(1.0+2.0*(0.98+ 0.9+ 0.8+ 0.7+ 0.4+ 0.1+ 0.05+ 0.02+ 0.001)))
//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
cbuffer cbPerResize : register(b0)
{
	float2 InvFinalReso;
	float2 InvTempReso;
	float glow_factor;
	float threshold_factor;
	float glowScale;
	float niu;
}


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
GS_INPUT VS()
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
	output.Pos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
	output.Tex = float2(0.0f, 0.0f);
	triStream.Append(output);

	output.Pos = float4(1.0f, 1.0f, 0.0f, 1.0f);
	output.Tex = float2(1.f, 0.0f);
	triStream.Append(output);

	output.Pos = float4(-1.0f, -1.0f, 0.0f, 1.0f);
	output.Tex = float2(0.0f, 1.f);
	triStream.Append(output);

	output.Pos = float4(1.0f, -1.0f, 0.0f, 1.0f);
	output.Tex = float2(1.f, 1.f);
	triStream.Append(output);
}

//--------------------------------------------------------------------------------------
// Pixel Shader just half the distance (test purpose)
//--------------------------------------------------------------------------------------
float4 PS_Glow_V(PS_INPUT input) : SV_Target
{
	float4 c2;
	float4 c = H_glowTex.Sample(samGeneral, input.Tex) * (WT_NORMALIZE);

	for (int i = 0; i<NUMWT; i++)
	{
		c2 = H_glowTex.Sample(samGeneral, input.Tex + (i+1) * float2( 0.f, InvTempReso.y));
		c += c2 * (Gauss[i] * WT_NORMALIZE);
		c2 = H_glowTex.Sample(samGeneral, input.Tex - (i+1) * float2( 0.f, InvTempReso.y));
		c += c2 * (Gauss[i] * WT_NORMALIZE);
	}
	return c * glow_factor;
}

float4 PS_Glow_H(PS_INPUT input) : SV_Target
{
	float4 c2;
	float4 c = downSampledTex.Sample(samGeneral, input.Tex) * (WT_NORMALIZE);
	for (int i = 0; i<NUMWT; i++)
	{
		c2 = downSampledTex.Sample(samGeneral, input.Tex + (i+1) * float2( InvTempReso.x, 0.f));
		c += c2 * (Gauss[i] * WT_NORMALIZE);
		c2 = downSampledTex.Sample(samGeneral, input.Tex - (i+1) * float2( InvTempReso.x, 0.f));
		c += c2 * (Gauss[i] * WT_NORMALIZE);
	}
	return c * glow_factor;
}

float4 PS_Glow_ALL(PS_INPUT input) : SV_Target
{
	float4 color = inputTex.Sample(samGeneral, input.Tex) +
	HV_glowTex.Sample(samGeneral, input.Tex);
	color.a = 1;
	return color;
	/*float2 freso = input.Tex/InvFinalReso;
	int2 reso = input.Tex / InvFinalReso;
	if(reso.x%2 && reso.y%2) return float4(1,1,freso.x,freso.y);
	return float4(0,0,freso.x,freso.y);*/
}

float4 PS_Glow_UpDown(PS_INPUT input) : SV_Target
{
	float4 color = inputTex.Sample(samGeneral, input.Tex);
	color.a = 1;
	if(length(color.xyz)<threshold_factor) color = float4(0,0,0,0);
	return color;
}