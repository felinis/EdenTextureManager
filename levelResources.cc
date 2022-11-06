#include "levelResources.h"
#include "binaryReader.h"
#include "binaryWriter.h"
#include "renderer.h"
#include <string.h>
#include <Windows.h>
#include <Shlobj.h>

//#define MAX_TEXTURE_SLOTS 512
u32 sFirstSlotID;
u32 sLastSlotID;
u32 sNumTextures;
Texture *sTextures;

u32 LevelResources::GetNumTextures()
{
	return sNumTextures;
}

u32 LevelResources::GetNumTextureSlots()
{
	return sLastSlotID;
}

u32 GetTexturesStartAddress(char* levelName)
{
	int levelID = levelName[0] | (levelName[1] << 8) | (levelName[2] << 16) | (levelName[3] << 24);
	switch (levelID)
	{
	case 'ltit': /* titlebackdrop */
		return 0x61538;
	case '10tc': /* cutscene01 */
		return 0x43F56A;
	case 'zalP': /* l1_plaza */
		return 0xA2A555;
	case 'tcaF': /* l2_factory */
		return 0x8F49CD;
	case 'snoC': /* l3_construction */
		return 0xA664C7;
	case 'pohS': /* l4_shoppingmall */
		return 0xC072F9;
	case 'gnaG': /* l5_gang */
		return 0xA43219;
	case '60tc': /* cutscene06 */
		return 0x567AA3;
	case 'psoH': /* l6_hospital */
		return 0xE48F4B;
	case '70tc': /* cutscene07 */
		return 0x403675;
	case 'TykS': /* l7_skytran */
		return 0xBAC844;
	case '80tc': /* cutscene08 */
		return 0x4116C4;
	case 'ooZ': /* l8_zoo */
		return 0xBA79DD;
	case '90tc': /* cutscene09 */
		return 0x3B7AD2;
	case 'vacS': /* l9_scavenge */
		return 0xD875EB;
	case 'nnuT': /* l10_tunnels */
		return 0xD778A0;
	case 'nedE': /* l11_eden */
		return 0xD258FD;
	case 'dntc': /* cutscene_end */
		return 0x400DE2;
	case '10FC': /* CaptureFlag_pc */
		return 0x6CAED3;
	case '20fc': /* CaptureFlag_02_pc */
		return 0x62B603;
	case '10MD':
		return 0x3A06F4;
	case '20MD':
		return 0x602F75;
	case '30md':
		return 0x37F442;
	case '40md':
		return 0x3A3540;
	case '50md':
		return 0x389D65;
	case '60MD':
		return 0x62B6FD;
	case '70MD':
		return 0x684C8E;
	case '80md':
		return 0x5F36B1;
	case '10RR':
		return 0x6875D1;
	default:
		return 0;
	}
}

/* this is a constant in Eden */
#define MAX_NUM_SLOTS 2048

static bool ReadDDS(BinaryReader &r)
{
	u32 firstSlotID, maxNumSlots;
	r.Read(&firstSlotID, 4);
	r.Read(&maxNumSlots, 4);
	if (maxNumSlots > MAX_NUM_SLOTS)
	{
//		fputs("Too much texture slots!", stderr);
		return false;
	}

	sFirstSlotID = firstSlotID;
	const u32 lastID = firstSlotID + maxNumSlots;
	sLastSlotID = lastID;
	sTextures = new Texture[lastID];
	for (u32 slotID = firstSlotID; slotID < lastID; slotID++)
	{
		Texture& ss = sTextures[slotID];
		ss.Deserialize(r);

		if (ss.GetNumFrames())
		{
			r.Read(&ss.t1, 2);
			r.Read(&ss.t2, 1);
			r.Read(&ss.t3, 1);
			r.Read(&ss.t4, 1);
		}
	}

	return true;
}

/* create the same function as for ReadDDS but this time for writing */
static bool WriteDDS(BinaryWriter &w)
{
	w.Write(&sFirstSlotID, 4);
	u32 maxNumSlots = MAX_NUM_SLOTS;
	w.Write(&maxNumSlots, 4);
	const u32 lastID = sFirstSlotID + maxNumSlots;
	for (u32 slotID = sFirstSlotID; slotID < lastID; slotID++)
	{
		Texture& ss = sTextures[slotID];
		ss.Serialize(w);

		if (ss.GetNumFrames())
		{
			w.Write(&ss.t1, 2);
			w.Write(&ss.t2, 1);
			w.Write(&ss.t3, 1);
			w.Write(&ss.t4, 1);
		}
	}

	return true;
}

