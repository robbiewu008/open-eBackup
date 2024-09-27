#include "stdafx.h"
#include "bitmap_alloc.h"
#include <Windows.h>


void* BitmapAlloc(uint64_t size, uint32_t tag)
{
	return malloc(size);
}

void BitmapFree(void* buffer, uint32_t tag)
{
	free(buffer);
}