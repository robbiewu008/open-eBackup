#ifndef _GROUP_MEM_H
#define _GROUP_MEM_H 

#include "driver.h"
#include "socket.h"
#include "section.h"
#include "regedit.h"
#include "cmd.h"
#include "Ntstrsafe.h"

#define CMD_HEAD (sizeof(IOMirrorCmd))

#define MAX_GROUP_NUM (32)

#define MAX_BLOCK_SIZE (2*1024*1024)
#define NINTH_LEVEL_BLOCK_SIZE (1*1024*1024)
#define EIGHTH_LEVEL_BLOCK_SIZE (512*1024)
#define SEVENTH_LEVEL_BLOCK_SIZE (256*1024)
#define SIXTH_LEVEL_BLOCK_SIZE (128*1024)
#define FIFTH_LEVEL_BLOCK_SIZE (64*1024)
#define FOURTH_LEVEL_BLOCK_SIZE (32*1024)
#define THIRD_LEVEL_BLOCK_SIZE (16*1024)
#define SECOND_LEVEL_BLOCK_SIZE (8 * 1024)
#define FIRST_LEVEL_BLOCK_SIZE (4*1024)
#define GROUND_LEVEL_BLOCK_SIZE (0)
#define MAX_DPP_PRE_ALLOC		(128)
#define MAX_PAGED_ALLOC_SIZE (MAX_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define NINTH_PAGE_ALLOC_SIZE (NINTH_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define EIGHTH_PAGE_ALLOC_SIZE (EIGHTH_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define SEVENTH_PAGE_ALLOC_SIZE (SEVENTH_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define SIXTH_PAGE_ALLOC_SIZE (SIXTH_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define FIFTH_PAGE_ALLOC_SIZE (FIFTH_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define FOURTH_PAGE_ALLOC_SIZE (FOURTH_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define THIRD_PAGE_ALLOC_SIZE (THIRD_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define SECOND_PAGE_ALLOC_SIZE (SECOND_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define FIRST_PAGE_ALLOC_SIZE (FIRST_LEVEL_BLOCK_SIZE+sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)
#define GROUND_PAGE_ALLOC_SIZE (sizeof(IOMirrorCmd) + MAX_DPP_PRE_ALLOC)

#define NORMAL_SEND_BUFFER_SIZE			MAX_PAGED_ALLOC_SIZE

enum im_pg_state
{
	IM_PG_STATE_STOP = 0,
	IM_PG_STATE_NORMAL,
	IM_PG_STATE_CBT,
	IM_PG_STATE_VERIFY,
	IM_PG_STATE_ERROR = 1000,
};

enum im_pg_inter_state
{
	IM_PG_INTER_STATE_STOP = 0,
	
	IM_PG_INTER_STATE_NORMAL,


	IM_PG_INTER_STATE_CBT_START = 5,
	IM_PG_INTER_STATE_CBT_END,
	IM_PG_INTER_STATE_CBT_DATA,

	IM_PG_INTER_STATE_VERIFY_START = 15,
	IM_PG_INTER_STATE_VERIFY_END,
	IM_PG_INTER_STATE_VERIFY_DATA,

	IM_PG_INTER_STATE_CONNECT_STAGE0,  /* 建立socket连接 */
	IM_PG_INTER_STATE_CONNECT_STAGE1,  /* 发送CONNECT_VRG报文 */
	IM_PG_INTER_STATE_CONNECT_STAGE1_P,
	IM_PG_INTER_STATE_CONNECT_STAGE2,  /* 发送IOMIRROR_START报文 */
	IM_PG_INTER_STATE_CONNECT_STAGE2_P,/* 发送IOMIRROR_START报文 */
};

enum extern_ctl_cmd
{
	IM_PG_IOCTL_STATE_NORMAL,
	IM_PG_IOCTL_STATE_MODIFY, /* 断开重连 */
	IM_PG_IOCTL_STATE_STOP,
	IM_PG_IOCTL_STATE_VOL_ADD,
	IM_PG_IOCTL_STATE_VOL_DEL,
	IM_PG_IOCTL_STATE_START,
	IM_PG_IOCTL_STATE_PAUSE,
	IM_PG_IOCTL_STATE_ERROR,
	IM_PG_IOCTL_STATE_DISCONNECT,
    IM_PG_IOCTL_STATE_VOL_MOD,
};

typedef struct tagQueueInfo QueueInfo;

typedef struct tagSocketInfo
{
	// tdi.sys相关
	DR_ADDRESS address;
	// socket对象相关
	DR_ENDPOINT end_point;
	char *rev_buf;
	PMDL rev_head_mdl;
	PKEVENT rev_event;
	PIO_STATUS_BLOCK rev_io_status;
	uint32_t is_receiving;
	uint32_t flags;
	uint32_t send_size;
	uint32_t pause_send;
	LARGE_INTEGER last_record_size_time;
}SocketInfo;

