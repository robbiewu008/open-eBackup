
#ifndef _WRITEAGENT_CONST_H_
#define _WRITEAGENT_CONST_H_

#include "ctl_define.h"




typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;




#define ALLOC_TAG 'bfDD'

#define NOPAGED_LS_QU_ALLOC_TAG 'QUdr'

#define BOOL BOOLEAN 

#define PROTECT_GROUP_MEMORY_NAME L"\\Device\\HW_DP_PROTECT_GROUPS_BK"


#define QUEUE_MAX_SIZE				(64<<20) //64M

#define VOL_NAME_LENGTH			(512)

#define SECOND_TO_NANOSECOND			(-1000*10000)

#define IOCTL_BUFFER_SIZE           (512)

#define SECTOR_SIZE              (512)
#define MAX_PROCESS_NUM				(1)
#define DEFAULT_BITMAP_GRANULARITY       (10)
#define MAX_BITMAP_COUNT			(2)

#define PERSIST_DATA_FILE			L"BKDataMap.bin"

#define MAX_ULONGLONG		((ULONGLONG)-1)

#define VOLUME_FS_TYPE_OFFSET							3
#define VOLUME_NTFS_SERIAL_NUM_OFFSET		0x48
#define VOLUME_REFS_SERIAL_NUM_OFFSET		0x38

#define VOLUME_FS_TYPE_NTFS								"NTFS"
#define VOLUME_FS_TYPE_REFS								"ReFS"

#endif

