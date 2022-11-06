#pragma once

#include "baseTypes.h"

class BinaryWriter
{
public:
	BinaryWriter(): mCurrentPointer(), mOriginalPointer(), mHFile() {}

	bool Open(const char* fileName);
	void Close();

	void Write(void* data, u32 numBytes);
//	void GetPointer(void** out, u32 numBytes);

//	u32 GetCurrentPosition() { return mCurrentPointer - mOriginalPointer; }
//	void* GetCurrentPointer() { return mCurrentPointer; }
//	unsigned long GetSize() { return mSize; }

//	void Skip(unsigned int numBytes) { mCurrentPointer += numBytes; }

private:
	char *mCurrentPointer, *mOriginalPointer;
//	u32 mSize;
	void *mHFile;
};
