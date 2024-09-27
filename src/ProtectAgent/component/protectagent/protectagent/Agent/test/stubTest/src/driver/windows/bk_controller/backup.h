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
#pragma once
#include "const.h"
#include "controller.h"

#include "om_bitmap.h"
#include "vol_bitmap.h"

#include "vds_wrapper.h"




typedef struct _DISK_INFO
{
	DWORD disk_num;
	HANDLE disk_handle;
	HANDLE write_to_handle;
	DWORD write_to;
	ULONGLONG disk_size;
	ULONG sector_size;
	ULONG disk_type;

	bool operator==(const _DISK_INFO& other) const
	{
		return (disk_num == other.disk_num);
	}

	bool operator <(const _DISK_INFO& other) const
	{
		return (disk_num < other.disk_num);
	}
}DISK_INFO;

typedef struct _DISK_EXTENT_INFO
{
	DISK_EXTENT disk_ext;
	DISK_INFO* disk_info;
	ULONGLONG vol_offset;
} DISK_EXTENT_INFO;


typedef struct _VOLUME_INFO
{
	CString vol_name;
	HANDLE vol_handle;
	ULONGLONG vol_size;
	OM_BITMAP* vol_bitmap;
	CVolBitmap* fs_bitmap;
	OM_BITMAP_IT *hbi;
	ULONG cluster_size;
	ULONG stripe_size;
	VDS_VOLUME_TYPE vds_type;
	map<DWORD, DISK_EXTENT_INFO> vol_disk;
} VOLUME_INFO;

class CBackup
{
public:
	CBackup(CBkController* pCtrl);
	~CBackup();

public:
	VOID DoBackup( BOOL bFull, const vector<ADD_DEL_PARTITION_PARAM>& vecVolume, DWORD dwFirstTarget);

private:
	HANDLE GetVolumeHandle(const CString& strVolume);
	DISK_INFO* GetDiskInfo(ULONG ulDiskNumber, ULONG dwFirstTarget);
	DWORD CreateSnapshot();

	DWORD OpenVolDisk(VOLUME_INFO& vol_info, DWORD dwFirstTarget);
	DWORD GetVolCBTBitmap(VOLUME_INFO& vol_info, unsigned char granularity, BOOL bInc);

	DWORD BackupDisk(const DISK_INFO& disk_info);
	DWORD BackupVolume(const VOLUME_INFO& vol_info, unsigned char granularity);

	DWORD WriteVolumeDisk(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size);
	DWORD WriteDiskForSimple(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size);
	DWORD WriteDiskForSpan(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size);
	DWORD WriteDiskForStripe(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size);
	DWORD WriteDiskForMirror(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size);

private:
	CBkController* m_pCtrl;
	map<CString, VOLUME_INFO> m_mpVolume;
	map<DWORD, DISK_INFO> m_mpDisk;
};