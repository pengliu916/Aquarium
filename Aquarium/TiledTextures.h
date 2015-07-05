#pragma once

#include <D3D11.h>
#include"DXUT.h"
#include <DirectXMath.h>
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include <sstream>
#include <vector>
#include <functional>
#include "header.h"

#ifndef COMPILE_FLAG
#define COMPILE_FLAG D3DCOMPILE_ENABLE_STRICTNESS
#endif

#ifndef SUB_TEXTUREWIDTH
#define SUB_TEXTUREWIDTH 800
#endif // !SUB_TEXTUREWIDTH

#ifndef SUB_TEXTUREHEIGHT
#define SUB_TEXTUREHEIGHT 600
#endif // !SUB_TEXTUREHEIGHT


using namespace std;
using namespace DirectX;

HRESULT CompileFormString(string code,
						  const D3D_SHADER_MACRO* pDefines,
						  LPCSTR pEntrypoint, LPCSTR pTarget,
						  UINT Flags1, UINT Flags2,
						  ID3DBlob** ppCode){
	HRESULT hr;
	UNREFERENCED_PARAMETER(pDefines);

#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	Flags1 |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompile(code.c_str(), code.size(), NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pEntrypoint, pTarget, Flags1, Flags2, ppCode, &pErrorBlob);
#pragma warning( suppress : 6102 )
	if (pErrorBlob)
	{
		OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		pErrorBlob->Release();
	}

	return hr;
};

struct TiledTexObj{
	ID3D11ShaderResourceView**		ppInputSRV;
	string							strTexFormat;
	string							strPScode;
	UINT							uResWidth;
	UINT							uResHeight;
	UINT							uOutWidth;	// This will be computed by TiledTexture
	UINT							uOutHeight;	// This will be computed by TiledTexture
	UINT							uLTcorner_x;
	UINT							uLTcorner_y;
	float							fValueMax;
	float							fvalueMin;
	function<HRESULT(ID3D11Device*, UINT, UINT)> resizeFunc;
	function<LRESULT(HWND, UINT, WPARAM, LPARAM)> msgFunc;
	function<void()> releaseFunc;
	TiledTexObj(){
		ppInputSRV = NULL;
		resizeFunc = nullptr;
		msgFunc = nullptr;
		releaseFunc = nullptr;
	}
};

class TiledTextures
{
public:
	ID3D11VertexShader*             m_pPassVS;
	ID3D11GeometryShader*           m_pTiledQuadGS;
	ID3D11PixelShader*              m_pTexPS;
	ID3D11InputLayout*              m_pPassIL;
	ID3D11Buffer*                   m_pPassVB;

	vector<TiledTexObj>             m_vecTiledObjs;

	ID3D11SamplerState*             m_pSS;

	ID3D11Texture2D*                m_pOutTex;
	ID3D11RenderTargetView*         m_pOutRTV;
	ID3D11ShaderResourceView*       m_pOutSRV;

	ID3D11ShaderResourceView**      m_ppInputSRVs;
	ID3D11ShaderResourceView**      m_ppNullSRVs;

	D3D11_VIEWPORT                  m_RTviewport;

	UINT                            m_uTexCount;
	UINT                            m_uTextureWidth;
	UINT                            m_uTextureHeight;
	UINT                            m_uRTwidth;
	UINT                            m_uRTheight;
	UINT                            m_uTileCount_x;
	UINT                            m_uTileCount_y;
	float                           m_fRTaspectRatio;

	XMFLOAT4*                       m_pf4TileLocation;
	ID3D11Buffer*                   m_pCBTileLocation;

	XMFLOAT4*                       m_pf4TexSizeValueRange;
	ID3D11Buffer*                   m_pCBTexSize;

	bool                            m_bDirectToBackBuffer;

