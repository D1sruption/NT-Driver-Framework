#pragma once

#include "includes.h"

struct ObjectManager
{
	DWORD Pointer;
	DWORD MaxObjects;
	DWORD UsedObjectsCount;
	DWORD HighestObjectId;
};