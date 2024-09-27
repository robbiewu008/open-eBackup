#include "om_bitmap.h"




#ifdef _WIN32
#define CopyMemory(Destination, DestSize, Source, Length) RtlCopyMemory((Destination), (Source), (Length))
#else
#define CopyMemory(Destination, DestSize, Source, Length) memcpy((Destination), (Source), (Length))
#endif


static inline uint32_t PopCount32(uint32_t value)
{
	value = (value & 0x55555555) + ((value >> 1) & 0x55555555);
	value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
	value = (value & 0x0f0f0f0f) + ((value >> 4) & 0x0f0f0f0f);
	value = (value & 0x00ff00ff) + ((value >> 8) & 0x00ff00ff);
	value = (value & 0x0000ffff) + ((value >> 16) & 0x0000ffff);

	return value;
}

static inline uint32_t PopCount64(uint64_t value)
{
	value = (value & 0x5555555555555555ULL) + ((value >> 1) & 0x5555555555555555ULL);
	value = (value & 0x3333333333333333ULL) + ((value >> 2) & 0x3333333333333333ULL);
	value = (value & 0x0f0f0f0f0f0f0f0fULL) + ((value >> 4) & 0x0f0f0f0f0f0f0f0fULL);
	value = (value & 0x00ff00ff00ff00ffULL) + ((value >> 8) & 0x00ff00ff00ff00ffULL);
	value = (value & 0x0000ffff0000ffffULL) + ((value >> 16) & 0x0000ffff0000ffffULL);
	value = (value & 0x00000000ffffffffULL) + ((value >> 32) & 0x00000000ffffffffULL);

	return (uint32_t)value;
}

static inline uint32_t SearchTrailBit32(uint32_t value)
{
	uint32_t ret = 0;

	if (!(value & 0x0000FFFFUL))
	{
		ret += 16;
		value >>= 16;
	}

	if (!(value & 0x000000FFUL))
	{
		ret += 8;
		value >>= 8;
	}

	if (!(value & 0x0000000FUL))
	{
		ret += 4;
		value >>= 4;
	}

	if (!(value & 0x00000003UL))
	{
		ret += 2;
		value >>= 2;
	}

	if (!(value & 0x00000001UL))
	{
		ret++;
		value >>= 1;
	}

	if (!(value & 0x00000001UL))
	{
		ret++;
	}

	return ret;
}

static inline int SearchTrailBit64(uint64_t value)
{
	uint32_t ret = 0;

	ret = 0;
	if (!((uint32_t)value))
	{
		ret += 32;
		value >>= 32;
	}

	return ret + SearchTrailBit32((uint32_t)value);
}

#if BITS_PER_LONG==32
#define PopCount				PopCount32
#define SearchTrailBit			SearchTrailBit32
#define ByteSwapToLte
#define ByteSwapFromLte
#else
#define PopCount				PopCount64
#define SearchTrailBit			SearchTrailBit64
#defien LittleEndianSwap
#define BigEndianSwap
#endif

uint32_t SetLongBit(bitmap_operator_t* value, uint64_t start_pos, uint64_t end_pos)
{
	uint32_t count = 0;

	bitmap_operator_t mask = 2UL << (end_pos & (BITS_PER_LONG - 1));
	mask -= 1UL << (start_pos & (BITS_PER_LONG - 1));

	count = PopCount((*value & mask) ^ mask);

	*value |= mask;

	return count;
}

uint32_t ResetLongBit(bitmap_operator_t* value, uint64_t start_pos, uint64_t end_pos)
{
	uint32_t count = 0;

	bitmap_operator_t mask = 2UL << (end_pos & (BITS_PER_LONG - 1));
	mask -= 1UL << (start_pos & (BITS_PER_LONG - 1));

	count = PopCount(*value & mask);

	*value &= ~mask;

	return count;
}


