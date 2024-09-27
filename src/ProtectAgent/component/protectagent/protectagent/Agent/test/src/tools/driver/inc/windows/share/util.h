#pragma once
#include "driver.h"
#include "event_log.h"


#define PAGEDCODE code_seg("PAGE")

#define LOCKEDCODE code_seg()

#define INITCODE code_seg("INIT")

#define PAGEDDATA data_seg("PAGEDATA")

#define LOCKEDDATA data_seg()

#define INITDATA data_seg("INITDATA")

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))





BOOLEAN AllocUnicodeString(IN PUNICODE_STRING str, IN USHORT size, IN POOL_TYPE type);
VOID FreeUnicodeString(IN PUNICODE_STRING str);
BOOLEAN IsUnicodeStringEmpty(IN PUNICODE_STRING str);
VOID GetParentPath(IN PUNICODE_STRING str);

_Acquires_exclusive_lock_(res)
VOID AcquireExclusiveResource(PERESOURCE res);

_Acquires_shared_lock_(res)
VOID AcquireShareResource(PERESOURCE res);

_Releases_lock_(res)
 VOID ReleaseResource(PERESOURCE res);


 uint32_t make_log_lower_bound(uint64_t value);
 uint32_t make_log_upper_bound(uint64_t value);
 bool is_int_power(uint64_t value);

 VOID LogEvent(IN PDRIVER_OBJECT drv_obj, IN NTSTATUS error_code, IN NTSTATUS final_status, IN PUNICODE_STRING param1, IN PUNICODE_STRING param2, IN PVOID dump_data, IN USHORT dump_size);