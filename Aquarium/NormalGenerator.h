#pragma once

#include <D3D11.h>
#include <DirectXMath.h>
#include "DXUT.h"
#include "SDKmisc.h"
#include <iostream>

struct int2{
	int x;
	int y;
	int2(){
		x = 0; y = 0;
	}
	int2(int a, int b){
		x = a;
		y = b;
	}
};

using namespace std;
struct NG_cb_perCall{
	int2 widthHeight;
	int2 niu;
};

class NormalGenerator
{
public:
	ID3D11VertexShader*				m_pVertexShader;
	ID3D11PixelShader*				m_pPixelShader;
	ID3D11GeometryShader*			m_pGeometryShader;
	ID3D11InputLayout*				m_pVertexLayout;
	ID3D11Buffer*					m_pVertexBuffer; 

	ID3D11ShaderResourceView*		m_pInputDepthTextureRV;

	ID3D11Texture2D*				m_pOutputTexture2D;
	ID3D11RenderTargetView*			m_pOutputTextureRTV;
	ID3D11ShaderResourceView*		m_pOutputTextureRV;

	D3D11_VIEWPORT					m_RTviewport;

	NG_cb_perCall					m_CBperCall;
	ID3D11Buffer*					m_pCBperCall;

	UINT			m_rendertargetWidth;
	UINT			m_rendertargetHeight;

	NormalGenerator(UINT width = 640, UINT height = 480)
	{
		m_rendertargetWidth=width;
		m_rendertargetHeight=height;
		m_pInputDepthTextureRV=NULL;
		m_pOutputTexture2D=NULL;
		m_pOutputTextureRTV=NULL;
		m_pOutputTextureRV=NULL;

		m_CBperCall.widthHeight = int2(width, height);
	}

	HRESULT Initial()
	{
		HRESULT hr=S_OK;
		return hr;
	}

	HRESULT CreateResource(ID3D11Device* pd3dDevice,
		ID3D11ShaderResourceView* pInputTextureRV)
	{
		HRESULT hr=S_OK;

		ID3DBlob* pVSBlob = NULL;
		wstring filename=L"NormalGenerator.fx";
		V_RETURN(DXUTCompileFromFile((WCHAR*)filename.c_str(), nullptr, "VS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pVertexShader));

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(DXUTCompileFromFile((WCHAR*)filename.c_str(), nullptr, "GS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(),pGSBlob->GetBufferSize(),NULL,&m_pGeometryShader));
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(DXUTCompileFromFile((WCHAR*)filename.c_str(), nullptr, "PS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),NULL,&m_pPixelShader));
		pPSBlob->Release();

		D3D11_INPUT_ELEMENT_DESC layout[] = { { "POSITION", 0, DXGI_FORMAT_R16_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
		V_RETURN(pd3dDevice->CreateInputLayout(layout,ARRAYSIZE(layout),pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),&m_pVertexLayout));
		pVSBlob->Release();

		m_pInputDepthTextureRV=pInputTextureRV;
		// Create the constant buffer
		D3D11_BUFFER_DESC bd =
		{
			sizeof(NG_cb_perCall),
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_CONSTANT_BUFFER,
			0,
			0
		};
		V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pCBperCall));
		DXUT_SetDebugName(m_pCBperCall, "m_pCBperCall");

		// Create the vertex buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(short);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer));

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

		m_RTviewport.Width=(float)m_rendertargetWidth;
		m_RTviewport.Height=(float)m_rendertargetHeight;
		m_RTviewport.MinDepth=0.0f;
		m_RTviewport.MaxDepth=1.0f;
		m_RTviewport.TopLeftX = 0;
		m_RTviewport.TopLeftY = 0;

		auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
		pd3dImmediateContext->UpdateSubresource(m_pCBperCall, 0, NULL, &m_CBperCall, 0, 0);
		//SAFE_RELEASE(pd3dImmediateContext);
		return hr;
	}

	~NormalGenerator()
	{

	}

	void Release()
	{
		SAFE_RELEASE(m_pVertexShader);
		SAFE_RELEASE(m_pPixelShader);
		SAFE_RELEASE(m_pGeometryShader);
		SAFE_RELEASE(m_pVertexLayout);
		SAFE_RELEASE(m_pVertexBuffer);

		SAFE_RELEASE(m_pOutputTexture2D);
		SAFE_RELEASE(m_pOutputTextureRTV);
		SAFE_RELEASE(m_pOutputTextureRV);

		SAFE_RELEASE(m_pCBperCall);
	}
	void ProcessImage(ID3D11DeviceContext* pd3dImmediateContext)
	{

		//m_pOutputTextureRV=m_pOutputTextureRV;
		pd3dImmediateContext->IASetInputLayout(m_pVertexLayout);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		UINT stride = 0;
		UINT offset = 0;
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		pd3dImmediateContext->OMSetRenderTargets(1,&m_pOutputTextureRTV,NULL);
		pd3dImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
		pd3dImmediateContext->GSSetShader(m_pGeometryShader,NULL,0);	
		pd3dImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
		pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pInputDepthTextureRV);
		pd3dImmediateContext->GSSetConstantBuffers(0, 1, &m_pCBperCall);
		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCBperCall);

		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);
		pd3dImmediateContext->Draw(1,0);	
	}
};