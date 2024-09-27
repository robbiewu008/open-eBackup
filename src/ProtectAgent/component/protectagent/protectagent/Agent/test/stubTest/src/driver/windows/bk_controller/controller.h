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