POM_BITMAP BitmapAlloc(uint64_t user_size, uint32_t granularity, PBITMAP_ALLOC_FUNCTION alloc_fun, PBITMAP_FREE_FUNCTION free_fun)
{
	POM_BITMAP ret = NULL;
	bool flag = false;
	bool succeed = false;

	do
	{
		ret = (POM_BITMAP)alloc_fun(sizeof(OM_BITMAP), BITMAP_HEADER_ALLOC_TAG);
		if (ret == NULL)
		{
			break;
		}

		memset(ret, 0, sizeof(OM_BITMAP));

		ret->granularity = granularity;
		ret->buffer_size = (user_size + (1ULL << granularity) - 1) >> granularity;
		ret->buffer_size = (ret->buffer_size + BITS_PER_LONG - 1) >> LOG_BITS_PER_LONG;
		if (ret->buffer_size == 0)
		{
			ret->buffer_size = 1;
		}

		ret->buffer = (bitmap_operator_t*)alloc_fun(ret->buffer_size * sizeof(bitmap_operator_t), BITMAP_BUFFER_ALLOC_TAG);
		if (ret->buffer == NULL)
		{
			break;
		}

		memset(ret->buffer, 0, ret->buffer_size * sizeof(bitmap_operator_t));

#ifdef DBG_OM_BITMAP
		ret->hbp = hbitmap_alloc(user_size,  granularity, alloc_fun, free_fun, BITMAP_VERIFY_ALLOC_TAG);
		if (ret->hbp == NULL)
		{
			break;
		}
#endif

		succeed = true;
	} while (flag);

	if (!succeed)
	{
		if (ret)
		{
			BitmapFree(ret, free_fun);
			ret = NULL;
		}
	}

	return ret;
}

bool IsBitmapEmpty(const POM_BITMAP om_bitmap)
{
#ifdef DBG_OM_BITMAP
	if (om_bitmap->hbp->count != om_bitmap->count)
	{
		RaiseException();
	}
#endif

	return (om_bitmap->count == 0);
}

uint64_t GetBitmapCount(const POM_BITMAP om_bitmap)
{
#ifdef DBG_OM_BITMAP
	if (om_bitmap->hbp->count != om_bitmap->count)
	{
		RaiseException();
	}
#endif

	return (om_bitmap->count << om_bitmap->granularity);
}

void BitmapSetBit(POM_BITMAP om_bitmap, uint64_t start_pos, uint64_t count)
{
	uint64_t start = start_pos >> om_bitmap->granularity;
	uint64_t end = (start_pos + count - 1) >> om_bitmap->granularity;

	uint64_t long_pos = start >> LOG_BITS_PER_LONG;
	uint64_t long_end = end >> LOG_BITS_PER_LONG;

	if (long_pos < long_end)
	{
		uint64_t next = (start | (BITS_PER_LONG - 1)) + 1;
		om_bitmap->count += SetLongBit(&om_bitmap->buffer[long_pos], start, next - 1);

		do
		{
			start = next;
			next += BITS_PER_LONG;
			if (++long_pos != long_end)
			{
				om_bitmap->count += BITS_PER_LONG - PopCount(om_bitmap->buffer[long_pos]);
				om_bitmap->buffer[long_pos] = ~0UL;
			}
		} while (long_pos < long_end);
	}

	om_bitmap->count += SetLongBit(&om_bitmap->buffer[long_pos], start, end);

#ifdef DBG_OM_BITMAP
	hbitmap_set(om_bitmap->hbp, start_pos, count);

	if (memcmp(om_bitmap->buffer, om_bitmap->hbp->levels[HBITMAP_LEVELS - 1], om_bitmap->buffer_size * sizeof(bitmap_operator_t)) != 0)
	{
		RaiseException();
	}

	if (om_bitmap->count != om_bitmap->hbp->count)
	{
		RaiseException();
	}
#endif
}

void BitmapResetBit(POM_BITMAP om_bitmap, uint64_t start_pos, uint64_t count)
{
	uint64_t start = start_pos >> om_bitmap->granularity;
	uint64_t end = (start_pos + count - 1) >> om_bitmap->granularity;

	uint64_t long_pos = start >> LOG_BITS_PER_LONG;
	uint64_t long_end = end >> LOG_BITS_PER_LONG;

	if (long_pos < long_end)
	{
		uint64_t next = (start | (BITS_PER_LONG - 1)) + 1;
		om_bitmap->count -= ResetLongBit(&om_bitmap->buffer[long_pos], start, next - 1);

		do
		{
			start = next;
			next += BITS_PER_LONG;
			if (++long_pos != long_end)
			{
				om_bitmap->count -= PopCount(om_bitmap->buffer[long_pos]);
				om_bitmap->buffer[long_pos] = 0UL;
			}
		} while (long_pos < long_end);
	}

	om_bitmap->count -= ResetLongBit(&om_bitmap->buffer[long_pos], start, end);

#ifdef DBG_OM_BITMAP
	hbitmap_reset(om_bitmap->hbp, start_pos, count);

	if (memcmp(om_bitmap->buffer, om_bitmap->hbp->levels[HBITMAP_LEVELS - 1], om_bitmap->buffer_size * sizeof(bitmap_operator_t)) != 0)
	{
		RaiseException();
	}

	if (om_bitmap->count != om_bitmap->hbp->count)
	{
		RaiseException();
	}
#endif
}

