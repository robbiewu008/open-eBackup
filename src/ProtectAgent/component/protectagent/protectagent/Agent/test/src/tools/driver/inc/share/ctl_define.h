#pragma once
#include "../share/kernel_alarm.h"

#ifndef VM_ID_LEN
#define VM_ID_LEN      (16)
#endif

#ifndef TOKEN_ID_LEN
#define TOKEN_ID_LEN   (16)
#endif

#ifndef VOL_ID_LEN
#define VOL_ID_LEN     (16)
#endif

#define IP_MAX_LEN     (4)
#define DISK_PATH_LEN  (128)

#define SNAP_ID_LEN    (16)

///common
typedef struct tagProtectVol
{
    char vol_id[VOL_ID_LEN];
    char disk_path[DISK_PATH_LEN];
    unsigned int disk_num;
    char old_vol_id[VOL_ID_LEN];
} ProtectVol;

typedef struct tagProtectStrategy
{
    char vm_id[VM_ID_LEN];
    char token_id[TOKEN_ID_LEN];
    char oma_id[VM_ID_LEN];
    unsigned int oma_ip[IP_MAX_LEN];
    unsigned int oma_port;
    unsigned int exp_rpo;
    unsigned int mem_threshold;    // units of MB
    unsigned int protect_size;     // uints of GB
    unsigned int send_now;         // 0 : driver will in pause state
} ProtectStrategy;

enum { NOTIFY_CHANGE_TYPE_UNKNOWN = 0, NOTIFY_CHANGE_DISK_ADD, NOTIFY_CHANGE_DISK_DELETE };
typedef struct tagDiskAdd
{
    char disk_path[DISK_PATH_LEN];
    unsigned int disk_num;
} DiskAdd;

typedef struct tagDiskDelete
{
    char disk_path[DISK_PATH_LEN];
    unsigned int disk_num;
} DiskDelete;


///om
typedef struct tagNotifyChange
{
	unsigned int change_type;
	union
	{
		DiskAdd disk_add;
		DiskDelete disk_delete;
	}u;
} NotifyChange;



///backup
 typedef struct tagTakeSnapshot
{
	char             snap_id[SNAP_ID_LEN];
	unsigned char    bitmap_granularity;
	unsigned char    max_vol_minor;
	unsigned int     vol_devno[128];
} TakeSnapshot;

typedef struct tagRemoveSnapshot
{
	char            snap_id[SNAP_ID_LEN];
	unsigned char   is_failed;
} RemoveSnapshot;

typedef struct tagGetBitmap
{
	unsigned char       vol_dev_id;
	char                vol_path[DISK_PATH_LEN];
	char                vol_snap_path[DISK_PATH_LEN];
	unsigned int        bitmap_size;
	unsigned char       *data;
} GetBitmap;

typedef struct tagGetAlarm {
    int return_alarm_num;  
    ALARM_ITEM pData[MAX_ALARM_NUM_ONCE_REPORT];
} GetAlarm;

typedef struct tagSetTokenID {
    char token_id[TOKEN_ID_LEN];
    char isValid;  // true: valid
} SetTokenID;

enum tagDataState {
    DATA_STATE_CBT = 0,
    DATA_STATE_NORMAL
};

enum tagLinkState {
    LINK_STATE_BREAK = 0,
    LINK_STATE_NORMAL,
    LINK_STATE_BREAK_WITH_VALID_TOKEN_ID
};

enum tagWorkMode {
    WORK_MODE_RESYNC = 0,
    WORK_MODE_INITIALIZE,
    WORK_MODE_SYNCING
};

enum tagWorkState {
    WORK_STATE_PAUSE = 0,
    WORK_STATE_NO_BUFFER,
    WORK_STATE_NORMAL
};

typedef struct tagPairState {
    int data_state;
    int link_state;
    int work_mode;
    int work_state;
} PairState;

typedef struct tagStatisticsInfo {
    unsigned long long remain_sync_data;    // 待同步数据量（字节）
    unsigned long long synced_data;         // 已同步数据量（字节）
    unsigned int       synced_data_rate;    // 已同步进度
    unsigned int       expected_time;       // 预计完成时间（秒）
    unsigned int       write_iops;          // 写IOPS
    unsigned long long write_throughout;    // driver 写IO的吞吐量 （KB/s）
    unsigned long long data_send_speed;     // 数据传输速率（KB/s）
    unsigned int       driver_rpo_time;     // driver 记录的RPO时间（秒）
    PairState          driver_pair_state;   // driver维护的pair状态
} StatisticsInfo;

typedef struct tagPauseWaitFlushQueue {
	int waitFlushQueueFlag;
}WaitFlushQueue;

#ifdef _WIN32
#ifdef _KERNEL_MODE
#include <ntifs.h>
#else
#include <windows.h>
#endif

#define IOSPILTER_DEVICE_NAME          "\\\\.\\IoMirrorDev"
#define IOMIRROR_DEVICE_NAME						L"\\Device\\IoMirrorDev"
#define IOMIRROR_DEVICE_LINK_NAME			L"\\DosDevices\\IoMirrorDev"
#define IOMIRROR_DEVICE_LINK_NAME_U		L"\\\\.\\IoMirrorDev"

#define IOTRACK_DEVICE_NAME							L"\\Device\\IoTrackDev"
#define IOTRACK_DEVICE_LINK_NAME				L"\\DosDevices\\IoTrackDev"
#define IOTRACK_DEVICE_LINK_NAME_U			L"\\\\.\\IoTrackDev"


