#pragma once

#include <DirectXMath.h>

#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"

#include "NormalGenerator.h"
#include "GlowEffect.h"
#include "Shoal.h"

#include "EnvironmentMap.h"
#include "header.h"

#ifndef SUB_TEXTUREWIDTH
#define SUB_TEXTUREWIDTH 500
#endif

#ifndef SUB_TEXTUREHEIGHT
#define SUB_TEXTUREHEIGHT 500
#endif

using namespace std;

typedef XMFLOAT2 float2;
typedef XMFLOAT3 float3;
typedef XMFLOAT4 float4;

class Water
{
public:
	// vertex structure for pool wall geometry
	// 5 components: pos.x, pos.y,pos.z, uv_x, uv_y
	struct PoolWall_Point{
		float m_vPosTex[5];
	};

	struct CB_Per_Frame{
		XMMATRIX	mViewProj;
		XMMATRIX	mReflectViewProj;
		XMMATRIX	mRefractViewProj;
		float3		f3LightDir;
		float		fDeltaTime;
		float3		f3ViewDir;
		float		niu;
		float3		f3Eyepos;
		float		niu2;
		float2		f2CausticTexOffset;
	};

	struct CB_Per_Call{
		XMUINT2 heightMapReso;		// resolution of water mesh
		float2 invHeightMapReso;

		float2 dropCenter;
		float cellSize;
		float dropRadius;

		float strength;
		float waterDepth_y;
		float waterSurface_y;
		float NIU;
	};

	// for boid simulation
	Shoal*							m_pFlock;

	// for environment cube map
	EnvironmentMap*					m_pSkyBox;
	
	// for glow effect
	GlowEffect*						m_pGlowEffect;

	// for water surface normal map
	NormalGenerator*				m_pNormalGenerator;

	// viewport for normal rendertarget
	D3D11_VIEWPORT					m_RTviewport;
	// viewport for refraction point of view
	D3D11_VIEWPORT					m_RTRefractViewport;

	// virtual camera
	CModelViewerCamera				m_Camera;

	// general texture sampler state
	ID3D11SamplerState*				m_pGeneralTexSS;

	// resource for rendering water surface
	ID3D11VertexShader*				m_pWaterRenderVS;
	ID3D11PixelShader*				m_pWaterRenderPS;
	ID3D11GeometryShader*			m_pWaterRenderGS;
	ID3D11InputLayout*				m_pWaterRenderIL;
	ID3D11Buffer*					m_pWaterVB;
	ID3D11Buffer*					m_pWaterRenderIB;
	ID3D11BlendState*				m_pWaterSurfaceBS; // for normal blending

	// general resource for quad effect (like generate normal map etc.)
	ID3D11VertexShader*				m_pQuadVS;
	ID3D11GeometryShader*			m_pQuadGS;
	ID3D11InputLayout*				m_pQuadIL;
	ID3D11Buffer*					m_pQuadVB;

	// For rendering pool walls
	ID3D11VertexShader*				m_pPoolWallVS;
	ID3D11GeometryShader*			m_pPoolWallGS;
	ID3D11PixelShader*				m_pPoolWallPS;
	ID3D11InputLayout*				m_pPoolWallIL;
	ID3D11Buffer*					m_pPoolWallVB;
	ID3D11ShaderResourceView*		m_pPoolWallSRV;
	ID3D11RasterizerState*			m_pPoolWallRSBackFace;
	ID3D11RasterizerState*			m_pPoolWallRSFrontFace;

	// For physical simulation
	D3D11_VIEWPORT					m_SimViewport;
	ID3D11PixelShader*				m_pSimUpdateWaterPS;
	ID3D11PixelShader*				m_pSimDropWaterPS;

	// For caustic rendering
	/* Using the change of triangle (in water surface grid) area to simulate
	caustic light intensity, the output caustic is a texture to be parallel
	projected into geometries*/
	D3D11_VIEWPORT					m_CausticViewport;// has the same reso of caustic tex
	ID3D11VertexShader*				m_pCausticVS;
	ID3D11GeometryShader*			m_pCausticGS;
	ID3D11PixelShader*				m_pCausticPS;
	ID3D11Texture2D*				m_pCausticTex;
	ID3D11ShaderResourceView*		m_pCausticSRV;
	ID3D11RenderTargetView*			m_pCausticRTV;
	ID3D11BlendState*				m_pCausticBS;
	float							m_fCausticTexWidth;
	float							m_fCausticTexHeight;
	// Texture related object for water surface height and velocity
	ID3D11Texture2D*				m_pWaterPosVelTex[2];
	ID3D11ShaderResourceView*		m_pWaterPosVelSRV[2];
	ID3D11RenderTargetView*			m_pWaterPosVelRTV[2];

	//For Texture output
	ID3D11Texture2D*				m_pOutputTexture2D;
	ID3D11Texture2D*				m_pOutputStencilTexture2D;
	ID3D11RenderTargetView*			m_pOutputTextureRTV;
	ID3D11ShaderResourceView*		m_pOutputTextureSRV;
	ID3D11DepthStencilView*			m_pOutputStencilView;

	// For reflection
	ID3D11Texture2D*				m_pOutReflectionTexture2D;
	ID3D11Texture2D*				m_pOutReflectionStencilTexture2D;
	ID3D11RenderTargetView*			m_pOutReflectionTextureRTV;
	ID3D11ShaderResourceView*		m_pOutReflectionTextureSRV;
	ID3D11DepthStencilView*			m_pOutReflectionStencilView;

	// For refraction	
	ID3D11Texture2D*				m_pOutRefractionTexture2D;
	ID3D11Texture2D*				m_pOutRefractionStencilTexture2D;
	ID3D11RenderTargetView*			m_pOutRefractionTextureRTV;
	ID3D11ShaderResourceView*		m_pOutRefractionTextureSRV;
	ID3D11DepthStencilView*			m_pOutRefractionStencilView;

	// rasterizer state for switching between solid and wireframe
	ID3D11RasterizerState*			m_pRasterizerStateSolid;
	ID3D11RasterizerState*			m_pRasterizerStateWireframe;
	bool							m_bWireframe;

	CB_Per_Frame					m_CBperFrame;
	ID3D11Buffer*					m_pCBperFrame;

	CB_Per_Call						m_CBperCall;
	ID3D11Buffer*					m_pCBperCall;

	// Water mesh setting
	DWORD							m_iNumVertices;
	DWORD							m_iNumRenderIndices;

	// initial water surface vertices
	float3*							m_pInitWaterVertexPos;

	// Pool wall geometry vertices
	PoolWall_Point					m_PoolWallMeshVertices[14];

	UINT							m_rendertargetWidth;
	UINT							m_rendertargetHeight;

	// for ping-pong buffer switching
	UINT							m_uCurrentSRVIdx;

	// indicator for water drop event
	bool							m_bDropWater;
	bool							m_bLeftButtonPressed;

	// Preventing adding drop at same place multiple times
	float2							m_f2PreviousDropPos;

	// fov
	float							m_fFov;

	// For flee force
	int								m_iFleeOn;

	// Position of the mouse in current subwindwos
	short							m_sMouse_x;
	short							m_sMouse_y;