static u32 sTexturesSectionStart;
static u32 sTexturesSectionEnd;
static char sLastFilePath[128];
bool LevelResources::Create(const char *fileName)
{
	BinaryReader r;
	typedef struct
	{
		int version;	/* always 72 in the final game version */
		int numLevel;
		int unk;
		int unk2;
		char levelID[4];
		char nextLevelID[4];
	} EDNHEADER;
	EDNHEADER h;
	u32 texturesSection;

	strncpy_s(sLastFilePath, fileName, sizeof(sLastFilePath));

	if (!r.Open(fileName))
		return false;

	/* read the header */
	r.Read(&h, sizeof(EDNHEADER));
	r.Skip(1);
	if (h.version != 72)
	{
		MessageBox(0, "Level file version is incorrect.", 0, MB_ICONERROR);
		goto err;
	}

	int numUnk;
	r.Read(&numUnk, 4);
	if (numUnk > 356)
	{
		MessageBox(0, "Number of objects is incorrect.", 0, MB_ICONERROR);
		goto err;
	}

	texturesSection = GetTexturesStartAddress(h.levelID);
	if (texturesSection == 0)
	{
		MessageBox(0, "Could not find that level's texture-start address.", 0, MB_ICONERROR);
		goto err;
	}

	sTexturesSectionStart = texturesSection;

	r.Seek(texturesSection);

	if (!ReadDDS(r))
	{
		MessageBox(0, "Failed to read the textures.", 0, MB_ICONERROR);
		goto err;
	}

	/* create the textures */
	for (u32 i = sFirstSlotID; i < sLastSlotID; i++)
	{
		Texture& t = sTextures[i];
		if (!t.IsValid())
			continue; //go to the next texture slot

		t.Create();
		
		sNumTextures++;
	}

	/* save the texture section end pointer */
	sTexturesSectionEnd = (u32)r.GetCurrentPosition();

	r.Close();
	return true;

err:
	r.Close();
	return false;
}

static bool Save()
{
	/* create a new file in write-mode, that will be the final EDN */
	BinaryWriter w;
	char fileName[128];
	strncpy_s(fileName, sLastFilePath, sizeof(fileName));
	/* append a .new to the filename so that we do not overwrite the original EDN */
	strcat_s(fileName, ".new");
	if (!w.Open(fileName))
		return false;

	/* open the level file in read-mode, this is the original EDN */
	BinaryReader r;
	if (!r.Open(sLastFilePath))
		return false;

	/* write the contents of the original until sTexturesSectionStart */
	void* currentReadPointer;
	r.GetPointer(&currentReadPointer, sTexturesSectionStart);
	w.Write(currentReadPointer, sTexturesSectionStart);

	/* at this point, we have copied everything before the texture section */

	/* now write the textures */
	WriteDDS(w);

	/* write the rest of the file */
	r.Seek(sTexturesSectionEnd);
	u32 sizeLeft = r.GetSize() - sTexturesSectionEnd;
	r.GetPointer(&currentReadPointer, sizeLeft);
	w.Write(currentReadPointer, sizeLeft);

	r.Close();
	w.Close();

	/* form the backup folder path */
	char backupFolderPath[128];
	GetCurrentDirectory(sizeof(backupFolderPath), backupFolderPath);
	strcat_s(backupFolderPath, "\\backup");

	/* create a backup folder using Windows API */
	int result = SHCreateDirectoryEx(nullptr, backupFolderPath, nullptr);
	if (result != ERROR_SUCCESS)
	{
		MessageBox(nullptr, "Failed to create a backup folder, proceed with caution.",
			"Warning", MB_ICONWARNING);
	}
//	else
//	{
		/* extract the file name from the absolute path sLastFilePath */
		char levelFileName[32] = {0};
		char* lastSlash = strrchr(sLastFilePath, '\\');
		if (lastSlash)
		{
			strcpy_s(levelFileName, lastSlash + 1);
		}

		/* MoveFile the original file in that backup folder */
		char backupFileName[128];
		strcpy_s(backupFileName, "backup\\");
		strcat_s(backupFileName, levelFileName);
		MoveFile(sLastFilePath, backupFileName);
//	}

	/* rename the modified file to sLastFileName, the original level name */
	MoveFile(fileName, sLastFilePath);

	return true;
}

