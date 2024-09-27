#include "regedit.h"

#include "Ntstrsafe.h"

#include "wpp_trace.h"
#include "reg_basic.tmh"




#define DISK_VOL_NAME_LENGTH			(512)



NTSTATUS RegReadDwordWithDefault(PWCH path, PWCH name, uint32_t default_value, uint32_t *value)
{
	NTSTATUS status;
	// 查询vol_num
	RTL_QUERY_REGISTRY_TABLE RegTable[2];
	memset(RegTable, 0, sizeof(RegTable));
	RegTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	RegTable[0].Name = name;
	RegTable[0].EntryContext = value;
	RegTable[0].DefaultType = REG_DWORD;
	RegTable[0].DefaultLength = sizeof(uint32_t);
	RegTable[0].DefaultData = &default_value;
	status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
		path,
		RegTable,
		NULL,
		NULL);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Query %ws failed, error %!STATUS!", name, status);
		return status;
	}
	return status;
}

NTSTATUS RegReadDword(PWCH path, PWCH name, uint32_t *value)
{
	return RegReadDwordWithDefault(path, name, 1234, value);
}

NTSTATUS RegReadQword(PWCH path, PWCH name, uint64_t *value)
{
	NTSTATUS status;
	HANDLE h_reg = NULL;
	PKEY_VALUE_PARTIAL_INFORMATION pvpi = NULL;
	UNICODE_STRING string;
	RtlInitUnicodeString(&string, path);
	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes(&attr, &string, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	status = ZwOpenKey(&h_reg, KEY_ALL_ACCESS, &attr);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Open h_reg failed, error %!STATUS!", status);
		goto end;
	}
	UNICODE_STRING value_name;
	RtlInitUnicodeString(&value_name, name);
	ULONG size;
	status = ZwQueryValueKey(h_reg, &value_name, KeyValuePartialInformation, NULL, 0, &size);
	if (status == STATUS_OBJECT_NAME_NOT_FOUND || size == 0)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Query value key failed, name = %ws, error %!STATUS!", name, status);
		goto end;
	}
	pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(PagedPool, size, ALLOC_TAG);
	if (pvpi == NULL)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc pvpi failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto end;
	}
	status = ZwQueryValueKey(h_reg, &value_name, KeyValuePartialInformation, pvpi, size, &size);
	if (status == STATUS_OBJECT_NAME_NOT_FOUND || size == 0)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Query value key failed, name = %ws, error %!STATUS!", name, status);
		goto end;
	}
	if (pvpi->Type == REG_QWORD && pvpi->DataLength == sizeof(uint64_t))
	{
		*value = *((uint64_t *)pvpi->Data);
	}
	else
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Query %ws failed, error %!STATUS!", name, status);
	}
end:
	if (NULL != pvpi)
	{
		ExFreePoolWithTag(pvpi, ALLOC_TAG);
	}
	if (NULL != h_reg)
	{
		ZwClose(h_reg);
	}

	return status;
}

NTSTATUS RegReadString(PWCH path, PWCH name, PUNICODE_STRING value)
{
	NTSTATUS status;
	// 查询vol_num
	RTL_QUERY_REGISTRY_TABLE RegTable[2];
	memset(RegTable, 0, sizeof(RegTable));
	RegTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	RegTable[0].Name = name;
	RegTable[0].EntryContext = value;
	RegTable[0].DefaultType = REG_SZ;
	RegTable[0].DefaultLength = 0;
	RegTable[0].DefaultData = L"";
	status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
		path,
		RegTable,
		NULL,
		NULL);
	if (!NT_SUCCESS(status) || value->Buffer == NULL)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Query %ws failed, error %!STATUS!", name, status);
		status = STATUS_UNSUCCESSFUL;
		return status;
	}
	return status;
}

NTSTATUS RegWriteQword(PWCH path, PWCH name, uint64_t value)
{
	NTSTATUS status;
	status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
		path,
		name,
		REG_QWORD,
		&value,
		sizeof(uint64_t));
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write %ws value failed, error %!STATUS!", name, status);
		return status;
	}

	return status;
}

