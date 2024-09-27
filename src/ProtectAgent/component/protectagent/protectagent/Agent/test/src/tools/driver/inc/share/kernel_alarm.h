#ifndef _OM_KERNEL_H_
#define _OM_KERNEL_H_

#ifdef _MSC_VER
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;
#endif

// 内核态错误码定义范围 0x40032C00 - 0x40032CFF
#define ERROR_CODE_BITMAP_NOT_SET                 0x40032C00
#define ERROR_CODE_BITMAP_RECV_FAILED_ACK         0x40032C01

// 用户态和内核态（Linux和windows）复用该信息
#define MAX_ALARM_NUM_ONCE_REPORT   10  
#define KERNEL_ALARM_INFO_DESC_LEN  256   

#ifdef LINUX_KERNEL
#define LIMIT_ALARM_REPORT_FREQUENCY  (60 * HZ) // Linux告警信息限制频率 1 分钟
#else
#define LIMIT_ALARM_REPORT_FREQUENCY  (60 * HZ)  // windows告警信息限制频率
#endif

#pragma pack(push)
#pragma pack(1)

typedef struct _ALARM_ITEM
{
    uint64_t error_code;                    // 告警的错误码
    struct _ALARM_ITEM *next;        
    char info[KERNEL_ALARM_INFO_DESC_LEN];  // 告警描述信息
}ALARM_ITEM, *PALARM_ITEM;

// 内核告警信息上传到用户态后，如果用户态需要保存在
typedef struct _ALARM_LSIT
{
    uint32_t alarm_total;              // 当前内核中存在多少个未上报的告警信息
    PALARM_ITEM p_first_alarm_item;    // first alarm
    PALARM_ITEM p_last_alarm_item;     // last alarm
}ALARM_LIST, *PALARM_LIST;   

#pragma pack(pop)

#endif

