#define PREDATOR 0
cbuffer cbDraw: register(b0)
{
	float4x4	g_mWorldViewProj;
	float3		lightDir;// = float3(0.4242641, 0.5656854, 0.7071068);
	float		g_fFishSize;

	float3		eyePos;
	float		g_minSpeed;			// For velocity color mapping

	float		g_maxSpeed;			// For velocity color mapping
	int2		texReso;
	float		cellSize;

	float		waterSurface_y;
	float		waterBottom_y;
};

struct PosVel
{
	float3 pos;
	float3 vel;
};

StructuredBuffer<PosVel> g_bufPosVel : register(t0);	// Contain fish vel&pos
Texture2D txFish : register(t1);		// Texture for fish
Texture2D txColor : register(t2);		// Velocity color mapping texture(index table)
Texture2D txCaustic : register(t3);		// Caustic Texture

SamplerState samGeneral : register(s0);

//static const float3 vLight = float3(0.4242641,0.5656854,0.7071068)£»
struct VS_Input
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD;
	uint	instanceID : SV_INSTANCEID;	// Used for Shark identification(First 2 instance)
};

struct GS_Input
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD;
	uint	instanceID : SV_INSTANCEID;
};

struct PS_Input
{
	float4	Pos		: SV_POSITION;
	float3	prePos	: POSITION;
	float2	Tex		: TEXCOORD;
	float3	Nor		: NORNAL;
	float	Vidx	: VINDEX;			// Index for velocity color mapping
};

GS_Input VS(GS_Input In)
{
	GS_Input output = In;
	return output;
}

[maxvertexcount(3)]
void GS(triangle GS_Input input[3], inout TriangleStream<PS_Input> SpriteStream)
{
	PS_Input output;
	float3 vertex[3];
	float3 pos = g_bufPosVel[input[0].instanceID].pos;
	float3 vel = g_bufPosVel[input[0].instanceID].vel;

	/*	int3 state = pos > float3( 0, 0, 0 );
		state = state * 2 - int3( 1, 1, 1 );
	pos *= state;
	vel *= state;*/

	float velLen = length(vel);

	vel /= velLen;										// Forward vector for each fish
	float3 fup = float3(0, 1, 0);						// Faked up vector for each fish
	float3 right = cross(fup, vel);						// Right vector for each fish
	float3 up = cross(right, vel);
	float3x3 rotMatrix = { vel, up, right };			// Rotation matrix to pose each fish heading forward

	float size = g_fFishSize;							// Set the fish size
	
	// Calculate color index for later velocity color mapping
	float vidx = (velLen - g_minSpeed) / (g_maxSpeed - g_minSpeed);
#if PREDATOR
	if(input[0].instanceID <= 1){
		size*=5;
		vidx = 0.5;
	}
#endif
	[unroll]for (int i = 0; i<3; i++)
	{	// Pose each fish and scale it based on its size
		vertex[i] = mul(input[i].pos*float3(1.5,1,1.5)*size, rotMatrix);
	}
	// Calculate the normals
	float3 v0 = vertex[1] - vertex[0];
	float3 v1 = vertex[2] - vertex[0];
	float3 n = normalize(cross(v1,v0));
	// Calculate lighting
	//float illumination = abs(dot(n, vLight));

	[unroll]for (int i = 0; i<3; i++)
	{
		output.Pos = mul(float4(vertex[i] + pos, 1), g_mWorldViewProj);
		output.prePos = vertex[i] + pos;
		output.Tex = input[i].tex;
		output.Vidx = vidx;
		output.Nor = n;
		SpriteStream.Append(output);
	}
	SpriteStream.RestartStrip();
}/*
static const float4 ambient = float4(0.9255, 0.9380, 0.9380, 1);
static const float4 diffuse = float4(0.8392, 0.8623, 0.8904, 1);
static const float4 specular = float4(1.0000, 0.9567, 0.6704, 1);
static const float4 ks = float4(0.15, 0.34, 0.82, 16);
static const float3 lightPos = float3(0, 10, 0);
static const float3 lightDirs = float3(-0.3010, -0.7262, 0.6181);*/

static const float4 ambient = float4(0.9255, 0.9380, 0.9380, 1);
static const float4 diffuse = float4(0.8392, 0.8623, 0.8904, 1);
static const float4 specular = float4(1.0000, 0.9567, 0.6704, 1);
static const float4 ks = float4(0.15, 0.34, 0.42, 16);
static const float3 lightPos = float3(0, 10, 0);
static const float3 lightDirs = float3(-0.3010, -0.7262, 0.6181);

static float IOR_AIR = 1.0;
static float IOR_WATER = 1.3333;
static float4 underWaterColor = float4(0.25, 1.0, 1.25, 1);



