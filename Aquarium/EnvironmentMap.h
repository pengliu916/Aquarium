#pragma once

#include <DirectXMath.h>
#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"

#ifndef D3DCOMPILE_FLAG
#define D3DCOMPILE_FLAG D3DCOMPILE_ENABLE_STRICTNESS
#endif
class EnvironmentMap
{
public:
	struct CB_PER_FRAME{
		XMMATRIX	mWordViewProj;
	};

	ID3D11Device*				m__pd3dDevice;

	// resource for environment map
	ID3D11ShaderResourceView*	m_pEnvironmentMapSRV;

	// pipeline resource for rendering environment map
	ID3D11VertexShader*			m_pVS;
	ID3D11InputLayout*			m_pIL;
	ID3D11PixelShader*			m_pPS;
	ID3D11Buffer*				m_pVB;
	ID3D11Buffer*				m_pCB;
	ID3D11SamplerState*			m_pSS;
	ID3D11DepthStencilState*	m_pDSS;

	float						m_fSize;
	EnvironmentMap(float fSize = 100.f){
		m_fSize = fSize;
	}

	HRESULT	CreateResource(ID3D11Device* _pd3dDevice){
		HRESULT hr = S_OK;

		ID3DBlob* pVSBlob = nullptr;
		V_RETURN(DXUTCompileFromFile(L"EnvironmentMap.fx",nullptr,"VS","vs_5_0",D3DCOMPILE_FLAG,0,&pVSBlob));
		V_RETURN(_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),nullptr,&m_pVS));
		DXUT_SetDebugName(m_pVS,"EnvironmentMap_VS");

		D3D11_INPUT_ELEMENT_DESC layout[] = {{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }};
		V_RETURN(_pd3dDevice->CreateInputLayout(layout,ARRAYSIZE(layout),pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),&m_pIL));
		DXUT_SetDebugName(m_pIL,"EnvironmentMap_IL");
		pVSBlob->Release();

		ID3DBlob* pPSBlob = nullptr;
		V_RETURN(DXUTCompileFromFile(L"EnvironmentMap.fx",nullptr,"PS","ps_5_0",D3DCOMPILE_FLAG,0,&pPSBlob));
		V_RETURN(_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),nullptr,&m_pPS));
		DXUT_SetDebugName(m_pPS,"EnvironmentMap_PS");
		pPSBlob->Release();

		// setup constant buffer
		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.MiscFlags = 0;
		Desc.ByteWidth = sizeof(CB_PER_FRAME);
		V_RETURN(_pd3dDevice->CreateBuffer(&Desc, NULL, &m_pCB));
		DXUT_SetDebugName(m_pCB, "CB_PER_FRAME");

		// setup sampler state
		D3D11_SAMPLER_DESC SamDesc;
		SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.MipLODBias = 0.0f;
		SamDesc.MaxAnisotropy = 1;
		SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
		SamDesc.MinLOD = 0;
		SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
		V_RETURN(_pd3dDevice->CreateSamplerState(&SamDesc, &m_pSS));
		DXUT_SetDebugName(m_pSS, "EnvironmentMap_SS");

		// get environment map cube texture
		WCHAR strPath[MAX_PATH];
		DXUTFindDXSDKMediaFileCch(strPath,MAX_PATH,L"misc\\sky.dds");
		V_RETURN(DXUTCreateShaderResourceViewFromFile(_pd3dDevice,strPath,&m_pEnvironmentMapSRV));
		DXUT_SetDebugName(m_pEnvironmentMapSRV,"EnvironmentMap_SRV");

		// depth stencil state
		D3D11_DEPTH_STENCIL_DESC DSDesc;
		ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		DSDesc.DepthEnable = TRUE;
		DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		DSDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		DSDesc.StencilEnable = FALSE;
		V_RETURN(_pd3dDevice->CreateDepthStencilState(&DSDesc, &m_pDSS));
		DXUT_SetDebugName(m_pDSS, "EnvironmentMap_DSS");

		return hr;
	}

	HRESULT Resize(ID3D11Device* _pd3dDevice, UINT _uWidth, UINT _uHeight ){
		HRESULT hr = S_OK;
		XMFLOAT4* pVertices = new XMFLOAT4[4];
		// map texels to pixels 
		float fHighW = -1.0f - (1.0f / (float)_uWidth);
		float fHighH = -1.0f - (1.0f / (float)_uHeight);
		float fLowW = 1.0f + (1.0f / (float)_uWidth);
		float fLowH = 1.0f + (1.0f / (float)_uHeight);

		pVertices[0] = XMFLOAT4(fLowW, fLowH, 1.0f, 1.0f);
		pVertices[1] = XMFLOAT4(fLowW, fHighH, 1.0f, 1.0f);
		pVertices[2] = XMFLOAT4(fHighW, fLowH, 1.0f, 1.0f);
		pVertices[3] = XMFLOAT4(fHighW, fHighH, 1.0f, 1.0f);

		UINT uiVertBufSize = 4 * sizeof(XMFLOAT4);
		//Vertex Buffer
		D3D11_BUFFER_DESC vbdesc;
		vbdesc.ByteWidth = uiVertBufSize;
		vbdesc.Usage = D3D11_USAGE_IMMUTABLE;
		vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbdesc.CPUAccessFlags = 0;
		vbdesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pVertices;
		V_RETURN(_pd3dDevice->CreateBuffer(&vbdesc, &InitData, &m_pVB));
		DXUT_SetDebugName(m_pVB, "EnvironmentMap_VB");
		SAFE_DELETE_ARRAY(pVertices);
		return hr;
	}

	void Render(ID3D11DeviceContext* _pd3dImmediateContext, XMMATRIX* _pmWorldViewProj){
		HRESULT hr;

		_pd3dImmediateContext->IASetInputLayout(m_pIL);

		UINT uStrides = sizeof(XMFLOAT4);
		UINT uOffsets = 0;
		ID3D11Buffer* pBuffers[1] = { m_pVB };
		_pd3dImmediateContext->IASetVertexBuffers(0, 1, pBuffers, &uStrides, &uOffsets);
		_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		_pd3dImmediateContext->VSSetShader(m_pVS, NULL, 0);
		_pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
		_pd3dImmediateContext->PSSetShader(m_pPS, NULL, 0);

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		V(_pd3dImmediateContext->Map(m_pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		CB_PER_FRAME* pVSPerObject = (CB_PER_FRAME*)MappedResource.pData;
		pVSPerObject->mWordViewProj = XMMatrixTranspose(XMMatrixInverse( NULL, *_pmWorldViewProj ));
		_pd3dImmediateContext->Unmap(m_pCB, 0);
		_pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pCB);

		_pd3dImmediateContext->PSSetSamplers(0, 1, &m_pSS);
		_pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pEnvironmentMapSRV);
		_pd3dImmediateContext->OMSetDepthStencilState(m_pDSS,0);
		_pd3dImmediateContext->Draw(4, 0);
	}

	void Release(){
		SAFE_RELEASE(m_pVB);
	}

	void Destroy(){
		SAFE_RELEASE(m_pVS);
		SAFE_RELEASE(m_pIL);
		SAFE_RELEASE(m_pPS);
		SAFE_RELEASE(m_pCB);
		SAFE_RELEASE(m_pSS);
		SAFE_RELEASE(m_pDSS);
		SAFE_RELEASE(m_pEnvironmentMapSRV);
	}
};