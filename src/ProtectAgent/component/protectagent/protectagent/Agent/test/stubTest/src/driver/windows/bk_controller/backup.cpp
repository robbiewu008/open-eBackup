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
#include "backup.h"
#include "controller.h"
#include "log.h"
#include "bitmap_alloc.h"
#include "disk_backup.h"

#include <set>
#include <ntddvol.h>




CBackup::CBackup(CBkController* pCtrl)
{
	m_pCtrl = pCtrl;
}

CBackup::~CBackup()
{
	map<CString, VOLUME_INFO>::iterator it = m_mpVolume.begin();
	while (it != m_mpVolume.end())
	{
		if (it->second.vol_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(it->second.vol_handle);
			it->second.vol_handle = INVALID_HANDLE_VALUE;
		}

		it++;
	}

	m_mpVolume.clear();

	map<DWORD, DISK_INFO>::iterator it_disk = m_mpDisk.begin();
	while (it_disk != m_mpDisk.end())
	{
		if (it_disk->second.disk_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(it_disk->second.disk_handle);
			it_disk->second.disk_handle = INVALID_HANDLE_VALUE;
		}

		if (it_disk->second.write_to_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(it_disk->second.write_to_handle);
			it_disk->second.write_to_handle = INVALID_HANDLE_VALUE;
		}

		it_disk++;
	}

	m_mpDisk.clear();
}

VOID CBackup::DoBackup(BOOL bFull, const vector<ADD_DEL_PARTITION_PARAM>& vecVolume, DWORD dwFirstTarget)
{
	DWORD dwError = 0;
	HANDLE hVolume = INVALID_HANDLE_VALUE;
	unsigned char granularity = 0;
	do
	{
		CVdsWrapper vdsWrapper;
		dwError = vdsWrapper.Initialize();
		if (dwError != 0)
		{
			IOMIRROR_ERROR(dwError, L"Failed to initialize vds wrapper");
			break;
		}

		dwError = m_pCtrl->StartSnapshot(&granularity);
		if (dwError != 0)
		{
			IOMIRROR_ERROR(dwError, L"Failed to start snapshot");
			break;
		}

		dwError = CreateSnapshot();
		if (dwError != 0)
		{
			IOMIRROR_ERROR(dwError, L"Failed to take snapshot");
			break;
		}

		dwError = m_pCtrl->FinishSnapshot();
		if (dwError != 0)
		{
			IOMIRROR_ERROR(dwError, L"Failed to start snapshot");
			break;
		}
		
		CString strVolume;
		CString strVolumeHandleName;
		ULARGE_INTEGER liVolSize;
		set<DISK_EXTENT_INFO> vol_disks;
		for (size_t i = 0; i < vecVolume.size(); i++)
		{
			VOLUME_INFO vol_info;
			strVolume = CString(vecVolume[i].disk_path);
			strVolumeHandleName = strVolume;
			strVolumeHandleName.Replace(L"\\Device", L"\\\\?");
			vol_info.vol_name = strVolume;
			vol_info.vol_bitmap = NULL;
			vol_info.vol_size = 0;
			vol_info.hbi = NULL;
			vol_info.vol_handle = GetVolumeHandle(strVolumeHandleName);
			if (vol_info.vol_handle == INVALID_HANDLE_VALUE)
			{
				break;
			}

			CVdsVolume* pVdsVolume = vdsWrapper.GetVdsVolume(strVolume);
			if (pVdsVolume == NULL)
			{
				break;
			}

			pair<map<CString, VOLUME_INFO>::iterator, bool> insert_ret = m_mpVolume.insert(make_pair(strVolume, vol_info));

			strVolumeHandleName += L"\\";
			if (!GetDiskFreeSpaceEx(strVolumeHandleName.GetString(), NULL, &liVolSize, NULL))
			{
				dwError = GetLastError();
				IOMIRROR_ERROR(dwError, L"Failed to get volume size of %s", strVolume.GetString());
				break;
			}

			DWORD dwClusterSize = 0, dwSectorSize = 0;
			if (!GetDiskFreeSpace(strVolumeHandleName.GetString(), &dwClusterSize, &dwSectorSize, NULL, NULL))
			{
				dwError = GetLastError();
				IOMIRROR_ERROR(dwError, L"Failed to get volume cluster size of %s", strVolume.GetString());
				break;
			}

			insert_ret.first->second.vol_size = liVolSize.QuadPart;
			insert_ret.first->second.cluster_size = dwClusterSize * dwSectorSize;
			insert_ret.first->second.vds_type = pVdsVolume->GetVolumeType();
			insert_ret.first->second.stripe_size = pVdsVolume->GetStripeSize();

			dwError = OpenVolDisk(insert_ret.first->second, dwFirstTarget);
			if (dwError != 0)
			{
				IOMIRROR_ERROR(dwError, L"Failed to open volume disk of %s", strVolume.GetString());
				break;
			}

			dwError = GetVolCBTBitmap(insert_ret.first->second, granularity, !bFull);
			if (dwError != 0)
			{
				IOMIRROR_ERROR(dwError, L"Failed to get volume CBT bitmap of %s", strVolume.GetString());
				break;
			}
		}

		if (dwError != 0)
		{
			break;
		}

		map<DWORD, DISK_INFO>::iterator it_disk = m_mpDisk.begin();
		while (it_disk != m_mpDisk.end())
		{
			dwError = BackupDisk(it_disk->second);
			if (dwError != 0)
			{
				IOMIRROR_ERROR(dwError, L"Failed to backup disk %d", it_disk->second.disk_num);
				break;
			}

			it_disk++;
		}

		if (dwError != 0)
		{
			break;
		}

		map<CString, VOLUME_INFO>::iterator it_vol = m_mpVolume.begin();
		while (it_vol != m_mpVolume.end())
		{
			dwError = BackupVolume(it_vol->second, granularity);
			if (dwError != 0)
			{
				IOMIRROR_ERROR(dwError, L"Failed to backup volume");
				break;
			}

			it_vol++;
		}
	} while (0);

	m_pCtrl->RemSnapshot((dwError != 0));
}





