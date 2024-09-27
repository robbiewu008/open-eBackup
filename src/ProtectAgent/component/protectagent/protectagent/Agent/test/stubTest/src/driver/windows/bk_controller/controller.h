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
	char disk_path[DISK_PATH_LEN];
}ADD_DEL_PARTITION_PARAM, *PADD_DEL_PARTITION_PARAM;

typedef struct _START_MIRROR_PARAM
{
}START_MIRROR_PARAM, *PSTART_MIRROR_PARAM;

typedef struct _GET_BITMAP_PARAM
{
	char disk_path[DISK_PATH_LEN];
	unsigned char* buffer;
	uint32_t size;
}GET_BITMAP_PARAM, *PGET_BITMAP_PARAM;





class CBkController
{
public:
	CBkController();
	~CBkController();

public:
	DWORD StartIoMirror(const START_MIRROR_PARAM& stParam);
	DWORD AddDelPartition(const ADD_DEL_PARTITION_PARAM& adParam, BOOL bAdd);
	DWORD StopIoMirror();
	DWORD StartSnapshot(unsigned char* granularity);
	DWORD FinishSnapshot();
	DWORD RemSnapshot(BOOL is_failed);
	DWORD GetBkBitmap(const GET_BITMAP_PARAM& param);

private:
	HANDLE GetDeviceHandle();
	DWORD SendIOControl(DWORD ctl, IoctlMessage *message);

private:
	HANDLE m_hDevice;
};