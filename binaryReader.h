#pragma once

#include "baseTypes.h"

class BinaryReader
{
public:
	BinaryReader(): mCurrentPointer(), mOriginalPointer(), mSize(0), mHFile() {}

	bool Open(const char *fileName);
	void Close();
	bool DoesFileExist(const char* fileName);

	void Read(void *out, u32 numBytes);
	void Seek(u32 address);
	void GetPointer(void **out, u32 numBytes);

	u64 GetCurrentPosition() { return mCurrentPointer - mOriginalPointer; }
	void* GetCurrentPointer() { return mCurrentPointer; }
	unsigned long GetSize() { return mSize; }

	void Skip(unsigned int numBytes) {mCurrentPointer += numBytes;}

private:
	char *mCurrentPointer, *mOriginalPointer;
	u32 mSize;
	void* mHFile;
};