void BitmapFree(POM_BITMAP om_bitmap, PBITMAP_FREE_FUNCTION free_fun)
{
	if (om_bitmap == NULL)
	{
		return;
	}

	if (om_bitmap->buffer)
	{
		free_fun((void*)om_bitmap->buffer, BITMAP_BUFFER_ALLOC_TAG);
	}

#ifdef DBG_OM_BITMAP
	if (om_bitmap->hbp)
	{
		hbitmap_free(om_bitmap->hbp, free_fun, BITMAP_VERIFY_ALLOC_TAG);
	}
#endif

	free_fun((void*)om_bitmap, BITMAP_HEADER_ALLOC_TAG);
}



void BitmapItInit(POM_BITMAP_IT om_it, const POM_BITMAP om_bitmap, uint64_t start_pos)
{
	om_it->om_bitmap = om_bitmap;

	om_it->pos = start_pos >> om_bitmap->granularity;
	uint32_t bit = om_it->pos & (BITS_PER_LONG - 1);
	om_it->pos = om_it->pos >> LOG_BITS_PER_LONG;
	om_it->cur = om_bitmap->buffer[om_it->pos] & ~((1UL << bit) - 1);

	if (om_bitmap->count == 0)
	{
		om_it->pos = om_bitmap->buffer_size;
	}

#ifdef DBG_OM_BITMAP
	hbitmap_iter_init(&om_it->hbi, om_bitmap->hbp, start_pos);

	if (om_it->cur != om_it->hbi.cur[HBITMAP_LEVELS - 1])
	{
		RaiseException();
	}
#endif
}

int64_t BitmapItNext(POM_BITMAP_IT om_it)
{
	int64_t ret = -1;

	bitmap_operator_t cur = om_it->cur;
	if (cur == 0)
	{
		while (om_it->pos < om_it->om_bitmap->buffer_size)
		{
			cur = om_it->om_bitmap->buffer[om_it->pos];
			if (cur != 0)
			{			
				break;
			}

			om_it->pos++;
		}
	}

	if (cur != 0)
	{
		om_it->cur = cur & (cur - 1);

		ret = (om_it->pos << LOG_BITS_PER_LONG) + SearchTrailBit(cur);
		ret = ret << om_it->om_bitmap->granularity;

		if (om_it->cur == 0)
		{
			om_it->pos++;
		}
	}

#ifdef DBG_OM_BITMAP
	int64_t hnext = hbitmap_iter_next(&om_it->hbi);

	bool do_check = true;
	if (ret != hnext)
	{
		if (hnext == -1)
		{
			hbitmap_iter_init(&om_it->hbi, om_it->om_bitmap->hbp, ret);
			hnext = hbitmap_iter_next(&om_it->hbi);
		}
		else if (ret == -1)
		{
			do_check = false;
		}
	}

	if (do_check)
	{
		if (ret != hnext)
		{
			RaiseException();
		}

		if (om_it->cur != om_it->hbi.cur[HBITMAP_LEVELS - 1])
		{
			RaiseException();
		}
	}
#endif

	return ret;
}

#ifdef DBG_OM_BITMAP
void VerifyItSucccessiveProlog(POM_BITMAP_IT om_it, int64_t ret)
{
	int64_t hnext = hbitmap_iter_next(&om_it->hbi);

	if (ret != hnext)
	{
		hbitmap_iter_init(&om_it->hbi, om_it->om_bitmap->hbp, ret);
		hnext = hbitmap_iter_next(&om_it->hbi);
	}

	if (ret != hnext)
	{
		RaiseException();
	}
}