	void ComputeTileLocation(UINT uScreenWidth, UINT uScreenHeight){
		float screenAspectRatio = (float)uScreenWidth / (float)uScreenHeight;
		float normalTileAspectRatio = (float)m_uTextureWidth / (float)m_uTextureHeight;
		float tileRatio_hv = screenAspectRatio / normalTileAspectRatio;
		float fTileNum_h = sqrt(tileRatio_hv*tileRatio_hv*m_vecTiledObjs.size());
		float fTileNum_v = fTileNum_h / tileRatio_hv / tileRatio_hv;
		int tileNum_h = (int)floor(fTileNum_h);
		if (tileNum_h > (int)m_vecTiledObjs.size()) tileNum_h = (int)m_vecTiledObjs.size();
		if (tileNum_h == 0) tileNum_h = 1;
		int tileNum_v = (int)floor(fTileNum_v);
		if (tileNum_v > (int)m_vecTiledObjs.size()) tileNum_v = (int)m_vecTiledObjs.size();
		if (tileNum_v == 0) tileNum_v = 1;
		while (tileNum_v*tileNum_h < m_vecTiledObjs.size()){
			if (fTileNum_h - tileNum_h <= fTileNum_v - tileNum_v) tileNum_v++;
			else tileNum_h++;
		}

		m_uTileCount_x = tileNum_h;
		m_uTileCount_y = tileNum_v;

		float normalSubScreen_x = 2.f / tileNum_h; // size of subscreen in screen space, x axis
		float normalSubScreen_y = 2.f / tileNum_v; // size of subscreen in screen space, y axis

		for (int i = 0; i < m_vecTiledObjs.size(); i++){
			float tileCenterPos_x = -1.f + (i % tileNum_h + 0.5f) * normalSubScreen_x;
			float tileCenterPos_y = 1.f - (i / tileNum_h + 0.5f) * normalSubScreen_y;
			float tileAspectRatio = (float)m_vecTiledObjs[i].uResWidth / (float)m_vecTiledObjs[i].uResHeight;

			float halfTileOffset_x, halfTileOffset_y;
			if (tileAspectRatio < normalTileAspectRatio){
				halfTileOffset_y = normalSubScreen_y * 0.5f;
				halfTileOffset_x = normalSubScreen_x * tileAspectRatio / normalTileAspectRatio * 0.5f;
			} else{
				halfTileOffset_x = normalSubScreen_x * 0.5f;
				halfTileOffset_y = normalSubScreen_y * normalTileAspectRatio / tileAspectRatio * 0.5f;
			}
			m_pf4TileLocation[i] = XMFLOAT4(tileCenterPos_x - halfTileOffset_x,
											tileCenterPos_y + halfTileOffset_y,
											tileCenterPos_x + halfTileOffset_x,
											tileCenterPos_y - halfTileOffset_y);
			m_vecTiledObjs[i].uLTcorner_x = (i % tileNum_h);
			m_vecTiledObjs[i].uLTcorner_y = (i / tileNum_h);
		}
	}

	void StringReplaceAll(string& inputStr, string& targetStr, string& replacementStr)
	{
		std::size_t startPos = 0;
		std::size_t index;
		while ((index = inputStr.find(targetStr, startPos)) != string::npos){
			inputStr.replace(index, targetStr.length(), replacementStr);
			startPos = index + replacementStr.length();
		}
		return;
	}

