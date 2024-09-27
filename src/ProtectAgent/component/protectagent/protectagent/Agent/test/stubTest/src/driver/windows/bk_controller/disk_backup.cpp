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
#include "disk_backup.h"

#include "log.h"



Disk_Backup::Disk_Backup(HANDLE disk_handle, HANDLE target_disk_handle, ULONG disk_num, ULONG target_disk_num, ULONGLONG disk_size, ULONG sector_size, ULONG disk_type)
{
	m_disk_handle = disk_handle;
	m_disk_num = disk_num;
	m_disk_size = disk_size;
	m_sector_size = sector_size;
	m_disk_type = disk_type;
	m_target_disk_handle = target_disk_handle;
	m_target_disk_num = target_disk_num;

	m_record_buffer = NULL;
	m_read_buffer = NULL;

	m_read_size = 2 * 1024 * 1024;
}

Disk_Backup::~Disk_Backup()
{
	if (m_record_buffer)
	{
		free(m_record_buffer);
		m_record_buffer = NULL;
	}

	if (m_read_buffer)
	{
		free(m_read_buffer);
		m_read_buffer = NULL;
	}
}

DWORD Disk_Backup::BackupDisk()
{
	DWORD dwRet = 0;

	m_read_buffer = (BYTE*)malloc(m_read_size);
	if (m_read_buffer == NULL)
	{
		dwRet = -1;
		IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for read buffer");
		return dwRet;
	}

	if (m_disk_type == DISK_TYPE_MBR_BASIC || m_disk_type == DISK_TYPE_MBR_DYNAMIC)
	{
		dwRet = BackupMbrDisk();
	}
	else
	{
		dwRet = BackupGptDisk();
	}

	return dwRet;
}


DWORD Disk_Backup::BackupMbrDisk()
{
	DWORD dwRet = 0;
	do
	{
		m_record_buffer = (BYTE*)malloc(m_sector_size);
		if (m_record_buffer == NULL)
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to alloc memory for mbr buffer");
			break;
		}

		dwRet = BackupBootSector();
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to backup boot sector");
			break;
		}

		dwRet = BackupBootPartitionMbr();
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to backup boot partition mbr");
			break;
		}

		dwRet = BackupExtPartition();
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to backup ext partition");
			break;
		}

		if (m_disk_type == DISK_TYPE_MBR_DYNAMIC)
		{
			dwRet = BackupLdmSection();
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to backup LDM section");
				break;
			}
		}
	} while (0);

	return dwRet;
}

DWORD Disk_Backup::BackupGptDisk()
{
	DWORD dwRet = 0;
	do
	{
		dwRet = BackupGptTable();
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to backup boot sector");
			break;
		}

		dwRet = BackupBootPartitionGpt();
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to backup boot partition gpt");
			break;
		}

		if (m_disk_type == DISK_TYPE_GPT_DYNAMIC)
		{
			dwRet = BackupLdmPartition();
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to backup LDM partition");
				break;
			}
		}
	} while (0);

	return dwRet;
}

DWORD Disk_Backup::BackupBootSector()
{
	DWORD dwRet = 0;

	do
	{
		dwRet = SetFilePointer(m_disk_handle, 0, NULL, FILE_BEGIN);
		if (dwRet == INVALID_SET_FILE_POINTER)
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to set pointer to beginning, disk %d", m_disk_num);
			break;
		}

		DWORD dwRead = 0;
		if (!ReadFile(m_disk_handle, m_read_buffer, m_sector_size, &dwRead, NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to read disk, disk %d", m_disk_num);
			break;
		}

		dwRet = SetFilePointer(m_target_disk_handle, 0, NULL, FILE_BEGIN);
		if (dwRet == INVALID_SET_FILE_POINTER)
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to set pointer to beginning, write disk %d", m_target_disk_num);
			break;
		}

		DWORD dwWritten = 0;
		if (!WriteFile(m_target_disk_handle, m_read_buffer, m_sector_size, &dwWritten, NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to write disk, disk %d", m_target_disk_num);
			break;
		}

		memcpy(m_record_buffer, m_read_buffer, m_sector_size);
	} while (0);

	return dwRet;
}

DWORD Disk_Backup::BackupExtPartition()
{
	return 0;
}

DWORD Disk_Backup::BackupLdmSection()
{
	return 0;
}

DWORD Disk_Backup::BackupBootPartitionMbr()
{
	return 0;
}

DWORD Disk_Backup::BackupGptTable()
{
	return 0;
}

DWORD Disk_Backup::BackupBootPartitionGpt()
{
	return 0;
}

DWORD Disk_Backup::BackupLdmPartition()
{
	return 0;
}