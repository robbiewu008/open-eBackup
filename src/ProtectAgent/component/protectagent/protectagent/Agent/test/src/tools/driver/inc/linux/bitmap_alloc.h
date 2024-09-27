#ifndef _IOMIRROR_BITMAP_ALLOC_H
#define _IOMIRROR_BITMAP_ALLOC_H

#include "../share/om_bitmap.h"

void* BitmapAllocFunc(uint64_t size, uint32_t tag);
void BitmapFreeFunc(void* buffer, uint32_t tag);

#endif
