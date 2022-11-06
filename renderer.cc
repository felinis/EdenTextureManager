#include "renderer.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include "texture.h"
#include <stdio.h>

static ID3D11Device* g_pd3dDevice;
static ID3D11DeviceContext* g_pd3dDeviceContext;
static IDXGISwapChain* g_pSwapChain;
static ID3D11RenderTargetView* g_mainRenderTargetView;

struct TextureInfo
{
	ID3D11Texture2D* tex;
	ID3D11ShaderResourceView* srv;
};
#define MAX_TEXTURES 512
int sTexturesCount;
static TextureInfo sTextures[MAX_TEXTURES];

static u32 GetStrideOrPitch(TEXTURE_FORMAT tf, u32 width)
{
	return (tf == TEXTURE_FORMAT::BC3_UNorm) ? width << 2 : width << 1; //TEMP!
}

ResourceID Renderer::CreateTexture(const TextureDesc& texDesc)
{
	assert(sTexturesCount < MAX_TEXTURES);

	D3D11_TEXTURE2D_DESC desc = {0};
	desc.Width = texDesc.width;
	desc.Height = texDesc.height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = (DXGI_FORMAT)texDesc.format;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;//D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA data = {0};
	data.pSysMem = texDesc.bits;
	data.SysMemPitch = GetStrideOrPitch(texDesc.format, texDesc.width);

	ID3D11Texture2D* tex;
	if FAILED(g_pd3dDevice->CreateTexture2D(&desc, &data, &tex))
		return ResourceID::INVALID;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	ID3D11ShaderResourceView* srv;
	if FAILED(g_pd3dDevice->CreateShaderResourceView(tex, &srvDesc, &srv))
		return ResourceID::INVALID;

	/* zapisz do listy */
	ResourceID id = sTexturesCount;
	TextureInfo* ti = &sTextures[id];
	ti->tex = tex;
	ti->srv = srv;

	return sTexturesCount++;
}

bool Renderer::ReplaceTexture(const TextureDesc& texDesc, ResourceID id)
{
	D3D11_TEXTURE2D_DESC desc = {0};
	desc.Width = texDesc.width;
	desc.Height = texDesc.height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = (DXGI_FORMAT)texDesc.format;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;//D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA data = {0};
	data.pSysMem = texDesc.bits;
	data.SysMemPitch = GetStrideOrPitch(texDesc.format, texDesc.width);

	ID3D11Texture2D* tex;
	if FAILED(g_pd3dDevice->CreateTexture2D(&desc, &data, &tex))
		return ResourceID::INVALID;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	ID3D11ShaderResourceView* srv;
	if FAILED(g_pd3dDevice->CreateShaderResourceView(tex, &srvDesc, &srv))
		return ResourceID::INVALID;

	/* uwolnij poprzedni¹ teksturê na tym miejscu */
	DeleteTexture(id);

	/* zast¹p now¹ */
	TextureInfo* ti = &sTextures[id];
	ti->tex = tex;
	ti->srv = srv;
	
	return true;
}

void Renderer::DeleteTexture(ResourceID id)
{
	assert(id < sTexturesCount);
	TextureInfo* ti = &sTextures[id];

	if (ti->srv)
	{
		ti->srv->Release();
		ti->srv = nullptr;
	}
	if (ti->tex)
	{
		ti->tex->Release();
		ti->tex = nullptr;
	}
}

void* Renderer::GetImGuiTexture(ResourceID id)
{
	assert(id < sTexturesCount);
	TextureInfo* ti = &sTextures[id];

	return ti->srv;
}
#if 0
static ID3D11Texture2D* staging;
static u32 lastSubresource;
bool Renderer::LockTexture(ResourceID id, void **bits)
{
	assert(staging == nullptr);
	assert(id < sTexturesCount);
	TextureInfo* ti = &sTextures[id];

	lastSubresource = D3D11CalcSubresource(0, 0, 0);

	/* try to copy the texture to a staging texture */
	D3D11_TEXTURE2D_DESC desc{};
	ti->tex->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	if FAILED(g_pd3dDevice->CreateTexture2D(&desc, nullptr, &staging))
		return false;

	g_pd3dDeviceContext->CopyResource(staging, ti->tex);

	D3D11_MAPPED_SUBRESOURCE sub{};
	HRESULT hr = g_pd3dDeviceContext->Map(staging, lastSubresource, D3D11_MAP_READ, 0, &sub);
	if SUCCEEDED(hr)
	{
		/* copy the texture bits */
		*bits = sub.pData;

		return true;
	}
	
	UnlockTexture(id);
	return false;
}

void Renderer::UnlockTexture(ResourceID id)
{
	g_pd3dDeviceContext->Unmap(staging, lastSubresource);
	staging->Release();
	staging = nullptr;
}

void Renderer::GetTextureDesc(ResourceID id, TextureDesc& td, u32 *size)
{
	assert(id < sTexturesCount);
	TextureInfo* ti = &sTextures[id];

	D3D11_TEXTURE2D_DESC desc;
	ti->tex->GetDesc(&desc);
	td.width = desc.Width;
	td.height = desc.Height;
	td.format = (TEXTURE_FORMAT)desc.Format;
	*size = GetStrideOrPitch(td.format, td.width) * td.height;
}
#endif

bool Renderer::InitImGui()
{
	return ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

bool Renderer::CreateDeviceD3D(void *hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = (HWND)hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = (DXGI_SWAP_EFFECT)4;//DXGI_SWAP_EFFECT_FLIP_DISCARD;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;
	
	CreateRenderTarget();
	return true;
}

void Renderer::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); }

	/* report live objects */
//	ID3D11Debug* debug;
//	if (SUCCEEDED(g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug)))
//	{
//		debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
//	}
}

void Renderer::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void Renderer::CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void Renderer::NewImGuiFrame()
{
	ImGui_ImplDX11_NewFrame();
}

void Renderer::ImGuiShutdown()
{
	ImGui_ImplDX11_Shutdown();
}

bool Renderer::IsReady()
{
	return g_pd3dDevice != 0;
}

void Renderer::Resize(u32 width, u32 height)
{
	CleanupRenderTarget();
	g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	CreateRenderTarget();
}

void Renderer::Render()
{
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, 0);
//	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(1, 0); // Present with vsync
	//g_pSwapChain->Present(0, 0); // Present without vsync
}
