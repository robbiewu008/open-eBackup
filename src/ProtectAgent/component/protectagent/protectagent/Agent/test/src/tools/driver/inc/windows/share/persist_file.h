#pragma once
#include "group_mem.h"
#include "persist_data.h"




VOID InitializePersistFile(IN GroupInfo* group_info);
VOID UnintializePersistFile();

VOID FirstTimeShutdown(GroupInfo* group_info);
VOID SecondTimeShutdown(GroupInfo* group_info);


VOID ShutdownPrepare(GroupInfo *group_info);
BOOLEAN OpenPersistFile(GroupInfo* group_info);
NTSTATUS GetPersistBitmapByVol(GroupInfo* group_info, VolInfo *vol_info, UCHAR* bit_data, ULONG size, PULONGLONG bit_count);




typedef struct _FILE_DISK_EXTENT
{
	PDEVICE_EXTENSION target_device_ext;
	ULONGLONG disk_lcn;
	ULONGLONG vcn;
	ULONG length;
}FILE_DISK_EXTENT, *PFILE_DISK_EXTENT;

typedef struct _FILE_DISK_EXTENTS
{
	ULONG sector_size;
	ULONG ext_count;
	FILE_DISK_EXTENT extents[0];
}FILE_DISK_EXTENTS, *PFILE_DISK_EXTENTS;


enum { PERSIST_DATA_STATE_UNKNOWN = 0, PERSIST_DATA_STATE_SUCCEED, PERSIST_DATA_STATE_FAILED, PERSIST_DATA_STATE_DISCARD };
enum { VOL_PERSIST_DATA_APPLY_STATE_NOT_START = 1, VOL_PERSIST_DATA_APPLY_STATE_DONE };
