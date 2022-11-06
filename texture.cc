#include "texture.h"
#include "renderer.h"
#include "binaryReader.h"
#include <assert.h>

TEXTURE_FORMAT TextureFrame::GetFormat() const
{
	switch (magic)
	{
	default:
	case '1TXD':
		return TEXTURE_FORMAT::BC1_UNorm;
	case '3TXD':
		return TEXTURE_FORMAT::BC2_UNorm; /* used for normal maps */
	case '5TXD':
		return TEXTURE_FORMAT::BC3_UNorm;
	}
}

u32 TextureFrame::GetWidth() const
{
	return width;
}

u32 TextureFrame::GetHeight() const
{
	return height;
}

const char* TextureFrame::GetFormatString()
{
	switch (GetFormat())
	{
	case TEXTURE_FORMAT::BC1_UNorm:
		return "BC1";
	case TEXTURE_FORMAT::BC2_UNorm:
		return "BC2";
	case TEXTURE_FORMAT::BC3_UNorm:
		return "BC3";
	default:
		return "Unknown";
	}
}

void TextureFrame::GetTextureDesc(TextureDesc& td) const
{
	td.width = width;
	td.height = height;
	td.format = GetFormat();
	td.bits = bits;
}

void TextureFrame::Deserialize(BinaryReader& f)
{
	s32 readValue;
	f.Read(&readValue, 4);
	width = (s16)readValue;
	f.Read(&height, 4);
	f.Read(&pixelSize, 4);
	f.Read(&dword18, 4);

	shifted = readValue >> 16;
	if (shifted <= 0)
		size = width * height * pixelSize;
	else
		f.Read(&size, 4);

	f.Read(skipped, sizeof(skipped));
	if (skipped[0] >> 16)
		f.Read(&magic, 4);

	bits = new u8[size];
	f.Read(bits, size);

	if (shifted > 1)
	{
		bool readAgain;
		f.Read(&readAgain, 1);
		assert(readAgain == false);
	}
}

void TextureFrame::Serialize(BinaryWriter& w)
{
	s32 writeValue = ((u32)shifted << 16) | (u16)width;
	w.Write(&writeValue, 4);
	w.Write(&height, 4);
	w.Write(&pixelSize, 4);
	w.Write(&dword18, 4);
	w.Write(&size, 4);
	w.Write(skipped, sizeof(skipped));
	if (skipped[0] >> 16)
		w.Write(&magic, 4);
	w.Write(bits, size);
	bool fls = false;
	w.Write(&fls, 1);
}

bool TextureFrame::Create()
{
	TextureDesc td;
	GetTextureDesc(td);
	mID = Renderer::CreateTexture(td);
	return mID.IsValid();
}

void TextureFrame::Destroy()
{
	Renderer::DeleteTexture(mID);
}

bool TextureFrame::ReplaceWith(TextureDesc& td)
{
	if (!Renderer::ReplaceTexture(td, mID))
		return false;

	/* update all the member variables */
	width = td.width;
	height = td.height;
//	pixelSize =
//	dword18 =
	switch (td.format)
	{
		case TEXTURE_FORMAT::BC1_UNorm:
			magic = '1TXD';
			break;
		case TEXTURE_FORMAT::BC2_UNorm:
			magic = '3TXD';
			break;
		case TEXTURE_FORMAT::BC3_UNorm:
			magic = '5TXD';
			break;
		default:
			assert(0 && "Unknown format!");
	}
	size = td.size;
	delete[] bits;
	bits = td.bits;

	return true;
}

void Texture::Deserialize(BinaryReader& f)
{
	f.Read(&numFrames, 4);

	/* sanity check... */
	/* as an example, one texture in cutscene_end.EDN */
	/* contains up to 15 frames of animation */
	assert(numFrames <= 30);

	if (numFrames)
	{
		frames = new TextureFrame[numFrames];
		for (u32 i = 0; i < numFrames; i++)
		{
			TextureFrame& frame = frames[i];
			frame.Deserialize(f);
		}
	}
}

void Texture::Serialize(BinaryWriter& w)
{
	w.Write(&numFrames, 4);
	if (numFrames)
	{
		for (u32 i = 0; i < numFrames; i++)
		{
			TextureFrame& frame = frames[i];
			frame.Serialize(w);
		}
	}
}

u32 Texture::GetNumFrames()
{
	return numFrames;
}

TextureFrame& Texture::GetFrame(u32 index)
{
	assert(frames);
	return frames[index];
}

bool Texture::Create()
{
	for (u32 i = 0; i < numFrames; i++)
	{
		TextureFrame& ds = frames[i];
		if (!ds.Create())
			return false;
	}

	return true;
}

void Texture::Destroy()
{
	for (u32 i = 0; i < numFrames; i++)
	{
		TextureFrame& ds = frames[i];
		ds.Destroy();
	}
}
