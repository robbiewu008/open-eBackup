#ifndef _IOMIRROR_BITMAP_H
#define _IOMIRROR_BITMAP_H


#ifdef DBG

#ifdef _KERNEL_MODE
#include <wdm.h>
#else
#include <windows.h>
#endif

#ifdef _WIN32
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;
#endif


#ifndef BITS_PER_LONG
#define BITS_PER_LONG										(32)
#endif

#if BITS_PER_LONG == 32
#define ctzl ctz32
#else
#define ctzl ctz64
#endif

#define MAX(x,y)    (((x) > (y)) ? (x) : (y))
#define BITS_PER_LEVEL         (BITS_PER_LONG == 32 ? 5 : 6)
/* For 32-bit, the largest that fits in a 4 GiB address space.
 * For 64-bit, the number of sectors in 1 PiB.  Good luck, in
 * either case... :)
 */
#define HBITMAP_LOG_MAX_SIZE   (BITS_PER_LONG == 32 ? 34 : 41)
#define HBITMAP_LEVELS         ((HBITMAP_LOG_MAX_SIZE / BITS_PER_LEVEL) + 1)

typedef struct HBitmap
{
    uint64_t size;
    uint64_t count;
    int granularity;
    unsigned long *levels[HBITMAP_LEVELS];
} HBitmap;

typedef struct HBitmapIter
{
    const HBitmap *hb;
    int granularity;
    size_t pos;
    unsigned long cur[HBITMAP_LEVELS];
} HBitmapIter;

typedef void* BITMAP_ALLOC_FUNCTION(uint64_t size, uint32_t tag);
typedef BITMAP_ALLOC_FUNCTION* PBITMAP_ALLOC_FUNCTION;

typedef void BITMAP_FREE_FUNCTION(void* buffer, uint32_t tag);
typedef BITMAP_FREE_FUNCTION* PBITMAP_FREE_FUNCTION;

HBitmap *hbitmap_alloc(uint64_t size, int granularity, PBITMAP_ALLOC_FUNCTION alloc_fun, PBITMAP_FREE_FUNCTION free_fun, uint32_t tag);
bool hbitmap_empty(const HBitmap *hb);
int hbitmap_granularity(const HBitmap *hb);
uint64_t hbitmap_count(const HBitmap *hb);
void hbitmap_set(HBitmap *hb, uint64_t start, uint64_t count);
void hbitmap_reset(HBitmap *hb, uint64_t start, uint64_t count);
bool hbitmap_get(const HBitmap *hb, uint64_t item);
void hbitmap_free(HBitmap *hb, PBITMAP_FREE_FUNCTION free_fun, uint32_t tag);
void hbitmap_iter_init(HBitmapIter *hbi, const HBitmap *hb, uint64_t first);
unsigned long hbitmap_iter_skip_words(HBitmapIter *hbi);
void RaiseException();


static inline int ctz32(uint32_t val)
{
    /* Binary search for the trailing one bit.  */
    int cnt;

    cnt = 0;
    if (!(val & 0x0000FFFFUL))
    {
        cnt += 16;
        val >>= 16;
    }
    if (!(val & 0x000000FFUL))
    {
        cnt += 8;
        val >>= 8;
    }
    if (!(val & 0x0000000FUL))
    {
        cnt += 4;
        val >>= 4;
    }
    if (!(val & 0x00000003UL))
    {
        cnt += 2;
        val >>= 2;
    }
    if (!(val & 0x00000001UL))
    {
        cnt++;
        val >>= 1;
    }
    if (!(val & 0x00000001UL))
    {
        cnt++;
    }

    return cnt;
}

static inline int ctz64(uint64_t val)
{
    int cnt;

    cnt = 0;
    if (!((uint32_t)val))
    {
        cnt += 32;
        val >>= 32;
    }

    return cnt + ctz32((uint32_t)val);
}

static inline int64_t hbitmap_iter_next(HBitmapIter *hbi)
{
    unsigned long cur = hbi->cur[HBITMAP_LEVELS - 1];
    int64_t item;

    if (cur == 0) {
        cur = hbitmap_iter_skip_words(hbi);
        if (cur == 0) {
            return -1;
        }
    }

    /* The next call will resume work from the next bit.  */
    hbi->cur[HBITMAP_LEVELS - 1] = cur & (cur - 1);
    item = ((uint64_t)hbi->pos << BITS_PER_LEVEL) + ctzl(cur);

    return item << hbi->granularity;
}

static inline size_t hbitmap_iter_next_word(HBitmapIter *hbi, unsigned long *p_cur)
{
    unsigned long cur = hbi->cur[HBITMAP_LEVELS - 1];

    if (cur == 0) {
        cur = hbitmap_iter_skip_words(hbi);
        if (cur == 0) {
            *p_cur = 0;
            return ((size_t)-1);
        }
    }

    /* The next call will resume work from the next word.  */
    hbi->cur[HBITMAP_LEVELS - 1] = 0;
    *p_cur = cur;
    return hbi->pos;
}
#endif

#endif

