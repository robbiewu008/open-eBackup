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