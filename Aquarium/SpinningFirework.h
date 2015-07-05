#pragma once

#include <D3D11.h>
#include <DirectXMath.h>
#include "DXUT.h"
#include "WICTextureLoader.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"

#ifndef SUB_TEXTUREWIDTH
#define SUB_TEXTUREWIDTH 640
#endif

#ifndef SUB_TEXTUREHEIGHT
#define SUB_TEXTUREHEIGHT 480
#endif

#define MAX_PARTICLES 300000


using namespace std;
using namespace DirectX;

struct SF_ConstBuffer
{
	XMMATRIX	mWorldViewProj;
	XMMATRIX	mInvView;
	XMFLOAT4	vGravity;

	float		fGlobalTime;
	float		fElapsedTime;
	float		fFireInterval;
	float		fMaxSubDetonates;
	
	int			iNumFirefly1s;
	float		fDetonateLife;
	float		fFirefly1Life;
	float		fSubDetonateLife;
	
	float		fFirefly2Life;
	int			notInUse;
	int			notInUse1;
	int			notInUse2;
};

struct PARTICLE_V
{
	XMFLOAT3	vPos;
	XMFLOAT3	vVel;
	XMFLOAT3	vCol;
	FLOAT		fTimer;
	UINT		uType;
};

class SpinningFirework
{
public:

	D3D11_VIEWPORT					m_RTviewport;
	CModelViewerCamera				m_Camera;

	ID3D11VertexShader*				m_pRenderParticleVS;
	ID3D11PixelShader*				m_pRenderParticlePS;
	ID3D11GeometryShader*			m_pRenderParticleGS;
	
	ID3D11VertexShader*				m_pAdvanceParticleVS;
	ID3D11GeometryShader*			m_pAdvanceParticleGS;
	
	ID3D11InputLayout*				m_pVertexLayout;

	ID3D11Buffer*					m_pTestVertexBuffer;

	ID3D11Buffer*					m_pParticleStartVB; 
	ID3D11Buffer*					m_pParticleStreamToVB; 
	ID3D11Buffer*					m_pParticleDrawFromVB; 

	ID3D11ShaderResourceView*		m_pSparkTexSRV;

	ID3D11SamplerState*				m_pGeneralTexSS;


	ID3D11ShaderResourceView*		m_pRandomTexSRV;
	ID3D11Texture1D*				m_pRandomTex1D;	

	//For Texture output
	ID3D11Texture2D*				m_pOutputTexture2D;
	ID3D11Texture2D*				m_pOutputStencilTexture2D;
	ID3D11RenderTargetView*			m_pOutputTextureRTV;
	ID3D11ShaderResourceView*		m_pOutputTextureRV;
	ID3D11DepthStencilView*			m_pOutputStencilView;
	ID3D11BlendState*				m_pOutputBlendState;

	SF_ConstBuffer					m_CBallInOne;
	ID3D11Buffer*					m_pCBallInOne;

	UINT			m_rendertargetWidth;
	UINT			m_rendertargetHeight;

	bool			m_bFirst;

	SpinningFirework(UINT width = SUB_TEXTUREWIDTH, UINT height = SUB_TEXTUREHEIGHT)
	{
		m_rendertargetWidth=width;
		m_rendertargetHeight=height;
		m_pOutputTexture2D=NULL;
		m_pOutputTextureRTV=NULL;
		m_pOutputTextureRV=NULL;
		m_bFirst = true;
		
		m_CBallInOne.fFireInterval = 1.0f;
		m_CBallInOne.fMaxSubDetonates = 40;
		m_CBallInOne.iNumFirefly1s = 60;
		m_CBallInOne.fDetonateLife = 3.5;
		m_CBallInOne.fFirefly1Life = 3.5;
		m_CBallInOne.fSubDetonateLife = 1.5;
		m_CBallInOne.fFirefly2Life = 2.5;
	}

	HRESULT Initial()
	{
		HRESULT hr=S_OK;
		
		return hr;
	}

