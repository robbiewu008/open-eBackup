#include <linux/slab.h>

#include "bitmap.h"
#include "util.h"


static inline int ctpop32(uint32_t val)
{
    val = (val & 0x55555555) + ((val >>  1) & 0x55555555);
    val = (val & 0x33333333) + ((val >>  2) & 0x33333333);
    val = (val & 0x0f0f0f0f) + ((val >>  4) & 0x0f0f0f0f);
    val = (val & 0x00ff00ff) + ((val >>  8) & 0x00ff00ff);
    val = (val & 0x0000ffff) + ((val >> 16) & 0x0000ffff);

    return val;
}

static inline int ctpop64(uint64_t val)
{
    val = (val & 0x5555555555555555ULL) + ((val >>  1) & 0x5555555555555555ULL);
    val = (val & 0x3333333333333333ULL) + ((val >>  2) & 0x3333333333333333ULL);
    val = (val & 0x0f0f0f0f0f0f0f0fULL) + ((val >>  4) & 0x0f0f0f0f0f0f0f0fULL);
    val = (val & 0x00ff00ff00ff00ffULL) + ((val >>  8) & 0x00ff00ff00ff00ffULL);
    val = (val & 0x0000ffff0000ffffULL) + ((val >> 16) & 0x0000ffff0000ffffULL);
    val = (val & 0x00000000ffffffffULL) + ((val >> 32) & 0x00000000ffffffffULL);

    return val;
}

static inline int popcountl(unsigned long l)
{
    return BITS_PER_LONG == 32 ? ctpop32(l) : ctpop64(l);
}

unsigned long hbitmap_iter_skip_words(HBitmapIter *hbi)
{
    size_t pos = hbi->pos;
    const HBitmap *hb = hbi->hb;
    unsigned i = HBITMAP_LEVELS - 1;

    unsigned long cur;
    do {
        cur = hbi->cur[--i];
        pos >>= BITS_PER_LEVEL;
    } while (cur == 0);

    /* Check for end of iteration.  We always use fewer than BITS_PER_LONG
     * bits in the level 0 bitmap; thus we can repurpose the most significant
     * bit as a sentinel.  The sentinel is set in hbitmap_alloc and ensures
     * that the above loop ends even without an explicit check on i.
     */

    if (i == 0 && cur == (1UL << (BITS_PER_LONG - 1))) {
        return 0;
    }
    for (; i < HBITMAP_LEVELS - 1; i++) {
        /* Shift back pos to the left, matching the right shifts above.
         * The index of this word's least significant set bit provides
         * the low-order bits.
         */
        pos = (pos << BITS_PER_LEVEL) + ctzl(cur);
        hbi->cur[i] = cur & (cur - 1);

        /* Set up next level for iteration.  */
        cur = hb->levels[i + 1][pos];
    }

    hbi->pos = pos;

    return cur;
}

void hbitmap_iter_init(HBitmapIter *hbi, const HBitmap *hb, uint64_t first)
{
    unsigned i, bit;
    uint64_t pos;

    hbi->hb = hb;
    pos = first >> hb->granularity;
    //assert(pos < hb->size);
    hbi->pos = pos >> BITS_PER_LEVEL;
    hbi->granularity = hb->granularity;

    for (i = HBITMAP_LEVELS; i-- > 0; ) {
        bit = pos & (BITS_PER_LONG - 1);
        pos >>= BITS_PER_LEVEL;

        /* Drop bits representing items before first.  */
        hbi->cur[i] = hb->levels[i][pos] & ~((1UL << bit) - 1);

        /* We have already added level i+1, so the lowest set bit has
         * been processed.  Clear it.
         */
        if (i != HBITMAP_LEVELS - 1) {
            hbi->cur[i] &= ~(1UL << bit);
        }
    }
}

bool hbitmap_empty(const HBitmap *hb)
{
    return hb->count == 0;
}

int hbitmap_granularity(const HBitmap *hb)
{
    return hb->granularity;
}

uint64_t hbitmap_count(const HBitmap *hb)
{
    return hb->count << hb->granularity;
}

