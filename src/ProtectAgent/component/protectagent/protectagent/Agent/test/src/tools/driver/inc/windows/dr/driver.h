#ifndef _DRIVER_H
#define _DRIVER_H 

#include <ntifs.h>
#include <windef.h>
#include <ntdddisk.h>

#include "const.h"
#include "om_bitmap.h"
#include "bitmap_alloc.h"
#include "util.h"




typedef struct tagDISPATCH_HOOK
{
	PDRIVER_DISPATCH org_fun;
	PDRIVER_DISPATCH new_fun;
}DISPATCH_HOOK, *PDISPATCH_HOOK;

typedef struct tag_DRIVER_HOOK
{
	KSPIN_LOCK lock;
	PDRIVER_OBJECT hook_driver;
	DISPATCH_HOOK hook_fun[IRP_MJ_MAXIMUM_FUNCTION + 1];
}DRIVER_HOOK, *PDRIVER_HOOK;

typedef struct tagDRIVER_HOOK_ENTRY
{
	LIST_ENTRY list_entry;
	DRIVER_HOOK driver_hook;
}DRIVER_HOOK_ENTRY, *PDRIVER_HOOK_ENTRY;


// Device extension structure
typedef struct tagSectionInfo
{
	SIZE_T nSecSize;
	HANDLE hSection;
	PVOID ppkoCB;
	PVOID pSecKMapping;
}SectionInfo;

typedef struct tagDEVICE_EXTENSION
{
	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT LowerDeviceObject;
	PDEVICE_OBJECT pdo;
	PDEVICE_OBJECT hook_device;
	UNICODE_STRING disk_name;
	IO_REMOVE_LOCK RemoveLock;
	PVOID		mem_info;
	KEVENT PagingPathCountEvent;
	LONG  PagingPathCount;
	ULONG disk_number;
	ULONG sector_size;
	ULONGLONG disk_size;
	volatile BOOLEAN first_shutdown;
	BOOLEAN need_clean;
	volatile BOOLEAN pending_stop;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct tagDeviceExtInfo
{
	LIST_ENTRY list_entry;
	PDEVICE_EXTENSION pdx;
}DeviceExtInfo, *PDeviceExtInfo;

typedef struct tagVolInfo
{
	LIST_ENTRY list_entry;
	uint64_t start_pos;
	uint64_t end_pos;
	char vol_id[VOL_ID_LEN];
	PDEVICE_EXTENSION pdx;
	OM_BITMAP     *bitmap;
	OM_BITMAP_IT *hbi;
	OM_BITMAP     *bitmap_verify;
	OM_BITMAP_IT *hbi_verify;
	uint64_t    sectors;
	UNICODE_STRING disk_name;
	PVOID persist_data;
	ULONG persist_len;
	ULONG persist_data_apply_state;
	BOOLEAN entire_disk;
	uint64_t verify_countdown;
	volatile LONG reference_count;
}VolInfo;

typedef struct tagRegInfo
{
	uint32_t iomirror_state;
	uint32_t vrg_ip;
	uint32_t vrg_port;
	uint32_t vol_num;
	uint32_t rpo;
	uint8_t granularity;
	uint64_t dataset_id;
	uint64_t dataset_id_done;
	char os_id[VM_ID_LEN];
	char oma_id[VM_ID_LEN];
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


PDEVICE_EXTENSION CreateDeviceExtensionByDiskNumAndHook(IN ULONG disk_num);
VOID UnhookDriver(IN PDRIVER_HOOK drv_hook);

PDEVICE_OBJECT GetIoDevice(IN PDEVICE_EXTENSION pdx);

#endif

