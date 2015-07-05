Texture1D txRandom : register( t0 );
Texture2D txSpark : register( t1 );
SamplerState samGeneral : register( s0 );

#define LAUNCHER		0
#define DETONAT			1
#define FIREFLY1		2
#define SUB_DETONATE	3
#define FIREFLY2		4

cbuffer cbAllInOne : register( b0 )
{
	matrix mWorldViewProj;
	matrix mInvView;
	float4 vFrameGravity;

	float fGlobalTime;
	float fElapsedTime;
	float fFireInterval;
	float fMaxSubDetonates;
	
	int iNumFirefly1s;
	float fDetonateLife;
	float fFirefly1Life;
	float fSubDetonateLife;
	
	float fFirefly2Life;
	int	iNotInUse0;
	int	iNotInUse1;
	int	iNotInUse2;

};

struct VS_Update_INPUT
{
	float3 pos : POSITION;
	float3 vel : NORMAL;
	float3 col : COLOR;
	float Timer : TIMER;
	uint Type : TYPE;
};

VS_Update_INPUT AdvanceVS( VS_Update_INPUT input)
{
	return input;
}
;
float3 RandomDir( float offset )
{
	float tCoord = (fGlobalTime + offset ) / 100.0;
	return txRandom.SampleLevel( samGeneral, tCoord, 0 ).xyz;
}

void GSGenericHandler( VS_Update_INPUT input, uint primID : SV_PrimitiveID, inout PointStream<VS_Update_INPUT> outputStream )
{
	input.pos += input.vel * fElapsedTime;
	input.vel += vFrameGravity;
	input.Timer -= fElapsedTime;
	
	outputStream.Append( input );
}

void GSLauncherHandler( VS_Update_INPUT input, uint primID : SV_PrimitiveID, inout PointStream<VS_Update_INPUT> outputStream )
{
	if( input.Timer <= 0 ){
		float3 vRandom = normalize( RandomDir( input.Type + primID) );
		VS_Update_INPUT output;
		output.pos = input.pos + input.vel * fElapsedTime;
		output.vel = input.vel + vRandom *8;
		output.col = 0.4 + abs( vRandom );
		output.Timer = fDetonateLife + vRandom.x * 0.5;
		output.Type = DETONAT;
		outputStream.Append( output );
		input.Timer = fFireInterval + vRandom.y * 0.03;
	}else{
		input.Timer -= fElapsedTime;
	}
	outputStream.Append( input );
}

void GSDetonateHandler( VS_Update_INPUT input, uint primID : SV_PrimitiveID, inout PointStream<VS_Update_INPUT> outputStream )
{
	if( input.Timer <= 0 ){
		VS_Update_INPUT output;
		float3 vRandom1=normalize( RandomDir( input.pos.x*8 + primID ));

		float3 vRandom;
		for( int i = 0; i < iNumFirefly1s; i++ ){
			vRandom = normalize( RandomDir( input.Type + i + primID));
			output.pos = input.pos + input.vel * fElapsedTime;
			output.vel = input.vel + vRandom * 15.0f;
			output.col = 0 + abs( vRandom1 );
			output.Timer = fFirefly1Life;
			output.Type = FIREFLY1;
			outputStream.Append( output );
		}
		for( int i = 0; i < abs( vRandom.x ) * fMaxSubDetonates; i++ ){
			vRandom = normalize( RandomDir( input.Type + i + primID));
			output.pos = input.pos + input.vel * fElapsedTime;
			output.vel = input.vel + vRandom * 10.0f;
			output.col = 0.4 + abs( vRandom1 );
			output.Timer = fSubDetonateLife + 0.4 * vRandom.x;
			output.Type = SUB_DETONATE;
			outputStream.Append( output );
		}
	}else{
		GSGenericHandler( input, primID, outputStream );
	}
}

void GSFirefly1Handler( VS_Update_INPUT input, uint primID : SV_PrimitiveID, inout PointStream<VS_Update_INPUT> outputStream )
{
	if( input.Timer > 0 ) GSGenericHandler( input, primID, outputStream );
}

