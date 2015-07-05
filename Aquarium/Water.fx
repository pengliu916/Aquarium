Texture2D<float4> txWaterHV : register( t0 );
Texture2D<float4> txNormal : register( t1 );
Texture2D<float4> txPoolWall : register( t2 );
Texture2D<float4> txCaustic : register(t3);
Texture2D<float4> txReflection : register(t4);
Texture2D<float4> txRefraction : register(t5);
SamplerState mirrorBorderSS : register( s0 );

static float waterParam = 15.0f;					// Related to wave moving speed
static float dampFactor = 0.995;				// Dampping waves
static float factorB = 0.304433;				// Make sure when adding water, the average height not changing
static float IOR_AIR = 1.0;
static float IOR_WATER = 1.3333;
static float4 underWaterColor = float4( 0.25, 1.0, 1.25, 1);
static float4 aboveWaterColor = float4( 0.4, 0.9, 1.0, 1);
float3 WaterDeepColor = float3(0.1, 0.4, 0.7);

cbuffer cbPerCall : register( b0 )
{
	int2 heightMapReso;							// Resolution of water mesh
	float2 invHeightMapReso;					// Inverse resolution of water mesh
	float2 dropCenter;							// 2D pos of drop center
	float cellSize;								// Distance between neighboring water vertices in meter
	float dropRadius;							// Drop dropRadius
	float strength;								// Drop height
	float waterBottom_y;						// Bottom y value
	float waterSurface_y;						// Water surface average y value
}
cbuffer cbPerFrame : register(b1){
	matrix mViewProj;
	matrix mReflectViewProj;
	matrix mRefractViewProj;
	float3 lightDir;
	float fDeltaTime;
	float3 viewDir;
	float niu;
	float3 eyePos;
	float niu2;
	float2 causticTexOffset;
};
struct GS_Water_INPUT{
};

