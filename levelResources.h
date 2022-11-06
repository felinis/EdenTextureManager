#pragma once

#include "texture.h"

namespace LevelResources
{
	bool Create(const char *fileName);
	void Destroy();

	u32 GetNumTextures();
	u32 GetNumTextureSlots();
	Texture& GetTexture(u32 slot);

	void ExportTexture(u32 slot, u32 frame);
	void ReplaceTexture(u32 slot, u32 frame);
}
