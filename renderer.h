#pragma once

#include "baseTypes.h"
#include "texture.h"

namespace Renderer
{
	ResourceID CreateTexture(const TextureDesc& td);
	bool ReplaceTexture(const TextureDesc& texDesc, ResourceID id);
	void DeleteTexture(ResourceID id);
	void* GetImGuiTexture(ResourceID id);
#if 0
	bool LockTexture(ResourceID id, void **bits);
	void UnlockTexture(ResourceID id);
	void GetTextureDesc(ResourceID id, TextureDesc& td, u32 *size);
#endif

	bool InitImGui();
	bool CreateDeviceD3D(void* hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

	void NewImGuiFrame();
	void ImGuiShutdown();

	bool IsReady();
	void Resize(u32 width, u32 height);
	void Render();
}
