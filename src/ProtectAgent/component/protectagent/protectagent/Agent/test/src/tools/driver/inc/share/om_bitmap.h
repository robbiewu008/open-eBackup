#ifndef _OM_BITMAP_H
#define _OM_BITMAP_H



#ifdef _WIN32
#ifdef _KERNEL_MODE
#include <ntifs.h>
#else
#include <windows.h>
#endif
#else
#include <linux/types.h>
#include <linux/slab.h>
#endif


#ifdef DBG_OM_BITMAP
#include "bitmap.h"
#endif


#ifdef _WIN32
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;
#endif


#ifdef BITS_PER_LONG
#undef BITS_PER_LONG
#endif

#define BITS_PER_LONG										(32)

#define LOG_BITS_PER_LONG							(5)

#define IM_PG_BITS_PER_BYTE						(8)
#define BITMAP_HEADER_ALLOC_TAG			'BTdr'
#define BITMAP_BUFFER_ALLOC_TAG				'BBdr'
#define BITMAP_VERIFY_ALLOC_TAG				'BVdr'

#ifdef _WIN32
typedef unsigned long						bitmap_operator_t;
#else
typedef unsigned int						bitmap_operator_t;
#endif

typedef struct _OM_BITMAP
{
	uint32_t granularity;
	uint64_t count;
	uint64_t buffer_size;
	bitmap_operator_t* buffer;
#ifdef DBG_OM_BITMAP
	HBitmap* hbp;
#endif
} OM_BITMAP, *POM_BITMAP;

typedef struct _OM_BITMAP_IT
{
	const OM_BITMAP* om_bitmap;
	uint64_t pos;
	bitmap_operator_t cur;
#ifdef DBG_OM_BITMAP
	HBitmapIter hbi;
#endif
} OM_BITMAP_IT, *POM_BITMAP_IT;



typedef void* BITMAP_ALLOC_FUNCTION(uint64_t size, uint32_t tag);
typedef BITMAP_ALLOC_FUNCTION* PBITMAP_ALLOC_FUNCTION;

typedef void BITMAP_FREE_FUNCTION(void* buffer, uint32_t tag);
typedef BITMAP_FREE_FUNCTION* PBITMAP_FREE_FUNCTION;

POM_BITMAP BitmapAlloc(uint64_t user_size, uint32_t granularity, PBITMAP_ALLOC_FUNCTION alloc_fun, PBITMAP_FREE_FUNCTION free_fun);
bool IsBitmapEmpty(const POM_BITMAP om_bitmap);
uint64_t GetBitmapCount(const POM_BITMAP om_bitmap);
void BitmapSetBit(POM_BITMAP om_bitmap, uint64_t start_pos, uint64_t count);
void BitmapResetBit(POM_BITMAP om_bitmap, uint64_t start_pos, uint64_t count);
void BitmapFree(POM_BITMAP om_bitmap, PBITMAP_FREE_FUNCTION free_fun);
void BitmapItInit(POM_BITMAP_IT om_it, const POM_BITMAP om_bitmap, uint64_t start_pos);
int64_t BitmapItNext(POM_BITMAP_IT om_it);
int64_t BitmapItNextSuccessive(POM_BITMAP_IT om_it, uint64_t max_count, uint64_t* count);
bool GetBitmapBuffer(POM_BITMAP om_bitmap, unsigned char* buffer, uint32_t size);
bool MergeBitmapByBuffer(POM_BITMAP om_bitmap, unsigned char* buffer, uint64_t user_size);
bool MergeBitmap(POM_BITMAP om_dest, POM_BITMAP om_src, PBITMAP_ALLOC_FUNCTION alloc_fun, PBITMAP_FREE_FUNCTION free_fun);
bool AndBitmap(POM_BITMAP om_dest, POM_BITMAP om_src, PBITMAP_ALLOC_FUNCTION alloc_fun, PBITMAP_FREE_FUNCTION free_fun);
bool CompareBitmap(POM_BITMAP om_bitmap1, POM_BITMAP om_bitmap2);
uint64_t GetBitmapBufferSize(uint64_t user_size, uint32_t granularity);
bool IsBitmapBitSet(POM_BITMAP om_bitmap, uint64_t start_pos);

#endif