void VerifyItSucccessiveEpilog(POM_BITMAP_IT om_it, int64_t ret, uint64_t ret_count)
{
	int64_t hnext = ret;
	int64_t hbpnext = -1;
	uint64_t i = 1;
	for (i = 1; i < ret_count; i++)
	{
		hnext += (1LL << om_it->om_bitmap->granularity);
		hbitmap_iter_init(&om_it->hbi, om_it->om_bitmap->hbp, hnext);
		hbpnext = hbitmap_iter_next(&om_it->hbi);

		if (hnext != hbpnext)
		{
			RaiseException();
		}
	}
}
#endif

uint64_t BitmapItCountSuccessive(POM_BITMAP_IT om_it, bitmap_operator_t cur, bitmap_operator_t mask, uint64_t max_count)
{
	uint64_t ret_count = 0;

	while (om_it->pos != om_it->om_bitmap->buffer_size)
	{
		if ((cur & mask) == 0)
		{
			break;
		}

		if ((cur == (bitmap_operator_t)-1) && (ret_count + BITS_PER_LONG <= max_count))
		{
			om_it->pos++;
			om_it->cur = 0;

			cur = om_it->om_bitmap->buffer[om_it->pos];
			ret_count += BITS_PER_LONG;
			mask = 1;
		}
		else
		{
			if (ret_count + 1 > max_count)
			{
				break;
			}

			ret_count++;

			om_it->cur = cur & (cur - 1);
			if (om_it->cur == 0)
			{
				om_it->pos++;
				cur = om_it->om_bitmap->buffer[om_it->pos];

				if (mask != ((bitmap_operator_t)1U << (BITS_PER_LONG - 1)))
				{
					break;
				}

				mask = 1;
			}
			else
			{
				cur = om_it->cur;
				mask = mask << 1;
			}
		}
	}

	return ret_count;
}

int64_t BitmapItNextSuccessive(POM_BITMAP_IT om_it, uint64_t max_count, uint64_t* count)
{
	int64_t ret = -1;
	*count = 0;

	max_count = (max_count >> om_it->om_bitmap->granularity);

	bitmap_operator_t mask = 0;
	bitmap_operator_t cur = om_it->cur;
	if (cur == 0)
	{
		while (om_it->pos < om_it->om_bitmap->buffer_size)
		{
			cur = om_it->om_bitmap->buffer[om_it->pos];
			if (cur != 0)
			{
				break;
			}

			om_it->pos++;
		}
	}

	if (cur == 0)
	{
		return ret;
	}

	if (cur == (bitmap_operator_t)-1)
	{
		ret = om_it->pos << LOG_BITS_PER_LONG;
		ret = ret << om_it->om_bitmap->granularity;
		mask = 1;
	}
	else
	{
		uint32_t trail_bit = SearchTrailBit(cur);
		ret = (om_it->pos << LOG_BITS_PER_LONG) + trail_bit;
		ret = ret << om_it->om_bitmap->granularity;
		mask = (1 << trail_bit);
	}

#ifdef DBG_OM_BITMAP
	VerifyItSucccessiveProlog(om_it, ret);
#endif

	uint64_t ret_count = BitmapItCountSuccessive(om_it, cur, mask, max_count);

#ifdef DBG_OM_BITMAP
	VerifyItSucccessiveEpilog(om_it, ret, ret_count);
#endif

	ret_count = (ret_count << om_it->om_bitmap->granularity);

	*count = ret_count;

	return ret;
}

bool GetBitmapBuffer(POM_BITMAP om_bitmap, unsigned char* buffer, uint32_t size)
{
	if (size < om_bitmap->buffer_size * sizeof(bitmap_operator_t))
	{
		return false;
	}

#ifdef BIG_ENDIAN_SUPPORT
	bitmap_operator_t value = 0;
	for (uint32_t i = 0; i < om_bitmap->buffer_size; i++)
	{
		value = ByteSwapToLte(om_bitmap->buffer[i]);
		CopyMemory(buffer + i * sizeof(bitmap_operator_t), sizeof(bitmap_operator_t), &value, sizeof(bitmap_operator_t));
	}
#else
	CopyMemory(buffer, size, om_bitmap->buffer, om_bitmap->buffer_size * sizeof(bitmap_operator_t));
#endif

	return true;
}