typedef struct tagProtectStrategyInter
{
	char osId[VM_ID_LEN];	/* required element of type Array16Ofbyte */
	char oma_id[VM_ID_LEN];
	uint32_t vrgIp;	/* required element of type Array128Ofbyte */
	uint32_t vrgPort;	/* required element of type xsd:unsignedInt */
	uint32_t rpo;
	uint32_t mem_threshold;
	uint32_t protect_size;
}ProtectStrategyInter;

typedef struct tagProtectVolInter
{
	uint32_t disk_num;
	char vol_id[VOL_ID_LEN];
	uint32_t need_set_flag;
	BOOLEAN entire_disk;
}ProtectVolInter;

enum custom_point_type
{
	CUSTOM_POINT_TYPE_UNKNOWN = 20,
	CUSTOM_POINT_TYPE_PAUSE
};

typedef struct tagGroupInfo
{
	// 保护组信息
	char group_id[VM_SUFFIX_LEN];
	ProtectStrategyInter protect_strategy;

	// 线程相关，启动和结束
	volatile BOOL send_thread_run_flag;
	volatile HANDLE send_thread_handle;
	
	// 数据报文相关队列
	QueueInfo *cmd_queue;//过滤驱动和保护组线程交互的队列
	QueueInfo* pending_cmd_queue;
	QueueInfo* delay_del_cmd_queue;
	QueueInfo *temp_cmd_queue;//发送出去且没有收到相应的ACK的数据报文所存放的队列

	QueueInfo* dev_ext_queue;

	// 卷相关队列
	QueueInfo *vol_queue;//存放所要保护卷的队列
	QueueInfo *temp_vol_queue;//存放虚拟机重启并且网络模块启动后还未加载的卷
	QueueInfo *delay_del_vol_queue;

	QueueInfo* driver_hook_queue;

	// 内存池
	NPAGED_LOOKASIDE_LIST max_page_list;//2M
	NPAGED_LOOKASIDE_LIST ninth_level_page_list;//1M
	NPAGED_LOOKASIDE_LIST eighth_level_page_list;//512K
	NPAGED_LOOKASIDE_LIST seventh_level_page_list;//256K
	NPAGED_LOOKASIDE_LIST sixth_level_page_list;//128K
	NPAGED_LOOKASIDE_LIST fifth_level_page_list;//64K
	NPAGED_LOOKASIDE_LIST fourth_level_page_list;//32K
	NPAGED_LOOKASIDE_LIST third_level_page_list;//16K
	NPAGED_LOOKASIDE_LIST second_level_page_list;//8K
	NPAGED_LOOKASIDE_LIST first_level_page_list;//4K
	NPAGED_LOOKASIDE_LIST ground_level_page_list;
	NPAGED_LOOKASIDE_LIST node_npage_list;//sizeof(CmdNode)
	NPAGED_LOOKASIDE_LIST queue_info_npage_list;//sizeof(QueueInfo)
	NPAGED_LOOKASIDE_LIST completion_context_npage_list;

	// 内存状态相关
	volatile int state;//外部状态
	volatile int inter_state;//内部状态
	volatile int flow_control_pause_flag;
	int speed_pause_flag;
	volatile BOOLEAN pause_pending;
	volatile BOOLEAN resume_pending;

	volatile custom_point_type custom_point_type;

	uint32_t maxspeed;

	volatile uint8_t bitmap_granularity;
	volatile BOOLEAN boot_flag;//表示虚拟机刚重启过，区分（重启后TRUE）跟（创建保护策略和断连后FALSE)start_iomirror的不同

	// 时间相关，包括控制报文重发，心跳，verify_check
	uint32_t heart_num;//心跳没有回ACK的次数
	LARGE_INTEGER last_heart_time;

	// 定时器
	KTIMER heart_beat_timer;
	KTIMER activity_timer;

	// 外部命令
	volatile uint32_t extern_ctl_type;
	KEVENT extern_ctl_event;
	uint8_t *extern_ctl_buffer;
	volatile NTSTATUS extern_ctl_result;
	KSPIN_LOCK extern_ctl_lock;

	// 网络信息
	SocketInfo socket_info;
	PDEVICE_OBJECT device_obj;	

	// 异步发送使用
	volatile uint32_t send_queue_size;
	volatile LONG send_queue_depth;
	FAST_MUTEX send_queue_mutex;

	volatile ULONG reg_select_cur;
	volatile ULONG per_data_state;

	KGUARDED_MUTEX group_lock;
	uint64_t sequence_id;
	uint64_t dataset_id;
	uint64_t dataset_id_done;
	uint64_t set_data_size;
	volatile LONG64 queue_set_data_size;
	uint64_t max_set_data_size;
	uint64_t seg_size;
	uint32_t global_credit;
	KTIMER dataset_timer;

	uint8_t* normal_send_buffer;
}GroupInfo;