struct PS_Water_INPUT{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader for water simulation
//--------------------------------------------------------------------------------------
GS_Water_INPUT QuadVS()
{
	GS_Water_INPUT output = (GS_Water_INPUT)0;
	return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader for water simulation
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void QuadGS(point GS_Water_INPUT particles[1], inout TriangleStream<PS_Water_INPUT> triStream)
{
	PS_Water_INPUT output;
	output.pos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
	output.tex = float2(0.0f, 0.0f);
	triStream.Append(output);

	output.pos = float4(1.0f, 1.0f, 0.0f, 1.0f);
	output.tex = float2(1.0f, 0.0f);
	triStream.Append(output);

	output.pos = float4(-1.0f, -1.0f, 0.0f, 1.0f);
	output.tex = float2(0.0f, 1.0f);
	triStream.Append(output);

	output.pos = float4(1.0f, -1.0f, 0.0f, 1.0f);
	output.tex = float2( 1.0f, 1.0f );
	triStream.Append(output);
}

//--------------------------------------------------------------------------------------
// Pixel Shader for water simulation
//--------------------------------------------------------------------------------------
float4 AverageWaterPS( PS_Water_INPUT input) : SV_Target
{
	// Read height and velocity for current vertex
	float4 PosVel = txWaterHV.SampleLevel(mirrorBorderSS, input.tex, 0);
	float average = (
		txWaterHV.SampleLevel(mirrorBorderSS, input.tex + float2(0.0f, invHeightMapReso.y), 0).y +
		txWaterHV.SampleLevel(mirrorBorderSS, input.tex - float2(0.0f, invHeightMapReso.y), 0).y +
		txWaterHV.SampleLevel(mirrorBorderSS, input.tex + float2(invHeightMapReso.x, 0.0f), 0).y +
		txWaterHV.SampleLevel(mirrorBorderSS, input.tex - float2(invHeightMapReso.x, 0.0f), 0).y 
		) * 0.25;
	PosVel.w += (average - PosVel.y) * waterParam / cellSize;
	PosVel.w *= dampFactor;
	PosVel.y += PosVel.w*fDeltaTime;
	return PosVel;
}

float4 DropWaterPS( PS_Water_INPUT input) : SV_Target
{
	float4 posVel = txWaterHV.SampleLevel( mirrorBorderSS, input.tex, 0 );
	float2 heightVel = posVel.yw ;
	// Add drop to the water height tex
	float2 currentPos = float2(input.tex.x - 0.5f, -0.5f + input.tex.y)* heightMapReso * cellSize;

	//a*(1 - (x*x + y*y))*pow(2.71828, (-x*x - y*y) / 2)
	float dist = length(dropCenter - currentPos) / dropRadius;

	//a*(cos(6.28*min(sqrt(x*x+y*y),6.28))+b)*max(0,1-sqrt(x*x+y*y))
	float drop = (cos(3.14 * min(dist, 6.28)) + factorB) * max(0, 1 - dist);
	//float drop = max(0.0f, 1.0 - length(dropCenter - currentPos) / dropRadius);
	//drop = 0.5f - cos(drop * 3.14159) * 0.5f;
	//if( drop>0) drop-=0.292;		// This make sure the surface height will not rise
	heightVel.x += drop * strength;
	posVel.yw = heightVel;
	return posVel;
}

struct VS_Render_INPUT{
	float3 pos : POSITION;
};

struct GS_Render_INPUT{
	float4 pos : POSITION;
	float4 lpos : POSITION1;
	float3 nor : NORMAL;
	float2 reflectTex : TEXCOORD0;
	float2 refractTex : TEXCOORD1;
};
struct PS_Render_INPUT{
	float4 pos :SV_POSITION;
	float4 lpos :POSITION1;
	float3 nor : NORMAL;
	float2 reflectTex : TEXCOORD0;
	float2 refractTex : TEXCOORD1;
};
static const float4 ambient = float4(0.9255, 0.9380, 0.9380, 1);
static const float4 diffuse = float4(0.8392, 0.8623, 0.8904, 1);
static const float4 specular = float4(1.0000, 0.9567, 0.6704, 1);
static const float4 ks = float4(0.15, 0.34, 0.42, 16);
static const float3 lightPos = float3(0, 10, 0);
static const float3 lightDirs = float3(-0.3010, -0.7262, 0.6181);
float4 Lighting(float3 lPos, float3 nor, float3 pos)
{
	float3 light = normalize(lPos - pos);
		float rdot = dot(nor, light);
	return diffuse * ks.y * max(0, rdot);

}
float2 intersectCube( float3 origin, float3 ray, float3 cubeMin, float3 cubeMax ) {
		float3 tMin = ( cubeMin - origin ) / ray;
		float3 tMax = ( cubeMax - origin ) / ray;
		float3 t1 = min( tMin, tMax );
		float3 t2 = max( tMin, tMax );
		float tNear = max( max( t1.x, t1.y ), t1.z );
	float tFar = min( min( t2.x, t2.y ), t2.z );
	return float2( tNear, tFar );
}
//--------------------------------------------------------------------------------------
// Vertex Shader for Render Water Surface
//--------------------------------------------------------------------------------------
GS_Render_INPUT RenderVS(VS_Render_INPUT input)
{
	// Calculate the sample index for height map
	float2 texIdx = input.pos.xz / (cellSize*(heightMapReso-1)) + 0.5f;
	texIdx.y = 1.0f - texIdx.y;
	// Get the height
	//float height = txWaterHV.SampleLevel(mirrorBorderSS, texIdx, 0);
	float3 rpos = txWaterHV.SampleLevel(mirrorBorderSS, texIdx, 0).xyz;
	float3 normal = txNormal.SampleLevel(mirrorBorderSS, texIdx, 0).xyz*2.0f - 1.0f;
	// Get right pos in local space
	float4 pos = float4(rpos, 1.0f);
	// Get projected pos in reflect screen space
	float4 reflectPos = mul(pos, mReflectViewProj);
	reflectPos /= reflectPos.w;
	// Get reflect tex coord
	float2 reflectTex = reflectPos.xy * 0.5f + 0.5f;
		reflectTex.y = 1 - reflectTex.y;

	// Get projected pos in refract screen space
	float4 refractPos = mul(pos, mRefractViewProj);
		refractPos /= refractPos.w;
	// Get refract tex coord
	float2 refractTex = refractPos.xy * 0.5f + 0.5f;
		refractTex.y = 1 - refractTex.y;
	GS_Render_INPUT output = (GS_Render_INPUT)0;
	output.lpos = pos;
	output.pos = mul(pos, mViewProj);
	output.reflectTex = reflectTex;
	output.refractTex = refractTex;
	output.nor = normal;
	return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader for Render Water Surface
//--------------------------------------------------------------------------------------
[maxvertexcount(3)]
void RenderGS(triangle GS_Render_INPUT input[3], inout TriangleStream<PS_Render_INPUT> outputStream)
{
	PS_Render_INPUT output;
	for (uint i = 0; i < 3; ++i){
		output.pos = input[i].pos;
		output.lpos = input[i].lpos;
		output.nor = input[i].nor;
		output.reflectTex = input[i].reflectTex;
		output.refractTex = input[i].refractTex;
		outputStream.Append(output);
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader for Render Water Surface
//--------------------------------------------------------------------------------------
float4 RenderPS(PS_Render_INPUT input) : SV_Target
{
	float3 light = normalize(lightPos - input.lpos);
	float4 AmbientColor = ambient * ks.x;
	float4 DiffuseColor = diffuse;
	float4 reflectColor = txReflection.SampleLevel(mirrorBorderSS, input.reflectTex, 0);
	float4 refractColor = txRefraction.SampleLevel(mirrorBorderSS, input.refractTex, 0);
	
	float3 pixelViewDir = normalize(input.lpos - eyePos);
	float3 reflectDir = reflect(pixelViewDir, input.nor);
	float3 refractDir = refract(pixelViewDir, input.nor, IOR_AIR/IOR_WATER);
	float ambientFactor = ks.x;
	float diffuseFactor =  ks.y;
	float specularFactor = ks.z;

	float4 water_color;
	float4 outputColor = float4(0,0,0,0);

	bool underWater = eyePos.y<waterSurface_y;
	float fresnel;

	float4 color = AmbientColor + DiffuseColor;// +SpecularColor;
	color.a = 1;

	if(underWater){
		fresnel = lerp(0.7, 1.0, clamp(1.5*pow(1.0 - dot(input.nor, -viewDir),1),0,1));
		//fresnel = max(0.0,lerp(-0.5, 1.0, clamp(pow(1.0 - dot(input.nor, -viewDir), 2),0,1)));
		diffuseFactor = 0.4*max(0, dot(float3(0,1,0), input.nor));
		specularFactor = 10*pow(max(0, dot(reflectDir, -lightDir)), 2);
		//return refractColor;
		outputColor =  (diffuseFactor + ambientFactor )* lerp( refractColor,reflectColor*underWaterColor*0.5, fresnel);
	}else{
		fresnel = lerp(0.1, 1, clamp(pow(1.0 - dot(input.nor, viewDir), 3),0,1));
		float3 lightPos = 1000*(-lightDir);
		float3 mLightDir = normalize(lightPos - input.lpos);
		specularFactor = 10*pow(max(0,dot(reflectDir,mLightDir)),350);
		diffuseFactor = 0.5*max(0,dot(mLightDir,input.nor));
		// Calculate the shadow cast from pool walls
		float2 cubeMaxXY = (heightMapReso-1)*cellSize * 0.5;
		float3 cubeMax = float3(cubeMaxXY.x, -waterBottom_y,cubeMaxXY.y);
		float t = ( -waterBottom_y - input.lpos.y ) / mLightDir.y;
		float2 testXZ = abs( input.lpos + t * mLightDir).xz;
		if( any( max( float2( 0, 0 ), testXZ - cubeMax.xz ))){
			diffuseFactor *= 0.3;
			specularFactor = 0;
		}
		refractColor *= underWaterColor;
		outputColor = (  ambientFactor+1*diffuseFactor ) *lerp(refractColor, reflectColor, fresnel);
		//outputColor = (  ambientFactor+1 ) *lerp(refractColor, reflectColor*aboveWaterColor*diffuseFactor+10*specularFactor*specular , fresnel);
		outputColor += specularFactor*specular;
	}
	return outputColor;
}



//--------------------------------------------------------------------------------------
// Vertex Shader for Render Caustic Texture
//--------------------------------------------------------------------------------------
struct GS_Caustic_INPUT{
	float3 oldPos : POSITION0;
	float3 newPos : POSITION1;
	float3 nor : NORMAL;
	float indicator : IDICATOR;
};
struct PS_Caustic_INPUT{
	float4 pos : SV_POSITION;
	float indicator : IDICATOR;
	 float intensity : INTENSITY;
};

GS_Caustic_INPUT CausticVS(VS_Render_INPUT input)
{
	GS_Caustic_INPUT output = (GS_Caustic_INPUT)0;
	// Calculate the sample index for height map
	float2 texIdx = input.pos.xz / (cellSize*(heightMapReso - 1)) + 0.5f;
	texIdx.y = 1.0f - texIdx.y;
	float3 rpos = txWaterHV.SampleLevel(mirrorBorderSS, texIdx, 0).xyz;
	float3 normal = txNormal.SampleLevel(mirrorBorderSS, texIdx, 0).xyz*2.0f - 1.0f;
	// Get right pos in local space
	float4 pos = float4(rpos, 1.0f);
	pos.xz = input.pos.xz;
	output.oldPos = pos.xyz;
	output.nor = normal;

	// Get the refraction ray based on normal and incoming ray vector
	float3 refractRayDir;
	if( dot(lightDir, normal) > 0 ) refractRayDir = reflect( lightDir, normal );
	else refractRayDir = refract( lightDir, normal, IOR_AIR/IOR_WATER );

	// Get t for refractRay to reach the bottom
	float t = ( waterBottom_y - pos.y ) / refractRayDir.y;
	// Get the projected vertex on the bottom
	float3 projectedPos = pos.xyz + refractRayDir * t;
	// Place the projected vetice almost inside the viewport(from (-1,-1) to (1,1))

	float3 srefractRayDir = refract(lightDir, float3(0, 1, 0), IOR_AIR / IOR_WATER);
	float st = ( waterBottom_y - waterSurface_y ) / srefractRayDir.y;
	float2 offset = srefractRayDir.xz * st;
	projectedPos.xz -= offset;
	//projectedPos.xz /= ( heightMapReso * cellSize * 0.5 );
	//projectedPos.xyz = projectedPos.xzy;
	projectedPos.y = 0.0;
	output.newPos = projectedPos;

	// Calculate whether it is inside the shadow area created by wall
	float indicator = 1;
	float3 backRayDir = -lightDir;
	t = (-waterBottom_y - waterSurface_y)/backRayDir.y;
	float3 topPos = rpos + t*backRayDir;
	if (any(abs(topPos.xz) > (heightMapReso - 1)*cellSize*0.5)) indicator = 0;
	output.indicator = indicator;
	return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader for Render Caustic Texture
//--------------------------------------------------------------------------------------
[maxvertexcount(3)]
void CausticGS(triangle GS_Caustic_INPUT input[3], inout TriangleStream<PS_Caustic_INPUT> outputStream)
{
	PS_Caustic_INPUT output;
	float3 e0 = input[1].newPos.xyz - input[0].newPos.xyz;
	float3 e1 = input[2].newPos.xyz - input[0].newPos.xyz;
	float newArea = 0.5 * length( cross( e0, e1 ) );

	float3 e2 = input[1].oldPos.xyz - input[0].oldPos.xyz;
	float3 e3 = input[2].oldPos.xyz - input[0].oldPos.xyz;
	float oldArea = 0.5 * length(cross(e2, e3));

	float3 avgNormal = normalize(input[0].nor + input[1].nor + input[2].nor);
	//float3 avgNormal = input[2].nor;
	float receiveIntensity = max( 0, dot(-lightDir, avgNormal));
	float resultIntensity = 80*receiveIntensity * oldArea / newArea;

	
	for (uint i = 0; i < 3; ++i){
		output.pos = float4(input[i].newPos,1);
		output.pos.xz /= (heightMapReso*cellSize*0.5);
		output.pos.xyz = output.pos.xzy;
		output.pos.z = 0.5;
		output.intensity = resultIntensity;
		output.indicator = input[i].indicator;
		outputStream.Append(output);
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader for Render Caustic Texture
//--------------------------------------------------------------------------------------
float4 CausticPS(PS_Caustic_INPUT input) : SV_Target
{
	//return input.indicator*float4(1, 0,0,0);
	return input.indicator*input.intensity * 0.003*float4(1, 1, 1, 1);
}





//--------------------------------------------------------------------------------------
// Vertex Shader for pool wall rendering
//--------------------------------------------------------------------------------------
struct VS_PoolWall_INPUT{
	float3 pos : POSITION;
	float2 tex : TEXCOORD;
};

struct GS_PoolWall_INPUT{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 prePos : TEXCOORD1;
};

struct PS_PoolWall_INPUT{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 prePos : TEXCOORD1;
	float3 nor : NORMAL;
};

PS_PoolWall_INPUT PoolWallVS( VS_PoolWall_INPUT input, uint vertexID : SV_VertexID )
{
	PS_PoolWall_INPUT output = (PS_PoolWall_INPUT)0;
	float3 scaleFactor = float3(heightMapReso.x * cellSize * 0.5, abs(waterBottom_y), heightMapReso.y * cellSize * 0.5);
	float4 pos = float4(input.pos * scaleFactor, 1);
	output.prePos = pos.xyz;
	output.pos = mul(pos, mViewProj);
	if( vertexID < 12)
		output.tex = input.tex*scaleFactor.xy/scaleFactor.x;
	else
		output.tex = input.tex + float2(0, scaleFactor.y/scaleFactor.x - 1);
	return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader for pool wall rendering
//--------------------------------------------------------------------------------------
[maxvertexcount(3)]
void PoolWallGS(triangle GS_PoolWall_INPUT input[3], inout TriangleStream<PS_PoolWall_INPUT> triStream)
{
	PS_PoolWall_INPUT output;

	float3 e0 = input[1].prePos.xyz - input[0].prePos.xyz;
	float3 e1 = input[2].prePos.xyz - input[0].prePos.xyz;
	float3 nor = normalize( cross(e1,e0));

	for (uint i = 0; i < 3; ++i){
		output.pos = input[i].pos;
		output.tex = input[i].tex;
		output.prePos = input[i].prePos;
		output.nor = nor;
		triStream.Append(output);
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader for pool wall rendering
//--------------------------------------------------------------------------------------
float4 PoolWallPS( PS_PoolWall_INPUT input ) : SV_Target
{
	// Calculate the sample index for height map and get pos from pos texture
	float2 texIdx = input.prePos.xz / (cellSize*(heightMapReso - 1)) + 0.5f;
	float4 surfacePos = txWaterHV.SampleLevel(mirrorBorderSS, texIdx, 0);
	float surfaceHeight = surfacePos.y;	// Get the actual height of that pixel pos

	// Get the texture color 
	float4 texColor = txPoolWall.SampleLevel(mirrorBorderSS, input.tex, 0);



	// Calculate the average reversed refraction ray
	float3 avgRefractDir = -refract( lightDir, float3(0,1,0), IOR_AIR/IOR_WATER );

	float RdotN = max( 0, dot( avgRefractDir, input.nor ));	// RefractionDir dot normal
	float LdotN = max( 0, dot( -lightDir, input.nor ));		// ReflectionDir dot normal

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
	float2 causticTexCoord = (input.prePos.xz + causticTexOffset.xz) / (cellSize*(heightMapReso - 1)) + 0.5f;
	float4 causticColor = txCaustic.SampleLevel(mirrorBorderSS, causticTexCoord*0.8 + 0.1, 0);
	float causticFactor = 2.0f * RdotN; // Control the intensity of caustic;
	// Based on the texture coord, preventing getting caustic outside of range since the sampler is in mirror state
	if(any(causticTexCoord>float2(1,1)) || any(causticTexCoord<float2(0,0)))	causticFactor = 0;

	// Based on light distance inside water body, decrease result lighting intensity
	float3 viewRayDir = normalize(input.prePos - eyePos);
	// Define bonding box for AABB box texting( to get distance inside waterbody)
	float2 cubeMaxXY = (heightMapReso-1)*cellSize * 0.5;
	float3 cubeMin = float3(-cubeMaxXY.x, waterBottom_y,-cubeMaxXY.y);
	float3 cubeMax = float3(cubeMaxXY.x, waterSurface_y,cubeMaxXY.y);
	float2 tt = intersectCube( eyePos,viewRayDir,cubeMin,cubeMax);
	float distInWater = tt.y-tt.x;
	// Change density of environment color based on the light ray distance in water body
	environmentFactor *= pow( 0.95, 1.0f + max( 0, distInWater ) );


	if( surfaceHeight > input.prePos.y){	// that pixel is under water
		diffuseFactor = 0;
		environmentCol = underWaterColor;
	}else{									// that pixel is above water
		diffuseFactor *= LdotN;
		// Calculate the shadow for pool walls
		t = ( -waterBottom_y - input.prePos.y ) / ( -lightDir.y );
		float2 testXZ = abs( input.prePos + t * ( -lightDir )).xz;
		if( any( max( float2( 0, 0 ), testXZ - cubeMax.xz ))) diffuseFactor = 0;

		environmentCol = float4(0.9,0.9,1,1);
		causticFactor = 0;
	}
	// Change density of environment color based on the distance between current pixel and surface height
	environmentFactor *= pow( 0.93, 1.0f + max( 0, waterSurface_y - input.prePos.y ) );

	environmentCol *= environmentFactor;
	diffuseColor *= diffuseFactor;
	ambientColor *= ambientFactor;

	float4 outputColor = texColor;
	outputColor *= diffuseColor + causticColor * causticFactor + ambientColor;
	return outputColor * environmentCol;
}