void LevelResources::Destroy()
{
	if (MessageBox(NULL, "Do you want to save the changes?", "Save changes?", MB_YESNO) == IDYES)
	{
		if (!Save())
			MessageBox(NULL, "Failed to save the changes", 0, MB_OK);
	}

	/* destroy all resources */
	for (u32 i = sFirstSlotID; i < sLastSlotID; i++)
	{
		Texture& t = sTextures[i];
		t.Destroy();
	}
	delete[] sTextures;
//	sNumTextureSlots = 0;
//	sNumTextures = 0;
}

Texture& LevelResources::GetTexture(u32 slot)
{
	return sTextures[slot];
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include "stb_image_write.h"
#include "openS3TC_DXT1.h"
#include "openS3TC_DXT5.h"
void LevelResources::ExportTexture(u32 slotID, u32 frameID)
{
	TextureFrame &frame = sTextures[slotID].GetFrame(frameID);
	ResourceID texID = frame.GetResourceID();
#if 0
	u8 *compressedBits;
	if (!Renderer::LockTexture(texID, (void**)&compressedBits))
		return;
	TextureDesc td;
	u32 textureSize;
	Renderer::GetTextureDesc(texID, td, &textureSize);
#else
	TextureDesc td;
	frame.GetTextureDesc(td);
	u8* compressedBits = (u8*)td.bits;
#endif

	/* zdekompresuj */
	u32 stride = td.width * 4;
	u8* decompressedBits = (u8*)malloc(stride * td.height); //always decompressing as RGBA

	u32 numChannels = 4;
	DXT1SetOutputPixelFormat(DXT1PixelFormat_RGBA);
	DXT5SetOutputPixelFormat(DXT5PixelFormat_RGBA);
	if (td.format == TEXTURE_FORMAT::BC1_UNorm)
	{
		DXT1Decompress(td.width, td.height, compressedBits, decompressedBits);
	}
//	else if (td.format == TEXTURE_FORMAT::BC2_UNorm)
//	{
//		BlockDecompressImageDXT3(td.width, td.height, compressedBits, decompressedBits);
//	}
	else if (td.format == TEXTURE_FORMAT::BC3_UNorm)
	{
		DXT5Decompress(td.width, td.height, compressedBits, decompressedBits);
	}
	else
		return;

	/* get the current directory */
	char currentDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, currentDir);

	/* uzyskaj nazwê pliku do zapisania */
	char fileName[MAX_PATH] = {0};
	_snprintf(fileName, sizeof(fileName), "image_s%u_f%u.png", slotID, frameID);
	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = ".png";
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.lpstrTitle = "Select where to save the texture";
//	ofn.lpstrInitialDir = "C:\\";

	if (!GetSaveFileName(&ofn))
		return;

	/* set the current directory */
	SetCurrentDirectory(currentDir); //WINDOWS BUG FIX...

	/* wyeksportuj */
	stbi_flip_vertically_on_write(1);
	if (!stbi_write_png(fileName, td.width, td.height, numChannels, decompressedBits, stride))
		OutputDebugString("Failed to export!\n");
	
	free(decompressedBits);
#if 0
	Renderer::UnlockTexture(texID);
#endif
}

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#include "stb_dxt.h"

static int imin(int x, int y)
{
	return (x < y) ? x : y;
}

#define NEW_OPTIMISATIONS
static void extractBlock(const unsigned char* src, int x, int y,
	int w, int h, unsigned char* block)
{
	int i, j;

#ifdef NEW_OPTIMISATIONS
	if ((w - x >= 4) && (h - y >= 4))
	{
		// Full Square shortcut
		src += x * 4;
		src += y * w * 4;
		for (i = 0; i < 4; ++i)
		{
			*(unsigned int*)block = *(unsigned int*)src; block += 4; src += 4;
			*(unsigned int*)block = *(unsigned int*)src; block += 4; src += 4;
			*(unsigned int*)block = *(unsigned int*)src; block += 4; src += 4;
			*(unsigned int*)block = *(unsigned int*)src; block += 4;
			src += (w * 4) - 12;
		}
		return;
	}
#endif

	int bw = imin(w - x, 4);
	int bh = imin(h - y, 4);
	int bx, by;

	const int rem[] =
	{
	   0, 0, 0, 0,
	   0, 1, 0, 1,
	   0, 1, 2, 0,
	   0, 1, 2, 3
	};

	for (i = 0; i < 4; ++i)
	{
		by = rem[(bh - 1) * 4 + i] + y;
		for (j = 0; j < 4; ++j)
		{
			bx = rem[(bw - 1) * 4 + j] + x;
			block[(i * 4 * 4) + (j * 4) + 0] =
				src[(by * (w * 4)) + (bx * 4) + 0];
			block[(i * 4 * 4) + (j * 4) + 1] =
				src[(by * (w * 4)) + (bx * 4) + 1];
			block[(i * 4 * 4) + (j * 4) + 2] =
				src[(by * (w * 4)) + (bx * 4) + 2];
			block[(i * 4 * 4) + (j * 4) + 3] =
				src[(by * (w * 4)) + (bx * 4) + 3];
		}
	}
}

