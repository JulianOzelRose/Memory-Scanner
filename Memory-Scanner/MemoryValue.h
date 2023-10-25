#pragma once
#include <Windows.h>

using namespace System;

public ref class MemoryValue
{
public:
	Object^ Value;
	String^ ModuleName;
	String^ DisplayAddress;
	BYTE* Address;
	int Type;					// 0 = Byte,	1 = 2-Bytes,	2 = 4-Bytes,	3 = 8-Bytes,	4 = String
	bool IsModuleBaseIncluded;
};