HANDLE CBackup::GetVolumeHandle(const CString& strVolume)
{
	HANDLE hRet = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	map<CString, VOLUME_INFO>::iterator it = m_mpVolume.find(strVolume);
	if (it == m_mpVolume.end())
	{
		hRet = CreateFile(strVolume.GetString(), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
		if (hRet == INVALID_HANDLE_VALUE)
		{
			dwError = GetLastError();
			IOMIRROR_ERROR(dwError, L"Failed to open volume %s", strVolume.GetString());
		}
	}
	else
	{
		hRet = it->second.vol_handle;
	}

	return hRet;
}

DISK_INFO* CBackup::GetDiskInfo(ULONG ulDiskNumber, ULONG dwFirstTarget)
{
	DISK_INFO* pRet = NULL;
	DWORD dwError = 0;

	map<DWORD, DISK_INFO>::iterator it = m_mpDisk.find(ulDiskNumber);
	if (it == m_mpDisk.end())
	{
		CString strDiskName;
		strDiskName.Format(L"\\\\.\\PhysicalDrive%d", ulDiskNumber);

		HANDLE disk_handle = CreateFile(strDiskName.GetString(), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
		if (disk_handle == INVALID_HANDLE_VALUE)
		{
			dwError = GetLastError();
			IOMIRROR_ERROR(dwError, L"Failed to open disk %d", ulDiskNumber);
		}
		else
		{
			DISK_INFO disk_info;
			disk_info.disk_handle = disk_handle;
			disk_info.disk_num = ulDiskNumber;
			pair<map<DWORD, DISK_INFO>::iterator, bool> insert_ret = m_mpDisk.insert(make_pair(ulDiskNumber, disk_info));

			DWORD dwOutput;
			GET_LENGTH_INFORMATION length_info;
			if (!DeviceIoControl(insert_ret.first->second.disk_handle, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &length_info, sizeof(GET_LENGTH_INFORMATION), &dwOutput, NULL))
			{
				dwError = GetLastError();
				IOMIRROR_ERROR(dwError, L"send IOCTL_DISK_GET_LENGTH_INFO failed");
				return pRet;
			}

			insert_ret.first->second.disk_size = length_info.Length.QuadPart;

			DISK_GEOMETRY disk_geo = { 0 };
			if (!DeviceIoControl(insert_ret.first->second.disk_handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &disk_geo, sizeof(DISK_GEOMETRY), &dwOutput, NULL))
			{
				dwError = GetLastError();
				IOMIRROR_ERROR(dwError, L"send IOCTL_DISK_GET_DRIVE_GEOMETRY failed");
				return pRet;
			}

			insert_ret.first->second.sector_size = disk_geo.BytesPerSector;
			insert_ret.first->second.disk_type = DISK_TYPE_MBR_BASIC;
			insert_ret.first->second.write_to = insert_ret.first->second.disk_num + dwFirstTarget;

			strDiskName;
			strDiskName.Format(L"\\\\.\\PhysicalDrive%d", insert_ret.first->second.write_to);
			disk_handle = CreateFile(strDiskName.GetString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);
			if (disk_handle == INVALID_HANDLE_VALUE)
			{
				dwError = GetLastError();
				IOMIRROR_ERROR(dwError, L"Failed to open target disk %d", insert_ret.first->second.write_to);
				return pRet;
			}

			insert_ret.first->second.write_to_handle = disk_handle;

			pRet = &insert_ret.first->second;
		}
	}
	else
	{
		pRet = &it->second;
	}

	return pRet;
}

DWORD CBackup::CreateSnapshot()
{
	//Sleep(50 * 1000);

	return 0;
}

DWORD CBackup::OpenVolDisk(VOLUME_INFO& vol_info, DWORD dwFirstTarget)
{
	DWORD dwRet = 0;
	VOLUME_DISK_EXTENTS* pExtent = NULL;
	do
	{
		ULONG ulSize = sizeof(VOLUME_DISK_EXTENTS) + 128 * sizeof(DISK_EXTENT);
		pExtent = (VOLUME_DISK_EXTENTS*)malloc(ulSize);
		if (pExtent == NULL)
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for disk extents");
			break;
		}

		DWORD dwReturned = 0;
		if (!DeviceIoControl(vol_info.vol_handle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, pExtent, ulSize, &dwReturned, NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get volume extents for %s", vol_info.vol_name.GetString());
			break;
		}

		ULONGLONG vol_ext = 0;
		for (int i = 0; i < pExtent->NumberOfDiskExtents; i++)
		{
			DISK_EXTENT_INFO disk_info;
			disk_info.disk_ext = pExtent->Extents[i];
			disk_info.disk_info = NULL;
			disk_info.vol_offset = vol_ext;
			vol_info.vol_disk.insert(make_pair(disk_info.disk_ext.DiskNumber, disk_info));

			disk_info.disk_info = GetDiskInfo(disk_info.disk_ext.DiskNumber, dwFirstTarget);
			if (disk_info.disk_info == NULL)
			{
				dwRet = -1;
				IOMIRROR_ERROR(dwRet, L"Failed to open disk handle of %d for volume %s", disk_info.disk_ext.DiskNumber, vol_info.vol_name.GetString());
				break;
			}

			vol_ext += pExtent->Extents[i].ExtentLength.QuadPart;
		}

		if (dwRet != 0)
		{
			break;
		}
	} while (0);

	if (pExtent)
	{
		free(pExtent);
		pExtent = NULL;
	}

	return dwRet;
}

DWORD CBackup::GetVolCBTBitmap(VOLUME_INFO& vol_info, unsigned char granularity, BOOL bInc)
{
	DWORD dwRet = 0;
	BYTE* pBuffer = NULL;
	ULONGLONG bit_size = 0;
	OM_BITMAP* vol_fs_bitmap = NULL;
	do
	{
		ULONGLONG sectors = vol_info.vol_size / SECTOR_SIZE;
		vol_info.vol_bitmap = BitmapAlloc(sectors, granularity, BitmapAlloc, BitmapFree);
		if (NULL == vol_info.vol_bitmap)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for om_bitmap");
			break;
		}

		vol_fs_bitmap = BitmapAlloc(sectors, granularity, BitmapAlloc, BitmapFree);
		if (NULL == vol_fs_bitmap)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for vol_fs_bitmap");
			break;
		}

		vol_info.hbi = (OM_BITMAP_IT *)malloc(sizeof(OM_BITMAP_IT));
		if (NULL == vol_info.hbi)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for om_bitmap iterator");
			break;
		}

		if (bInc)
		{
			bit_size = GetBitmapBufferSize(vol_info.vol_size, granularity);
			pBuffer = (BYTE*)malloc(bit_size);
			if (pBuffer == NULL)
			{
				dwRet = -1;
				IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for bitmap buffer");
				break;
			}

			GET_BITMAP_PARAM get_bit_param = { 0 };
			get_bit_param.buffer = pBuffer;
			get_bit_param.size = bit_size;
			CStringA strVolName = vol_info.vol_name;
			memcpy(get_bit_param.disk_path, strVolName.GetString(), DISK_PATH_LEN);
			dwRet = m_pCtrl->GetBkBitmap(get_bit_param);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to get bitmap for volume %s", vol_info.vol_name.GetString());
				break;
			}

			if (!MergeBitmapByBuffer(vol_info.vol_bitmap, pBuffer, bit_size))
			{
				dwRet = -1;
				IOMIRROR_ERROR(dwRet, L"Failed to merge bitmap for volume %s", vol_info.vol_name.GetString());
				break;
			}
		}
		else
		{
			BitmapSetBit(vol_info.vol_bitmap, 0, sectors);
		}

		uint64_t bk_size = GetBitmapCount(vol_info.vol_bitmap) * SECTOR_SIZE;
		IOMIRROR_ERROR(0, L"Pre prune size %llu for volume %s", bk_size, vol_info.vol_name.GetString());

		CVolBitmap vol_bitmap(vol_info.vol_handle, vol_info.cluster_size, vol_info.vol_size);
		dwRet = vol_bitmap.GetBitmap(vol_fs_bitmap);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to get fs bitmap for volume %s", vol_info.vol_name.GetString());
			break;
		}

		if (!AndBitmap(vol_info.vol_bitmap, vol_fs_bitmap, BitmapAlloc, BitmapFree))
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to And fs bitmap for volume %s", vol_info.vol_name.GetString());
			break;
		}

		bk_size = GetBitmapCount(vol_info.vol_bitmap) * SECTOR_SIZE;
		IOMIRROR_ERROR(0, L"Post prune size %llu for volume %s", bk_size, vol_info.vol_name.GetString());
	} while (0);

	if (pBuffer)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	if (vol_fs_bitmap)
	{
		BitmapFree(vol_fs_bitmap, BitmapFree);
		vol_fs_bitmap = NULL;
	}

	return dwRet;
}

