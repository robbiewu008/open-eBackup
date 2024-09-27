#ifndef _GROUP_MEM_H
#define _GROUP_MEM_H 

#include "driver.h"
#include "section.h"
#include "regedit.h"
#include "Ntstrsafe.h"



#define MAX_GROUP_NUM (32)

enum im_pg_state
{
	IM_PG_STATE_STOP = 0,
	IM_PG_STATE_CBT,
	IM_PG_STATE_SNAPSHOT,
	IM_PG_STATE_ERROR = 1000,
};

typedef struct tagQueueInfo QueueInfo;


typedef struct tagProtectStrategyInter
{
	uint32_t mem_threshold;
	uint32_t protect_size;
}ProtectStrategyInter;

typedef struct tagGroupInfo
{
	ProtectStrategyInter protect_strategy;
	QueueInfo* dev_ext_queue;
	QueueInfo *vol_queue;
	QueueInfo *snap_vol_queue;
	QueueInfo *delay_del_vol_queue;
	NPAGED_LOOKASIDE_LIST queue_info_npage_list;

	volatile LONG state;
	volatile LONG per_data_state;
	volatile ULONG reg_select_cur;

	volatile PDEVICE_OBJECT device_obj;
	volatile uint8_t bitmap_granularity;

	char	 snap_id[SNAP_ID_LEN];

	ERESOURCE	bitmap_lock;

	KGUARDED_MUTEX group_lock;
}GroupInfo;

typedef struct tagQueueInfo 
{
	LIST_ENTRY head;
	KSPIN_LOCK lock;
	ERESOURCE sync_resource;
	volatile uint32_t num;
	volatile uint32_t size;
	GroupInfo *group_info;
}QueueInfo;

typedef struct tagGroupMemInfo
{
	GroupInfo group_info[MAX_GROUP_NUM];
}GroupMemInfo;



VOID ImSleep(int time);
LARGE_INTEGER GetCurTime();

VolInfo *NeedProtected(PDEVICE_EXTENSION pdx);

QueueInfo *InitQueueInfo(GroupInfo *group_info);
uint32_t GetQueueNum(QueueInfo *queue_info);

VOID FreeVolQueue(QueueInfo *queue_info, GroupInfo *group_info);
VOID PushVolQueue(QueueInfo *queue_info, VolInfo *vol_info);
BOOL RemoveVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info);
BOOL RemoveVolInfoByPdx(QueueInfo *vol_queue, PDEVICE_EXTENSION pdx);
VOID ClearVolQueue(QueueInfo *queue_info);
VOID CheckRemoveDelayDeleteVolQueue(GroupInfo *group_info);
BOOL IsEmptyVolQueue(QueueInfo *queue_info);
BOOLEAN FindVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info);
VolInfo* GetVolInfoByPdx(QueueInfo *vol_queue, PDEVICE_EXTENSION pdx);
NTSTATUS StartProtectByPhysicalVol(GroupInfo *group_info, PDEVICE_EXTENSION pdx);


VOID PushDeviceExtQueue(QueueInfo *dev_ext_queue, PDeviceExtInfo dev_ext_info);
VOID ClearDeviceExtQueue(QueueInfo *dev_ext_queue);
BOOL RemoveDeviceExtByPdx(QueueInfo *dev_ext_queue, PDEVICE_EXTENSION pdx);
PDEVICE_EXTENSION FindDeviceExtensionByVolName(QueueInfo *dev_ext_queue, PUNICODE_STRING vol_name);
PDEVICE_EXTENSION FindDeviceExtensionByVolDev(QueueInfo *dev_ext_queue, PDEVICE_OBJECT dev_obj);

VolInfo *BuildVolInfo(GroupInfo* group_info, PDEVICE_EXTENSION pdx, VolInfo* phy_vol);
VolInfo *BuildSimpleVolInfo(GroupInfo* group_info, PDEVICE_EXTENSION pdx);
VOID FreeVolInfo(VolInfo *vol_info);

NTSTATUS InitGroupInfo(GroupInfo *group_info, PDEVICE_OBJECT device_obj);
VOID DestroyGroupInfo(GroupInfo *group_info);
GroupInfo* GetGroupInfo(PDEVICE_EXTENSION pdx);

NTSTATUS CreateGroupMem(PDEVICE_EXTENSION pdx, PDEVICE_OBJECT device_obj);
VOID ReleaseGroupMem(PDEVICE_EXTENSION pdx, PDEVICE_OBJECT device_obj);

NTSTATUS BitmapSunset(GroupInfo *group_info, char* snap_id);
NTSTATUS BitmapMidnight(GroupInfo *group_info);
NTSTATUS BitmapSunrise(GroupInfo *group_info, char* snap_id, BOOLEAN failed);
NTSTATUS GetYesterdayBitmap(GroupInfo *group_info, VolInfo* vol_info, BOOLEAN need_snap, unsigned char* data, uint32_t size);

BOOLEAN ShutdownBitmapPrepare(GroupInfo *group_info);

VOID ProcessStop(GroupInfo *group_info);
VOID SwitchState(GroupInfo *group_info, LONG state);
BOOLEAN IsInStopState(GroupInfo *group_info);
BOOLEAN IsInSnapshotState(GroupInfo *group_info);

VOID SetSnapId(GroupInfo *group_info, char* snap_id);
VOID ResetSnapId(GroupInfo *group_info);
BOOLEAN CompareSnapId(GroupInfo *group_info, char* snap_id);

VOID MergePersistBitmap(GroupInfo *group_info);
VOID WriteSetBitmap(GroupInfo* group_info, VolInfo *vol_info, uint64_t sector_offset, uint64_t sector_length);

#endif