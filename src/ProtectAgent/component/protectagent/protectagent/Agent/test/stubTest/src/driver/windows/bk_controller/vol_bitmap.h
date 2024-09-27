#pragma once

#include <Windows.h>

#include "om_bitmap.h"


class CVolBitmap
{
public:
	CVolBitmap(HANDLE hVolume, ULONG ulClusterSize, ULONGLONG ullVolSize);
	~CVolBitmap();

public:
	DWORD GetBitmap( OM_BITMAP* out_bitmap);

private:
	void CVolBitmap::ConvertBitmap(PVOLUME_BITMAP_BUFFER vol_bit_buffer, OM_BITMAP* vol_bitmap);

private:
	HANDLE m_hVolume;
	ULONG m_ulClusterSize;
	ULONGLONG m_ullClusterCount;
};
