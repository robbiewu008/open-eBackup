#ifndef _IOCTL_H
#define _IOCTL_H

#include "driver.h"
#include "group_mem.h"

NTSTATUS IomirrorVolAdd(GroupInfo *group_info, ProtectVol *vol_outer);
NTSTATUS IomirrorVolDel(GroupInfo *group_info, ProtectVol *vol_outer);
NTSTATUS IomirrorVolMod(GroupInfo *group_info, ProtectVol *vol_outer);
NTSTATUS IomirrorStart(GroupInfo *group_info, ProtectStrategy *protect_strategy);
NTSTATUS IomirrorStartWithVerify(GroupInfo *group_info, ProtectStrategy *protect_strategy);
NTSTATUS IomirrorModify(GroupInfo *group_info, ProtectStrategy *protect_strategy);
NTSTATUS IomirrorStop(GroupInfo *group_info);
NTSTATUS IomirrorNotifyChange(GroupInfo *group_info, NotifyChange* change);
NTSTATUS IomirrorPause(GroupInfo *group_info);
NTSTATUS IomirrorResume(GroupInfo *group_info);
NTSTATUS IomirrorQueryStart(GroupInfo *group_info, QueryStart* query_start);

NTSTATUS StartProtectByReg(PDEVICE_EXTENSION pdx);

#endif

