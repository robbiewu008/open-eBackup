#pragma once
#include "driver.h"


NTSTATUS RegReadDwordWithDefault(PWCH path, PWCH name, uint32_t default_value, uint32_t *value);
NTSTATUS RegReadDword(PWCH path, PWCH name, uint32_t *value);
NTSTATUS RegReadDword(PWCH path, PWCH name, uint32_t *value);
NTSTATUS RegReadQword(PWCH path, PWCH name, uint64_t *value);
NTSTATUS RegReadString(PWCH path, PWCH name, PUNICODE_STRING value);
NTSTATUS RegWriteQword(PWCH path, PWCH name, uint64_t value);
NTSTATUS RegWriteDword(PWCH path, PWCH name, uint32_t value);
NTSTATUS RegWriteString(PWCH path, PWCH name, PWCH value);
NTSTATUS RegReadBinary(PWCH path, PWCH name, PVOID value, uint32_t length);
NTSTATUS RegWriteBinary(PWCH path, PWCH name, PVOID value, uint32_t length);
NTSTATUS RegDeleteKey(PWCH name);

VolInfo *RegMallocVolInfo(uint32_t num);
VOID RegFreeVolInfo(VolInfo *vol_info);
PWCH ConvertString(VolInfo *vol_info);
PWCH CreateSubKeyName(PWCH buffer);


NTSTATUS RegInit();
NTSTATUS RegCreateProtect(RegInfo *reg_info);
NTSTATUS RegModifyProtect(RegInfo *reg_info);
NTSTATUS RegDeleteProtect();

NTSTATUS RegWriteVolAdd(VolInfo *vol_info);
NTSTATUS RegWriteVolDel(VolInfo *vol_info);
NTSTATUS RegWriteVolMod(VolInfo *vol_info);
NTSTATUS RegWriteIomirrorState(uint32_t state);

NTSTATUS RegReadAllVol(VolInfo **vol_infos, uint32_t *vol_num);
NTSTATUS RegReadVolNum(uint32_t* num);

NTSTATUS RegWritePersistState(ULONG state);
NTSTATUS RegReadPersistState(ULONG* state, ULONG default_val);

NTSTATUS RegReadSelect(PULONG select);