static uint64_t hb_count_between(HBitmap *hb, uint64_t start, uint64_t last)
{
    HBitmapIter hbi;
    uint64_t count = 0;
    uint64_t end = last + 1;
    unsigned long cur;
    size_t pos;

    hbitmap_iter_init(&hbi, hb, start << hb->granularity);
    for (;;) {
        pos = hbitmap_iter_next_word(&hbi, &cur);
        if (pos >= (end >> BITS_PER_LEVEL)) {
            break;
        }
        count += popcountl(cur);
    }

    if (pos == (end >> BITS_PER_LEVEL)) {
        /* Drop bits representing the END-th and subsequent items.  */
        int bit = end & (BITS_PER_LONG - 1);
        cur &= (1UL << bit) - 1;
        count += popcountl(cur);
    }

    return count;
}

static inline bool hb_set_elem(unsigned long *elem, uint64_t start, uint64_t last)
{
    unsigned long mask;
    bool changed;

    //assert((last >> BITS_PER_LEVEL) == (start >> BITS_PER_LEVEL));
    //assert(start <= last);

    mask = 2UL << (last & (BITS_PER_LONG - 1));
    mask -= 1UL << (start & (BITS_PER_LONG - 1));
    changed = (*elem == 0);
    *elem |= mask;
    return changed;
}

static void hb_set_between(HBitmap *hb, int level, uint64_t start, uint64_t last)
{
    size_t pos = start >> BITS_PER_LEVEL;
    size_t lastpos = last >> BITS_PER_LEVEL;
    bool changed = false;
    size_t i;

    i = pos;
    if (i < lastpos) {
        uint64_t next = (start | (BITS_PER_LONG - 1)) + 1;
        changed |= hb_set_elem(&hb->levels[level][i], start, next - 1);
        for (;;) {
            start = next;
            next += BITS_PER_LONG;
            if (++i == lastpos) {
                break;
            }
            changed |= (hb->levels[level][i] == 0);
            hb->levels[level][i] = ~0UL;
        }
    }
    changed |= hb_set_elem(&hb->levels[level][i], start, last);

    /* If there was any change in this layer, we may have to update
     * the one above.
     */
    if (level > 0 && changed) {
        hb_set_between(hb, level - 1, pos, lastpos);
    }
}

void hbitmap_set(HBitmap *hb, uint64_t start, uint64_t count)
{
    /* Compute range in the last layer.  */
    uint64_t last = start + count - 1;

    //trace_hbitmap_set(hb, start, count,
    //                  start >> hb->granularity, last >> hb->granularity);

    start >>= hb->granularity;
    last >>= hb->granularity;
    count = last - start + 1;

    hb->count += count - hb_count_between(hb, start, last);
    hb_set_between(hb, HBITMAP_LEVELS - 1, start, last);
}

static inline bool hb_reset_elem(unsigned long *elem, uint64_t start, uint64_t last)
{
    unsigned long mask;
    bool blanked;

    //assert((last >> BITS_PER_LEVEL) == (start >> BITS_PER_LEVEL));
    //assert(start <= last);

    mask = 2UL << (last & (BITS_PER_LONG - 1));
    mask -= 1UL << (start & (BITS_PER_LONG - 1));
    blanked = *elem != 0 && ((*elem & ~mask) == 0);
    *elem &= ~mask;
    return blanked;
}

static void hb_reset_between(HBitmap *hb, int level, uint64_t start, uint64_t last)
{
    size_t pos = start >> BITS_PER_LEVEL;
    size_t lastpos = last >> BITS_PER_LEVEL;
    bool changed = false;
    size_t i;

    i = pos;
    if (i < lastpos) {
        uint64_t next = (start | (BITS_PER_LONG - 1)) + 1;

        /* Here we need a more complex test than when setting bits.  Even if
         * something was changed, we must not blank bits in the upper level
         * unless the lower-level word became entirely zero.  So, remove pos
         * from the upper-level range if bits remain set.
         */
        if (hb_reset_elem(&hb->levels[level][i], start, next - 1)) {
            changed = true;
        } else {
            pos++;
        }

        for (;;) {
            start = next;
            next += BITS_PER_LONG;
            if (++i == lastpos) {
                break;
            }
            changed |= (hb->levels[level][i] != 0);
            hb->levels[level][i] = 0UL;
        }
    }

    /* Same as above, this time for lastpos.  */
    if (hb_reset_elem(&hb->levels[level][i], start, last)) {
        changed = true;
    } else {
        lastpos--;
    }

    if (level > 0 && changed) {
        hb_reset_between(hb, level - 1, pos, lastpos);
    }
}

