#ifndef _REGEDIT_H
#define _REGEDIT_H

#include "reg_basic.h"

#define PATH(n) L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\BK\\" n


NTSTATUS RegInitProtect(RegInfo *reg_info);
NTSTATUS RegWriteProtect(RegInfo *reg_info);
NTSTATUS RegReadProtect(RegInfo *reg_info);

NTSTATUS RegReadVol(PWCH path, VolInfo *vol_info);
NTSTATUS RegWriteVol(VolInfo *vol_info);

NTSTATUS RegWriteSnapId(char* snap_id);
#endif