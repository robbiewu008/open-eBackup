#ifndef _REGEDIT_H
#define _REGEDIT_H

#include "reg_basic.h"

#define PATH(n) L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\DR\\" n


NTSTATUS RegInitProtect(RegInfo *reg_info);
NTSTATUS RegWriteProtect(RegInfo *reg_info);
NTSTATUS RegReadProtect(RegInfo *reg_info);

NTSTATUS RegReadVol(PWCH path, VolInfo *vol_info);
NTSTATUS RegWriteVol(VolInfo *vol_info);

NTSTATUS RegWriteDatasetId(uint64_t dataset_id, uint64_t dataset_id_done);
#endif