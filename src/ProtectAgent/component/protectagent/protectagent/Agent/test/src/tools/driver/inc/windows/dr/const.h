
#ifndef _WRITEAGENT_CONST_H_
#define _WRITEAGENT_CONST_H_

#include "ctl_define.h"





#define VM_SUFFIX_LEN   (8)


#define ALLOC_TAG 'bfDD'

#define NOPAGED_LS_ND_ALLOC_TAG 'NDdr'
#define NOPAGED_LS_QU_ALLOC_TAG 'QUdr'
#define NOPAGED_LS_CC_ALLOC_TAG 'CCdr'

#define NOPAGED_LS_CD_ALLOC_TAG 'CDdr'
#define SOCKET_ALLOC_TAG					  'STdr'

#define BOOL BOOLEAN 

#define PROTECT_GROUP_MEMORY_NAME L"\\Device\\HW_DP_PROTECT_GROUPS"

#define HTONS(x)    (  \
	(((x)>>8)&0xff) | \
	(((x)&0xff)<<8)        \
	)

#define NTOHS(x)    HTONS(x)

#define HTONL(x)    ( \
	(((x)>>24)&0xff) | \
	(((x)&0xff)<<24) | \
	(((x)&0xff0000)>>8) | \
	(((x)&0xff00)<<8) \
	) 
#define NTOHL(x)    HTONL(x)

#define QUEUE_MAX_SIZE				(64<<20) //64M

#define DISK_NAME_LENGTH			(512)

#define CONNECT_TIMOUT				(-1000*10000)
#define SECOND_TO_NANOSECOND			(-1000*10000)

#define IM_SECTOR_SIZE              (512)
#define MAX_PROCESS_NUM				(1)
#define IM_BITMAP_GRANULARITY       (3)
#define IM_MAX_BITMAP_GRANULARITY		(11)
#define IM_MAX_BITMAP_COUNT			(2)

#define IM_MIN_RPO									(3)

#define IM_PG_HEARTBEAT_INTERVAL    (5*1000*10000)
#define IM_PG_ACTIVITY_INTERVAL		(5*1000*10000)


#define IOCTL_WAIT_TIME				(-10*1000*10000)


#define ERROR_PAUSE_TIME			(-3000*10000)
#define PAUSE_TIME					(-100*10000)
#define RECONNECT_TIME				(-500*10000)
#define CREATE_SOCKET_TIME			(-500*10000)

#define SEND_MIN_WAIT_TIME			(-50*10000)

#define SOCKET_WAIT_TIME			(-10*1000*10000)//10s
#define RETRY_TIME						(-300*10000)

#define HOOK_RELEASE_TIME			(-1*1000*10000)

#define REV_BUF_SIZE			    (2*1024*1024)



#define MAX_SEND_QUEUE_SIZE			(10)


#define PERSIST_DATA_FILE			L"DataMap.bin"

#define IM_DEFAULT_MAX_DATASET_SIZE							(1ULL << 30)
#define IM_SEG_SIZE_OPERATOR											(2)

#define MAX_ULONGLONG		((ULONGLONG)-1)

#define DISK_DEVICE_SL_NAME_FORMAT			L"\\Device\\Harddisk%d\\DR0"

#endif