	Water(float deltaTime, XMFLOAT3 waterBodyCenter, XMFLOAT3 waterXYZRadius,
		  UINT width = SUB_TEXTUREWIDTH, UINT height = SUB_TEXTUREHEIGHT,
		  UINT gridwidth = 256, UINT gridheight = 256)
	{
		m_pSkyBox = new EnvironmentMap();
		m_pFlock = new Shoal(256, deltaTime, waterBodyCenter,
							 float3(waterXYZRadius.x*0.8, waterXYZRadius.y*0.8, waterXYZRadius.z*0.8));
		m_pNormalGenerator = new NormalGenerator(gridwidth, gridheight);
		m_pGlowEffect = new GlowEffect();
		m_iFleeOn = 0;
		m_CBperFrame.fDeltaTime = deltaTime;
		
		// cell is the elemental quad in water simulation grid
		float cellsize = waterXYZRadius.x * 2.0f / gridwidth;
		m_fFov = XM_PI / 4.0f;
		m_rendertargetWidth = width;
		m_rendertargetHeight = height;
		m_pOutputTexture2D = nullptr;
		m_pOutputTextureRTV = nullptr;
		m_pOutputTextureSRV = nullptr;

		m_fCausticTexWidth = 1024.0f * m_rendertargetWidth / m_rendertargetHeight;
		m_fCausticTexHeight = 1024;

		m_bWireframe = false;

		m_CBperCall.cellSize = cellsize;
		m_CBperCall.heightMapReso.x = gridwidth;
		m_CBperCall.heightMapReso.y = gridheight;
		m_CBperCall.invHeightMapReso.x = 1.0f / gridwidth;
		m_CBperCall.invHeightMapReso.y = 1.0f / gridheight;
		m_CBperCall.dropCenter = float2(0, 0);
		m_CBperCall.dropRadius = 0.8f;
		m_CBperCall.strength = 0.2f;
		m_CBperCall.waterDepth_y = waterBodyCenter.y - waterXYZRadius.y;
		m_CBperCall.waterSurface_y = waterXYZRadius.y + waterBodyCenter.y;
		m_iNumVertices = gridheight * gridwidth;
		m_pInitWaterVertexPos = new float3[m_iNumVertices];
		m_f2PreviousDropPos = float2(0, 0);

		// initial pool wall geometry
		m_PoolWallMeshVertices[0] = { -1.0f, -1.0f, -1.0f, 0.0f, 1.0f };
		m_PoolWallMeshVertices[1] = { -1.0f, 1.0f, -1.0f, 0.0f, 0.0f };
		m_PoolWallMeshVertices[2] = { -1.0f, -1.0f, -1.0f, 0.0f, 1.0f };
		m_PoolWallMeshVertices[3] = { 1.0f, 1.0f, -1.0f, 1.0f, 0.0f };
		m_PoolWallMeshVertices[4] = { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f };
		m_PoolWallMeshVertices[5] = { 1.0f, 1.0f, 1.0f, 0.0f, 0.0f };
		m_PoolWallMeshVertices[6] = { 1.0f, -1.0f, 1.0f, 0.0f, 1.0f };
		m_PoolWallMeshVertices[7] = { -1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
		m_PoolWallMeshVertices[8] = { -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
		m_PoolWallMeshVertices[9] = { -1.0f, 1.0f, -1.0f, 0.0f, 0.0f };
		m_PoolWallMeshVertices[10] = { -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
		m_PoolWallMeshVertices[11] = { -1.0f, -1.0f, -1.0f, 0.0f, 1.0f };
		m_PoolWallMeshVertices[12] = { 1.0f, -1.0f, 1.0f, 1.0f, 0.0f };
		m_PoolWallMeshVertices[13] = { 1.0f, -1.0f, -1.0f, 0.0f, 0.0f };

		m_bDropWater = false;
		m_bLeftButtonPressed = false;
	}

	HRESULT Initial()
	{
		HRESULT hr = S_OK;
		V_RETURN(m_pFlock->Initial());

		return hr;
	}

	HRESULT CreateWaterPosVelTex(ID3D11Device* pd3dDeivce)
	{
		HRESULT hr;
		float* texArray = new float[4 * m_CBperCall.heightMapReso.x*m_CBperCall.heightMapReso.y];
		for (int y = 0; y < m_CBperCall.heightMapReso.y; ++y){
			for (int x = 0; x < m_CBperCall.heightMapReso.x; ++x){
				texArray[m_CBperCall.heightMapReso.x * 4 * y + x * 4 + 0] = -1.0*(m_CBperCall.heightMapReso.x - 1) * m_CBperCall.cellSize / 2.0f + x * m_CBperCall.cellSize;
				texArray[m_CBperCall.heightMapReso.x * 4 * y + x * 4 + 1] = m_CBperCall.waterSurface_y;
				texArray[m_CBperCall.heightMapReso.x * 4 * y + x * 4 + 2] = -1.0*(m_CBperCall.heightMapReso.y - 1) * m_CBperCall.cellSize / 2.0f + y * m_CBperCall.cellSize;
				texArray[m_CBperCall.heightMapReso.x * 4 * y + x * 4 + 3] = 0.0f;
			}
		}
		//texArray[116] = 1.0f;
		D3D11_SUBRESOURCE_DATA initialData;
		ZeroMemory(&initialData, sizeof(D3D11_SUBRESOURCE_DATA));
		initialData.pSysMem = texArray;
		initialData.SysMemPitch = m_CBperCall.heightMapReso.x * 4 * sizeof(float);
		initialData.SysMemSlicePitch = m_CBperCall.heightMapReso.x * m_CBperCall.heightMapReso.y * 4 * sizeof(float);


		D3D11_TEXTURE2D_DESC dstex;
		ZeroMemory(&dstex, sizeof(D3D11_TEXTURE2D_DESC));
		dstex.Width = m_CBperCall.heightMapReso.x;
		dstex.Height = m_CBperCall.heightMapReso.y;
		dstex.MipLevels = 1;
		dstex.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		dstex.SampleDesc.Count = 1;
		dstex.Usage = D3D11_USAGE_DEFAULT;
		dstex.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = 0;
		dstex.ArraySize = 1;
		V_RETURN(pd3dDeivce->CreateTexture2D(&dstex, &initialData, &m_pWaterPosVelTex[0]));
		DXUT_SetDebugName(m_pWaterPosVelTex[0], "m_pWaterPosVelTex[0]");
		V_RETURN(pd3dDeivce->CreateTexture2D(&dstex, &initialData, &m_pWaterPosVelTex[1]));
		DXUT_SetDebugName(m_pWaterPosVelTex[1], "m_pWaterPosVelTex[1]");
		delete texArray;

		// Create the resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory(&SRVDesc, sizeof(SRVDesc));
		SRVDesc.Format = dstex.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = dstex.MipLevels;
		char temp[100];
		for (int i = 0; i < 2; ++i){
			V_RETURN(pd3dDeivce->CreateShaderResourceView(m_pWaterPosVelTex[i], &SRVDesc, &m_pWaterPosVelSRV[i]));
			sprintf_s(temp, "m_pWaterPosVelSRV[%d]", i);
			DXUT_SetDebugName(m_pWaterPosVelSRV[i], temp);
		}

		//Create rendertaget resource
		D3D11_RENDER_TARGET_VIEW_DESC	RTviewDesc;
		RTviewDesc.Format = dstex.Format;
		RTviewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTviewDesc.Texture2D.MipSlice = 0;
		for (int i = 0; i < 2; ++i){
			V_RETURN(pd3dDeivce->CreateRenderTargetView(m_pWaterPosVelTex[i], &RTviewDesc, &m_pWaterPosVelRTV[i]));
			sprintf_s(temp, "m_pWaterPosVelRTV[%d]", i);
			DXUT_SetDebugName(m_pWaterPosVelRTV[i], temp);
		}
	}

	HRESULT CreateWaterPosVelVB(ID3D11Device* pd3dDevice)
	{
		HRESULT hr;
		DWORD v = 0;
		for (DWORD y = 0; y < m_CBperCall.heightMapReso.y; ++y){
			for (DWORD x = 0; x < m_CBperCall.heightMapReso.x; ++x){
				float3	pos;
				pos.x = -1.0*(m_CBperCall.heightMapReso.x - 1) * m_CBperCall.cellSize / 2.0f + x * m_CBperCall.cellSize;
				pos.z = -1.0*(m_CBperCall.heightMapReso.y - 1) * m_CBperCall.cellSize / 2.0f + y * m_CBperCall.cellSize;
				pos.y = m_CBperCall.waterSurface_y;
				m_pInitWaterVertexPos[v] = pos;
				++v;
			}
		}
		//m_pInitWaterVertexPos[512].y = 1;
		D3D11_BUFFER_DESC bd =
		{
			m_iNumVertices * sizeof(float3),
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0
		};
		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = m_pInitWaterVertexPos;
		initialData.SysMemPitch = sizeof(float3);
		V_RETURN(pd3dDevice->CreateBuffer(&bd, &initialData, &m_pWaterVB));
		DXUT_SetDebugName(m_pWaterRenderVS, "m_pWaterRenderVS");
	}

	HRESULT CreateWaterRenderIB(ID3D11Device* pd3dDevice)
	{
		HRESULT hr;

		m_iNumRenderIndices = 2 * (m_CBperCall.heightMapReso.x - 1) * (m_CBperCall.heightMapReso.y - 1) * 3;
		int size = m_iNumRenderIndices;
		WORD(*indices)[3] = new WORD[size / 3][3];
		// Break grid into several part can make a index buffer more friendly to vertex cache
		const int subWidth = 10;
		DWORD n = 0;
		for (DWORD x0 = 0; x0 < m_CBperCall.heightMapReso.x - 1; x0 += subWidth){
			DWORD x1 = x0 + subWidth;
			for (DWORD y = 0; y < m_CBperCall.heightMapReso.y - 1; ++y){
				for (DWORD x = x0; (x < m_CBperCall.heightMapReso.x - 1) && (x < x1); ++x){
					indices[n][0] = (y + 1) * m_CBperCall.heightMapReso.x + (x + 0);
					indices[n][1] = (y + 0) * m_CBperCall.heightMapReso.x + (x + 0);
					indices[n][2] = (y + 1) * m_CBperCall.heightMapReso.x + (x + 1);
					++n;
					indices[n][0] = (y + 0) * m_CBperCall.heightMapReso.x + (x + 0);
					indices[n][1] = (y + 0) * m_CBperCall.heightMapReso.x + (x + 1);
					indices[n][2] = (y + 1) * m_CBperCall.heightMapReso.x + (x + 1);
					++n;
				}
			}
		}
		D3D11_BUFFER_DESC bd =
		{
			size * sizeof(WORD),
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_INDEX_BUFFER,
			0,
			0
		};
		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = indices;
		initialData.SysMemPitch = sizeof(WORD);
		V_RETURN(pd3dDevice->CreateBuffer(&bd, &initialData, &m_pWaterRenderIB));
		DXUT_SetDebugName(m_pWaterRenderIB, "m_pWaterRenderIB");
	}

	HRESULT CreatePoolWallResource(ID3D11Device* pd3dDevice)
	{
		HRESULT hr = S_OK;
		ID3DBlob* pVSBlob = NULL;
		wstring filename = L"Simulate.fx";

		D3D11_INPUT_ELEMENT_DESC layout0[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "PoolWallVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pPoolWallVS));
		DXUT_SetDebugName(m_pPoolWallVS, "m_pPoolWallVS");

		V_RETURN(pd3dDevice->CreateInputLayout(layout0, ARRAYSIZE(layout0), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pPoolWallIL));
		DXUT_SetDebugName(m_pPoolWallIL, "m_pPoolWallIL");
		pVSBlob->Release();

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "PoolWallGS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &m_pPoolWallGS));
		DXUT_SetDebugName(m_pPoolWallGS, "m_pPoolWallGS");
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "PoolWallPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPoolWallPS));
		DXUT_SetDebugName(m_pPoolWallPS, "m_pPoolWallPS");
		pPSBlob->Release();

		// Create vertex buffer
		D3D11_BUFFER_DESC vbDesc;
		ZeroMemory(&vbDesc, sizeof(D3D11_BUFFER_DESC));
		vbDesc.ByteWidth = sizeof(PoolWall_Point)* ARRAYSIZE(m_PoolWallMeshVertices);
		vbDesc.Usage = D3D11_USAGE_DEFAULT;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vbInitData;
		ZeroMemory(&vbInitData, sizeof(vbInitData));
		vbInitData.pSysMem = m_PoolWallMeshVertices;
		V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, &vbInitData, &m_pPoolWallVB));
		DXUT_SetDebugName(m_pPoolWallVB, "m_pPoolWallVB");

		// Create pool wall texture srv
		V_RETURN(DXUTCreateShaderResourceViewFromFile(pd3dDevice, L"misc\\poolWall.jpg", &m_pPoolWallSRV));

		// Create backface culling rasterizer state
		D3D11_RASTERIZER_DESC RasterDesc;
		ZeroMemory(&RasterDesc, sizeof(D3D11_RASTERIZER_DESC));
		RasterDesc.FillMode = D3D11_FILL_SOLID;
		RasterDesc.CullMode = D3D11_CULL_FRONT;
		RasterDesc.DepthClipEnable = TRUE;
		V_RETURN(pd3dDevice->CreateRasterizerState(&RasterDesc, &m_pPoolWallRSBackFace));
		DXUT_SetDebugName(m_pPoolWallRSBackFace, "m_pPoolWallRSBackFace");
		RasterDesc.CullMode = D3D11_CULL_BACK;
		V_RETURN(pd3dDevice->CreateRasterizerState(&RasterDesc, &m_pPoolWallRSFrontFace));
		DXUT_SetDebugName(m_pPoolWallRSFrontFace, "m_pPoolWallRSFrontFace");

		return hr;
	}

	HRESULT CreateCausticResource(ID3D11Device* pd3dDevice)
	{
		HRESULT hr = S_OK;

		ID3DBlob* pVSBlob = NULL;
		wstring filename = L"Simulate.fx";

		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "CausticVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pCausticVS));
		DXUT_SetDebugName(m_pWaterRenderVS, "m_pCausticVS");
		pVSBlob->Release();

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "CausticGS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &m_pCausticGS));
		DXUT_SetDebugName(m_pWaterRenderGS, "m_pCausticGS");
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "CausticPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pCausticPS));
		DXUT_SetDebugName(m_pCausticPS, "m_pCausticPS");
		pPSBlob->Release();

		// Create Caustic related texture resource
		D3D11_TEXTURE2D_DESC	CausticTexDesc = { 0 };
		CausticTexDesc.Width = m_fCausticTexWidth;
		CausticTexDesc.Height = m_fCausticTexHeight;
		CausticTexDesc.MipLevels = 1;
		CausticTexDesc.ArraySize = 1;
		CausticTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		CausticTexDesc.SampleDesc.Count = 1;
		CausticTexDesc.Usage = D3D11_USAGE_DEFAULT;
		CausticTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		CausticTexDesc.CPUAccessFlags = 0;
		CausticTexDesc.MiscFlags = 0;

		V_RETURN(pd3dDevice->CreateTexture2D(&CausticTexDesc, NULL, &m_pCausticTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC CausticTexSRDesc;
		CausticTexSRDesc.Format = CausticTexDesc.Format;
		CausticTexSRDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		CausticTexSRDesc.Texture2D.MostDetailedMip = 0;
		CausticTexSRDesc.Texture2D.MipLevels = 1;
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pCausticTex, &CausticTexSRDesc, &m_pCausticSRV));

		D3D11_RENDER_TARGET_VIEW_DESC	CausticTexRTVDesc;
		CausticTexRTVDesc.Format = CausticTexDesc.Format;
		CausticTexRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		CausticTexRTVDesc.Texture2D.MipSlice = 0;
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pCausticTex, &CausticTexRTVDesc, &m_pCausticRTV));

		// Create the blend state for rendering caustic
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;        ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;     ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		V_RETURN(pd3dDevice->CreateBlendState(&blendDesc, &m_pCausticBS));

		// Create viewport for rendering caustic texture
		m_CausticViewport.Width = m_fCausticTexWidth;
		m_CausticViewport.Height = m_fCausticTexHeight;
		m_CausticViewport.MinDepth = 0.0f;
		m_CausticViewport.MaxDepth = 1.0f;
		m_CausticViewport.TopLeftX = 0;
		m_CausticViewport.TopLeftY = 0;

		return hr;
	}