typedef struct tagQueueInfo 
{
	LIST_ENTRY head;
	KSPIN_LOCK lock;
	ERESOURCE sync_resource;
	volatile uint32_t num;
	volatile uint64_t size;
	GroupInfo *group_info;
}QueueInfo;

typedef struct tagGroupMemInfo
{
	GroupInfo group_info[MAX_GROUP_NUM];
}GroupMemInfo;

enum {IO_COMPLETION_STATUS_ON_GOING = 0, IO_COMPLETION_STATUS_SUCCEED, IO_COMPLETION_STATUS_FAILED};
typedef struct tagCmdNode
{
	LIST_ENTRY list_entry;
	GroupInfo *group_info;
	VolInfo *vol_info;
	IOMirrorCmd *cmd;
	volatile LONG io_status;
}CmdNode;

#define MAX_CMD_PER_COMPLETION			32
typedef struct tagIO_COMP_CONTEXT
{
	GroupInfo* group_info;
	ULONG node_count;
	CmdNode* cmd_node_list[MAX_CMD_PER_COMPLETION];
	PIO_COMPLETION_ROUTINE comp_routine;
	PVOID real_context;
	UCHAR comp_control;
	PDEVICE_EXTENSION pdx;
}IO_COMP_CONTEXT, *PIO_COMP_CONTEXT;


enum {WORK_MODE_UNKNOWN = 0, WORK_MODE_CBT, WORK_MODE_NORMAL, WORK_MODE_RESYNC};


VOID ImSleep(int time);
LARGE_INTEGER GetCurTime();

VolInfo *NeedProtected(PDEVICE_EXTENSION pdx, uint64_t start_pos);

BOOLEAN MergeBitMap(GroupInfo *group_info, VolInfo *vol_info, unsigned char* data, unsigned int size, uint64_t bit_count);

NTSTATUS WaitForExternCtl(uint32_t ioctl_type, uint8_t* ctl_buffer, GroupInfo *group_info);
NTSTATUS SendErrorExternCtl(GroupInfo *group_info);
VOID SetEventForExternCtl(GroupInfo *group_info, NTSTATUS status);

///Common queue
QueueInfo *InitQueueInfo(GroupInfo *group_info);
uint32_t GetQueueNum(QueueInfo *queue_info);

///Cmd queue
VOID FreeCmdQueue(QueueInfo *cmd_queue, GroupInfo *group_info);
VOID FreeCmdQueueNLock(QueueInfo *cmd_queue, GroupInfo *group_info);
QueueInfo* BuildCmdQueue(uint32_t length, uint32_t msg_len, GroupInfo *group_info, VolInfo *vol_info);
VOID PushCmdQueue(QueueInfo *queue_info, CmdNode *cmd_node);
VOID PushCmdQueueNLock(QueueInfo *queue_info, CmdNode *cmd_node);
CmdNode *PopCmdQueue(QueueInfo *queue_info);
CmdNode *PopCmdQueueBySize(QueueInfo *cmd_queue, uint32_t size);
CmdNode *PopCmdQueueNLock(QueueInfo *cmd_queue);
BOOL IsEmptyCmdQueue(QueueInfo *cmd_queue);
BOOL IsEmptyCmdQueueNLock(QueueInfo *cmd_queue);
uint64_t GetCmdQueueSize(QueueInfo *queue_info);
VOID ClearCmdQueue(QueueInfo *queue_info, GroupInfo *group_info);
VOID ClearCmdQueueNLock(QueueInfo *cmd_queue, GroupInfo *group_info);
VOID FlushCmdQueue(QueueInfo *src_queue, QueueInfo *dst_queue);
BOOLEAN RemovePendingCmd(GroupInfo* group_info, uint64_t dataset_id);
VOID CheckRemoveDelayDeleteQueue(GroupInfo *group_info);

///Vol queue
VOID FreeVolQueue(QueueInfo *queue_info, GroupInfo *group_info);
VolInfo *BuildVolInfo(PDEVICE_EXTENSION pdx, GroupInfo *group_info, ProtectVolInter *vol);
VOID PushVolQueue(QueueInfo *queue_info, VolInfo *vol_info);
BOOL RemoveVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info);
BOOL RemoveVolInfoByPdx(GroupInfo* group_info, PDEVICE_EXTENSION pdx);
VOID ClearVolQueue(QueueInfo *queue_info);
VOID CheckRemoveDelayDeleteVolQueue(GroupInfo *group_info);
BOOL IsEmptyVolQueue(QueueInfo *queue_info);
BOOL ModifyVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info);