NTSTATUS RegWriteDword(PWCH path, PWCH name, uint32_t value)
{
	NTSTATUS status;
	status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
		path,
		name,
		REG_DWORD,
		&value,
		sizeof(uint32_t));
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write %ws value failed, error %!STATUS!", name, status);
		return status;
	}
	return status;
}

NTSTATUS RegWriteString(PWCH path, PWCH name, PWCH value)
{
	NTSTATUS status;
	status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
		path,
		name,
		REG_SZ,
		value,
		(ULONG)wcslen(value) * 2 + 2);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write %ws failed, error %!STATUS!", name, status);
		return status;
	}
	return status;
}



typedef struct _REG_QUERY_CONTEXT
{
	PVOID buffer;
	ULONG length;
}REG_QUERY_CONTEXT, *PREG_QUERY_CONTEXT;

NTSTATUS BinaryRegistryQueryRoutine(PWSTR name, ULONG type, PVOID data, ULONG length, PVOID context, PVOID entry_context)
{
	UNREFERENCED_PARAMETER(name);

	if (!data || !entry_context || !context)
	{
		return STATUS_OBJECT_NAME_NOT_FOUND;
	}

	if (type != REG_BINARY)
	{
		return STATUS_OBJECT_NAME_NOT_FOUND;
	}

	PREG_QUERY_CONTEXT query_context = (PREG_QUERY_CONTEXT)context;

	if (length < query_context->length)
	{
		return STATUS_OBJECT_NAME_NOT_FOUND;
	}

	RtlCopyMemory(entry_context, data, query_context->length);
	return STATUS_SUCCESS;
}


NTSTATUS RegReadBinary(PWCH path, PWCH name, PVOID value, uint32_t length)
{
	NTSTATUS status;

	REG_QUERY_CONTEXT reg_context = { 0 };
	reg_context.buffer = value;
	reg_context.length = length;

	RTL_QUERY_REGISTRY_TABLE RegTable[2];
	memset(RegTable, 0, sizeof(RegTable));

	RegTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
	RegTable[0].QueryRoutine = BinaryRegistryQueryRoutine;
	RegTable[0].EntryContext = value;
	RegTable[0].Name = name;
	RegTable[0].DefaultType = REG_NONE;

	status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, path, RegTable, &reg_context, NULL);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Query %ws failed, error %!STATUS!", name, status);
		status = STATUS_UNSUCCESSFUL;
		return status;
	}

	return status;
}

NTSTATUS RegWriteBinary(PWCH path, PWCH name, PVOID value, uint32_t length)
{
	NTSTATUS status;
	status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
		path,
		name,
		REG_BINARY,
		value,
		length);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write %ws value failed, error %!STATUS!", name, status);
		return status;
	}

	return status;
}

NTSTATUS RegDeleteKey(PWCH name)
{
	NTSTATUS status;
	HANDLE h_reg;
	UNICODE_STRING string;
	RtlInitUnicodeString(&string, name);
	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes(&attr, &string, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwOpenKey(&h_reg, KEY_ALL_ACCESS, &attr);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Open h_reg_sub failed, error %!STATUS!", status);
		return status;
	}
	status = ZwDeleteKey(h_reg);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Delete item failed, key_name = %ws, error %!STATUS!", name, status);
	}
	ZwClose(h_reg);
	return status;
}

NTSTATUS RegReadSelect(PULONG select)
{
	uint32_t cur_select = 0;
	NTSTATUS status = RegReadDwordWithDefault(L"\\Registry\\Machine\\SYSTEM\\Select", L"Current", 0, &cur_select);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read reg current failed, error %!STATUS!", status);
	}
	else
	{
		*select = cur_select;
	}

	return status;
}



/*++

函数描述:

分配操作注册表时所使用的分区信息结构体

参数:

num：申请分区的个数

返回值:

分区信息结构体地址

--*/
VolInfo *RegMallocVolInfo(uint32_t num)
{
	VolInfo *info = (VolInfo *)ExAllocatePoolWithTag(PagedPool, sizeof(VolInfo)*num, ALLOC_TAG);
	if (NULL == info)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc PartToGIdInfo node failed");
		return NULL;
	}
	memset(info, 0, sizeof(VolInfo)*num);
	return info;
}

