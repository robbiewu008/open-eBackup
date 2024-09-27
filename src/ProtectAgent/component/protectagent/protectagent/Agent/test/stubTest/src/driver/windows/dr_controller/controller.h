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
#include "control.h"




typedef struct _ADD_DEL_PARTITION_PARAM
{
	ULONG ulDiskNumber;
	char cPartitionId[VOL_ID_LEN];
	ULONGLONG ullPartitionSize;
}ADD_DEL_PARTITION_PARAM, *PADD_DEL_PARTITION_PARAM;



enum StartType { NORMAL_START, VERIFY_START, MODIFY_START };
typedef struct _START_MIRROR_PARAM
{
	StartType sType;
	char szOsId[VM_ID_LEN];
	char szOmaId[VM_ID_LEN];
	ULONG ulIp;
	ULONG ulPort;
	ULONG exp_rpo;
	ULONG ulMaxSpeed;
	ULONG ulDblgLevel;
	BOOL bDelayAdd;
}START_MIRROR_PARAM, *PSTART_MIRROR_PARAM;

typedef struct _INSTALL_PARAM
{
	CString strInfPath;
	BOOL bForce;
}INSTALL_PARAM, *PINSTALL_PARAM;





class CDrController
{
public:
	CDrController();
	~CDrController();

public:
	DWORD StartIoMirror(const START_MIRROR_PARAM& stParam);
	DWORD AddDelPartition(const ADD_DEL_PARTITION_PARAM& adParam, BOOL bAdd);
	DWORD StopIoMirror();
	DWORD PauseIoMirror();
	DWORD ResumeIoMirror();
	BOOL IsIoMirrorRunning();

private:
	HANDLE GetDeviceHandle();
	DWORD SendIOControl(DWORD ctl, IoctlMessage *message);

	HANDLE GetDiskHandle(ULONG ulDiskNumber);
	DWORD GetProtectPartition(HANDLE hDisk, ULONGLONG partition_size, IoctlMessage *message);

private:
	HANDLE m_hDevice;
	map<ULONG, HANDLE> m_mpDisk;
};