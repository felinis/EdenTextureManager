#pragma once

class ResourceID
{
public:
	const static int INVALID = -1; /* okreœla i¿ zasób jest nieprawid³owy */

	ResourceID() : mIndex(INVALID) {} /* domyœlna inicjacja na INVALID */
	ResourceID(int index) : mIndex(index) {}

	void operator=(int index) { mIndex = index; }
	operator int() { return mIndex; }

	bool IsValid() { return mIndex > INVALID; }

private:
	int mIndex;
};