#define  IOCTL_IOMIRROR_START										 CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_START_WITH_VERIFY				 CTL_CODE( FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_MODIFY									 CTL_CODE( FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_STOP											 CTL_CODE( FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_VOL_ADD									 CTL_CODE( FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_VOL_DELETE							 CTL_CODE( FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_NOTIFY_CHANGE					 CTL_CODE( FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_PAUSE										CTL_CODE( FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_RESUME									CTL_CODE( FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IOCTL_IOMIRROR_VOL_MODIFY								CTL_CODE( FILE_DEVICE_UNKNOWN, 0x80A, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define  IM_CTL_GET_STATISTICS_INFO                             CTL_CODE( FILE_DEVICE_UNKNOWN, 0x80B, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_BK_TAKE_SNAPSHOT									CTL_CODE( FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_BK_FINISH_SNAPSHOT									CTL_CODE( FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_BK_REMOVE_SNAPSHOT								CTL_CODE( FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_BK_GET_BITMAP											CTL_CODE( FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS )
#else
#include <linux/ioctl.h>

#define IOSPILTER_DEVICE_NAME        "/dev/im_ctldev"
#define IM_CTL_MAGIC                 'k'

typedef struct tagProtectVolSize
{
    uint64_t bitmap_size;
    uint32_t disk_num;
}ProtectVolSize;

/*
#define OM_IOCTL_START														_IOWR(IM_CTL_MAGIC,  1, om_drv_protect_strategy_t)
#define OM_IOCTL_STOP														_IO(IM_CTL_MAGIC,    2)
#define OM_IOCTL_VERIFY              
#define OM_IOCTL_MODIFY              
#define OM_IOCTL_VOL_ADD												_IOW(IM_CTL_MAGIC,   8, om_drv_protect_vol_t)
#define OM_IOCTL_VOL_DELETE											_IOW(IM_CTL_MAGIC,   9, om_drv_protect_vol_t)
#define OM_IOCTL_NOTIFY_CHANGE
*/
#define IM_CTL_START_IOMIRROR     _IOWR(IM_CTL_MAGIC,  1, ProtectStrategy)
#define IM_CTL_STOP_IOMIRROR      _IO(IM_CTL_MAGIC,    2)
#define IM_CTL_MODIFY_IOMIRROR    _IOWR(IM_CTL_MAGIC,  3, ProtectStrategy)
#define IM_CTL_PAUSE_IOMIRROR     _IOW(IM_CTL_MAGIC,   4, WaitFlushQueue)
#define IM_CTL_RESUME_IOMIRROR    _IO(IM_CTL_MAGIC,    5)
#define IM_CTL_ADD_VOLUME         _IOW(IM_CTL_MAGIC,   7, ProtectVol)
#define IM_CTL_DEL_VOLUME         _IOW(IM_CTL_MAGIC,   8, ProtectVol)
#define IM_CTL_VOLUME_READY       _IOW(IM_CTL_MAGIC,   9, ProtectVol)
// 关机前关机服务设置persistent bitmap的磁盘位置，用于driver写persitent bitmap的数据到文件中
#define IM_CTL_SETBITMAPEXTENT    _IOW(IM_CTL_MAGIC,   10, struct im_ctl_bitmap_extent_setting)
#define IM_CTL_SETBITMAP          _IOWR(IM_CTL_MAGIC,  11, struct im_ctl_bitmap)
#define IM_CTL_START_IOMIRROR_WITH_VERIFY             _IOWR(IM_CTL_MAGIC,  12, ProtectStrategy)
#define IM_CTL_NOTIFY_CHANGE      _IOWR(IM_CTL_MAGIC,  13, struct im_ctl_notify_change)
#define IM_CTL_GET_VOLUMES_SIZE   _IOR(IM_CTL_MAGIC,  14, ProtectVolSize)

#define IM_CTL_START_IOMIRROR_FOR_BACKUP             _IOWR(IM_CTL_MAGIC,  20, ProtectStrategy)
#define IM_CTL_BACKUP_TAKESNAPSHOT     _IOWR(IM_CTL_MAGIC,  21, struct im_ctl_backup_take_snapshot)
#define IM_CTL_BACKUP_REMOVESNAPSHOT   _IOW(IM_CTL_MAGIC,  22, struct im_ctl_backup_remove_snapshot)
#define IM_CTL_BACKUP_SETFILTERBITMAP  _IOWR(IM_CTL_MAGIC,  23, struct im_ctl_bitmap)
#define IM_CTL_BACKUP_GETBITMAP        _IOWR(IM_CTL_MAGIC,  24, struct im_ctl_bitmap)

// 关机前关机服务设置配置文件的磁盘位置，用于driver写flushtime等数据
#define IM_CTL_SETCONFIG_EXTENT    _IOW(IM_CTL_MAGIC,   25, struct im_ctl_bitmap_extent_setting)
// 系统启动时agent读取内容，并通过IOCTL传递driver flushtime等配置数据
#define IM_CTL_SETCONFIG_CONTENT   _IOW(IM_CTL_MAGIC,   26, struct im_config_variable)
// 系统卷信息发生变化时，通过IOCTL传递信息
#define IM_CTL_MOD_VOLUME          _IOW(IM_CTL_MAGIC,   27, ProtectVol)
#define IM_CTL_STOP_SEND_DATA      _IO(IM_CTL_MAGIC,    28)      // To support clean failover
#define IM_CTL_GET_STATISTICS_INFO _IOR(IM_CTL_MAGIC,   29, StatisticsInfo) 
#define IM_CTL_GET_KERNEL_ALARM    _IOR(IM_CTL_MAGIC,   30, GetAlarm)
#define IM_CTL_SET_TOKEN_ID        _IOW(IM_CTL_MAGIC,   31, SetTokenID)
#endif
