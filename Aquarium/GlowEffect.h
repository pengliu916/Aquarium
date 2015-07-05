#pragma once

#include <D3D11.h>
#include <DirectXMath.h>
#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"

#include "header.h"

#ifndef SUB_TEXTUREWIDTH
#define SUB_TEXTUREWIDTH 640
#endif

#ifndef SUB_TEXTUREHEIGHT
#define SUB_TEXTUREHEIGHT 480
#endif

using namespace std;

struct GE_ConstBuffer
{
	float InvFinalReso_x;
	float InvFinalReso_y;
	float InvTempReso_x;
	float InvTempReso_y;
	float glow_factor;
	float threshold;
	float glowScale;
	float niu;
};

class GlowEffect
{
public:
	ID3D11VertexShader*				m_pPassVS;
	ID3D11PixelShader*				m_pPS_H;
	ID3D11PixelShader*				m_pPS_V;
	ID3D11PixelShader*				m_pPS_Combine;
	ID3D11PixelShader*				m_pPS_UpDownSample;
	ID3D11GeometryShader*			m_pQuadGS;
	ID3D11InputLayout*				m_pIL;
	ID3D11Buffer*					m_pVB;

	ID3D11SamplerState*				m_pGeneralSS;

	ID3D11Texture2D*				m_pGlow_H_Tex;
	ID3D11Texture2D*				m_pGlow_HV_Tex;
	ID3D11Texture2D*				m_pUpDownSampleTex;
	ID3D11Texture2D*				m_pOutTex;
	ID3D11RenderTargetView*			m_pGlow_H_RTV;
	ID3D11RenderTargetView*			m_pGlow_HV_RTV;
	ID3D11RenderTargetView*			m_pUpDownSampledRTV;
	ID3D11RenderTargetView*			m_pOutRTV;
	ID3D11ShaderResourceView*		m_pGlow_H_SRV;
	ID3D11ShaderResourceView*		m_pGlow_HV_SRV;
	ID3D11ShaderResourceView*		m_pUpDownSampledSRV;
	ID3D11ShaderResourceView*		m_pOurSRV;

	ID3D11BlendState*				m_pOutBS;

	D3D11_VIEWPORT					m_RTviewport;

	GE_ConstBuffer					m_CBperResize;
	ID3D11Buffer*					m_pCBperResize;

	GlowEffect()
	{
		
		m_CBperResize.glow_factor = 0.5f;
		m_CBperResize.threshold = 0.3f;
		//m_CBperResize.glow_factor = 0.9f;
		//m_CBperResize.threshold = 0.1f;
		m_CBperResize.glowScale = 0.75f;
	}

	HRESULT Initial()
	{
		HRESULT hr = S_OK;
		return hr;
	}