VOID RegFreeVolInfo(VolInfo *vol_info)
{
	ExFreePoolWithTag(vol_info, ALLOC_TAG);
}

//Key代表子项，Value代表子键

//将字符串里的\变成_
PWCH ConvertString(VolInfo *vol_info)
{
	PWCH convert_buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPool, DISK_VOL_NAME_LENGTH, ALLOC_TAG);
	if (NULL == convert_buffer)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc buffer_new failed");
		return NULL;
	}
	memset(convert_buffer, 0, DISK_VOL_NAME_LENGTH);
#ifdef _DR
	RtlCopyMemory(convert_buffer, vol_info->pdx->disk_name.Buffer, vol_info->pdx->disk_name.Length);
#else
	RtlCopyMemory(convert_buffer, vol_info->pdx->vol_name.Buffer, vol_info->pdx->vol_name.Length);
#endif
	int length = (int)wcslen(convert_buffer);
	for (int i = 0; i < length; i++)
	{
		if (L'\\' == convert_buffer[i])
		{
			convert_buffer[i] = L'_';
		}
	}
	return convert_buffer;
}

PWCH CreateSubKeyName(PWCH buffer)
{
	//创建子项目
	PWCH name = (PWCH)ExAllocatePoolWithTag(PagedPool, 1024, ALLOC_TAG);
	if (NULL == name)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc buffer failed");
		return NULL;
	}
	memset(name, 0, DISK_VOL_NAME_LENGTH);
	RtlStringCbPrintfW(name, 1024, PATH(L"%ws"), buffer);

	return name;
}

