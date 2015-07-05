#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "DXUTgui.h"

#include "TiledTextures.h"
#include "Water.h"

#include "header.h"


using namespace std::placeholders;


//--------------------------------------------------------------------------------------
//Global Variables
//--------------------------------------------------------------------------------------

float								g_fDeltaTime = 0.01;	// Global dt for simulation
float								g_fThreasholdDt = 0.08;	// Global maximum dt allowed for simulation
float								g_fdtAccumulator = 0;	// Global accumulator for dt
bool								g_bSimulationOn = true;

// Define aquarium size
XMFLOAT3							g_f3WaterBodyCenter = XMFLOAT3( 0, -1.5, 0 );
XMFLOAT3							g_f3WaterXYZRadius = XMFLOAT3( 10, 5, 10);

TiledTextures						MultiTexture = TiledTextures(true);
Water								AquariumWater = Water(g_fDeltaTime, g_f3WaterBodyCenter,g_f3WaterXYZRadius,
														  SUB_TEXTUREWIDTH, SUB_TEXTUREHEIGHT);

//--------------------------------------------------------------------------------------
//Initialization
//--------------------------------------------------------------------------------------
HRESULT Initial()
{ 
	HRESULT hr = S_OK;

	V_RETURN( MultiTexture.Initial() );
	V_RETURN(AquariumWater.Initial());

	MultiTexture.AddTexture(&AquariumWater.m_pGlowEffect->m_pOurSRV,SUB_TEXTUREWIDTH,SUB_TEXTUREHEIGHT,
							1.f, 0.f, "","<float4>",
							std::bind(&Water::Resize, &AquariumWater, _1, _2, _3),
							std::bind(&Water::Release, &AquariumWater),
							std::bind(&Water::HandleMessages,&AquariumWater,_1,_2,_3,_4));
	/*MultiTexture.AddTexture(&AquariumWater.m_pGlowEffect->m_pGlow_HV_SRV,
							SUB_TEXTUREWIDTH*AquariumWater.m_pGlowEffect->m_CBperResize.glowScale,
							SUB_TEXTUREHEIGHT*AquariumWater.m_pGlowEffect->m_CBperResize.glowScale);*/
	return hr;
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
									   DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	MultiTexture.ModifyDeviceSettings( pDeviceSettings );
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
									  void* pUserContext )
{
	HRESULT hr = S_OK;
	V_RETURN( AquariumWater.CreateResource( pd3dDevice));
	V_RETURN(MultiTexture.CreateResource(pd3dDevice));
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										  const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;
	V_RETURN(MultiTexture.Resize(pd3dDevice, pBackBufferSurfaceDesc));
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	AquariumWater.Update(fElapsedTime,fTime);
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
								  double fTime, float fElapsedTime, void* pUserContext )
{
	// Decoupe simulation from rendering
	if (fElapsedTime > g_fThreasholdDt) fElapsedTime = g_fThreasholdDt;
	g_fdtAccumulator += fElapsedTime;
	while (g_fdtAccumulator >= g_fDeltaTime){
		AquariumWater.Simulate(pd3dImmediateContext, g_fDeltaTime);
		g_fdtAccumulator -= g_fDeltaTime;
	}
	AquariumWater.Render(pd3dImmediateContext);
	MultiTexture.Render( pd3dImmediateContext );
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	MultiTexture.Release();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	MultiTexture.Destroy();
	AquariumWater.Destroy();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						  bool* pbNoFurtherProcessing, void* pUserContext )
{
	MultiTexture.HandleMessages(hWnd, uMsg, wParam, lParam);
	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	if (bKeyDown)
	{
		switch (nChar)
		{
		case VK_SPACE:
			g_bSimulationOn=!g_bSimulationOn;
			if(g_bSimulationOn)
				DXUTGetGlobalTimer()->Start();
			else
				DXUTGetGlobalTimer()->Stop();
			break;
		}
	}
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
					   bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
					   int xPos, int yPos, void* pUserContext )
{
}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// DXUT will create and use the best device (either D3D9 or D3D11) 
	// that is available on the system depending on which D3D callbacks are set below

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMouse( OnMouse );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );


	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

	// Perform any application-level initialization here

	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	
	Initial();
	
	DXUTCreateWindow( L"Aquarium" );

	// Only require 10-level hardware
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, SUB_TEXTUREWIDTH, SUB_TEXTUREHEIGHT);
	DXUTMainLoop(); // Enter into the DXUT render loop

	// Perform any application-level cleanup here

	return DXUTGetExitCode();
}