DWORD CBackup::BackupDisk(const DISK_INFO& disk_info)
{
	Disk_Backup disk_backup(disk_info.disk_handle, disk_info.write_to_handle, disk_info.disk_num, disk_info.write_to, disk_info.disk_size, disk_info.sector_size, disk_info.disk_type);
	DWORD dwRet = disk_backup.BackupDisk();
	if (dwRet != 0)
	{
		IOMIRROR_ERROR(dwRet, L"Failed to backup disk %d to %d", disk_info.disk_num, disk_info.write_to);
	}

	return dwRet;

	//DWORD dwRet = 0;
	//BYTE* pBuffer = NULL;
	//do
	//{
	//	DWORD dwSize = 2 * 1024 * 1024;
	//	pBuffer = (BYTE*)malloc(dwSize);
	//	if (pBuffer == NULL)
	//	{
	//		dwRet = -1;
	//		IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for read buffer");
	//		break;
	//	}

	//	dwRet = SetFilePointer(disk_info.disk_handle, 0, NULL, FILE_BEGIN);
	//	if (dwRet == INVALID_SET_FILE_POINTER)
	//	{
	//		dwRet = GetLastError();
	//		IOMIRROR_ERROR(dwRet, L"Failed to set pointer to beginning, disk %d", disk_info.disk_num);
	//		break;
	//	}

	//	DWORD dwRead = 0;
	//	if (!ReadFile(disk_info.disk_handle, pBuffer, disk_info.sector_size, &dwRead, NULL))
	//	{
	//		dwRet = GetLastError();
	//		IOMIRROR_ERROR(dwRet, L"Failed to read disk, disk %d", disk_info.disk_num);
	//		break;
	//	}

	//	dwRet = SetFilePointer(disk_info.write_to_handle, 0, NULL, FILE_BEGIN);
	//	if (dwRet == INVALID_SET_FILE_POINTER)
	//	{
	//		dwRet = GetLastError();
	//		IOMIRROR_ERROR(dwRet, L"Failed to set pointer to beginning, write disk %d", disk_info.write_to);
	//		break;
	//	}

	//	DWORD dwWritten = 0;
	//	if (!WriteFile(disk_info.write_to_handle, pBuffer, disk_info.sector_size, &dwWritten, NULL))
	//	{
	//		dwRet = GetLastError();
	//		IOMIRROR_ERROR(dwRet, L"Failed to write disk, disk %d", disk_info.write_to);
	//		break;
	//	}
	//} while (0);

	//if (pBuffer)
	//{
	//	free(pBuffer);
	//	pBuffer = NULL;
	//}

	return dwRet;
}