/*++

函数描述:

将注册表中的分区个数加一

参数:

无

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
NTSTATUS RegAddVolNum()
{
	// 查询vol_num
	uint32_t num = 0;
	NTSTATUS status = RegReadDword(PATH(L""), L"vol_num", &num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Get vol num value failed, error %!STATUS!", status);
		return status;
	}
	// 记录vol_num
	num++;
	status = RegWriteDword(PATH(L""), L"vol_num", num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Update vol num value failed, error %!STATUS!", status);
		return status;
	}
	return status;
}

/*++

函数描述:

将注册表中的分区个数减一

参数:

无

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
NTSTATUS RegSubVolNum()
{
	// 查询vol_num
	uint32_t num = 0;
	NTSTATUS status = RegReadDword(PATH(L""), L"vol_num", &num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Get vol num value failed, error %!STATUS!", status);
		return status;
	}
	// 记录vol_num
	num--;
	status = RegWriteDword(PATH(L""), L"vol_num", num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Update vol num value failed, error %!STATUS!", status);
		return status;
	}
	return status;
}

NTSTATUS RegReadVolNum(uint32_t *num)
{
	NTSTATUS status = RegReadDword(PATH(L""), L"vol_num", num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Get vol num failed, error %!STATUS!", status);
		return status;
	}
	return status;
}

/*++

函数描述:

向注册表写入保护策略

参数:

reg_info：写入注册表的结构体信息

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
NTSTATUS RegWriteIomirrorState(uint32_t state)
{
	NTSTATUS status;
	//检查某项是否存在
	status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, PATH(L""));
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::The path does not exist, error %!STATUS!", status);
		return status;
	}
	// 记录state
	status = RegWriteDword(PATH(L""), L"state", state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the state value failed, error %!STATUS!", status);
		return status;
	}
	return status;
}

/*++

函数描述:

从注册表删除所有需要保护的分区信息

参数:

无

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
PWCH * AllocSubkeyNameBuffer(ULONG subkeys)
{
	NTSTATUS status = STATUS_SUCCESS;

	PWCH *name = (PWCH *)ExAllocatePoolWithTag(PagedPool, subkeys * sizeof(PWCH), ALLOC_TAG);
	if (NULL == name)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc name failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		return name;
	}

	memset(name, 0, subkeys * sizeof(PWCH));
	for (ULONG i = 0; i < subkeys; i++)
	{
		name[i] = (PWCH)ExAllocatePoolWithTag(PagedPool, DISK_VOL_NAME_LENGTH * 2, ALLOC_TAG);
		if (NULL == name[i])
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc buffer failed");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		memset(name[i], 0, DISK_VOL_NAME_LENGTH * 2);
	}

	if (!NT_SUCCESS(status))
	{
		for (ULONG i = 0; i < subkeys; i++)
		{
			if (name[i] != NULL)
			{
				ExFreePoolWithTag(name[i], ALLOC_TAG);
			}
		}
		ExFreePoolWithTag(name, ALLOC_TAG);

		name = NULL;
	}

	return name;
}

NTSTATUS EnumVolumeSubkey(HANDLE h_reg, ULONG subkeys, PWCH *name)
{
	NTSTATUS status = STATUS_SUCCESS;

	ULONG size = 0;
	PKEY_BASIC_INFORMATION pbi = NULL;
	PWCH name_buffer = NULL;
	for (ULONG i = 0; i < subkeys; i++)
	{
		ZwEnumerateKey(h_reg, i, KeyBasicInformation, NULL, 0, &size);
		pbi = (PKEY_BASIC_INFORMATION)ExAllocatePoolWithTag(PagedPool, size, ALLOC_TAG);
		if (NULL == pbi)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc buffer failed");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		ZwEnumerateKey(h_reg, i, KeyBasicInformation, pbi, size, &size);
		name_buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, DISK_VOL_NAME_LENGTH, ALLOC_TAG);
		if (NULL == name_buffer)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc key name buffer failed");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		memset(name_buffer, 0, DISK_VOL_NAME_LENGTH);
		RtlCopyMemory(name_buffer, pbi->Name, pbi->NameLength);
		status = RtlStringCbPrintfW(name[i], DISK_VOL_NAME_LENGTH, PATH(L"%ws"), name_buffer);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RtlStringCbPrintfW failed, error %!STATUS!", status);
			break;
		}

		ExFreePoolWithTag(name_buffer, ALLOC_TAG);
		name_buffer = NULL;

		ExFreePoolWithTag(pbi, ALLOC_TAG);
		pbi = NULL;
	}

	if (name_buffer)
	{
		ExFreePoolWithTag(name_buffer, ALLOC_TAG);
		name_buffer = NULL;
	}

	if (pbi)
	{
		ExFreePoolWithTag(pbi, ALLOC_TAG);
		pbi = NULL;
	}

	return status;
}

NTSTATUS RemoveVolumeSubkey(ULONG subkeys, PWCH *name, VolInfo *vol_infos)
{
	NTSTATUS status = STATUS_SUCCESS;

	for (ULONG in = 0; in < subkeys; in++)
	{
		status = RegDeleteKey(name[in]);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Delete path failed, name = %ws, error %!STATUS!", name[in], status);
			break;
		}
	}

	if (!NT_SUCCESS(status))
	{
		if (NULL != vol_infos)
		{
			NTSTATUS write_status;
			for (ULONG i = 0; i < subkeys; i++)
			{
				write_status = RegWriteVol(&vol_infos[i]);
				if (!NT_SUCCESS(write_status))
				{
					TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write vol failed, error %!STATUS!", write_status);
				}
			}
		}
	}

	return status;
}

NTSTATUS RegClearAllVol()
{
	NTSTATUS status;
	HANDLE h_reg = NULL;
	ULONG size = 0;
	PKEY_FULL_INFORMATION pfi = NULL;
	PWCH *name = NULL;

	UNICODE_STRING string;
	RtlInitUnicodeString(&string, PATH(L""));
	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes(&attr, &string, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	// 1. 读取并保存原来的vol_infos，以便后面处理失败后回滚
	VolInfo *vol_infos = NULL;
	uint32_t vol_num = 0;
	status = RegReadAllVol(&vol_infos, &vol_num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RegReadAllvol failed, error %!STATUS!", status);
		return status;
	}

	if (vol_num == 0)
	{
		return status;
	}

	// 2. 子项个数即为卷的个数，获得卷的个数为pfi->SubKeys并为每个卷的名字分配内存name
	status = ZwOpenKey(&h_reg, KEY_ALL_ACCESS, &attr);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Open h_reg failed, error %!STATUS!", status);
		return status;
	}
	ZwQueryKey(h_reg, KeyFullInformation, NULL, 0, &size);
	pfi = (PKEY_FULL_INFORMATION)ExAllocatePoolWithTag(PagedPool, size, ALLOC_TAG);
	if (NULL == pfi)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc pfi failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto end;
	}
	ZwQueryKey(h_reg, KeyFullInformation, pfi, size, &size);
	if (pfi->SubKeys == 0)
	{
		status = STATUS_SUCCESS;
		goto end;
	}

	name = AllocSubkeyNameBuffer(pfi->SubKeys);
	if (name == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto end;
	}

	// 3. 通过每个子项的名字就可以得到每个卷的name
	status = EnumVolumeSubkey(h_reg, pfi->SubKeys, name);
	if (!NT_SUCCESS(status))
	{
		goto end;
	}

	// 4. 根据每个卷的name删除相应的子项
	status = RemoveVolumeSubkey(pfi->SubKeys, name, vol_infos);
	if (!NT_SUCCESS(status))
	{
		goto end;
	}

end:
	if (h_reg != NULL)
	{
		ZwClose(h_reg);
	}
	if (pfi != NULL)
	{
		if (NULL != name)
		{
			for (ULONG i = 0; i < pfi->SubKeys; i++)
			{
				if (name[i] != NULL)
				{
					ExFreePoolWithTag(name[i], ALLOC_TAG);
				}
			}
			ExFreePoolWithTag(name, ALLOC_TAG);
		}
		ExFreePoolWithTag(pfi, ALLOC_TAG);
	}
	if (vol_infos != NULL)
	{
		ExFreePoolWithTag(vol_infos, ALLOC_TAG);
	}
	return status;
}

/*++

函数描述:

从注册表读取所有需要保护的分区信息

参数:

vol_info：得到的分区信息
num: 所有需要保护的分区信息个数

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
NTSTATUS ReadVolumes(ULONG subkeys, PWCH *name, VolInfo **vol_infos, uint32_t *vol_num)
{
	NTSTATUS status = STATUS_SUCCESS;

	*vol_num = subkeys;
	*vol_infos = RegMallocVolInfo(*vol_num);
	if (NULL == vol_infos)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc vols failed, num %d", subkeys);
		status = STATUS_INSUFFICIENT_RESOURCES;
		return status;
	}

	for (ULONG i = 0; i < subkeys; i++)
	{
		status = RegReadVol(name[i], &(*vol_infos)[i]);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read vol failed, name = %ws, error %!STATUS!", name[i], status);
			break;
		}
	}

	if (!NT_SUCCESS(status))
	{
		RegFreeVolInfo(*vol_infos);
		*vol_infos = NULL;
	}

	return status;
}

NTSTATUS RegReadAllVol(VolInfo **vol_infos, uint32_t *vol_num)
{
	NTSTATUS status;
	HANDLE h_reg = NULL;
	PWCH *name = NULL;
	PKEY_FULL_INFORMATION pfi = NULL;
	ULONG size = 0;

	*vol_infos = NULL;
	*vol_num = 0;

	// 1. 子项个数即为卷的个数，获得卷的个数为pfi->SubKeys并为每个卷的名字分配内存name
	UNICODE_STRING string;
	RtlInitUnicodeString(&string, PATH(L""));
	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes(&attr, &string, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwOpenKey(&h_reg, KEY_ALL_ACCESS, &attr);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Open h_reg failed, error %!STATUS!", status);
		goto end;
	}
	status = ZwQueryKey(h_reg, KeyFullInformation, NULL, 0, &size);
	if (status != STATUS_BUFFER_TOO_SMALL && status != STATUS_BUFFER_OVERFLOW)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Query key failed, error %!STATUS!", status);
		goto end;
	}
	pfi = (PKEY_FULL_INFORMATION)ExAllocatePoolWithTag(PagedPool, size, ALLOC_TAG);
	if (NULL == pfi)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc pfi failed");
		goto end;
	}
	status = ZwQueryKey(h_reg, KeyFullInformation, pfi, size, &size);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Query key failed after alloc, error %!STATUS!", status);
		goto end;
	}
	if (pfi->SubKeys == 0)
	{
		status = STATUS_SUCCESS;
		goto end;
	}

	name = AllocSubkeyNameBuffer(pfi->SubKeys);
	if (name == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto end;
	}

	// 2. 通过每个子项的名字就可以得到每个卷的name
	status = EnumVolumeSubkey(h_reg, pfi->SubKeys, name);
	if (!NT_SUCCESS(status))
	{
		goto end;
	}

	// 3. 分配所有vol_info的内存，并通过卷的name获得所有vol_info的信息
	status = ReadVolumes(pfi->SubKeys, name, vol_infos, vol_num);
	if (!NT_SUCCESS(status))
	{
		goto end;
	}

end:
	if (NULL != h_reg)
	{
		ZwClose(h_reg);
	}
	if (NULL != pfi)
	{
		if (NULL != name)
		{
			for (ULONG i = 0; i < pfi->SubKeys; i++)
			{
				if (NULL != name[i])
				{
					ExFreePoolWithTag(name[i], ALLOC_TAG);
				}
			}
			ExFreePoolWithTag(name, ALLOC_TAG);
		}
		ExFreePoolWithTag(pfi, ALLOC_TAG);
	}
	return status;
}

/*++

函数描述:

初始化注册表中的容灾属性，如果路径存在，则直接返回

参数:

无

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
NTSTATUS RegInit()
{
	NTSTATUS status;
	//检查某项是否存在
	status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, PATH(L""));
	if (NT_SUCCESS(status))
	{
		return status;
	}
	//创建子项目
	status = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, PATH(L""));
	if (NT_SUCCESS(status))
	{
	}
	else
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Create the item failed, error %!STATUS!", status);
		return status;
	}
	RegInfo reg_info;
	memset(&reg_info, 0, sizeof(reg_info));
	status = RegInitProtect(&reg_info);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RegInitProtect failed, error %!STATUS!", status);
	}
	return status;
}

/*++

函数描述:

将保护策略写入注册表

参数:

reg_info: 注册表对应的内存信息

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
NTSTATUS RegCreateProtect(RegInfo *reg_info)
{
	NTSTATUS 	status = RegInit();
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RegInit failed, error %!STATUS!", status);
		return status;
	}

	status = RegWriteProtect(reg_info);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg write protection, error %!STATUS!", status);
		return status;
	}

	return status;
}

NTSTATUS RegModifyProtect(RegInfo *reg_info)
{
	NTSTATUS status;
	// 查询vol_num
	uint32_t num = 0;
	status = RegReadDword(PATH(L""), L"vol_num", &num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Get vol num failed, error %!STATUS!", status);
		return status;
	}
	reg_info->vol_num = num;//保持vol_num不变
	status = RegWriteProtect(reg_info);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write protect failed, error %!STATUS!", status);
		return status;
	}
	return status;
}

/*++

函数描述:

从注册表删除所有分区信息并删除保护策略

参数:

无

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
NTSTATUS RegDeleteProtect()
{
	NTSTATUS status;

	RegInfo reg_info;
	memset(&reg_info, 0, sizeof(RegInfo));

	RegInfo reg_info_orig;
	memset(&reg_info_orig, 0, sizeof(RegInfo));
	status = RegReadProtect(&reg_info_orig);
	if (status == STATUS_OBJECT_NAME_NOT_FOUND)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Thre reg path doesn't exist, error %!STATUS!", status);
		return STATUS_SUCCESS;
	}

	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read protect failed, error %!STATUS!", status);
		return status;
	}
	status = RegWriteProtect(&reg_info);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write protect failed, error %!STATUS!", status);
		goto end;
	}

	status = RegClearAllVol();
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Clear all sub key failed, error %!STATUS!", status);

		NTSTATUS write_status = RegWriteProtect(&reg_info_orig);
		if (!NT_SUCCESS(write_status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Rollback protect failed, error %!STATUS!", write_status);
		}
		goto end;
	}

end:

	return status;
}

/*++

函数描述:

向注册表添加分区信息

参数:

vol_info：分区信息

返回值:

STATUS_SUCCESS： 成功
其它: 失败

--*/
NTSTATUS RegWriteVolAdd(VolInfo *vol_info)
{
	NTSTATUS status;
	// vol_num加1
	status = RegAddVolNum();
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Add vol num failed, error %!STATUS!", status);
		return status;
	}

	status = RegWriteVol(vol_info);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write vol failed, error %!STATUS!", status);

		NTSTATUS sub_status = RegSubVolNum();
		if (!NT_SUCCESS(sub_status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Del vol num failed, error %!STATUS!", sub_status);
		}
		return status;
	}

	return status;
}