	string GenerateShaderCode()
	{
		stringstream  shaderCode;
		for (int i = 0; i < m_vecTiledObjs.size(); i++){
			shaderCode << "Texture2D" << m_vecTiledObjs[i].strTexFormat << " textures_" << i << " : register(t"<<i<<");\n";
		}
		//int idx = 0;
		//for(auto i:m_vecTiledObjs)
		//    shaderCode << "Texture2D" << i.strTexFormat << " textures_" << idx++ << ";\n";
		//
		shaderCode << "SamplerState samColor : register(s0);\n";
		shaderCode << "cbuffer cbTileLocation : register(b0)\n";
		shaderCode << "{\n float4 g_f4TilePos[" << m_vecTiledObjs.size() << "];\n }; \n";
		shaderCode << "cbuffer cbTileSize : register(b1)\n";
		shaderCode << "{\n float4 g_f4TexSizeValueRange[" << m_vecTiledObjs.size() << "];\n }; \n";
		shaderCode << "struct GS_INPUT{};\n";
		shaderCode << "struct PS_INPUT{float4 Pos:SV_POSITION; sample float2 Tex:TEXCOORD0;uint PrimID:SV_PrimitiveID;};\n";
		shaderCode << "GS_INPUT VS(){GS_INPUT output=(GS_INPUT)0;return output;}\n";
		m_uTileCount_x = (UINT)ceil(sqrt(m_vecTiledObjs.size()));// Tile count along horizontal axis
		m_uTileCount_y = (UINT)ceil(m_vecTiledObjs.size() / (float)m_uTileCount_x);// Tile count along vertical axis

		// For generating geometry shader
		shaderCode << "[maxvertexcount(4)]\n";
		shaderCode << "void GS(point GS_INPUT particles[1], uint primID:SV_PrimitiveID, inout TriangleStream<PS_INPUT> triStream){\n";
		shaderCode << "   PS_INPUT output;\n";
		shaderCode << "   float offset=(float)primID;\n";
		shaderCode << "   output.Pos=float4(g_f4TilePos[primID].x,g_f4TilePos[primID].y,0.f,1.f);\n";
		shaderCode << "   output.Tex=float2(0.f,0.f);\n";
		shaderCode << "   output.PrimID=primID;\n";
		shaderCode << "   triStream.Append(output);\n";
		shaderCode << "   output.Pos=float4(g_f4TilePos[primID].z,g_f4TilePos[primID].y,0.f,1.f);\n";
		shaderCode << "   output.Tex=float2(g_f4TexSizeValueRange[primID].x,0.f);\n";
		shaderCode << "   output.PrimID=primID;\n";
		shaderCode << "   triStream.Append(output);\n";
		shaderCode << "   output.Pos=float4(g_f4TilePos[primID].x,g_f4TilePos[primID].w,0.f,1.f);\n";
		shaderCode << "   output.Tex=float2(0.f,g_f4TexSizeValueRange[primID].y);\n";
		shaderCode << "   output.PrimID=primID;\n";
		shaderCode << "   triStream.Append(output);\n";
		shaderCode << "   output.Pos=float4(g_f4TilePos[primID].z,g_f4TilePos[primID].w,0.f,1.f);\n";
		shaderCode << "   output.Tex=float2(g_f4TexSizeValueRange[primID].x,g_f4TexSizeValueRange[primID].y);\n";
		shaderCode << "   output.PrimID=primID;\n";
		shaderCode << "   triStream.Append(output);\n";
		shaderCode << "}\n";

		// For generating pixel shader
		shaderCode << "float4 PS(PS_INPUT input):SV_Target{\n";
		shaderCode << "   float4 color;\n";
		shaderCode << "   [branch] switch(input.PrimID){\n";

		size_t index;
		for (int i = 0; i < m_vecTiledObjs.size(); i++){
			string targetStr = "texture.";
			
			stringstream replace;
			string replacement;
			replace << "textures_" << i << ".";
			replacement = replace.str();
			StringReplaceAll(m_vecTiledObjs[i].strPScode,targetStr, replacement);

			// replace general texture size with specific texture size
			targetStr = "texSize";
			stringstream replace1;
			string replacement1;
			replace1 << "g_f4TexSizeValueRange[" << i <<"]";
			replacement1 = replace1.str();
			StringReplaceAll(m_vecTiledObjs[i].strPScode, targetStr, replacement1);

			shaderCode << "case " << i << ":\n";
			shaderCode << m_vecTiledObjs[i].strPScode << "\n";
		}
		shaderCode << "default:\n";
		shaderCode << "color=float4(input.Tex.xy,0,1);\n";
		shaderCode << "return color;\n";
		shaderCode << "}}\n" << endl;
		return shaderCode.str();
	}

	TiledTextures(bool bRenderToBackbuffer = true, UINT width = SUB_TEXTUREWIDTH, UINT height = SUB_TEXTUREHEIGHT)
	{
		m_bDirectToBackBuffer = bRenderToBackbuffer;
		m_uTextureWidth = width;
		m_uTextureHeight = height;
	}

	HRESULT Initial()
	{
		return S_OK;
	}