float2 intersectCube( float3 origin, float3 ray, float3 cubeMin, float3 cubeMax ) {
		float3 tMin = ( cubeMin - origin ) / ray;
		float3 tMax = ( cubeMax - origin ) / ray;
		float3 t1 = min( tMin, tMax );
		float3 t2 = max( tMin, tMax );
		float tNear = max( max( t1.x, t1.y ), t1.z );
	float tFar = min( min( t2.x, t2.y ), t2.z );
	return float2( tNear, tFar );
}

float4 PS(PS_Input input) : SV_Target
{
	float4 texColor = txColor.Sample(samGeneral, float2(0.78, input.Vidx));

	//float3 avgRefractDur = -refract(lightDir, float3(0, 1, 0), IOR_AIR / IOR_WATER);
	//float4 diffuseColor = diffuse * ks.y;
	//float4 ambientColor = ambient * ks.x;
	//float4 waterColor;
	//// Calculate caustic texcoord
	//float t = (input.prePos.y - waterSurface_y) / (-avgRefractDur.y);
	//float3 causticTexOffset = t*avgRefractDur; 
	//t = (-waterBottom_y - waterSurface_y) / (-lightDir.y);
	//causticTexOffset -= t*lightDir;
	//float2 causticTexCoord = (input.prePos.xz + causticTexOffset.xz) / (cellSize*(texReso - 1)) + 0.5f;
	//	//causticTexCoord.y = 1.0f - causticTexCoord.y;
	//float4 causticColor;
	//if (any(causticTexCoord>float2(1, 1)) || any(causticTexCoord<float2(0, 0))){
	//	causticColor = float4(0, 0, 0, 0);
	//	diffuseColor *= 0.6;
	//} else
	//	causticColor = txCaustic.SampleLevel(samGeneral, causticTexCoord*0.8 + 0.1, 0);
	//diffuseColor *= (max(0, dot(avgRefractDur, -input.Nor))+ 0.5*max(0,dot(float3(0,1,0),input.Nor)));
	//waterColor = underWaterColor;

	//return (col*(diffuseColor*(causticColor * 10) + ambientColor)) *waterColor;

	// Get the texture color 
	//float4 texColor = txPoolWall.SampleLevel(mirrorBorderSS, input.tex, 0);



	// Calculate the average reversed refraction ray
	float3 avgRefractDir = -refract( lightDir, float3(0,1,0), IOR_AIR/IOR_WATER );

	float RdotN = max( 0, dot( avgRefractDir, input.Nor ));	// RefractionDir dot normal

	// Calculate the base lighting component
	float4 diffuseColor = diffuse;
	float diffuseFactor = ks.y;

	float4 ambientColor = ambient;
	float ambientFactor = ks.x;

	// Environment color, change result color based on whether this pixel is underwater or not
	float4 environmentCol;
	float environmentFactor = 1.0f;

	// Calculate caustic color
	float t = (input.prePos.y - waterSurface_y) / (-avgRefractDir.y);
	float3 causticTexOffset = t*avgRefractDir; // offset compensating for underwater part
	t = (-waterBottom_y-waterSurface_y)/(-lightDir.y);
	causticTexOffset -= t*lightDir; // offset compensating for abovewater part
	float2 causticTexCoord = (input.prePos.xz + causticTexOffset.xz) / (cellSize*(texReso - 1)) + 0.5f;
	float4 causticColor = txCaustic.SampleLevel(samGeneral, causticTexCoord*0.8 + 0.1, 0);
	float causticFactor = 5.0f * RdotN; // Control the intensity of caustic;
	// Based on the texture coord, preventing getting caustic outside of range since the sampler is in mirror state
	if(any(causticTexCoord>float2(1,1)) || any(causticTexCoord<float2(0,0)))	causticFactor = 0;

	// Based on light distance inside water body, decrease result lighting intensity
	float3 viewRayDir = normalize(input.prePos - eyePos);
	// Define bonding box for AABB box texting( to get distance inside waterbody)
	float2 cubeMaxXY = (texReso-1)*cellSize * 0.5;
	float3 cubeMin = float3(-cubeMaxXY.x, waterBottom_y,-cubeMaxXY.y);
	float3 cubeMax = float3(cubeMaxXY.x, waterSurface_y,cubeMaxXY.y);
	float2 tt = intersectCube( eyePos,viewRayDir,cubeMin,cubeMax);
	float dist = length(input.prePos - eyePos);
	float distInWater = dist - tt.x;
	// Change density of environment color based on the light ray distance in water body
	environmentFactor *= pow( 0.95, 1.0f + max( 0, distInWater ) );
	diffuseFactor *= max(0, dot( float3( 0, -1, 0), input.Nor));
	environmentCol = underWaterColor;
	
	// Change density of environment color based on the distance between current pixel and surface height
	environmentFactor *= pow( 0.95, 1.0f + max( 0, waterSurface_y - input.prePos.y ) );

	environmentCol *= environmentFactor;
	diffuseColor *= diffuseFactor;
	ambientColor *= ambientFactor;

	float4 outputColor = texColor;
	outputColor *= diffuseColor  + causticColor * causticFactor + ambientColor;
	return outputColor * environmentCol*1.5;
}