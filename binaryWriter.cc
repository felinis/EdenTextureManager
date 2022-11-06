#include "binaryWriter.h"
#include <Windows.h>
#include <assert.h>

bool BinaryWriter::Open(const char *filename)
{
	mHFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (mHFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return true;
}

void BinaryWriter::Close()
{
	CloseHandle(mHFile);
}

void BinaryWriter::Write(void *data, u32 numBytes)
{
	DWORD bytesWritten;
	WriteFile(mHFile, data, numBytes, &bytesWritten, NULL);
	assert(bytesWritten == numBytes);
}
