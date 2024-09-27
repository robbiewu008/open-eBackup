/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "stdafx.h"
#include "vol_bitmap.h"

#include "const.h"



CVolBitmap::CVolBitmap(HANDLE hVolume, ULONG ulClusterSize, ULONGLONG ullVolSize)
{
	m_hVolume = hVolume;
	m_ulClusterSize = ulClusterSize;
	m_ullClusterCount = ullVolSize  / m_ulClusterSize;
}

CVolBitmap::~CVolBitmap()
{
}


DWORD CVolBitmap::GetBitmap(OM_BITMAP* vol_bitmap)
{
	DWORD dwRet = 0;

	STARTING_LCN_INPUT_BUFFER  start_lcn = { 0 };
	ULONGLONG vol_bit_buf_size = sizeof(VOLUME_BITMAP_BUFFER) + (m_ullClusterCount + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
	PVOLUME_BITMAP_BUFFER vol_bit_buffer = NULL;

	do
	{
		vol_bit_buffer = (PVOLUME_BITMAP_BUFFER)malloc(vol_bit_buf_size);
		if (!vol_bit_buffer)
		{
			dwRet = -1;
			break;
		}

		memset(vol_bit_buffer, 0, vol_bit_buf_size);
		
		DWORD dwReturned = 0;
		if (!DeviceIoControl(m_hVolume, FSCTL_GET_VOLUME_BITMAP, &start_lcn, sizeof(STARTING_LCN_INPUT_BUFFER), vol_bit_buffer,  vol_bit_buf_size, &dwReturned, NULL))
		{
			dwRet = GetLastError();
			break;
		}

		ConvertBitmap(vol_bit_buffer, vol_bitmap);
	} while (0);

	if (vol_bit_buffer)
	{
		free(vol_bit_buffer);
		vol_bit_buffer = NULL;
	}

	return dwRet;
}



static inline int GetNextBit(unsigned char c)
{
	if (0 == c)
	{
		return -1;
	}
	if (c & 0x01)
	{
		return 0;
	}
	if (c & 0x02)
	{
		return 1;
	}
	if (c & 0x04)
	{
		return 2;
	}
	if (c & 0x08)
	{
		return 3;
	}
	if (c & 0x10)
	{
		return 4;
	}
	if (c & 0x20)
	{
		return 5;
	}
	if (c & 0x40)
	{
		return 6;
	}
	if (c & 0x80)
	{
		return 7;
	}
	return -1;
}

static inline void ResetBit(int nr, unsigned char* addr)
{
	uint8_t* p = addr;
	uint8_t mask = ~(1U << nr);
	*p &= mask;
}

void CVolBitmap::ConvertBitmap(PVOLUME_BITMAP_BUFFER vol_bit_buffer, OM_BITMAP* vol_bitmap)
{
	ULONGLONG vol_bit_size = (vol_bit_buffer->BitmapSize.QuadPart + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
	int bit = 0;
	uint64_t start_pos = 0;
	uint64_t count = 0;
	uint64_t size = 0;
	for (ULONG i = 0; i < vol_bit_size; i++)
	{
		while (1)
		{
			bit = GetNextBit(vol_bit_buffer->Buffer[i]);
			if (-1 == bit)
			{
				break;
			}

			size += m_ulClusterSize;

			start_pos = ((ULONGLONG)m_ulClusterSize * (BITS_PER_BYTE * i + bit)) / SECTOR_SIZE;
			count = m_ulClusterSize / SECTOR_SIZE;

			BitmapSetBit(vol_bitmap, start_pos, count);

			ResetBit(bit, &vol_bit_buffer->Buffer[i]);
		}
	}

#ifdef DBG_OM_BITMAP
	uint64_t om_bit_size = GetBitmapCount(vol_bitmap) * SECTOR_SIZE;
	if (om_bit_size < size)
	{
		RaiseException();
	}
#endif
}