bool MergeBitmapByBuffer(POM_BITMAP om_bitmap, unsigned char* buffer, uint64_t size)
{
	uint64_t oper_count = (size + sizeof(bitmap_operator_t) - 1) / sizeof(bitmap_operator_t);
	if (oper_count < om_bitmap->buffer_size)
	{
		return false;
	}

	uint64_t i = 0;
	bitmap_operator_t value = 0;
	for (i = 0; i < om_bitmap->buffer_size; i++)
	{
		if (i == size / sizeof(bitmap_operator_t))
		{
			value = 0;
			CopyMemory(&value, sizeof(bitmap_operator_t), buffer + i * sizeof(bitmap_operator_t), size - i * sizeof(bitmap_operator_t));
		}
		else
		{
			value = *((bitmap_operator_t*)(buffer + i * sizeof(bitmap_operator_t)));
		}

#ifdef BIG_ENDIAN_SUPPORT
		value = ByteSwapFromLte(value);
#endif
		om_bitmap->count += PopCount((om_bitmap->buffer[i] & value) ^ value);
		om_bitmap->buffer[i] |= value;
	}

#ifdef DBG_OM_BITMAP
	uint64_t nb_sectors = (uint64_t)1U << om_bitmap->granularity;
	uint64_t start_pos = 0;
	uint32_t trail_bit = 0;
	for (i = 0; i < om_bitmap->buffer_size; i ++)
	{
		if (i == size / sizeof(bitmap_operator_t))
		{
			value = 0;
			CopyMemory(&value, sizeof(bitmap_operator_t), buffer + i * sizeof(bitmap_operator_t), size - i * sizeof(bitmap_operator_t));
		}
		else
		{
			value = *((bitmap_operator_t*)(buffer + i * sizeof(bitmap_operator_t)));
		}

		value = ByteSwapFromLte(value);

		while (value)
		{
			trail_bit = SearchTrailBit(value);
			start_pos = ((i << LOG_BITS_PER_LONG) + trail_bit) << om_bitmap->granularity;
			hbitmap_set(om_bitmap->hbp, start_pos, nb_sectors);

			value &= ~(1 << trail_bit);
		}
	}

	if (memcmp(om_bitmap->buffer, om_bitmap->hbp->levels[HBITMAP_LEVELS - 1], om_bitmap->buffer_size * sizeof(bitmap_operator_t)) != 0)
	{
		RaiseException();
	}

	if (om_bitmap->count != om_bitmap->hbp->count)
	{
		RaiseException();
	}
#endif

	return true;
}

bool MergeBitmap(POM_BITMAP om_dest, POM_BITMAP om_src, PBITMAP_ALLOC_FUNCTION alloc_fun, PBITMAP_FREE_FUNCTION free_fun)
{
	if (om_dest->buffer_size != om_src->buffer_size)
	{
		return false;
	}

	uint64_t i = 0;
	for (i = 0; i < om_dest->buffer_size; i++)
	{
		om_dest->count += PopCount((om_dest->buffer[i] & om_src->buffer[i]) ^ om_src->buffer[i]);
		om_dest->buffer[i] |= om_src->buffer[i];
	}

#ifdef DBG_OM_BITMAP
	uint64_t size = om_src->buffer_size * sizeof(bitmap_operator_t);
	unsigned char* buffer = (unsigned char*)alloc_fun(size, BITMAP_BUFFER_ALLOC_TAG);
	if (buffer == NULL)
	{
		RaiseException();
	}

	if (!GetBitmapBuffer(om_src, buffer, (uint32_t)size))
	{
		free_fun(buffer, BITMAP_BUFFER_ALLOC_TAG);
		RaiseException();
	}

	bitmap_operator_t value = 0;
	uint64_t nb_sectors = (uint64_t)1U << om_dest->granularity;
	uint64_t start_pos = 0;
	uint32_t trail_bit = 0;
	for (i = 0; i < om_dest->buffer_size; i++)
	{
		value = *((bitmap_operator_t*)(buffer + i * sizeof(bitmap_operator_t)));
		value = ByteSwapFromLte(value);

		while (value)
		{
			trail_bit = SearchTrailBit(value);
			start_pos = ((i << LOG_BITS_PER_LONG) + trail_bit) << om_dest->granularity;
			hbitmap_set(om_dest->hbp, start_pos, nb_sectors);

			value &= ~(1 << trail_bit);
		}
	}

	if (memcmp(om_dest->buffer, om_dest->hbp->levels[HBITMAP_LEVELS - 1], om_dest->buffer_size * sizeof(bitmap_operator_t)) != 0)
	{
		RaiseException();
	}

	if (om_dest->count != om_dest->hbp->count)
	{
		RaiseException();
	}

	free_fun(buffer, BITMAP_BUFFER_ALLOC_TAG);
#else
#ifdef _WIN32
	UNREFERENCED_PARAMETER(alloc_fun);
	UNREFERENCED_PARAMETER(free_fun);
#endif
#endif

	return true;
}

