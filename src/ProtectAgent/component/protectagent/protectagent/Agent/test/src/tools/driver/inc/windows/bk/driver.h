#ifndef _DRIVER_H
#define _DRIVER_H 

#include <ntifs.h>
#include <windef.h>
#include <ntddvol.h>

#include "const.h"
#include "om_bitmap.h"
#include "bitmap_alloc.h"
#include "util.h"



#pragma warning(disable : 4200)


typedef struct tagSectionInfo
{
	SIZE_T nSecSize;
	HANDLE hSection;
	PVOID ppkoCB;
	PVOID pSecKMapping;
}SectionInfo;

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT LowerDeviceObject;	
	PDEVICE_OBJECT pdo;
	IO_REMOVE_LOCK RemoveLock;
	SectionInfo protect_group_memory_info;
	PVOID		mem_info;
	KEVENT PagingPathCountEvent;
	LONG  PagingPathCount;
	ULONGLONG vol_size;
	ULONG vol_type;
	ULONG vol_flag;
	UNICODE_STRING vol_name;
	_DEVICE_EXTENSION* orig_pdx;
	ULONG vol_serial_num;
	UNICODE_STRING vol_unique_id;
	volatile BOOLEAN first_shutdown;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct tagDeviceExtInfo
{
	LIST_ENTRY list_entry;
	PDEVICE_EXTENSION pdx;
}DeviceExtInfo, *PDeviceExtInfo;

enum { VOLUME_TYPE_UNKNOWN = 0, VOLUME_TYPE_PHYSICAL, VOLUME_TYPE_SW_SNAPSHOT, VOLUME_TYPE_HW_SNAPSHOT};
enum {VOLUME_FLAG_NONE = 0, VOLUME_FLAG_CSV = 1};

enum { CBT_BITMAP_SET_SHORT_FALSE = 0, CBT_BITMAP_SET_SHORT_TRUE = 1 };
typedef struct tagCbtBitmap
{
	OM_BITMAP* bit_today;
	OM_BITMAP* bit_tomorrow;
	OM_BITMAP* bit_yesterday;
}CbtBitmap;

typedef struct tagVolInfo
{
	LIST_ENTRY list_entry;
	ULONGLONG sectors;
	ULONG vol_type;
	ULONG vol_flag;
	UNICODE_STRING vol_name;
	CbtBitmap cbt_bitmap;
	CbtBitmap cbt_bitmap_copy;
	PVOID persist_data;
	ULONG persist_len;
	ULONG persist_data_apply_state;
	PDEVICE_EXTENSION pdx;
	tagVolInfo* phy_vol;
	UNICODE_STRING vol_unique_id;
	KGUARDED_MUTEX bitmap_set_lock;
	volatile LONG reference_count;
}VolInfo;

typedef struct tagRegInfo
{
	uint32_t state;
	uint32_t vol_num;
	uint8_t granularity;
	char	 snap_id[SNAP_ID_LEN];
}RegInfo;

typedef struct tagPdxInfo
{
	LIST_ENTRY list_entry;
	PDEVICE_EXTENSION pdx;
}PdxInfo;

typedef struct tagVOLUME_ALLOC_INFO
{
	uint32_t sector_size;
	uint32_t cluster_size;
}VOLUME_ALLOC_INFO, *PVOLUME_ALLOC_INFO;

ULONG GetVolumeSerialNumber(ULONG disk_num, ULONGLONG vol_offset, PUNICODE_STRING vol_name);
NTSTATUS GetVolumeGuid(PDEVICE_EXTENSION pdx, GUID* vol_guid);
NTSTATUS GetVolumeBeginSectorOffset(PDEVICE_EXTENSION pdx, ULONG* disk_num, LONGLONG* offset);
#endif