VolInfo *FindVolInfoByVerifyMap(QueueInfo *vol_queue);
VolInfo *FindVolInfoByCbtMap(QueueInfo *vol_queue);
VOID GetAllVolBitmapSize(QueueInfo *vol_queue, uint64_t* cbt_size, uint64_t* verify_size);
BOOLEAN FindVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info);

BOOL InitAllVolVerifyMap(QueueInfo *vol_queue, uint8_t bitmap_granularity);
BOOL InitVolumeVerifyBitmap(VolInfo *vol_info, uint8_t bitmap_granularity);
VOID ClearAllVolVerifyMap(QueueInfo *vol_queue);

BOOL IsAllVolVerifyCease(QueueInfo *vol_queue);

///Dev ext queue
VOID PushDeviceExtQueue(QueueInfo *dev_ext_queue, PDeviceExtInfo dev_ext_info);
VOID ClearDeviceExtQueue(QueueInfo *dev_ext_queue);
VOID RemoveDeviceExtByPdx(QueueInfo *dev_ext_queue, PDEVICE_EXTENSION pdx);
PDEVICE_EXTENSION FindDeviceExtensionByDiskNum(QueueInfo *dev_ext_queue, uint32_t disk_num);
PDEVICE_EXTENSION FindDeviceExtensionByHookDevice(QueueInfo *dev_ext_queue, PDEVICE_OBJECT hook_dev);
VOID FreeDeviceExtension(PDEVICE_EXTENSION pdx);

VOID LockDriverHookQueue(QueueInfo* driver_hook_queue);
VOID ReleaseDriverHookQueue(QueueInfo* driver_hook_queue);
VOID PushDriverHookQueue(QueueInfo* driver_hook_queue, PDRIVER_HOOK_ENTRY hook_entry);
VOID ClearDriverHookQueue(QueueInfo *driver_hook_queue); 
VOID UndoDriverHookQueue(QueueInfo *driver_hook_queue);
PDRIVER_HOOK FindHookByDriverObjectNLock(QueueInfo *driver_hook_queue, PDRIVER_OBJECT drv_obj);
BOOL FindHookByDriverObject(QueueInfo *driver_hook_queue, PDRIVER_OBJECT drv_obj);

VOID FreeVolInfo(VolInfo *vol_info);

VOID* MallocCmdNode(uint32_t length, uint32_t msg_len, GroupInfo *group_info, VolInfo *vol_info, LONG io_status);
VOID FreeCmdNode(CmdNode *cmd_node, GroupInfo *group_info);
VOID* MallocCmd(uint32_t length, uint32_t msg_len, GroupInfo *group_info);
VOID FreeCmd(IOMirrorCmd *cmd, GroupInfo *group_info);
VOID CheckFreeCmd(IOMirrorCmd *cmd, GroupInfo *group_info);

VOID FillDataCmd(IOMirrorCmd *cmd, char *data, uint32_t length, uint32_t alloc_len, uint64_t byte_offset, VolInfo *vol_info, GroupInfo *group_info);
VOID CopyDataCmdThin(IOMirrorCmd *dest_cmd, IOMirrorCmd *src_cmd);
VOID FillDatasetStartCmd(uint32_t work_mode, IOMirrorCmd *cmd, GroupInfo *group_info);
VOID FillResyncsetStartCmd(IOMirrorCmd *cmd, GroupInfo *group_info, VolInfo *vol_info, uint64_t seg_offset, uint64_t seg_size);
VOID FillHeartbeatCmd(IOMirrorCmd *cmd, GroupInfo *group_info);
VOID FillActivityCmd(IOMirrorCmd *cmd, GroupInfo *group_info, uint64_t cbt_backlog, uint64_t resync_remaining);
VOID FillSessionLoginCmd(IOMirrorCmd *cmd, GroupInfo *group_info);
VOID FillFlushCmd(IOMirrorCmd *cmd, GroupInfo *group_info);

// 共享内存************************************
NTSTATUS InitGroupInfo(GroupInfo *group_info, PDEVICE_OBJECT device_obj);
VOID DestroyGroupInfo(GroupInfo *group_info);
GroupInfo* GetGroupInfo(PDEVICE_EXTENSION pdx);

NTSTATUS CreateGroupMem(PDEVICE_EXTENSION pdx, PDEVICE_OBJECT device_obj, BOOLEAN create_new);
VOID ReleaseGroupMem(PDEVICE_EXTENSION pdx, PDEVICE_OBJECT device_obj);

VOID CmdHeaderByteSwap(DPP_HEADER* cmd_header);
VOID CmdBodyByteHostSwap(IOMirrorCmd* cmd);
VOID CmdByteNetworkSwap(IOMirrorCmd* cmd);
VOID CmdByteHostSwap(IOMirrorCmd* cmd);

#endif