DWORD CBackup::BackupVolume(const VOLUME_INFO& vol_info, unsigned char granularity)
{
	DWORD dwRet = 0;
	BYTE* pBuffer = NULL;
	ULONGLONG bk_size = 0;
	ULONGLONG bit_it_size = 0;
	ULONGLONG bit_cnt_size = GetBitmapCount(vol_info.vol_bitmap) * SECTOR_SIZE;
	do
	{
		BitmapItInit(vol_info.hbi, vol_info.vol_bitmap, 0);

		DWORD dwSize = 16 * 1024 * 1024;
		pBuffer = (BYTE*)malloc(dwSize);
		if (pBuffer == NULL)
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for read buffer");
			break;
		}

		int64_t sector = 0;
		uint64_t size = 0;
		DWORD dwReturned = 0;
		ULONGLONG sectors = vol_info.vol_size / SECTOR_SIZE;
		uint64_t nb_sectors = (uint64_t)1U << granularity;
		LARGE_INTEGER file_pos;
		while (GetBitmapCount(vol_info.vol_bitmap) != 0)
		{
			sector = BitmapItNextSuccessive(vol_info.hbi, dwSize / SECTOR_SIZE, &nb_sectors);
			if ((sectors <= sector + nb_sectors))
			{
				size = SECTOR_SIZE * (sectors - sector);
			}
			else
			{
				size = SECTOR_SIZE * nb_sectors;
			}

			bk_size += size;
			bit_it_size += nb_sectors * SECTOR_SIZE;

			if (sector == -1 || nb_sectors == 0)
			{
				dwRet = -1;
				IOMIRROR_ERROR(dwRet, L"Failed to iterator bitmap");
				break;
			}

			file_pos.QuadPart = sector * SECTOR_SIZE;
			dwRet = SetFilePointerEx(vol_info.vol_handle, file_pos, NULL, FILE_BEGIN);
			if (dwRet == INVALID_SET_FILE_POINTER)
			{
				dwRet = GetLastError();
				IOMIRROR_ERROR(dwRet, L"Failed to set volume pointer, volume %s", vol_info.vol_name.GetString());
				break;
			}

			DWORD dwRead = 0;
			if (!ReadFile(vol_info.vol_handle, pBuffer, size, &dwRead, NULL))
			{
				dwRet = GetLastError();
				IOMIRROR_ERROR(dwRet, L"Failed to read volume, volume %s", vol_info.vol_name.GetString());
				break;
			}

			dwRet = WriteVolumeDisk(vol_info, pBuffer, file_pos.QuadPart, size);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to write volume disk, volume %s", vol_info.vol_name.GetString());
				break;
			}

			uint64_t left;
			left = sectors - sector;
			nb_sectors = (nb_sectors < left) ? nb_sectors : left;
			BitmapResetBit(vol_info.vol_bitmap, sector, nb_sectors);
		}

		if (dwRet != 0)
		{
			break;
		}

		if (bit_it_size != bit_cnt_size)
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Bitmap it size %llu is different from count size %llu, volume %s", bit_it_size, bit_cnt_size, vol_info.vol_name.GetString());