NTSTATUS RegWriteVolDel(VolInfo *vol_info)
{
	NTSTATUS status;
	PWCH convert_buffer = NULL;
	PWCH name = NULL;
	convert_buffer = ConvertString(vol_info);
	if (NULL == convert_buffer)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!:Alloc buffer failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto end;
	}
	name = CreateSubKeyName(convert_buffer);
	if (NULL == convert_buffer)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc name failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto end;
	}

	// 检查name对应的注册表项是否存在
	status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, name);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!:The key does not exist, error %!STATUS!", status);
		goto end;
	}
	// vol_num减1
	status = RegSubVolNum();
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Sub vol num failed, error %!STATUS!", status);
		goto end;
	}
	// 删除name对应的注册表项
	status = RegDeleteKey(name);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Delete item failed, name = %ws, error %!STATUS!", name, status);

		NTSTATUS add_status = RegAddVolNum();
		if (!NT_SUCCESS(add_status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Add vol num failed, error %!STATUS!", add_status);
		}
		goto end;
	}

end:
	if (NULL != name)
	{
		ExFreePoolWithTag(name, ALLOC_TAG);
	}
	if (NULL != convert_buffer)
	{
		ExFreePoolWithTag(convert_buffer, ALLOC_TAG);
	}
	return status;
}

