#pragma once

#include "binaryReader.h"
#include "texture.h"

class DDSLoader: private BinaryReader
{
public:
	bool LoadFromFile(const char *fileName, TextureDesc &desc);
	void Close();
};
