#ifndef _IOCTL_H
#define _IOCTL_H

#include "driver.h"
#include "group_mem.h"

NTSTATUS IoctlVolAdd(GroupInfo *group_info, ProtectVol *vol_outer);
NTSTATUS IoctlVolDel(GroupInfo *group_info, ProtectVol *vol_outer);
NTSTATUS IoctlStartProtection(GroupInfo *group_info, ProtectStrategy *protect_strategy);
NTSTATUS IoctlStopProtection(GroupInfo *group_info);
NTSTATUS IoctlStartSnapshot(GroupInfo *group_info, TakeSnapshot* take_snapshot);
NTSTATUS IoctlFinishSnapshot(GroupInfo *group_info);
NTSTATUS IoctlGetBitmap(GroupInfo *group_info, GetBitmap* get_bitmap);
NTSTATUS IoctlRemoveSnapshot(GroupInfo *group_info, RemoveSnapshot* get_bitmap);

NTSTATUS StartProtectByReg(PDEVICE_EXTENSION pdx);

#endif