void GSSubDetonateHandler( VS_Update_INPUT input, uint primID : SV_PrimitiveID, inout PointStream<VS_Update_INPUT> outputStream )
{
	if( input.Timer <= 0 ){
		VS_Update_INPUT output;
		float3 vRandom=normalize( RandomDir( input.pos.x*8 + primID ));
		for( int i = 0; i < 10; i++ ){
			output.pos = input.pos + input.vel * fElapsedTime;
			output.vel = input.vel + normalize( RandomDir( input.Type + i )) * 10.0f;
			output.col = 0.4 + abs( vRandom );
			output.Timer = fFirefly2Life;
			output.Type = FIREFLY2;
			outputStream.Append( output );
		}
	}else{
		GSGenericHandler( input, primID, outputStream );
	}
}


[maxvertexcount(93)]
void AdvanceGS( point VS_Update_INPUT input[1], uint primID : SV_PrimitiveID, inout PointStream<VS_Update_INPUT> outputStream )
{
	if( input[0].Type == LAUNCHER )	GSLauncherHandler( input[0], primID, outputStream );
	else if( input[0].Type == DETONAT ) GSDetonateHandler( input[0], primID, outputStream);
	else if( input[0].Type == FIREFLY1 || input[0].Type == FIREFLY2 ) GSFirefly1Handler( input[0], primID, outputStream);
	else if( input[0].Type == SUB_DETONATE ) GSSubDetonateHandler( input[0], primID, outputStream);
}




struct GS_Render_INPUT
{
	float3 pos : POSITION;
	float4 color : COLOR0;
	float radius : RADIUS;
};

GS_Render_INPUT RenderVS( VS_Update_INPUT input )
{
	GS_Render_INPUT output = ( GS_Render_INPUT )0;
	output.pos = input.pos;
	output.radius = 1.5;

		float3 vRandom=normalize( RandomDir( input.pos.y*8 ));
		float initAlpha = abs( vRandom.x );


	if( input.Type == LAUNCHER ){
		output.color = float4(input.col,1);
		output.radius = 1.0;
	}else if( input.Type == DETONAT ){
		output.color = float4(input.col,initAlpha);
		output.radius = 1.0;
	}else if( input.Type == FIREFLY1 ){
		output.color =float4(input.col,initAlpha*3);
		output.color.gba *= ( input.Timer / fFirefly1Life );
		output.color.g = 1.0f - output.color.g;
		float flashing = clamp( normalize(RandomDir( input.vel.y)).x,0,1);
		output.color *=  float4(flashing,flashing,flashing,1);
		//output.color.a = 0.5;
	}else if( input.Type == SUB_DETONATE ){
		output.color = float4(input.col,1);
	}else if( input.Type == FIREFLY2 ){
		output.color = float4(input.col,initAlpha);
		output.color.rba *= ( input.Timer / fFirefly2Life );
		output.color.r = 1.0f - output.color.r;
	}
	return output;
}

struct PS_Render_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXTURE0;
	float4 color : COLOR0;
};

cbuffer cbImmutable
{
	static const float3 positions[4] =
	{
		float3( -1, 1, 0 ),
		float3( 1, 1, 0 ),
		float3( -1, -1, 0 ),
		float3( 1, -1, 0 ),
	};
	static const float2 texcoords[4] = 
	{ 
		float2(0,1), 
		float2(1,1),
		float2(0,0),
		float2(1,0),
	};
};


[maxvertexcount(4)]
void RenderGS( point GS_Render_INPUT input[1], inout TriangleStream<PS_Render_INPUT> outputStream )
{
	PS_Render_INPUT output;
	for( int i = 0; i < 4; i++ ){
		float3 position = positions[i] * input[0].radius;
		position = mul( position, (float3x3)mInvView ) + input[0].pos;
		output.pos = mul( float4( position, 1.0 ), mWorldViewProj );

		output.color = input[0].color;
		output.tex = texcoords[i];
		outputStream.Append( output );
	}
	outputStream.RestartStrip();
}

float4 RenderPS( PS_Render_INPUT input ) : SV_Target
{
	return txSpark.Sample( samGeneral, input.tex ) * input.color;
}