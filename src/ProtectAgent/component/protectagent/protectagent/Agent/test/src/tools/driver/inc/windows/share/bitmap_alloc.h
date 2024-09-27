#pragma once
#include "om_bitmap.h"

void* BitmapAlloc(uint64_t size, uint32_t tag);
void BitmapFree(void* buffer, uint32_t tag);