	HRESULT CreateResource(ID3D11Device* pd3dDevice)
	{
		HRESULT hr=S_OK;

		ID3DBlob* pVSBlob = NULL;
		wstring filename=L"SpinningFirework.fx";

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TIMER", 0, DXGI_FORMAT_R32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		V_RETURN(DXUTCompileFromFile(L"SpinningFirework.fx", nullptr, "RenderVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pRenderParticleVS));
		V_RETURN(DXUTCompileFromFile(L"SpinningFirework.fx", nullptr, "AdvanceVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pAdvanceParticleVS));
		V_RETURN(pd3dDevice->CreateInputLayout(layout,ARRAYSIZE(layout),pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),&m_pVertexLayout));
		pVSBlob->Release();

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(L"SpinningFirework.fx", nullptr, "RenderGS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(),pGSBlob->GetBufferSize(),NULL,&m_pRenderParticleGS));
		V_RETURN(DXUTCompileFromFile(L"SpinningFirework.fx", nullptr, "AdvanceGS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		D3D11_SO_DECLARATION_ENTRY pDecl[] =
		{
			{ 0, "POSITION", 0, 0, 3, 0 },   
			{ 0, "NORMAL", 0, 0, 3, 0 },    
			{ 0, "COLOR", 0, 0, 3, 0 },    
			{ 0, "TIMER", 0, 0, 1, 0 },     
			{ 0, "TYPE", 0, 0, 1, 0 },     
		};
		UINT stride = 10 * sizeof( float ) + 1 * sizeof( UINT );
		UINT elems = sizeof( pDecl ) / sizeof( D3D11_SO_DECLARATION_ENTRY );

		//device->CreateGeometryShaderWithStreamOutput(this->mGSBlobSO->GetBufferPointer(), this->mGSBlobSO->GetBufferSize(), so_decl, 1, &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM, NULL, &this->mGeometryShaderSO)
		V_RETURN(pd3dDevice->CreateGeometryShaderWithStreamOutput(pGSBlob->GetBufferPointer(),pGSBlob->GetBufferSize(), pDecl, elems, &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM, NULL, &m_pAdvanceParticleGS ));
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(L"SpinningFirework.fx", nullptr, "RenderPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),NULL,&m_pRenderParticlePS));
		pPSBlob->Release();

		


		// Create the vertex buffer
		D3D11_BUFFER_DESC bd =
		{
			4 * sizeof( PARTICLE_V ),
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0
		};

		D3D11_SUBRESOURCE_DATA vbInitData;
		ZeroMemory( &vbInitData, sizeof( D3D11_SUBRESOURCE_DATA ));  

		PARTICLE_V vertStart[4] =
		{
			{ XMFLOAT3( 120, -120, 0 ), XMFLOAT3( 0, 60, 0 ), XMFLOAT3( 0.5, 1, 1 ),float( 0 ), UINT( 0 ) },
			{ XMFLOAT3( -120, -120, 0 ), XMFLOAT3( 0, 60, 0 ), XMFLOAT3( 1, 0, 0 ),float( 0 ), UINT( 0 ) },
			{ XMFLOAT3( 0, -120, 120 ), XMFLOAT3( 0, 60, 0 ), XMFLOAT3( 1, 0.5, 0 ),float( 0 ), UINT( 0 ) },
			{ XMFLOAT3( 0, -120, -120 ), XMFLOAT3( 0, 60, 0 ), XMFLOAT3( 1, 0, 1 ),float( 0 ), UINT( 0 ) },
		};
		vbInitData.pSysMem = vertStart;
		vbInitData.SysMemPitch = sizeof( PARTICLE_V );

		V_RETURN( pd3dDevice->CreateBuffer( &bd, &vbInitData, &m_pParticleStartVB ) );

		bd.ByteWidth = MAX_PARTICLES * sizeof( PARTICLE_V );
		bd.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
		V_RETURN( pd3dDevice->CreateBuffer( &bd, NULL, &m_pParticleStreamToVB ) );
		V_RETURN( pd3dDevice->CreateBuffer( &bd, NULL, &m_pParticleDrawFromVB ) );
		DXUT_SetDebugName( m_pParticleStreamToVB, "m_pParticleStreamToVB");
		DXUT_SetDebugName( m_pParticleDrawFromVB, "m_pParticleDrawFromVB");

		// Create the constant buffers
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0    ;
		bd.ByteWidth = sizeof(SF_ConstBuffer);
		V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pCBallInOne ));
		DXUT_SetDebugName( m_pCBallInOne, "m_pCBallInOne");

		auto pDevContext = DXUTGetD3D11DeviceContext();
		V_RETURN(DXUTCreateShaderResourceViewFromFile(pd3dDevice, L"misc\\firework.png", &m_pSparkTexSRV));

		//Create random texture resource
		int iNumRandValues = 1024;
		srand( timeGetTime() );
		//create the data
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = new float[iNumRandValues * 4];
		if( !InitData.pSysMem )
			return E_OUTOFMEMORY;
		InitData.SysMemPitch = iNumRandValues * 4 * sizeof( float );
		InitData.SysMemSlicePitch = iNumRandValues * 4 * sizeof( float );
		for( int i = 0; i < iNumRandValues * 4; i++ )
		{
			( ( float* )InitData.pSysMem )[i] = float( ( rand() % 10000 ) - 5000 );
		}

		// Create the texture
		D3D11_TEXTURE1D_DESC dstex;
		dstex.Width = iNumRandValues;
		dstex.MipLevels = 1;
		dstex.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		dstex.Usage = D3D11_USAGE_DEFAULT;
		dstex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = 0;
		dstex.ArraySize = 1;
		V_RETURN( pd3dDevice->CreateTexture1D( &dstex, &InitData, &m_pRandomTex1D ) );
		DXUT_SetDebugName( m_pRandomTex1D, "m_pRandomTex1D");
		SAFE_DELETE_ARRAY( InitData.pSysMem );

		// Create the resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory( &SRVDesc, sizeof( SRVDesc ) );
		SRVDesc.Format = dstex.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		SRVDesc.Texture2D.MipLevels = dstex.MipLevels;
		V_RETURN( pd3dDevice->CreateShaderResourceView( m_pRandomTex1D, &SRVDesc, &m_pRandomTexSRV ) );
		DXUT_SetDebugName( m_pRandomTexSRV, "m_pRandomTexSRV");

		//Create rendertaget resource
		D3D11_TEXTURE2D_DESC	RTtextureDesc = {0};
		RTtextureDesc.Width=m_rendertargetWidth;
		RTtextureDesc.Height=m_rendertargetHeight;
		RTtextureDesc.MipLevels=1;
		RTtextureDesc.ArraySize=1;
		RTtextureDesc.Format=DXGI_FORMAT_R16G16B16A16_FLOAT;
		RTtextureDesc.SampleDesc.Count=1;
		RTtextureDesc.Usage=D3D11_USAGE_DEFAULT;
		RTtextureDesc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
		RTtextureDesc.CPUAccessFlags=0;
		RTtextureDesc.MiscFlags=0;

		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc,NULL,&m_pOutputTexture2D));

		D3D11_SHADER_RESOURCE_VIEW_DESC RTshaderResourceDesc;
		RTshaderResourceDesc.Format=RTtextureDesc.Format;
		RTshaderResourceDesc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
		RTshaderResourceDesc.Texture2D.MostDetailedMip=0;
		RTshaderResourceDesc.Texture2D.MipLevels=1;
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pOutputTexture2D,&RTshaderResourceDesc,&m_pOutputTextureRV));

		D3D11_RENDER_TARGET_VIEW_DESC	RTviewDesc;
		RTviewDesc.Format=RTtextureDesc.Format;
		RTviewDesc.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
		RTviewDesc.Texture2D.MipSlice=0;
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pOutputTexture2D,&RTviewDesc,&m_pOutputTextureRTV));


		//Create DepthStencil buffer and view
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth,sizeof(descDepth));
		descDepth.Width = m_rendertargetWidth;
		descDepth.Height = m_rendertargetHeight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format =DXUTGetDeviceSettings().d3d11.AutoDepthStencilFormat;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = pd3dDevice->CreateTexture2D( &descDepth, NULL, &m_pOutputStencilTexture2D );


		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV,sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		// Create the depth stencil view
		V_RETURN( pd3dDevice->CreateDepthStencilView( m_pOutputStencilTexture2D, // Depth stencil texture
			&descDSV, // Depth stencil desc
			&m_pOutputStencilView ));  // [out] Depth stencil view

		// Create the blend state
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC) );
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;        
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA         ;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE        ;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE    ;        ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE   ;     ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL ;
		//blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F ;
		V_RETURN( pd3dDevice->CreateBlendState( &blendDesc, &m_pOutputBlendState ));

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &m_pGeneralTexSS ));

		m_RTviewport.Width=(float)m_rendertargetWidth;
		m_RTviewport.Height=(float)m_rendertargetHeight;
		m_RTviewport.MinDepth=0.0f;
		m_RTviewport.MaxDepth=1.0f;
		m_RTviewport.TopLeftX = 0;
		m_RTviewport.TopLeftY = 0;

		XMVECTORF32 vecEye = { 0.0f, 0.0f, -500.0f, 0.f };
		XMVECTORF32 vecAt = { 0.0f, 0.0f, 0.0f, 0.f };
		m_Camera.SetViewParams( vecEye, vecAt );
		m_Camera.SetRadius( 500.0f, 170.0f, 1000.0f );

		return hr;
	}
	
	void Resize()
	{
		// Setup the camera's projection parameters
			float fAspectRatio = m_rendertargetWidth / ( FLOAT )m_rendertargetHeight;
			m_Camera.SetProjParams( XM_PI / 4, fAspectRatio, 0.1f, 5000.0f );
			m_Camera.SetWindow(m_rendertargetWidth,m_rendertargetHeight );
			m_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );
	}

	void UpdateParticles( ID3D11DeviceContext* pd3dImmediateContext, float fGlobalTime, float fElapsedTime, XMVECTOR vGravity )
	{
		// Set IA parameters
		ID3D11Buffer* pBuffers[1];
		if( m_bFirst )
			pBuffers[0] = m_pParticleStartVB;
		else
			pBuffers[0] = m_pParticleDrawFromVB;
		UINT stride[1] = { sizeof( PARTICLE_V ) };
		UINT offset[1] = { 0 };
		pd3dImmediateContext->IASetInputLayout( m_pVertexLayout );
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, stride, offset );
		pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

		// Point to the correct output buffer
		pBuffers[0] = m_pParticleStreamToVB;
		pd3dImmediateContext->SOSetTargets( 1, pBuffers, offset );

		// Set Effects Parameters
		m_CBallInOne.fGlobalTime = fGlobalTime;
		m_CBallInOne.fElapsedTime = fElapsedTime;
		XMVECTOR vG = fElapsedTime * vGravity;
		XMStoreFloat4( &(m_CBallInOne.vGravity), vG );
		pd3dImmediateContext->UpdateSubresource( m_pCBallInOne, 0, NULL, &m_CBallInOne, 0, 0 );

		pd3dImmediateContext->GSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->VSSetShader( m_pAdvanceParticleVS, NULL, 0 );
		pd3dImmediateContext->GSSetShader( m_pAdvanceParticleGS, NULL, 0 );
		pd3dImmediateContext->PSSetShader( NULL, NULL, 0 );
		pd3dImmediateContext->GSSetShaderResources( 0, 1, &m_pRandomTexSRV );
		pd3dImmediateContext->GSSetSamplers(0,1,&m_pGeneralTexSS);


		if( m_bFirst )
			pd3dImmediateContext->Draw( 4, 0 );
		else
			pd3dImmediateContext->DrawAuto();


		// Get back to normal
		pBuffers[0] = NULL;
		pd3dImmediateContext->SOSetTargets( 1, pBuffers, offset );

		// Swap particle buffers
		ID3D11Buffer* pTemp = m_pParticleDrawFromVB;
		m_pParticleDrawFromVB = m_pParticleStreamToVB;
		m_pParticleStreamToVB = pTemp;

		m_bFirst = false;
	}

	void RenderParticles( ID3D11DeviceContext* pd3dImmediateContext )
	{
		UINT stride[1] = { sizeof( PARTICLE_V ) };
		UINT offset[1] = { 0 };
		pd3dImmediateContext->IASetInputLayout( m_pVertexLayout );
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pParticleDrawFromVB, stride, offset );
		pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		//pd3dImmediateContext->OMSetRenderTargets(1,&m_pOutputTextureRTV,m_pOutputStencilView);
		pd3dImmediateContext->OMSetRenderTargets(1,&m_pOutputTextureRTV,NULL);
		pd3dImmediateContext->ClearRenderTargetView( m_pOutputTextureRTV, ClearColor );
		pd3dImmediateContext->ClearDepthStencilView( m_pOutputStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		pd3dImmediateContext->RSSetViewports( 1, &m_RTviewport );
		pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS);
		pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pSparkTexSRV );
		pd3dImmediateContext->VSSetShader( m_pRenderParticleVS, NULL, 0 );
		pd3dImmediateContext->GSSetShader( m_pRenderParticleGS, NULL, 0 );
		pd3dImmediateContext->PSSetShader( m_pRenderParticlePS, NULL, 0 );
		pd3dImmediateContext->OMSetBlendState( m_pOutputBlendState, NULL, 0xffffffff );
		pd3dImmediateContext->VSSetShaderResources( 0, 1, &m_pRandomTexSRV );
		pd3dImmediateContext->VSSetSamplers(0,1,&m_pGeneralTexSS);
		pd3dImmediateContext->DrawAuto();
		//pd3dImmediateContext->Draw( 3, 0 );
	}

	void Update( float fElapsedTime )
	{
		m_Camera.FrameMove( fElapsedTime );
	}

	void Render(ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime)
	{
		XMMATRIX m_Proj = m_Camera.GetProjMatrix();
		XMMATRIX m_View = m_Camera.GetViewMatrix();
		XMMATRIX m_World =m_Camera.GetWorldMatrix();

		XMVECTOR t;
		m_CBallInOne.mInvView = XMMatrixTranspose(XMMatrixInverse(&t,m_View)); 
		m_CBallInOne.mWorldViewProj = XMMatrixTranspose( m_View*m_Proj );
		XMFLOAT4 vGravity( 0, -9.8, 0, 0 );		
		UpdateParticles( pd3dImmediateContext, fTime, fElapsedTime, XMLoadFloat4( &vGravity ));
		RenderParticles( pd3dImmediateContext );
		
	}

	~SpinningFirework()
	{

	}

	void Release()
	{
		SAFE_RELEASE(m_pRenderParticleVS);
		SAFE_RELEASE(m_pRenderParticleGS);
		SAFE_RELEASE(m_pRenderParticlePS);
		SAFE_RELEASE(m_pAdvanceParticleVS);
		SAFE_RELEASE(m_pAdvanceParticleGS);
		SAFE_RELEASE(m_pVertexLayout);
		SAFE_RELEASE(m_pParticleStartVB);
		SAFE_RELEASE(m_pParticleStreamToVB);
		SAFE_RELEASE(m_pParticleDrawFromVB);

		SAFE_RELEASE(m_pOutputTexture2D);
		SAFE_RELEASE(m_pOutputTextureRTV);
		SAFE_RELEASE(m_pOutputTextureRV);
		SAFE_RELEASE(m_pOutputStencilTexture2D);
		SAFE_RELEASE(m_pOutputStencilView);
		SAFE_RELEASE(m_pOutputBlendState);

		SAFE_RELEASE(m_pSparkTexSRV);

		SAFE_RELEASE(m_pRandomTexSRV);
		SAFE_RELEASE(m_pRandomTex1D);
		SAFE_RELEASE(m_pGeneralTexSS);
		SAFE_RELEASE(m_pCBallInOne);


	}



	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
		return 0;
	}
};