	void ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings)
	{
		pDeviceSettings->d3d11.sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	void AddTexture(ID3D11ShaderResourceView** ppInputSRV,
					UINT _uResWidth = SUB_TEXTUREWIDTH, UINT _uResHeight = SUB_TEXTUREHEIGHT,
					float _maxValue = 1.f, float _minValue = 0.f,
					string strPScode = "",
					string strFormat = "<float4>",
					function<HRESULT(ID3D11Device*, UINT, UINT)> _resizeFunc = nullptr,
					function<void()> _releaseFunc = nullptr,
					function<LRESULT(HWND, UINT, WPARAM, LPARAM)> _msgFunc = nullptr)
	{
		TiledTexObj texObj;
		string defaultPScode;
		texObj.uResWidth = _uResWidth;
		texObj.uResHeight = _uResHeight;
		texObj.strTexFormat = strFormat;
		texObj.fValueMax = _maxValue;
		texObj.fvalueMin = _minValue;
		texObj.resizeFunc = _resizeFunc;
		texObj.releaseFunc = _releaseFunc;
		texObj.msgFunc = _msgFunc;
		if (texObj.resizeFunc==nullptr && strFormat.compare(1, 5, "float") == 0){
			defaultPScode = "color = texture.SampleLevel(samColor, input.Tex/texSize,0);\n return color;";
			//defaultPScode = "color = float4(input.Tex, texSize.xy);\n return color;";
		} else{
			std::ostringstream buff;
			//buff << "color=float4(input.Tex,texSize.xy);\n return color;";
			buff << "color=texture.Load(int3(input.Tex,0));\n return color;";
			defaultPScode = buff.str();
		}
		texObj.ppInputSRV = ppInputSRV;
		texObj.strPScode = strPScode.empty() ? defaultPScode : strPScode;
		m_vecTiledObjs.push_back(texObj);
	}

	HRESULT CreateResource(ID3D11Device* pd3dDevice)
	{
		HRESULT hr = S_OK;

		// Generate tile location array for shader
		m_pf4TileLocation = new XMFLOAT4[m_vecTiledObjs.size()];
		// Generate texture sizes array for shader
		m_pf4TexSizeValueRange = new XMFLOAT4[m_vecTiledObjs.size()];
		// Create constant buffer for rendering (for creating quads in render GS)
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		//cbDesc.Usage = D3D11_USAGE_DYNAMIC;			//Dynamic usage uses map and unmap to upload
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = 0;
		//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.ByteWidth = (UINT)m_vecTiledObjs.size() * sizeof(XMFLOAT4);
		V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pCBTileLocation));
		DXUT_SetDebugName(m_pCBTileLocation,"m_pCBTileLocation");
		V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pCBTexSize));
		DXUT_SetDebugName(m_pCBTexSize, "m_pCBTexSize");


		string shaderCode = GenerateShaderCode();

		ID3DBlob* pVSBlob = NULL;

		V_RETURN(CompileFormString(shaderCode, nullptr, "VS", "vs_5_0", COMPILE_FLAG, 0, &pVSBlob))
			V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pPassVS));
		DXUT_SetDebugName(m_pPassVS, "m_pPassVS");

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(CompileFormString(shaderCode, nullptr, "GS", "gs_5_0", COMPILE_FLAG, 0, &pGSBlob))
			V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &m_pTiledQuadGS));
		DXUT_SetDebugName(m_pTiledQuadGS, "m_pTiledQuadGS");
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(CompileFormString(shaderCode, nullptr, "PS", "ps_5_0", COMPILE_FLAG, 0, &pPSBlob))
			V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pTexPS));
		DXUT_SetDebugName(m_pTexPS, "m_pTexPS");
		pPSBlob->Release();

		D3D11_INPUT_ELEMENT_DESC layout[] = { { "POSITION", 0, DXGI_FORMAT_R16_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
		V_RETURN(pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pPassIL));
		DXUT_SetDebugName(m_pPassIL, "m_pPassIL");
		pVSBlob->Release();

		// Create the vertex buffer
		D3D11_BUFFER_DESC bd = { 0 };
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(short);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pPassVB));
		DXUT_SetDebugName(m_pPassVB, "m_pPassVB");

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &m_pSS));
		DXUT_SetDebugName(m_pSS, "m_pSS");

		m_uTexCount = (UINT)m_vecTiledObjs.size();

		m_ppInputSRVs = new ID3D11ShaderResourceView*[m_uTexCount];
		m_ppNullSRVs = new ID3D11ShaderResourceView*[m_uTexCount];

		for (UINT i = 0; i < m_uTexCount; i++)
			m_ppNullSRVs[i] = NULL;

		return hr;
	}

	void SetupPipeline(ID3D11DeviceContext* pd3dImmediateContext)
	{
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pOutRTV, NULL);
		pd3dImmediateContext->IASetInputLayout(m_pPassIL);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		UINT stride = 0;
		UINT offset = 0;
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pPassVB, &stride, &offset);
		pd3dImmediateContext->VSSetShader(m_pPassVS, NULL, 0);
		pd3dImmediateContext->GSSetShader(m_pTiledQuadGS, NULL, 0);
		pd3dImmediateContext->PSSetShader(m_pTexPS, NULL, 0);
		pd3dImmediateContext->GSSetConstantBuffers(0, 1, &m_pCBTileLocation);
		pd3dImmediateContext->GSSetConstantBuffers(1, 1, &m_pCBTexSize);

		int idx = 0;
		for (auto item : m_vecTiledObjs)
			m_ppInputSRVs[idx++] = item.ppInputSRV ? *item.ppInputSRV : NULL;

		pd3dImmediateContext->PSSetShaderResources(0, (UINT)m_vecTiledObjs.size(), m_ppInputSRVs);
		pd3dImmediateContext->PSSetConstantBuffers(1, 1, &m_pCBTexSize);
		pd3dImmediateContext->PSSetSamplers(0, 1, &m_pSS);
		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);

		//float ClearColor[4] = { 0.2f, 0.2f, 0.2f, 0.0f };
		//pd3dImmediateContext->ClearRenderTargetView( m_pOutRTV, ClearColor );
	}

	HRESULT Resize(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
	{
		HRESULT hr = S_OK;

		int winWidth = pBackBufferSurfaceDesc->Width;
		int winHeight = pBackBufferSurfaceDesc->Height;

		// Create rendertarget resource
		if (!m_bDirectToBackBuffer)
		{
			D3D11_TEXTURE2D_DESC   RTtextureDesc = { 0 };
			RTtextureDesc.Width = winWidth;
			RTtextureDesc.Height = winHeight;
			RTtextureDesc.MipLevels = 1;
			RTtextureDesc.ArraySize = 1;
			RTtextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			//RTtextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

			RTtextureDesc.SampleDesc.Count = 1;
			RTtextureDesc.SampleDesc.Quality = 0;
			RTtextureDesc.Usage = D3D11_USAGE_DEFAULT;
			RTtextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			RTtextureDesc.CPUAccessFlags = 0;
			RTtextureDesc.MiscFlags = 0;
			V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc, NULL, &m_pOutTex));
			DXUT_SetDebugName(m_pOutTex, "m_pOutTex");

			D3D11_RENDER_TARGET_VIEW_DESC      RTViewDesc;
			ZeroMemory(&RTViewDesc, sizeof(RTViewDesc));
			RTViewDesc.Format = RTtextureDesc.Format;
			RTViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			RTViewDesc.Texture2D.MipSlice = 0;
			V_RETURN(pd3dDevice->CreateRenderTargetView(m_pOutTex, &RTViewDesc, &m_pOutRTV));
			DXUT_SetDebugName(m_pOutRTV, "m_pOutRTV");

			D3D11_SHADER_RESOURCE_VIEW_DESC      SRViewDesc;
			ZeroMemory(&SRViewDesc, sizeof(SRViewDesc));
			SRViewDesc.Format = RTtextureDesc.Format;
			SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRViewDesc.Texture2D.MostDetailedMip = 0;
			SRViewDesc.Texture2D.MipLevels = 1;
			V_RETURN(pd3dDevice->CreateShaderResourceView(m_pOutTex, &SRViewDesc, &m_pOutSRV));
			DXUT_SetDebugName(m_pOutSRV, "m_pOutSRV");
		}

		ComputeTileLocation(winWidth, winHeight);

		m_RTviewport.MinDepth = 0.0f;
		m_RTviewport.MaxDepth = 1.0f;

		m_fRTaspectRatio = (float)m_uTextureWidth*m_uTileCount_x / ((float)m_uTextureHeight*m_uTileCount_y);

		float winAspectRatio = (float)winWidth / (float)winHeight;

		if (winAspectRatio > m_fRTaspectRatio){
			m_RTviewport.Height = (float)winHeight;
			m_RTviewport.Width = (float)winHeight*m_fRTaspectRatio;
		} else{
			m_RTviewport.Width = (float)winWidth;
			m_RTviewport.Height = (float)winWidth / m_fRTaspectRatio;
		}

		m_RTviewport.TopLeftX = (winWidth - m_RTviewport.Width) / 2.f;
		m_RTviewport.TopLeftY = (winHeight - m_RTviewport.Height) / 2.f;
		if (m_bDirectToBackBuffer)
		{
			m_pOutRTV = DXUTGetD3D11RenderTargetView();
		}

		// Update the real resolution of each subtexture
		int subWidth = (int)m_RTviewport.Width / m_uTileCount_x;
		int subHeight = (int)m_RTviewport.Height / m_uTileCount_y;
		for (int i = 0; i < m_vecTiledObjs.size(); i++){
			m_vecTiledObjs[i].uOutWidth = subWidth;
			m_vecTiledObjs[i].uOutHeight = subHeight;
			m_pf4TexSizeValueRange[i] = XMFLOAT4((float)subWidth,(float)subHeight,m_vecTiledObjs[i].fValueMax,m_vecTiledObjs[i].fvalueMin);
			m_vecTiledObjs[i].uLTcorner_x *= subWidth;
			m_vecTiledObjs[i].uLTcorner_y *= subHeight;
			m_vecTiledObjs[i].uLTcorner_x += (int)m_RTviewport.TopLeftX;
			m_vecTiledObjs[i].uLTcorner_y += (int)m_RTviewport.TopLeftY;
		}

		// Updating constant buffer
		ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
		pd3dImmediateContext->UpdateSubresource(m_pCBTileLocation, 0, NULL, m_pf4TileLocation, 0, 0);
		pd3dImmediateContext->UpdateSubresource(m_pCBTexSize, 0, NULL, m_pf4TexSizeValueRange, 0, 0);

		// Call tile objects' resize function
		for (int i = 0; i < m_vecTiledObjs.size(); i++){
			if (m_vecTiledObjs[i].resizeFunc != nullptr){
				V_RETURN(m_vecTiledObjs[i].resizeFunc(pd3dDevice, m_vecTiledObjs[i].uOutWidth, m_vecTiledObjs[i].uOutHeight));
			}
		}

		return hr;
	}

	void Update()
	{

	}

	void Render(ID3D11DeviceContext* pd3dImmediateContext)
	{
		this->SetupPipeline(pd3dImmediateContext);
		// Clear the three render targets
		float ClearColor[4] = { 0.02f, 0.02f, 0.02f, -1.0f };

		pd3dImmediateContext->ClearRenderTargetView(m_pOutRTV, ClearColor);
		pd3dImmediateContext->Draw(m_uTexCount, 0);
		pd3dImmediateContext->PSSetShaderResources(0, m_uTexCount, m_ppNullSRVs);
	}

	void Release()
	{
		// Call tile objects' release function
		for (int i = 0; i < m_vecTiledObjs.size(); i++){
			if (m_vecTiledObjs[i].releaseFunc != nullptr){
				m_vecTiledObjs[i].releaseFunc();
			}
		}
	}

	void Destroy()
	{
		SAFE_RELEASE(m_pPassVS);
		SAFE_RELEASE(m_pTexPS);
		SAFE_RELEASE(m_pTiledQuadGS);
		SAFE_RELEASE(m_pPassIL);
		SAFE_RELEASE(m_pPassVB);
		SAFE_RELEASE(m_pSS);
		SAFE_RELEASE(m_pCBTileLocation);
		SAFE_RELEASE(m_pCBTexSize);
		if (!m_bDirectToBackBuffer)
		{
			SAFE_RELEASE(m_pOutTex);
			SAFE_RELEASE(m_pOutRTV);
			SAFE_RELEASE(m_pOutSRV);
		}
		delete[] m_ppInputSRVs;
		delete[] m_ppNullSRVs;
		delete m_pf4TileLocation;
		delete m_pf4TexSizeValueRange;
	}

	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		short curse_x, curse_y;
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		ScreenToClient(hWnd, &cursorPos);
		curse_x = (short)cursorPos.x;
		curse_y = (short)cursorPos.y;

		LPARAM nlParam = lParam;
		for (int i = 0; i<m_vecTiledObjs.size(); i++){
			if (m_vecTiledObjs[i].msgFunc != nullptr){

				if (curse_x > (short)m_vecTiledObjs[i].uLTcorner_x &&
					curse_x < (short)(m_vecTiledObjs[i].uLTcorner_x + m_vecTiledObjs[i].uOutWidth) &&
					curse_y > (short)m_vecTiledObjs[i].uLTcorner_y &&
					curse_y < (short)(m_vecTiledObjs[i].uLTcorner_y + m_vecTiledObjs[i].uOutHeight)){
					short new_x = curse_x - (short)m_vecTiledObjs[i].uLTcorner_x;
					short new_y = curse_y - (short)m_vecTiledObjs[i].uLTcorner_y;
					new_x = (short)(new_x);
					new_y = (short)(new_y);
					nlParam = ((long)new_y << 16) | (long)new_x;
					UINT nuMsg = uMsg;
					WPARAM nwParam = wParam;
					m_vecTiledObjs[i].msgFunc(hWnd, nuMsg, nwParam, nlParam);
				}

			}
		}
		return 0;
	}
};
