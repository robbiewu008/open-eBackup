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

#include "controller.h"
#include "log.h"
#include "control.h"


CBkController::CBkController()
{
	m_hDevice = INVALID_HANDLE_VALUE;
}

CBkController::~CBkController()
{
	if (m_hDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}
}

DWORD CBkController::StartIoMirror(const START_MIRROR_PARAM& stParam)
{
	DWORD dwRet = 0;
	IoctlMessage *pMessage = NULL;
	do
	{
		pMessage = (IoctlMessage *)malloc(sizeof(IoctlMessage));
		if (NULL == pMessage)
		{
			IOMIRROR_ERROR(0, L"Failed to malloc message");
			break;
		}

		memset(pMessage, 0, sizeof(IoctlMessage));
		pMessage->u.protect_strategy.mem_threshold = 0;
		pMessage->u.protect_strategy.protect_size = 0;

		dwRet = SendIOControl(IOCTL_IOMIRROR_START, pMessage);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to send Io control");
			break;
		}
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return dwRet;
}

DWORD CBkController::AddDelPartition(const ADD_DEL_PARTITION_PARAM& adParam, BOOL bAdd)
{
	DWORD dwRet = 0;
	IoctlMessage *pMessage = NULL;

	do
	{
		pMessage = (IoctlMessage *)malloc(sizeof(IoctlMessage));
		if (NULL == pMessage)
		{
			IOMIRROR_ERROR(0, L"Failed to malloc message");
			break;
		}

		memset(pMessage, 0, sizeof(IoctlMessage));
		memcpy(pMessage->u.protect_vol.disk_path, adParam.disk_path, DISK_PATH_LEN);

		dwRet = SendIOControl((bAdd ? IOCTL_IOMIRROR_VOL_ADD : IOCTL_IOMIRROR_VOL_DELETE), pMessage);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to send Io control");
			break;
		}
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return dwRet;
}

DWORD CBkController::StopIoMirror()
{
	DWORD dwRet = 0;
	IoctlMessage *pMessage = NULL;

	do
	{
		pMessage = (IoctlMessage *)malloc(sizeof(IoctlMessage));
		if (NULL == pMessage)
		{
			IOMIRROR_ERROR(0, L"Failed to malloc message");
			break;
		}

		memset(pMessage, 0, sizeof(IoctlMessage));

		dwRet = SendIOControl(IOCTL_IOMIRROR_STOP, pMessage);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to send Io control");
			break;
		}
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return dwRet;
}

// {8C4906C3-C768-4B24-959C-BE6EFD3A343D}
static const GUID  snapshot_id = { 0x8c4906c3, 0xc768, 0x4b24,{ 0x95, 0x9c,  0xbe,  0x6e,  0xfd,  0x3a,  0x34,  0x3d } };

DWORD CBkController::StartSnapshot(unsigned char* granularity)
{
	DWORD dwRet = 0;
	IoctlMessage *pMessage = NULL;
	*granularity = 0;

	do
	{
		pMessage = (IoctlMessage *)malloc(sizeof(IoctlMessage));
		if (NULL == pMessage)
		{
			IOMIRROR_ERROR(0, L"Failed to malloc message");
			break;
		}

		memset(pMessage, 0, sizeof(IoctlMessage));
		memcpy(pMessage->u.take_snapshot.snap_id, &snapshot_id, SNAP_ID_LEN);

		dwRet = SendIOControl(IOCTL_BK_TAKE_SNAPSHOT, pMessage);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to send Io control");
			break;
		}

		*granularity = pMessage->u.take_snapshot.bitmap_granularity;
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return dwRet;
}

DWORD CBkController::FinishSnapshot()
{
	DWORD dwRet = 0;
	IoctlMessage *pMessage = NULL;

	do
	{
		pMessage = (IoctlMessage *)malloc(sizeof(IoctlMessage));
		if (NULL == pMessage)
		{
			IOMIRROR_ERROR(0, L"Failed to malloc message");
			break;
		}

		memset(pMessage, 0, sizeof(IoctlMessage));

		dwRet = SendIOControl(IOCTL_BK_FINISH_SNAPSHOT, pMessage);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to send Io control");
			break;
		}
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return dwRet;
}