	HRESULT CreateResource(ID3D11Device* pd3dDevice)
	{
		HRESULT hr = S_OK;

		ID3DBlob* pVSBlob = NULL;
		wstring filename = L"Simulate.fx";

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "RenderVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pWaterRenderVS));
		DXUT_SetDebugName(m_pWaterRenderVS, "m_pWaterRenderVS");
		V_RETURN(pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pWaterRenderIL));
		DXUT_SetDebugName(m_pWaterRenderIL, "m_pWaterRenderIL");

		D3D11_INPUT_ELEMENT_DESC layout1[] = { { "POSITION", 0, DXGI_FORMAT_R16_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };

		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "QuadVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pQuadVS));
		DXUT_SetDebugName(m_pQuadVS, "m_pQuadVS");
		V_RETURN(pd3dDevice->CreateInputLayout(layout1, ARRAYSIZE(layout1), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pQuadIL));
		DXUT_SetDebugName(m_pQuadIL, "m_pQuadIL");
		pVSBlob->Release();

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "RenderGS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &m_pWaterRenderGS));
		DXUT_SetDebugName(m_pWaterRenderGS, "m_pWaterRenderGS");
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "QuadGS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &m_pQuadGS));
		DXUT_SetDebugName(m_pQuadGS, "m_pQuadGS");
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "RenderPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pWaterRenderPS));
		DXUT_SetDebugName(m_pWaterRenderPS, "m_pWaterRenderPS");
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "AverageWaterPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pSimUpdateWaterPS));
		DXUT_SetDebugName(m_pSimUpdateWaterPS, "m_pSimUpdateWaterPS");
		V_RETURN(DXUTCompileFromFile(L"Water.fx", nullptr, "DropWaterPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pSimDropWaterPS));
		DXUT_SetDebugName(m_pSimDropWaterPS, "m_pSimDropWaterPS");
		pPSBlob->Release();

		// Create Water grid mesh
		V_RETURN(CreateWaterPosVelTex(pd3dDevice));
		V_RETURN(CreateWaterPosVelVB(pd3dDevice));
		V_RETURN(CreateWaterRenderIB(pd3dDevice));
		V_RETURN(CreateCausticResource(pd3dDevice));
		V_RETURN(CreatePoolWallResource(pd3dDevice));
		V_RETURN(m_pNormalGenerator->CreateResource(pd3dDevice, m_pWaterPosVelSRV[0]));

		D3D11_BUFFER_DESC bd =
		{
			sizeof(CB_Per_Frame),
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_CONSTANT_BUFFER,
			0,
			0
		};
		V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pCBperFrame));
		DXUT_SetDebugName(m_pCBperFrame, "m_pCBperFrame");
		bd.ByteWidth = sizeof(CB_Per_Call);
		V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pCBperCall));
		DXUT_SetDebugName(m_pCBperCall, "m_pCBperCall");

		

		// Create solid and wireframe rasterizer state objects
		D3D11_RASTERIZER_DESC RasterDesc;
		ZeroMemory(&RasterDesc, sizeof(D3D11_RASTERIZER_DESC));
		RasterDesc.FillMode = D3D11_FILL_SOLID;
		RasterDesc.CullMode = D3D11_CULL_NONE;
		RasterDesc.DepthClipEnable = TRUE;
		V_RETURN(pd3dDevice->CreateRasterizerState(&RasterDesc, &m_pRasterizerStateSolid));
		DXUT_SetDebugName(m_pRasterizerStateSolid, "Solid");

		RasterDesc.FillMode = D3D11_FILL_WIREFRAME;
		V_RETURN(pd3dDevice->CreateRasterizerState(&RasterDesc, &m_pRasterizerStateWireframe));
		DXUT_SetDebugName(m_pRasterizerStateWireframe, "Wireframe");

		//Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &m_pGeneralTexSS));

		m_SimViewport.Width = m_CBperCall.heightMapReso.x;
		m_SimViewport.Height = m_CBperCall.heightMapReso.y;
		m_SimViewport.MinDepth = 0.0f;
		m_SimViewport.MaxDepth = 1.0f;
		m_SimViewport.TopLeftX = 0;
		m_SimViewport.TopLeftY = 0;

		XMVECTORF32 vecEye = { 30.0f, 30.0f, 50.0f, 0.f };
		XMVECTORF32 vecAt = { 0.0f, 0.0f, 0.0f, 0.f };
		m_Camera.SetViewParams(vecEye, vecAt);
		m_Camera.SetRadius(50, 35, 80);
		//m_Camera.SetRadius(m_CBperCall.heightMapReso.x*m_CBperCall.cellSize*1.2,
		//				   m_CBperCall.heightMapReso.x*m_CBperCall.cellSize*0.1,
		//				   m_CBperCall.heightMapReso.x*m_CBperCall.cellSize * 10);

		ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
		pd3dImmediateContext->UpdateSubresource(m_pCBperCall, 0, NULL, &m_CBperCall, 0, 0);
		pd3dImmediateContext->RSSetState(m_pRasterizerStateSolid);
		// Create resource for normalGenerator
		// Create resource for Shoal

		// Create resource for skybox
		V_RETURN(m_pSkyBox->CreateResource(pd3dDevice));
		V_RETURN(m_pFlock->CreateResource(pd3dDevice, pd3dImmediateContext));
		V_RETURN(m_pGlowEffect->CreateResource(pd3dDevice));
		//SAFE_RELEASE(pd3dImmediateContext);

		return hr;
	}

	HRESULT Resize(ID3D11Device* pd3dDevice, UINT width, UINT height)
	{
		HRESULT hr = S_OK;

		//Create rendertaget resource
		D3D11_TEXTURE2D_DESC	RTtextureDesc = { 0 };
		RTtextureDesc.Width = width;
		RTtextureDesc.Height = height;
		RTtextureDesc.MipLevels = 1;
		RTtextureDesc.ArraySize = 1;
		RTtextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		RTtextureDesc.SampleDesc.Count = 1;
		RTtextureDesc.Usage = D3D11_USAGE_DEFAULT;
		RTtextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		RTtextureDesc.CPUAccessFlags = 0;
		RTtextureDesc.MiscFlags = 0;

		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc, NULL, &m_pOutputTexture2D));
		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc, NULL, &m_pOutReflectionTexture2D));
		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc, NULL, &m_pOutRefractionTexture2D));

		D3D11_SHADER_RESOURCE_VIEW_DESC RTshaderResourceDesc;
		RTshaderResourceDesc.Format = RTtextureDesc.Format;
		RTshaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		RTshaderResourceDesc.Texture2D.MostDetailedMip = 0;
		RTshaderResourceDesc.Texture2D.MipLevels = 1;
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pOutputTexture2D, &RTshaderResourceDesc, &m_pOutputTextureSRV));
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pOutReflectionTexture2D, &RTshaderResourceDesc, &m_pOutReflectionTextureSRV));
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pOutRefractionTexture2D, &RTshaderResourceDesc, &m_pOutRefractionTextureSRV));

		D3D11_RENDER_TARGET_VIEW_DESC	RTviewDesc;
		RTviewDesc.Format = RTtextureDesc.Format;
		RTviewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTviewDesc.Texture2D.MipSlice = 0;
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pOutputTexture2D, &RTviewDesc, &m_pOutputTextureRTV));
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pOutReflectionTexture2D, &RTviewDesc, &m_pOutReflectionTextureRTV));
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pOutRefractionTexture2D, &RTviewDesc, &m_pOutRefractionTextureRTV));

		//Create DepthStencil buffer and view
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXUTGetDeviceSettings().d3d11.AutoDepthStencilFormat;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pOutputStencilTexture2D);
		hr = pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pOutReflectionStencilTexture2D);
		hr = pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pOutRefractionStencilTexture2D);

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		// Create the depth stencil view
		V_RETURN(pd3dDevice->CreateDepthStencilView(m_pOutputStencilTexture2D, &descDSV, &m_pOutputStencilView));
		V_RETURN(pd3dDevice->CreateDepthStencilView(m_pOutReflectionStencilTexture2D, &descDSV, &m_pOutReflectionStencilView));
		V_RETURN(pd3dDevice->CreateDepthStencilView(m_pOutRefractionStencilTexture2D, &descDSV, &m_pOutRefractionStencilView));

		m_RTviewport.Width = (float)width;
		m_RTviewport.Height = (float)height;
		m_RTviewport.MinDepth = 0.0f;
		m_RTviewport.MaxDepth = 1.0f;
		m_RTviewport.TopLeftX = 0;
		m_RTviewport.TopLeftY = 0;

		m_RTRefractViewport.Width = (float)width;
		m_RTRefractViewport.Height = (float)height;
		m_RTRefractViewport.MinDepth = 0.0f;
		m_RTRefractViewport.MaxDepth = 1.0f;
		m_RTRefractViewport.TopLeftX = 0;
		m_RTRefractViewport.TopLeftY = 0;

		m_rendertargetWidth = width;
		m_rendertargetHeight = height;

		// Setup the camera's projection parameters
		float fAspectRatio = width / (FLOAT)height;
		m_Camera.SetProjParams(m_fFov, fAspectRatio, 0.1f, 100.0f);
		m_Camera.SetWindow(width, height);
		m_Camera.SetButtonMasks(MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_RIGHT_BUTTON);

		m_pFlock->Resize();
		m_pSkyBox->Resize(pd3dDevice, width, height);
		m_pGlowEffect->Resize(pd3dDevice,width,height);
		return hr;
	}

	void Update(float fElapsedTime, double fTime)
	{
		m_Camera.FrameMove(fElapsedTime);
		float3 lightDir;
		//XMVECTOR lDir = -XMVector3Normalize(XMLoadFloat3(&float3(sin(fTime*0.5), 1, 1)));
		//XMVECTOR lDir = XMVector3Normalize( -m_Camera.GetEyePt());
		XMVECTOR lDir = -XMVector3Normalize(XMLoadFloat3(&float3(sin(fTime*0.1) * 2 - 3,
																10,
																cos(fTime*0.1)*2.2 - 5)));
		XMStoreFloat3(&lightDir, lDir);
		m_CBperFrame.f3LightDir = lightDir;
		m_CBperFrame.f2CausticTexOffset = float2(0, 0);
		m_pFlock->Update(fElapsedTime);
	}

	void RenderWaterSurface(ID3D11DeviceContext* pd3dImmediateContext)
	{
		UINT Stride = sizeof(float3);
		UINT Offset = 0;
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pWaterVB, &Stride, &Offset);
		pd3dImmediateContext->IASetInputLayout(m_pWaterRenderIL);
		pd3dImmediateContext->IASetIndexBuffer(m_pWaterRenderIB, DXGI_FORMAT_R16_UINT, 0);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pd3dImmediateContext->VSSetShader(m_pWaterRenderVS, NULL, 0);
		pd3dImmediateContext->GSSetShader(m_pWaterRenderGS, NULL, 0);
		pd3dImmediateContext->PSSetShader(m_pWaterRenderPS, NULL, 0);
		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->VSSetConstantBuffers(1, 1, &m_pCBperFrame);
		pd3dImmediateContext->PSSetConstantBuffers(1, 1, &m_pCBperFrame);
		pd3dImmediateContext->VSSetShaderResources(0, 1, &m_pWaterPosVelSRV[1 - m_uCurrentSRVIdx]);
		pd3dImmediateContext->VSSetShaderResources(1, 1, &m_pNormalGenerator->m_pOutputTextureRV);
		pd3dImmediateContext->PSSetShaderResources(4, 1, &m_pOutReflectionTextureSRV);
		pd3dImmediateContext->PSSetShaderResources(5, 1, &m_pOutRefractionTextureSRV);
		pd3dImmediateContext->VSSetSamplers(0, 1, &m_pGeneralTexSS);
		pd3dImmediateContext->PSSetSamplers(0, 1, &m_pGeneralTexSS);
		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);

		ID3D11BlendState* temp;
		FLOAT		tempFactor[4];
		UINT			tempMask;
		pd3dImmediateContext->OMGetBlendState(&temp, tempFactor, &tempMask);
		pd3dImmediateContext->OMSetBlendState(m_pCausticBS, NULL, 0xffffffff);

		if (m_bWireframe) pd3dImmediateContext->RSSetState(m_pRasterizerStateWireframe);
		pd3dImmediateContext->DrawIndexed(m_iNumRenderIndices, 0, 0);
		if (m_bWireframe) pd3dImmediateContext->RSSetState(m_pRasterizerStateSolid);
		ID3D11ShaderResourceView* pSRVNULLs[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
		pd3dImmediateContext->PSSetShaderResources(0, 6, pSRVNULLs);

		pd3dImmediateContext->OMSetBlendState(temp, tempFactor, tempMask);
		SAFE_RELEASE(temp);

		pd3dImmediateContext->VSSetShaderResources(0, 2, pSRVNULLs);
	}
	void RenderCausticTex(ID3D11DeviceContext* pd3dImmediateContext)
	{
		// Render Caustic Texture
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pCausticRTV, NULL);
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		pd3dImmediateContext->ClearRenderTargetView(m_pCausticRTV, ClearColor);
		UINT Stride = sizeof(float3);
		UINT Offset = 0;
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pWaterVB, &Stride, &Offset);
		pd3dImmediateContext->IASetInputLayout(m_pWaterRenderIL);
		pd3dImmediateContext->IASetIndexBuffer(m_pWaterRenderIB, DXGI_FORMAT_R16_UINT, 0);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pd3dImmediateContext->RSSetViewports(1, &m_CausticViewport);
		pd3dImmediateContext->VSSetShader(m_pCausticVS, NULL, 0);
		pd3dImmediateContext->GSSetShader(m_pCausticGS, NULL, 0);
		pd3dImmediateContext->PSSetShader(m_pCausticPS, NULL, 0);
		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->VSSetConstantBuffers(1, 1, &m_pCBperFrame);
		pd3dImmediateContext->VSSetShaderResources(0, 1, &m_pWaterPosVelSRV[1 - m_uCurrentSRVIdx]);
		pd3dImmediateContext->VSSetShaderResources(1, 1, &m_pNormalGenerator->m_pOutputTextureRV);
		pd3dImmediateContext->VSSetSamplers(0, 1, &m_pGeneralTexSS);
		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->GSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->GSSetConstantBuffers(1, 1, &m_pCBperFrame);
		pd3dImmediateContext->RSSetState(m_pRasterizerStateSolid);

		ID3D11BlendState* temp;
		FLOAT		tempFactor[4];
		UINT			tempMask;
		pd3dImmediateContext->OMGetBlendState(&temp, tempFactor, &tempMask);
		pd3dImmediateContext->OMSetBlendState(m_pCausticBS, NULL, 0xffffffff);
		pd3dImmediateContext->DrawIndexed(m_iNumRenderIndices, 0, 0);

		pd3dImmediateContext->OMSetBlendState(temp, tempFactor, tempMask);
		ID3D11ShaderResourceView* pSRVNULLs[2] = { NULL, NULL };
		pd3dImmediateContext->VSSetShaderResources(0, 2, pSRVNULLs);
		SAFE_RELEASE(temp);
	}
	void RenderPoolWall(ID3D11DeviceContext* pd3dImmediateContext, bool cullBack = false)
	{
		UINT Stride = sizeof(PoolWall_Point);
		UINT Offset = 0;
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pPoolWallVB, &Stride, &Offset);
		pd3dImmediateContext->IASetInputLayout(m_pPoolWallIL);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		pd3dImmediateContext->VSSetShader(m_pPoolWallVS, NULL, 0);
		pd3dImmediateContext->GSSetShader(m_pPoolWallGS, NULL, 0);
		pd3dImmediateContext->PSSetShader(m_pPoolWallPS, NULL, 0);
		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->VSSetConstantBuffers(1, 1, &m_pCBperFrame);
		pd3dImmediateContext->PSSetConstantBuffers(1, 1, &m_pCBperFrame);
		pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pWaterPosVelSRV[1 - m_uCurrentSRVIdx]);
		pd3dImmediateContext->PSSetShaderResources(2, 1, &m_pPoolWallSRV);
		pd3dImmediateContext->PSSetShaderResources(3, 1, &m_pCausticSRV);
		pd3dImmediateContext->PSSetSamplers(0, 1, &m_pGeneralTexSS);
		ID3D11RasterizerState* temp;
		pd3dImmediateContext->RSGetState(&temp);
		if (!cullBack) pd3dImmediateContext->RSSetState(m_pPoolWallRSBackFace);
		else pd3dImmediateContext->RSSetState(m_pPoolWallRSFrontFace);
		pd3dImmediateContext->Draw(ARRAYSIZE(m_PoolWallMeshVertices), 0);
		pd3dImmediateContext->RSSetState(temp);
		SAFE_RELEASE(temp);

	}
	float2 FindVirtualRefractCam(float2 camXY)
	{
		XMVECTOR vLpos = XMLoadFloat2(&float2(-m_CBperCall.cellSize*m_CBperCall.heightMapReso.x*0.5, 0));
		XMVECTOR vRpos = XMLoadFloat2(&float2(m_CBperCall.cellSize*m_CBperCall.heightMapReso.x*0.5, 0));
		XMVECTOR vNormal = XMLoadFloat2(&float2(0, 1));
		XMVECTOR vReal = XMLoadFloat2(&camXY);
		XMVECTOR vLiRay = XMVector2Normalize(vLpos - vReal);
		XMVECTOR vRiRay = XMVector2Normalize(vRpos - vReal);
		XMVECTOR vLrefractRay = XMVector2Refract(vLiRay, vNormal, 1.0 / 1.33333);
		XMVECTOR vRrefractRay = XMVector2Refract(vRiRay, vNormal, 1.0 / 1.33333);
		XMVECTOR vXpositiveDir = XMLoadFloat2(&float2(1, 0));
		float fLxDir, fRxDir;
		XMStoreFloat(&fLxDir, XMVector2Dot(-vLrefractRay, vXpositiveDir));
		XMStoreFloat(&fRxDir, XMVector2Dot(-vRrefractRay, vXpositiveDir));
		float fVelDiff = fRxDir - fLxDir;
		float fDist;
		XMStoreFloat(&fDist, vRpos - vLpos);
		float t = fDist / fVelDiff;
		if (!_finite(t)) t = 2e16;
		XMVECTOR vVirtual = vLpos + t*vLrefractRay;
		float2 fVirtual;
		XMStoreFloat2(&fVirtual, vVirtual);
		return fVirtual;
	}
	void Render(ID3D11DeviceContext* pd3dImmediateContext)
	{
		XMMATRIX m_Proj = m_Camera.GetProjMatrix();
		XMMATRIX m_View = m_Camera.GetViewMatrix();
		XMMATRIX m_World = XMMatrixIdentity();
		XMMATRIX m_ViewProj = m_View*m_Proj;

		float3 f3ViewDir;
		XMStoreFloat3(&f3ViewDir, XMVector3Normalize(m_Camera.GetEyePt()));
		m_CBperFrame.f3ViewDir = f3ViewDir;

		float3 f3Eyepos;
		XMStoreFloat3(&f3Eyepos, m_Camera.GetEyePt());
		m_CBperFrame.f3Eyepos = f3Eyepos;

		m_CBperFrame.mViewProj = XMMatrixTranspose(m_ViewProj);
		pd3dImmediateContext->UpdateSubresource(m_pCBperFrame, 0, NULL, &m_CBperFrame, 0, 0);

		m_pNormalGenerator->m_pInputDepthTextureRV = m_pWaterPosVelSRV[m_uCurrentSRVIdx];
		m_pNormalGenerator->ProcessImage(pd3dImmediateContext);

		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		pd3dImmediateContext->ClearRenderTargetView(m_pOutputTextureRTV, ClearColor);
		pd3dImmediateContext->ClearDepthStencilView(m_pOutputStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Render Caustic Texture
		RenderCausticTex(pd3dImmediateContext);

		// Render Surface
		m_CBperFrame.mViewProj = XMMatrixTranspose(m_ViewProj);
		pd3dImmediateContext->UpdateSubresource(m_pCBperFrame, 0, NULL, &m_CBperFrame, 0, 0);
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pOutputTextureRTV, m_pOutputStencilView);
		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);
		RenderWaterSurface(pd3dImmediateContext);


		// Render pool wall with normal cam
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pOutputTextureRTV, m_pOutputStencilView);
		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);
		RenderPoolWall(pd3dImmediateContext, false);
		m_pFlock->Render(pd3dImmediateContext, m_CBperCall.waterSurface_y, m_CBperCall.waterDepth_y,
						 m_CBperCall.heightMapReso, m_CBperCall.cellSize,
						 m_CBperFrame.f3LightDir, m_pCausticSRV, m_RTviewport, m_ViewProj, f3Eyepos);

		// Render reflection texture
		XMMATRIX mReflectView;
		XMFLOAT4X4 f4x4ReflectView;
		XMStoreFloat4x4(&f4x4ReflectView, m_View);

		f4x4ReflectView._12 = -f4x4ReflectView._12;
		f4x4ReflectView._32 = -f4x4ReflectView._32;
		f4x4ReflectView._21 = -f4x4ReflectView._21;
		f4x4ReflectView._23 = -f4x4ReflectView._23;
		f4x4ReflectView._42 = -f4x4ReflectView._42;

		XMMATRIX m_offset = XMMatrixTranslation(0, -m_CBperCall.waterSurface_y*2.0f, 0);

		mReflectView = XMLoadFloat4x4(&f4x4ReflectView);
		m_CBperFrame.mViewProj = XMMatrixTranspose(m_offset*mReflectView*m_Proj);
		m_CBperFrame.mReflectViewProj = m_CBperFrame.mViewProj;
		pd3dImmediateContext->UpdateSubresource(m_pCBperFrame, 0, NULL, &m_CBperFrame, 0, 0);
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pOutReflectionTextureRTV, m_pOutReflectionStencilView);
		pd3dImmediateContext->ClearRenderTargetView(m_pOutReflectionTextureRTV, ClearColor);
		pd3dImmediateContext->ClearDepthStencilView(m_pOutReflectionStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		RenderPoolWall(pd3dImmediateContext, false);
		m_pSkyBox->Render(pd3dImmediateContext, &m_CBperFrame.mViewProj);
		float3 cameraXYZ;
		XMStoreFloat3(&cameraXYZ, m_Camera.GetEyePt());
		if (cameraXYZ.y < m_CBperCall.waterSurface_y)
			m_pFlock->Render(pd3dImmediateContext, m_CBperCall.waterSurface_y, m_CBperCall.waterDepth_y,
						 m_CBperCall.heightMapReso, m_CBperCall.cellSize,
						 m_CBperFrame.f3LightDir, m_pCausticSRV, m_RTviewport, m_offset*mReflectView*m_Proj, f3Eyepos);


		// Render refraction texture
		float3 eyeRelatedPos;
		XMStoreFloat3(&eyeRelatedPos, m_Camera.GetEyePt() - XMLoadFloat3(&float3(0.0f, m_CBperCall.waterSurface_y, 0.0f)));
		float2 result = FindVirtualRefractCam(float2(sqrt(eyeRelatedPos.x*eyeRelatedPos.x + eyeRelatedPos.z*eyeRelatedPos.z), eyeRelatedPos.y));
		if (abs(result.x) < 0.00001) result.x = 0.00001;
		float newY = sqrt(eyeRelatedPos.x*eyeRelatedPos.x + eyeRelatedPos.z*eyeRelatedPos.z) * result.y / result.x;
		float3 newEyeRelatedPos = float3(eyeRelatedPos.x, newY, eyeRelatedPos.z);
		XMMATRIX mRefractView = XMMatrixLookAtLH(XMLoadFloat3(&newEyeRelatedPos) + XMLoadFloat3(&float3(0.0f, m_CBperCall.waterSurface_y, 0.0f)),
													XMLoadFloat3(&float3(0.0f, m_CBperCall.waterSurface_y, 0.0f)),
													XMLoadFloat3(&float3(0.0f, 1.0f, 0.0f)));
		float fOrigFocusDist;
		XMStoreFloat(&fOrigFocusDist, XMVector3Length(XMLoadFloat3(&eyeRelatedPos)));
		float fNewFocusDist;
		XMStoreFloat(&fNewFocusDist, XMVector3Length(XMLoadFloat3(&newEyeRelatedPos)));
		float fFov = atan(fOrigFocusDist * tan(m_fFov) / fNewFocusDist);
		if (fFov < 0.0001) fFov = 0.0001;
		float fAspectRatio = m_rendertargetWidth / (FLOAT)m_rendertargetHeight;
		XMMATRIX mRefractProj = XMMatrixPerspectiveFovLH(fFov, fAspectRatio, 0.1, 100);
		if (cameraXYZ.y < m_CBperCall.waterSurface_y){
			m_CBperFrame.mRefractViewProj = XMMatrixTranspose(m_ViewProj);
			m_CBperFrame.mViewProj = XMMatrixTranspose(m_ViewProj);
		} else{
			m_CBperFrame.mViewProj = XMMatrixTranspose(mRefractView*mRefractProj);
			m_CBperFrame.mRefractViewProj = m_CBperFrame.mViewProj;
		}
		pd3dImmediateContext->UpdateSubresource(m_pCBperFrame, 0, NULL, &m_CBperFrame, 0, 0);
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pOutRefractionTextureRTV, m_pOutRefractionStencilView);
		pd3dImmediateContext->ClearRenderTargetView(m_pOutRefractionTextureRTV, ClearColor);
		pd3dImmediateContext->ClearDepthStencilView(m_pOutRefractionStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		pd3dImmediateContext->RSSetViewports(1, &m_RTRefractViewport);
		RenderPoolWall(pd3dImmediateContext, false);
		if (cameraXYZ.y >= m_CBperCall.waterSurface_y){
			m_pFlock->Render(pd3dImmediateContext, m_CBperCall.waterSurface_y, m_CBperCall.waterDepth_y,
						 m_CBperCall.heightMapReso, m_CBperCall.cellSize,
						 m_CBperFrame.f3LightDir, m_pCausticSRV, m_RTRefractViewport, mRefractView*mRefractProj, f3Eyepos);
		}
		m_pSkyBox->Render(pd3dImmediateContext, &m_CBperFrame.mViewProj);
		m_pGlowEffect->Render(pd3dImmediateContext,&m_pOutputTextureSRV);
	}

	void Simulate(ID3D11DeviceContext* pd3dImmediateContext, float fElapsedTime)
	{
		if (m_bLeftButtonPressed) m_bDropWater = Pick(m_sMouse_x, m_sMouse_y);

		if (m_bDropWater){
			m_iFleeOn = 50;
			AddOneDrop(pd3dImmediateContext);
			m_bDropWater = false;
		}
		//if (Pick()) AddOneDrop(pd3dImmediateContext);
		pd3dImmediateContext->IASetInputLayout(m_pQuadIL);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		UINT stride = 0;
		UINT offset = 0;
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pQuadVB, &stride, &offset);
		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);
		pd3dImmediateContext->VSSetShader(m_pQuadVS, NULL, 0);
		pd3dImmediateContext->GSSetShader(m_pQuadGS, NULL, 0);
		pd3dImmediateContext->PSSetShader(m_pSimUpdateWaterPS, NULL, 0);
		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->PSSetConstantBuffers(1, 1, &m_pCBperFrame);
		m_uCurrentSRVIdx = 1 - m_uCurrentSRVIdx;
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pWaterPosVelRTV[m_uCurrentSRVIdx], NULL);
		pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pWaterPosVelSRV[1 - m_uCurrentSRVIdx]);
		pd3dImmediateContext->PSSetSamplers(0, 1, &m_pGeneralTexSS);
		pd3dImmediateContext->RSSetViewports(1, &m_SimViewport);
		pd3dImmediateContext->Draw(1, 0);
		ID3D11ShaderResourceView* pSRVNULLs = NULL;
		pd3dImmediateContext->PSSetShaderResources(0, 1, &pSRVNULLs);

		// Do flock simulation
		if (m_iFleeOn){
			m_iFleeOn--;
			m_pFlock->m_bParamChanged = true;
			m_pFlock->m_CBsimParams.fFleeFactor = 10;
			m_pFlock->m_CBsimParams.f3FleeSourcePos = XMFLOAT3(m_CBperCall.dropCenter.x,
															   m_CBperCall.waterSurface_y,
															   m_CBperCall.dropCenter.y);
		} else{
			if (m_pFlock->m_CBsimParams.fFleeFactor > 0.1){
				m_pFlock->m_CBsimParams.fFleeFactor = 0;
				m_pFlock->m_bParamChanged = true;
			}
		}
		m_pFlock->Simulate(pd3dImmediateContext);
	}

	bool IntersectTest(XMFLOAT4& orig, XMFLOAT4& dir, float2* result)
	{
		float t = (m_CBperCall.waterSurface_y - orig.y) / dir.y;
		//XMStoreFloat(&t, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(orig.x,orig.y,orig.z)),XMLoadFloat3(&XMFLOAT3(0,1,0))));
		result->x = orig.x + dir.x * t;
		result->y = orig.z + dir.z * t;
		if (abs(result->x) < (m_CBperCall.heightMapReso.x - 1)*0.5f*m_CBperCall.cellSize && abs(result->y) < (m_CBperCall.heightMapReso.y - 1)*0.5f*m_CBperCall.cellSize)
			return true;
		else
			return false;
	}

	bool Pick(short _x, short _y)
	{
		XMFLOAT4 vPickRayDir;
		XMFLOAT4 vPickRayPos;
		//if (GetCapture())
		{
			XMMATRIX* pmatProj = &m_Camera.GetProjMatrix();

			XMFLOAT4X4 matProj;
			XMStoreFloat4x4(&matProj, *pmatProj);

			XMStoreFloat4(&vPickRayPos, m_Camera.GetEyePt());

			// Compute the vector of the pick ray in screen space
			XMFLOAT4 v;


			v.x = (((2.0f * _x) / m_rendertargetWidth) - 1) / matProj._11;
			v.y = -(((2.0f * _y) / m_rendertargetHeight) - 1) / matProj._22;
			v.z = 1.0f;

			v.w = 0.0f;
			vPickRayDir = v;
			// Get the inverse view matrix
			const XMMATRIX matView = m_Camera.GetViewMatrix();
			const XMMATRIX matWorld = m_Camera.GetWorldMatrix();
			XMMATRIX mWorldView = matView;
			//XMMATRIX m = XMMatrixInverse(NULL, mWorldView);

			XMFLOAT4X4 m;
			XMStoreFloat4x4(&m, XMMatrixInverse(NULL, mWorldView));
			// Transform the screen space pick ray into 3D space
			vPickRayDir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
			vPickRayDir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
			vPickRayDir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
			vPickRayPos.x = m._41;
			vPickRayPos.y = m._42;
			vPickRayPos.z = m._43;

			float2 hitPos = float2(0, 0);
			bool hit = IntersectTest(vPickRayPos, vPickRayDir, &hitPos);

			if (hit){
				float dist_x = hitPos.x - m_f2PreviousDropPos.x;
				float dist_y = hitPos.y - m_f2PreviousDropPos.y;
				if (sqrt(dist_x*dist_x + dist_y*dist_y) < m_CBperCall.dropRadius*0.3)
					return false;
				else
				{
					m_f2PreviousDropPos = hitPos;
					m_CBperCall.dropCenter = hitPos;
					return true;
				}
			}
		}
		return false;
	}

	void AddOneDrop(ID3D11DeviceContext* pd3dImmediateContext)
	{
		//m_CBperCall.dropCenter = dropCenter;
		pd3dImmediateContext->UpdateSubresource(m_pCBperCall, 0, NULL, &m_CBperCall, 0, 0);

		pd3dImmediateContext->IASetInputLayout(m_pQuadIL);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		UINT stride = 0;
		UINT offset = 0;
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pQuadVB, &stride, &offset);
		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);
		pd3dImmediateContext->VSSetShader(m_pQuadVS, NULL, 0);
		pd3dImmediateContext->GSSetShader(m_pQuadGS, NULL, 0);
		pd3dImmediateContext->PSSetShader(m_pSimDropWaterPS, NULL, 0);
		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCBperCall);
		m_uCurrentSRVIdx = 1 - m_uCurrentSRVIdx;
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pWaterPosVelRTV[m_uCurrentSRVIdx], NULL);
		pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pWaterPosVelSRV[1 - m_uCurrentSRVIdx]);
		pd3dImmediateContext->PSSetSamplers(0, 1, &m_pGeneralTexSS);
		pd3dImmediateContext->RSSetViewports(1, &m_SimViewport);
		pd3dImmediateContext->Draw(1, 0);
		ID3D11ShaderResourceView* pSRVNULLs = NULL;
		pd3dImmediateContext->PSSetShaderResources(0, 1, &pSRVNULLs);
	}

	void Release()
	{
		SAFE_RELEASE(m_pOutputTexture2D);
		SAFE_RELEASE(m_pOutputTextureRTV);
		SAFE_RELEASE(m_pOutputTextureSRV);
		SAFE_RELEASE(m_pOutputStencilTexture2D);
		SAFE_RELEASE(m_pOutputStencilView);

		SAFE_RELEASE(m_pOutReflectionTexture2D);
		SAFE_RELEASE(m_pOutReflectionTextureRTV);
		SAFE_RELEASE(m_pOutReflectionTextureSRV);
		SAFE_RELEASE(m_pOutReflectionStencilTexture2D);
		SAFE_RELEASE(m_pOutReflectionStencilView);

		SAFE_RELEASE(m_pOutRefractionTexture2D);
		SAFE_RELEASE(m_pOutRefractionTextureRTV);
		SAFE_RELEASE(m_pOutRefractionTextureSRV);
		SAFE_RELEASE(m_pOutRefractionStencilTexture2D);
		SAFE_RELEASE(m_pOutRefractionStencilView);
		m_pSkyBox->Release();
		m_pGlowEffect->Release();
	}

	void Destroy()
	{
		SAFE_RELEASE(m_pWaterRenderVS);
		SAFE_RELEASE(m_pWaterRenderGS);
		SAFE_RELEASE(m_pWaterRenderPS);
		SAFE_RELEASE(m_pWaterRenderIL);
		SAFE_RELEASE(m_pWaterVB);
		SAFE_RELEASE(m_pWaterRenderIB);

		SAFE_RELEASE(m_pCausticVS);
		SAFE_RELEASE(m_pCausticGS);
		SAFE_RELEASE(m_pCausticPS);

		SAFE_RELEASE(m_pQuadVS);
		SAFE_RELEASE(m_pQuadGS);
		SAFE_RELEASE(m_pSimUpdateWaterPS);
		SAFE_RELEASE(m_pSimDropWaterPS);
		SAFE_RELEASE(m_pQuadIL);
		SAFE_RELEASE(m_pQuadVB);

		SAFE_RELEASE(m_pCausticTex);
		SAFE_RELEASE(m_pCausticRTV);
		SAFE_RELEASE(m_pCausticSRV);
		SAFE_RELEASE(m_pCausticBS);

		SAFE_RELEASE(m_pPoolWallVS);
		SAFE_RELEASE(m_pPoolWallGS);
		SAFE_RELEASE(m_pPoolWallPS);
		SAFE_RELEASE(m_pPoolWallIL);
		SAFE_RELEASE(m_pPoolWallVB);
		SAFE_RELEASE(m_pPoolWallSRV);
		SAFE_RELEASE(m_pPoolWallRSBackFace);
		SAFE_RELEASE(m_pPoolWallRSFrontFace);


		SAFE_RELEASE(m_pCBperCall);
		SAFE_RELEASE(m_pGeneralTexSS);

		m_pNormalGenerator->Release();
		m_pSkyBox->Destroy();
		m_pGlowEffect->Destroy();
		//SAFE_RELEASE(m_pNormalGenerator);

		m_pFlock->Release();
		//SAFE_RELEASE(m_pFlock);

		for (int i = 0; i < 2; ++i){
			SAFE_RELEASE(m_pWaterPosVelTex[i]);
			SAFE_RELEASE(m_pWaterPosVelSRV[i]);
			SAFE_RELEASE(m_pWaterPosVelRTV[i]);
		}
		SAFE_RELEASE(m_pRasterizerStateSolid);
		SAFE_RELEASE(m_pRasterizerStateWireframe);

		SAFE_RELEASE(m_pCBperFrame);
	}


	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);
		int nKey = static_cast<int>(wParam);
		switch (uMsg)
		{
		case WM_KEYDOWN:
			if (nKey == 'F')
			{
				m_bWireframe = !m_bWireframe;
			}
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			m_bLeftButtonPressed = true;
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			m_bLeftButtonPressed = false;
			break;

		case WM_CAPTURECHANGED:
			if ((HWND)lParam != hWnd)
			{
				ReleaseCapture();
			}
			break;
		}

		m_sMouse_x = (short)lParam;
		m_sMouse_y = (short)(lParam >> 16);

		return 0;
	}
};