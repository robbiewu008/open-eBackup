#include "bitmap_alloc.h"
#include <linux/vmalloc.h>

void* BitmapAllocFunc(uint64_t size, uint32_t tag)
{
    return vmalloc(size * sizeof(uint32_t));
}

void BitmapFreeFunc(void* buffer, uint32_t tag)
{
    if (buffer != NULL) {
        vfree(buffer);
    }
}