#ifdef DBG_OM_BITMAP
			RaiseException();
#else
			break;
#endif
		}
	} while (0);

	if (dwRet == 0)
	{
		IOMIRROR_ERROR(0, L"Backup size %llu(vs. %llu), volume %s", bk_size, bit_it_size, vol_info.vol_name.GetString());
	}
	else
	{
		IOMIRROR_ERROR(dwRet, L"Backup failed for volume %s", vol_info.vol_name.GetString());
	}

	if (pBuffer)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	return dwRet;
}

DWORD CBackup::WriteVolumeDisk(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size)
{
	DWORD dwRet = 0;

	do
	{
		if (vol_info.vds_type == VDS_VT_SIMPLE)
		{
			dwRet = WriteDiskForSimple(vol_info, pBuffer, vol_pos, size);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to write disk for simple volume %s", vol_info.vol_name.GetString());
				break;
			}
		}
		else if (vol_info.vds_type == VDS_VT_SPAN)
		{
			dwRet = WriteDiskForSpan(vol_info, pBuffer, vol_pos, size);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to write disk for span volume %s", vol_info.vol_name.GetString());
				break;
			}
		}
		else if (vol_info.vds_type == VDS_VT_STRIPE)
		{
			dwRet = WriteDiskForStripe(vol_info, pBuffer, vol_pos, size);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to write disk for stripe volume %s", vol_info.vol_name.GetString());
				break;
			}
		}
		else if (vol_info.vds_type == VDS_VT_MIRROR)
		{
			dwRet = WriteDiskForMirror(vol_info, pBuffer, vol_pos, size);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to write disk for mirror volume %s", vol_info.vol_name.GetString());
				break;
			}
		}
		else
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Unsupported volume type %d, volume %s", vol_info.vds_type, vol_info.vol_name.GetString());
			break;
		}
	} while (0);

	return dwRet;
}

