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

#include "header.h"



enum {DISK_TYPE_UNKNOWN = 0, DISK_TYPE_MBR_BASIC, DISK_TYPE_MBR_DYNAMIC, DISK_TYPE_GPT_BASIC, DISK_TYPE_GPT_DYNAMIC};
class Disk_Backup
{
public:
	Disk_Backup(HANDLE disk_handle, HANDLE target_disk_handle, ULONG disk_num, ULONG target_disk_num, ULONGLONG disk_size, ULONG sector_size, ULONG disk_type);
	~Disk_Backup();

public:
	DWORD BackupDisk();

private:
	DWORD BackupMbrDisk();
	DWORD BackupGptDisk();
	DWORD BackupBootSector();
	DWORD BackupBootPartitionMbr();
	DWORD BackupExtPartition();
	DWORD BackupLdmSection();
	DWORD BackupGptTable();
	DWORD BackupBootPartitionGpt();
	DWORD BackupLdmPartition();

private:
	HANDLE m_disk_handle;
	HANDLE m_target_disk_handle;
	ULONG m_disk_num;
	ULONG m_target_disk_num;
	ULONGLONG m_disk_size;
	ULONG m_sector_size;
	ULONG m_disk_type;

	BYTE* m_record_buffer;
	BYTE* m_read_buffer;
	ULONG m_read_size;
};
