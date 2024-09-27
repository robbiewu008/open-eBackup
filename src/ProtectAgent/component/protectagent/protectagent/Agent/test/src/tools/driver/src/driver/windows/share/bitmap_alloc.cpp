#include "bitmap_alloc.h"


void* BitmapAlloc(uint64_t size, uint32_t tag)
{
	return ExAllocatePoolWithTag(NonPagedPool, size * sizeof(uint32_t), tag);
}

void BitmapFree(void* buffer, uint32_t tag)
{
	ExFreePoolWithTag(buffer, tag);
}