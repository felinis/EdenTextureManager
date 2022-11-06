/*
	Platform Submodule
	binaryReader.cc (Windows version)

	Moczulski Alan, 2022.
*/

#include "binaryReader.h"
#include <Windows.h>
#include <string.h>

bool BinaryReader::Open(const char* fileName)
{
	mHFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (mHFile == INVALID_HANDLE_VALUE)
		return false;

	mSize = GetFileSize(mHFile, 0);

	HANDLE hMap = CreateFileMapping(mHFile, 0, PAGE_READONLY, 0, 0, 0);
	if (!hMap)
		return false;

	mOriginalPointer = (char*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (!mOriginalPointer)
		return false;

	mCurrentPointer = mOriginalPointer;

	CloseHandle(hMap);
	return true;
}

void BinaryReader::Close()
{
	if (!mHFile)
		return;

	UnmapViewOfFile(mOriginalPointer);
	CloseHandle(mHFile);
	mHFile = nullptr;
}

bool BinaryReader::DoesFileExist(const char *fileName)
{
	return GetFileAttributes(fileName) != 0;
}

void BinaryReader::Read(void *out, u32 numBytes)
{
	memcpy(out, mCurrentPointer, numBytes);
	mCurrentPointer += numBytes;
}

void BinaryReader::Seek(u32 address)
{
	mCurrentPointer = mOriginalPointer + address;
}

void BinaryReader::GetPointer(void **out, unsigned int numBytes)
{
	*out = mCurrentPointer;
	mCurrentPointer += numBytes;
}