	HRESULT CreateResource(ID3D11Device* pd3dDevice)
	{
		HRESULT hr = S_OK;

		ID3DBlob* pVSBlob = NULL;
		wstring filename = L"GlowEffect.fx";
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "VS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pPassVS));

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "GS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &m_pQuadGS));
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "PS_Glow_H", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPS_H));
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "PS_Glow_V", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPS_V));
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "PS_Glow_UpDown", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPS_UpDownSample));
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "PS_Glow_ALL", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPS_Combine));
		pPSBlob->Release();

		D3D11_INPUT_ELEMENT_DESC layout[] = { { "POSITION", 0, DXGI_FORMAT_R16_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
		V_RETURN(pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pIL));
		pVSBlob->Release();

		// Create the vertex buffer
		D3D11_BUFFER_DESC bd = { 0 };
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(short);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pVB));

		// Create the constant buffers
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.ByteWidth = sizeof(GE_ConstBuffer);
		V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pCBperResize));
		DXUT_SetDebugName(m_pCBperResize, "m_pCBperResize");

		// Create the blend state
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = false;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;        ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;     ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		//blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F ;
		V_RETURN(pd3dDevice->CreateBlendState(&blendDesc, &m_pOutBS));

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &m_pGeneralSS));

		return hr;
	}

	HRESULT Resize(ID3D11Device* pd3dDevice, UINT width, UINT height){
		HRESULT hr = S_OK;

		m_CBperResize.InvFinalReso_x = 1.f / width;
		m_CBperResize.InvFinalReso_y = 1.f / height;

		m_CBperResize.InvTempReso_x = m_CBperResize.InvFinalReso_x / m_CBperResize.glowScale;
		m_CBperResize.InvTempReso_y = m_CBperResize.InvFinalReso_y / m_CBperResize.glowScale;

		ID3D11DeviceContext* pd3dImmediateContext =	DXUTGetD3D11DeviceContext();
		pd3dImmediateContext->UpdateSubresource(m_pCBperResize, 0, NULL, &m_CBperResize, 0, 0);

		//Create rendertaget resource
		D3D11_TEXTURE2D_DESC	RTtextureDesc = { 0 };
		RTtextureDesc.Width = width * m_CBperResize.glowScale;
		RTtextureDesc.Height = height * m_CBperResize.glowScale;

		RTtextureDesc.MipLevels = 1;
		RTtextureDesc.ArraySize = 1;
		RTtextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		RTtextureDesc.SampleDesc.Count = 1;
		RTtextureDesc.Usage = D3D11_USAGE_DEFAULT;
		RTtextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		RTtextureDesc.CPUAccessFlags = 0;
		RTtextureDesc.MiscFlags = 0;

		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc, NULL, &m_pGlow_H_Tex));
		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc, NULL, &m_pGlow_HV_Tex));
		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc, NULL, &m_pUpDownSampleTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC RTshaderResourceDesc;
		RTshaderResourceDesc.Format = RTtextureDesc.Format;
		RTshaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		RTshaderResourceDesc.Texture2D.MostDetailedMip = 0;
		RTshaderResourceDesc.Texture2D.MipLevels = 1;
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pGlow_H_Tex, &RTshaderResourceDesc, &m_pGlow_H_SRV));
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pGlow_HV_Tex, &RTshaderResourceDesc, &m_pGlow_HV_SRV));
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pUpDownSampleTex, &RTshaderResourceDesc, &m_pUpDownSampledSRV));

		D3D11_RENDER_TARGET_VIEW_DESC	RTviewDesc;
		RTviewDesc.Format = RTtextureDesc.Format;
		RTviewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTviewDesc.Texture2D.MipSlice = 0;
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pGlow_H_Tex, &RTviewDesc, &m_pGlow_H_RTV));
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pGlow_HV_Tex, &RTviewDesc, &m_pGlow_HV_RTV));
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pUpDownSampleTex, &RTviewDesc, &m_pUpDownSampledRTV));

		RTtextureDesc.Width = width;
		RTtextureDesc.Height = height;
		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc, NULL, &m_pOutTex));

		RTshaderResourceDesc.Format = RTtextureDesc.Format;
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pOutTex, &RTshaderResourceDesc, &m_pOurSRV));

		RTviewDesc.Format = RTtextureDesc.Format;
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pOutTex, &RTviewDesc, &m_pOutRTV));

		m_RTviewport.MinDepth = 0.0f;
		m_RTviewport.MaxDepth = 1.0f;
		m_RTviewport.TopLeftX = 0;
		m_RTviewport.TopLeftY = 0;
	}
	~GlowEffect()
	{

	}

	void Release()
	{
		SAFE_RELEASE(m_pGlow_H_Tex);
		SAFE_RELEASE(m_pGlow_H_RTV);
		SAFE_RELEASE(m_pGlow_H_SRV);
		SAFE_RELEASE(m_pGlow_HV_Tex);
		SAFE_RELEASE(m_pGlow_HV_RTV);
		SAFE_RELEASE(m_pGlow_HV_SRV);
		SAFE_RELEASE(m_pOutTex);
		SAFE_RELEASE(m_pOurSRV);
		SAFE_RELEASE(m_pOutRTV);
		SAFE_RELEASE(m_pUpDownSampleTex);
		SAFE_RELEASE(m_pUpDownSampledRTV);
		SAFE_RELEASE(m_pUpDownSampledSRV);
	}

	void Destroy(){
		SAFE_RELEASE(m_pPassVS);
		SAFE_RELEASE(m_pPS_H);
		SAFE_RELEASE(m_pPS_V);
		SAFE_RELEASE(m_pPS_Combine);
		SAFE_RELEASE(m_pPS_UpDownSample);
		SAFE_RELEASE(m_pQuadGS);
		SAFE_RELEASE(m_pIL);
		SAFE_RELEASE(m_pVB);

		SAFE_RELEASE(m_pCBperResize);
		SAFE_RELEASE(m_pOutBS);

		SAFE_RELEASE(m_pGeneralSS);
	}

	void Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView** pInputSRV)
	{
		pd3dImmediateContext->IASetInputLayout(m_pIL);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		UINT stride = 0;
		UINT offset = 0;
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);
		pd3dImmediateContext->OMSetBlendState(m_pOutBS, NULL, 0xffffffff);
		pd3dImmediateContext->PSSetSamplers(0, 1, &m_pGeneralSS);

		m_RTviewport.Height = 1.f / m_CBperResize.InvTempReso_y;
		m_RTviewport.Width = 1.f / m_CBperResize.InvTempReso_x;
		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);
		
		pd3dImmediateContext->GSSetConstantBuffers(0, 1, &m_pCBperResize);
		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCBperResize);
		pd3dImmediateContext->VSSetShader(m_pPassVS, NULL, 0);
		pd3dImmediateContext->GSSetShader(m_pQuadGS, NULL, 0);

		pd3dImmediateContext->OMSetRenderTargets(1, &m_pUpDownSampledRTV, NULL);
		pd3dImmediateContext->PSSetShader(m_pPS_UpDownSample, NULL, 0);
		pd3dImmediateContext->PSSetShaderResources(0, 1, pInputSRV);
		pd3dImmediateContext->Draw(1, 0);

		pd3dImmediateContext->OMSetRenderTargets(1, &m_pGlow_H_RTV, NULL);
		pd3dImmediateContext->PSSetShader(m_pPS_H, NULL, 0);
		pd3dImmediateContext->PSSetShaderResources(1, 1, &m_pUpDownSampledSRV);
		pd3dImmediateContext->Draw(1, 0);

		pd3dImmediateContext->OMSetRenderTargets(1, &m_pGlow_HV_RTV, NULL);
		pd3dImmediateContext->PSSetShader(m_pPS_V, NULL, 0);
		pd3dImmediateContext->PSSetShaderResources(2, 1, &m_pGlow_H_SRV);
		pd3dImmediateContext->Draw(1, 0);

		m_RTviewport.Height = 1.f / m_CBperResize.InvFinalReso_y;
		m_RTviewport.Width = 1.f / m_CBperResize.InvFinalReso_x;
		pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);

		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		//pd3dImmediateContext->OMSetRenderTargets(1,&m_pOutRTV,m_pOutputStencilView);
		pd3dImmediateContext->OMSetRenderTargets(1, &m_pOutRTV, NULL);
		pd3dImmediateContext->ClearRenderTargetView(m_pOutRTV, ClearColor);
		pd3dImmediateContext->PSSetShader(m_pPS_Combine, NULL, 0);
		pd3dImmediateContext->PSSetShaderResources(3, 1, &m_pGlow_HV_SRV);
		pd3dImmediateContext->Draw(1, 0);

		ID3D11ShaderResourceView* ppSRVNULL[4] = { NULL, NULL, NULL, NULL };
		pd3dImmediateContext->PSSetShaderResources(0, 4, ppSRVNULL);

	}

};