void CompressDXT1(u32 w, u32 h, unsigned char* dst, unsigned char* src)
{
	unsigned char block[64];
	u32 x, y;

	for (y = 0; y < h; y += 4)
	{
		for (x = 0; x < w; x += 4)
		{
			extractBlock(src, x, y, w, h, block);
			stb_compress_dxt_block(dst, block, 0, 10);
			dst += 8;
		}
	}
}

void CompressDXT5(u32 w, u32 h, unsigned char* dst, unsigned char* src)
{
	unsigned char block[64];
	u32 x, y;

	for (y = 0; y < h; y += 4)
	{
		for (x = 0; x < w; x += 4)
		{
			extractBlock(src, x, y, w, h, block);
			stb_compress_dxt_block(dst, block, 1, 10);
			dst += 16;
		}
	}
}

void LevelResources::ReplaceTexture(u32 slotID, u32 frameID)
{
	/* get the current directory */
	char currentDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, currentDir);

	/* ask the user for the image file to load using GetOpenFileName */
	char fileName[MAX_PATH] = {0};
	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = ".png";
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.lpstrTitle = "Select the image file to load";
//	ofn.lpstrInitialDir = "C:\\Users\\Feliks\\Desktop";
	if (!GetOpenFileName(&ofn))
		return;

	/* set the current directory */
	SetCurrentDirectory(currentDir); //WINDOWS BUG FIX...
	
	/* load the image */
	u8 *imageBits;
	int width, height, numChannels;
	stbi_set_flip_vertically_on_load(1);
	imageBits = stbi_load(fileName, &width, &height, &numChannels, 4);
	if (!imageBits)
	{
		OutputDebugString("Failed to load image.\n");
		return;
	}

	TextureFrame& frame = sTextures[slotID].GetFrame(frameID);
#ifdef OLD_HACK
	/*
		HACKHACK:
		find the real number of channels by checking whether
		the first pixel has an alpha of less than 255
	*/
	if (numChannels == 4)
	{
		if (imageBits[3] == 255)
			numChannels = 3;
	}
#else
	/* the number of channels should be the same in the source as well as replacement texture */
	TEXTURE_FORMAT format = frame.GetFormat();
	if (format == TEXTURE_FORMAT::BC1_UNorm)
		numChannels = 3;
#endif

	/* compress the image bits using CompressDXT1 or CompressDXT5 */
	u32 compressedSize;
	u8 *compressedBits;
	if (numChannels == 4)
	{
		compressedSize = ((width + 3) / 4) * ((height + 3) / 4) * 16;
		compressedBits = new u8[compressedSize];
		CompressDXT5(width, height, compressedBits, imageBits);
//		format = TEXTURE_FORMAT::BC3_UNorm;
	}
	else
	{
		compressedSize = ((width + 3) / 4) * ((height + 3) / 4) * 8;
		compressedBits = new u8[compressedSize];
		CompressDXT1(width, height, compressedBits, imageBits);
//		format = TEXTURE_FORMAT::BC1_UNorm;
	}

	/* free the uncompressed image */
	stbi_image_free(imageBits);
	
	/* replace the existing texture with the new one */
	TextureDesc td;
	td.width = width;
	td.height = height;
	td.format = format;
	td.size = compressedSize;
	td.bits = compressedBits;
	ResourceID texID = frame.GetResourceID();
	if (!frame.ReplaceWith(td))
	{
		OutputDebugString("Failed to replace texture.\n");
		return;
	}
}