DWORD CBkController::RemSnapshot(BOOL is_failed)
{
	DWORD dwRet = 0;
	IoctlMessage *pMessage = NULL;

	do
	{
		pMessage = (IoctlMessage *)malloc(sizeof(IoctlMessage));
		if (NULL == pMessage)
		{
			IOMIRROR_ERROR(0, L"Failed to malloc message");
			break;
		}

		memset(pMessage, 0, sizeof(IoctlMessage));
		memcpy(pMessage->u.rm_snapshot.snap_id, &snapshot_id, SNAP_ID_LEN);
		pMessage->u.rm_snapshot.is_failed = is_failed;

		dwRet = SendIOControl(IOCTL_BK_REMOVE_SNAPSHOT, pMessage);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to send Io control");
			break;
		}
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return dwRet;
}

DWORD CBkController::GetBkBitmap(const GET_BITMAP_PARAM& param)
{
	DWORD dwRet = 0;
	IoctlMessage *pMessage = NULL;

	do
	{
		pMessage = (IoctlMessage *)malloc(sizeof(IoctlMessage));
		if (NULL == pMessage)
		{
			IOMIRROR_ERROR(0, L"Failed to malloc message");
			break;
		}

		memset(pMessage, 0, sizeof(IoctlMessage));
		memcpy(pMessage->u.get_bitmap.vol_path, &param.disk_path, DISK_PATH_LEN);
		pMessage->u.get_bitmap.data = param.buffer;
		pMessage->u.get_bitmap.bitmap_size = param.size;

		dwRet = SendIOControl(IOCTL_BK_GET_BITMAP, pMessage);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to send Io control");
			break;
		}
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return dwRet;
}


HANDLE CBkController::GetDeviceHandle()
{
	HANDLE hRet = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	if (m_hDevice == INVALID_HANDLE_VALUE)
	{
		hRet = CreateFile(IOTRACK_DEVICE_LINK_NAME_U, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hRet == INVALID_HANDLE_VALUE)
		{
			dwError = GetLastError();
			IOMIRROR_ERROR(dwError, L"Failed to open driver device");
		}
		else
		{
			m_hDevice = hRet;
		}
	}
	else
	{
		hRet = m_hDevice;
	}

	return hRet;
}

DWORD CBkController::SendIOControl(DWORD ctl, IoctlMessage *message)
{
	DWORD dwRet = 0;

	HANDLE hDevice = GetDeviceHandle();
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		dwRet = GetLastError();
		IOMIRROR_ERROR(dwRet, L"Failed to open disk %d", 0);

		return dwRet;
	}

	LPVOID lpBuffer = NULL;
	ULONG ulSize = 0;
	LPVOID lpOutputBuffer = NULL;
	ULONG ulOutputSize = 0;
	switch (ctl)
	{
	case IOCTL_IOMIRROR_START:
		lpBuffer = &message->u.protect_strategy;
		ulSize = sizeof(ProtectStrategy);
		break;
	case  IOCTL_IOMIRROR_VOL_ADD:
		lpBuffer = &message->u.protect_vol;
		ulSize = sizeof(ProtectVol);
		break;
	case IOCTL_IOMIRROR_VOL_DELETE:
		lpBuffer = &message->u.protect_vol;
		ulSize = sizeof(ProtectVol);
		break;
	case IOCTL_BK_TAKE_SNAPSHOT:
		lpBuffer = &message->u.take_snapshot;
		ulSize = sizeof(TakeSnapshot);
		lpOutputBuffer = &message->u.take_snapshot;
		ulOutputSize = sizeof(TakeSnapshot);
		break;
	case IOCTL_BK_FINISH_SNAPSHOT:
		lpBuffer = NULL;
		ulSize = 0;
		break;
	case IOCTL_BK_REMOVE_SNAPSHOT:
		lpBuffer = &message->u.rm_snapshot;
		ulSize = sizeof(RemoveSnapshot);
		break;
	case IOCTL_BK_GET_BITMAP:
		lpBuffer = &message->u.get_bitmap;
		ulSize = sizeof(GetBitmap);
		break;
	default:
		lpBuffer = NULL;
		ulSize = 0;
		break;
	}

	DWORD dwOutput;
	if(!DeviceIoControl(hDevice, ctl, lpBuffer, ulSize, lpOutputBuffer, ulOutputSize, &dwOutput, NULL))
	{
		dwRet = GetLastError();
		IOMIRROR_ERROR(dwRet, L"send message failed, dwOutput = %d", dwOutput);
	}

	return dwRet;
}
