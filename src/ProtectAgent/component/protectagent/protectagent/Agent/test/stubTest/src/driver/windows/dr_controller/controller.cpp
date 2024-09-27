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
#include "util.h"



CDrController::CDrController()
{
	m_hDevice = INVALID_HANDLE_VALUE;
}

CDrController::~CDrController()
{
	if (m_hDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}

	map<ULONG, HANDLE>::iterator it = m_mpDisk.begin();
	while (it != m_mpDisk.end())
	{
		if (it->second != INVALID_HANDLE_VALUE)
		{
			CloseHandle(it->second);
			it->second = INVALID_HANDLE_VALUE;
		}

		it++;
	}

	m_mpDisk.clear();
}

DWORD CDrController::StartIoMirror(const START_MIRROR_PARAM& stParam)
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
		pMessage->u.protect_strategy.oma_port = stParam.ulPort;
		pMessage->u.protect_strategy.oma_ip[0] = stParam.ulIp;
		pMessage->u.protect_strategy.exp_rpo = stParam.exp_rpo;
		pMessage->u.protect_strategy.mem_threshold = 0;
		pMessage->u.protect_strategy.protect_size = 0;
		//pMessage->u.protect_strategy.debug_level = stParam.ulDblgLevel;
		memcpy(pMessage->u.protect_strategy.vm_id, stParam.szOsId, VM_ID_LEN);
		memcpy(pMessage->u.protect_strategy.oma_id, stParam.szOmaId, VM_ID_LEN);

		if (stParam.sType == NORMAL_START)
		{
			dwRet = SendIOControl(IOCTL_IOMIRROR_START, pMessage);
		}
		else if(stParam.sType == VERIFY_START)
		{
			dwRet = SendIOControl(IOCTL_IOMIRROR_START_WITH_VERIFY, pMessage);
		}
		else
		{
			dwRet = SendIOControl(IOCTL_IOMIRROR_MODIFY, pMessage);
		}

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

DWORD CDrController::AddDelPartition(const ADD_DEL_PARTITION_PARAM& adParam, BOOL bAdd)
{
	DWORD dwRet = 0;
	IoctlMessage *pMessage = NULL;

	do
	{
		HANDLE hDisk = GetDiskHandle(adParam.ulDiskNumber);
		if (hDisk == INVALID_HANDLE_VALUE)
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to open disk %d", adParam.ulDiskNumber);

			break;
		}

		pMessage = (IoctlMessage *)malloc(sizeof(IoctlMessage));
		if (NULL == pMessage)
		{
			IOMIRROR_ERROR(0, L"Failed to malloc message");
			break;
		}

		memset(pMessage, 0, sizeof(IoctlMessage));
		pMessage->u.protect_vol.disk_num = adParam.ulDiskNumber;
		memcpy(pMessage->u.protect_vol.vol_id, adParam.cPartitionId, VOL_ID_LEN);

		//dwRet = GetProtectPartition(hDisk, adParam.ullPartitionSize, pMessage);
		//if (dwRet != 0)
		//{
		//	IOMIRROR_ERROR(0, L"Failed to get protect partition");
		//	break;
		//}

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

DWORD CDrController::StopIoMirror()
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

		dwRet = RegDeleteKeyRecursive(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\DR");
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to delete DR reg key");
			break;
		}
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return dwRet;
}

DWORD CDrController::PauseIoMirror()
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

		dwRet = SendIOControl(IOCTL_IOMIRROR_PAUSE, pMessage);
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

DWORD CDrController::ResumeIoMirror()
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

		dwRet = SendIOControl(IOCTL_IOMIRROR_RESUME, pMessage);
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

BOOL CDrController::IsIoMirrorRunning()
{
	BOOL bRet = FALSE;
	DWORD dwError = 0;
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

		dwError = SendIOControl(IOCTL_IOMIRROR_QUERY_START, pMessage);
		if (dwError != 0)
		{
			IOMIRROR_ERROR(dwError, L"Failed to send Io control");
			break;
		}

		bRet = pMessage->u.query_start.start;
	} while (0);

	if (pMessage)
	{
		free(pMessage);
	}

	return bRet;
}

HANDLE CDrController::GetDeviceHandle()
{
	HANDLE hRet = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	if (m_hDevice == INVALID_HANDLE_VALUE)
	{
		hRet = CreateFile(IOMIRROR_DEVICE_LINK_NAME_U, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

DWORD CDrController::GetProtectPartition(HANDLE hDisk, ULONGLONG partition_size, IoctlMessage *message)
{
	DWORD dwRet = 0;
	do
	{
		DWORD dwOutput;
		GET_LENGTH_INFORMATION length_info;
		if (!DeviceIoControl(hDisk, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &length_info, sizeof(GET_LENGTH_INFORMATION), &dwOutput, NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"send IOCTL_DISK_GET_LENGTH_INFO failed");
			break;
		}

		//message->u.protect_vol.start_pos = 0;
		//if (partition_size < length_info.Length.QuadPart)
		//{
		//	IOMIRROR_INFO(L"vol len = %d, protect len = %d", message->u.protect_vol.end_pos, partition_size);
		//	message->u.protect_vol.end_pos = partition_size;
		//}
		//else
		//{
		//	message->u.protect_vol.end_pos = length_info.Length.QuadPart;
		//}

		//IOMIRROR_INFO(L"start = %llu, end = %llu", message->u.protect_vol.start_pos, message->u.protect_vol.end_pos);
	} while (0);

	return dwRet;
}

HANDLE CDrController::GetDiskHandle(ULONG ulDiskNumber)
{
	HANDLE hRet = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	map<ULONG, HANDLE>::iterator it = m_mpDisk.find(ulDiskNumber);
	if (it == m_mpDisk.end())
	{
		CString strDiskName;
		strDiskName.Format(L"\\\\.\\PhysicalDrive%d", ulDiskNumber);

		hRet = CreateFile(strDiskName.GetString(), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hRet == INVALID_HANDLE_VALUE)
		{
			dwError = GetLastError();
			IOMIRROR_ERROR(dwError, L"Failed to open disk %d", ulDiskNumber);
		}
		else
		{
			m_mpDisk.insert(make_pair(ulDiskNumber, hRet));
		}
	}
	else
	{
		hRet = it->second;
	}

	return hRet;
}

DWORD CDrController::SendIOControl(DWORD ctl, IoctlMessage *message)
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
	case IOCTL_IOMIRROR_START_WITH_VERIFY:
		lpBuffer = &message->u.protect_strategy;
		ulSize = sizeof(ProtectStrategy);
		break;
	case IOCTL_IOMIRROR_MODIFY:
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
	case IOCTL_IOMIRROR_NOTIFY_CHANGE:
		lpBuffer = &message->u.notify_change;
		ulSize = sizeof(NotifyChange);
		break;
	case IOCTL_IOMIRROR_QUERY_START:
		lpBuffer = &message->u.query_start;
		ulSize = sizeof(QueryStart);
		lpOutputBuffer = &message->u.query_start;
		ulOutputSize = sizeof(QueryStart);
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