NTSTATUS RegWriteVolMod(VolInfo *vol_info)
{
    NTSTATUS status;
    PWCH convert_buffer = NULL;
    PWCH name = NULL;
    convert_buffer = ConvertString(vol_info);
    if (NULL == convert_buffer) {
        TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!:Alloc buffer failed");
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }
    name = CreateSubKeyName(convert_buffer);
    if (NULL == convert_buffer) {
        TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc name failed");
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }

    // 检查name对应的注册表项是否存在
    status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, name);
    if (!NT_SUCCESS(status)) {
        TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!:The key does not exist, error %!STATUS!", status);
        goto end;
    }

    // 记录disk_name
    status = RegWriteString(name, L"disk_name", vol_info->pdx->disk_name.Buffer);
    if (!NT_SUCCESS(status)) {
        TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the disk_name value failed, error %!STATUS!", status);
        goto end;
    }

    status = RegWriteBinary(name, L"vol_id", vol_info->vol_id, VOL_ID_LEN);
    if (!NT_SUCCESS(status)) {
        TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vol_id value failed, error %!STATUS!", status);

        goto end;
    }

    status = RegWriteDword(name, L"entire_disk", vol_info->entire_disk);
    if (!NT_SUCCESS(status)) {
        TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the entire_disk value failed, error %!STATUS!", status);

        goto end;
    }

end:
    if (NULL != name) {
        ExFreePoolWithTag(name, ALLOC_TAG);
    }
    if (NULL != convert_buffer) {
        ExFreePoolWithTag(convert_buffer, ALLOC_TAG);
    }
    return status;
}


NTSTATUS RegWritePersistState(ULONG state)
{
	NTSTATUS status = RegWriteDword(PATH(L""), L"persist_state", state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the persist_state value failed, error %!STATUS!", status);
	}

	return status;
}

NTSTATUS RegReadPersistState(ULONG* state, ULONG default_val)
{
	NTSTATUS status = RegReadDwordWithDefault(PATH(L""), L"persist_state", default_val, (uint32_t*)state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the persist_state value failed, error %!STATUS!", status);
	}

	return status;
}