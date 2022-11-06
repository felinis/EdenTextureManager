/*
	ddsReader.cc - Reads DDS files and creates textures from it

	Moczulski Alan, 2022.
*/

#include "ddsLoader.h"
#include "baseTypes.h"

struct PixelFormat
{
	u32 size;
	u32 flags;
	u32 fourCC;
	u32 bitCount;
	u32 RBitMask;
	u32 GBitMask;
	u32 BBitMask;
	u32 ABitMask;
};

struct Header
{
	u32 size;
	u32 flags;
	u32 height;
	u32 width;
	u32 pitchOrLinearSize;
	u32 depth;
	u32 mipMapCount;
	u32 reserved1[11];
	PixelFormat pixelFormat;
	u32 caps1;
	u32 caps2;
	u32 caps3;
	u32 caps4;
	u32 reserved2;
};

typedef enum
{
//	Unknown = 0,
	Texture1D = 2,
	Texture2D = 3,
	Texture3D = 4
} TEXTURE_DIMENSION;

struct HeaderDXT10
{
	TEXTURE_FORMAT format;
	TEXTURE_DIMENSION resourceDimension;
	u32 miscFlag;
	u32 arraySize;
	u32 miscFlag2;
};

bool DDSLoader::LoadFromFile(const char *fileName, TextureDesc &desc)
{
	if (!Open(fileName))
		return false;

	s32 magic;
	Read(&magic, 4);
	if (magic != ' SDD')
		return false;

	Header h;
	Read(&h, sizeof(h));
	if (h.size != sizeof(Header) || h.pixelFormat.size != sizeof(PixelFormat))
		return false;

	if (h.mipMapCount > 1)
		return false;
//	h.mipMapCount = 1; /* jeœli by³o 0, to zmieñ na 1 */

	if (h.pixelFormat.fourCC != '01XD')
	{
		HeaderDXT10 dxt;
		Read(&dxt, sizeof(dxt));
		desc.format = dxt.format;
	}
	else
		desc.format = TEXTURE_FORMAT::BC1_UNorm; //TEMP!

	desc.width = h.width;
	desc.height = h.height;
	desc.bits = (u8*)GetCurrentPointer();

	return true;
}

void DDSLoader::Close()
{
	BinaryReader::Close();
}
