#pragma once

class ResourceID
{
public:
	const static int INVALID = -1; /* okre�la i� zas�b jest nieprawid�owy */

	ResourceID() : mIndex(INVALID) {} /* domy�lna inicjacja na INVALID */
	ResourceID(int index) : mIndex(index) {}

	void operator=(int index) { mIndex = index; }
	operator int() { return mIndex; }

	bool IsValid() { return mIndex > INVALID; }

private:
	int mIndex;
};