DWORD CBackup::WriteDiskForSimple(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size)
{
	DWORD dwRet = 0;
	do
	{
		map<DWORD, DISK_INFO>::iterator it_disk = m_mpDisk.find(vol_info.vol_disk.begin()->first);
		if (it_disk == m_mpDisk.end())
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to find disk with number %d", vol_info.vol_disk.begin()->first);
			break;
		}

		LARGE_INTEGER file_pos;
		file_pos.QuadPart = vol_info.vol_disk.begin()->second.disk_ext.StartingOffset.QuadPart + vol_pos;
		if(!SetFilePointerEx(it_disk->second.write_to_handle, file_pos, NULL, FILE_BEGIN))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to set pointer to ending, write disk %d", it_disk->second.write_to);
			break;
		}

		DWORD dwWritten = 0;
		if (!WriteFile(it_disk->second.write_to_handle, pBuffer, size, &dwWritten, NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to write disk, disk %d", it_disk->second.write_to);
			break;
		}
	} while (0);

	return dwRet;
}

DWORD CBackup::WriteDiskForSpan(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size)
{
	return -1;
}

DWORD CBackup::WriteDiskForStripe(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size)
{
	return -1;
}

DWORD CBackup::WriteDiskForMirror(const VOLUME_INFO& vol_info, BYTE* pBuffer, ULONGLONG vol_pos, ULONG size)
{
	return -1;
}