bool AndBitmap(POM_BITMAP om_dest, POM_BITMAP om_src, PBITMAP_ALLOC_FUNCTION alloc_fun, PBITMAP_FREE_FUNCTION free_fun)
{
	if (om_dest->buffer_size != om_src->buffer_size)
	{
		return false;
	}

	uint64_t i = 0;
	for (i = 0; i < om_dest->buffer_size; i++)
	{
		om_dest->count -= PopCount((om_dest->buffer[i] ^ om_src->buffer[i]) & om_dest->buffer[i]);
		om_dest->buffer[i] &= om_src->buffer[i];
	}

#ifdef DBG_OM_BITMAP
	uint64_t size = om_src->buffer_size * sizeof(bitmap_operator_t);
	unsigned char* buffer = (unsigned char*)alloc_fun(size, BITMAP_BUFFER_ALLOC_TAG);
	if (buffer == NULL)
	{
		RaiseException();
	}

	if (!GetBitmapBuffer(om_src, buffer, (uint32_t)size))
	{
		free_fun(buffer, BITMAP_BUFFER_ALLOC_TAG);
		RaiseException();
	}

	bitmap_operator_t value = 0;
	uint64_t nb_sectors = (uint64_t)1U << om_dest->granularity;
	uint64_t start_pos = 0;
	uint32_t trail_bit = 0;
	for (i = 0; i < om_dest->buffer_size; i++)
	{
		value = *((bitmap_operator_t*)(buffer + i * sizeof(bitmap_operator_t)));
		value = ByteSwapFromLte(value);

		value = ~value;

		while (value)
		{
			trail_bit = SearchTrailBit(value);
			start_pos = ((i << LOG_BITS_PER_LONG) + trail_bit) << om_dest->granularity;
			hbitmap_reset(om_dest->hbp, start_pos, nb_sectors);

			value &= ~(1 << trail_bit);
		}
	}

	if (memcmp(om_dest->buffer, om_dest->hbp->levels[HBITMAP_LEVELS - 1], om_dest->buffer_size * sizeof(bitmap_operator_t)) != 0)
	{
		RaiseException();
	}

	if (om_dest->count != om_dest->hbp->count)
	{
		RaiseException();
	}

	free_fun(buffer, BITMAP_BUFFER_ALLOC_TAG);
#else
#ifdef _WIN32
	UNREFERENCED_PARAMETER(alloc_fun);
	UNREFERENCED_PARAMETER(free_fun);
#endif
#endif

	return true;
}

bool CompareBitmap(POM_BITMAP om_bitmap1, POM_BITMAP om_bitmap2)
{
	if (om_bitmap1->count != om_bitmap2->count)
	{
		return false;
	}

	if (om_bitmap1->buffer_size != om_bitmap2->buffer_size)
	{
		return false;
	}

	if (memcmp(om_bitmap1->buffer, om_bitmap2->buffer, om_bitmap1->buffer_size * sizeof(bitmap_operator_t)) != 0)
	{
		return false;
	}

	return true;
}

uint64_t GetBitmapBufferSize(uint64_t user_size, uint32_t granularity)
{
	uint64_t ret = (user_size + (1ULL << granularity) - 1) >> granularity;
	ret = (ret + BITS_PER_LONG - 1) >> LOG_BITS_PER_LONG;

	return ret;
}

bool IsBitmapBitSet(POM_BITMAP om_bitmap, uint64_t start_pos)
{
	uint64_t pos = start_pos >> om_bitmap->granularity;
	uint32_t bit = 1UL << (pos & (BITS_PER_LONG - 1));

	return ((om_bitmap->buffer[pos >> LOG_BITS_PER_LONG] & bit) != 0);
}