void hbitmap_reset(HBitmap *hb, uint64_t start, uint64_t count)
{
    /* Compute range in the last layer.  */
    uint64_t last = start + count - 1;

    //trace_hbitmap_reset(hb, start, count,
    //                    start >> hb->granularity, last >> hb->granularity);

    start >>= hb->granularity;
    last >>= hb->granularity;

    hb->count -= hb_count_between(hb, start, last);
    hb_reset_between(hb, HBITMAP_LEVELS - 1, start, last);
}

bool hbitmap_get(const HBitmap *hb, uint64_t item)
{
    /* Compute position and bit in the last layer.  */
    uint64_t pos = item >> hb->granularity;
    unsigned long bit = 1UL << (pos & (BITS_PER_LONG - 1));

    return (hb->levels[HBITMAP_LEVELS - 1][pos >> BITS_PER_LEVEL] & bit) != 0;
}

void hbitmap_free(HBitmap *hb, PBITMAP_FREE_FUNCTION_TYPE free_fun, uint32_t tag)
{
    unsigned i = 0;

    if (NULL != hb)
    {
        for (i = HBITMAP_LEVELS; i-- > 0; )
        {
            if (NULL != hb->levels[i])
            {
                free_fun(hb->levels[i], tag);
            }
        }
        kfree(hb);
    }
}


HBitmap *hbitmap_alloc(uint64_t size, int granularity, PBITMAP_ALLOC_FUNCTION_TYPE alloc_fun, PBITMAP_FREE_FUNCTION_TYPE free_fun, uint32_t tag)
{
    unsigned i = 0;
    HBitmap *hb = NULL;

    hb = kzalloc(sizeof (struct HBitmap), GFP_KERNEL);
    if (NULL == hb)
    {
        IOMIRROR_ERR("kzalloc for hb failed.");
        goto fail;
    }

    //assert(granularity >= 0 && granularity < 64);
    size = (size + (1ULL << granularity) - 1) >> granularity;
    //assert(size <= ((uint64_t)1 << HBITMAP_LOG_MAX_SIZE));

    hb->size = size;
    hb->granularity = granularity;
    for (i = HBITMAP_LEVELS; i-- > 0; )
    {
        size = MAX((size + BITS_PER_LONG - 1) >> BITS_PER_LEVEL, 1);
        hb->levels[i] = alloc_fun(size * sizeof(unsigned long), tag);
        if (NULL == hb->levels[i])
        {
            IOMIRROR_ERR("kzalloc for hb->levels failed.");
            goto fail;
        }
        memset(hb->levels[i], 0, size * sizeof(unsigned long));
    }

    /* We necessarily have free bits in level 0 due to the definition
     * of HBITMAP_LEVELS, so use one for a sentinel.  This speeds up
     * hbitmap_iter_skip_words.
     */
    //assert(size == 1);
    hb->levels[0][0] |= 1UL << (BITS_PER_LONG - 1);
    return hb;

fail:
    if (NULL != hb)
    {
        for (i = HBITMAP_LEVELS; i-- > 0; )
        {
            if (NULL != hb->levels[i])
            {
                free_fun(hb->levels[i], tag);
            }
        }
        kfree(hb);
    }
    return NULL;
}


void hbitmap_copy(HBitmap *dst, HBitmap *src)
{
    unsigned  i     = 0;
    uint64_t  size  = src->size;

    dst->count = src->count;

    for (i = HBITMAP_LEVELS; i-- > 0; )
    {
        size = MAX((size + BITS_PER_LONG - 1) >> BITS_PER_LEVEL, 1);
        memcpy(dst->levels[i], src->levels[i], size * sizeof(unsigned long));
    }
}


#ifdef SUPPORT_BACKUP
void hbitmap_merge(HBitmap *dst, HBitmap *src)
{
    unsigned  i     = 0, j;
    uint64_t  size  = src->size;
 
    dst->count += src->count;
 
    for (i = HBITMAP_LEVELS; i-- > 0; )
    {
        size = MAX((size + BITS_PER_LONG - 1) >> BITS_PER_LEVEL, 1);
		for (j = 0; j < size; j++)
			dst->levels[i][j] |= src->levels[i][j];
    }
}
#endif

void RaiseException()
{
    if(3/0){ }
}

