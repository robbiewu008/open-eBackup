#include "regedit.h"
#include "Ntstrsafe.h"

#include "wpp_trace.h"
#include "regedit.tmh"


/*++

函数描述:

    向注册表写入保护策略

参数:

	reg_info：写入注册表的结构体信息

返回值:

	STATUS_SUCCESS： 成功
	其它: 失败

--*/
NTSTATUS RegWriteProtect(RegInfo *reg_info)
{
	NTSTATUS status = STATUS_SUCCESS;

	RegInfo reg_info_orig;
	memset(&reg_info_orig, 0, sizeof(RegInfo));
	status = RegReadProtect(&reg_info_orig);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RegReadProtect failed, error %!STATUS!", status);
		return status;
	}

	//检查某项是否存在
	status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, PATH(L""));
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::The path does not, error %!STATUS!", status);
		goto end;
	}
	// 记录state
	status = RegWriteDword(PATH(L""), L"state", reg_info->iomirror_state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the state value failed, error %!STATUS!", status);
		goto end;
	}
	// 记录vrg_ip
	status = RegWriteDword(PATH(L""), L"vrg_ip", reg_info->vrg_ip);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vrg_ip value failed, error %!STATUS!", status);
		goto end;
	}
	// 记录vrg_port
	status = RegWriteDword(PATH(L""), L"vrg_port", reg_info->vrg_port);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vrg_port value failed, error %!STATUS!", status);
		goto end;
	}
	// 记录vol_num
	status = RegWriteDword(PATH(L""), L"vol_num", reg_info->vol_num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vol_num value failed, error %!STATUS!", status);
		goto end;
	}
	status = RegWriteDword(PATH(L""), L"rpo", reg_info->rpo);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the rpo value failed, error %!STATUS!", status);
		goto end;
	}
	status = RegWriteDword(PATH(L""), L"granularity", reg_info->granularity);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the granularity value failed, error %!STATUS!", status);
		goto end;
	}
	status = RegWriteQword(PATH(L""), L"dataset_id", reg_info->dataset_id);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the dataset_id value failed, error %!STATUS!", status);
		goto end;
	}
	status = RegWriteQword(PATH(L""), L"dataset_id_done", reg_info->dataset_id_done);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the dataset_id_done value failed, error %!STATUS!", status);
		goto end;
	}

	status = RegWriteBinary(PATH(L""), L"os_id", reg_info->os_id, VM_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the os_id value failed, error %!STATUS!", status);
		goto end;
	}

	status = RegWriteBinary(PATH(L""), L"oma_id", reg_info->oma_id, VM_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the oma_id value failed, error %!STATUS!", status);
		goto end;
	}
end:
	if (!NT_SUCCESS(status))
	{
		// roll back
		NTSTATUS write_status = RegWriteProtect(&reg_info_orig);
		if (!NT_SUCCESS(write_status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Rollback failed, error %!STATUS!", write_status);
		}
	}

	return status;
}

/*++

函数描述:

    从注册表读取保护策略

参数:

	reg_info：将保护策略写入这个结构体中

返回值:

	STATUS_SUCCESS： 成功
	其它: 失败

--*/
NTSTATUS RegReadProtect(RegInfo *reg_info)
{
	NTSTATUS status;
	//检查某项是否存在
	status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, PATH(L""));
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::The path does not exist, error %!STATUS!", status);
		return status;
	}

	status = RegReadDword(PATH(L""), L"state", &reg_info->iomirror_state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the state value failed, error %!STATUS!", status);
		return status;
	}
	status = RegReadDword(PATH(L""), L"vrg_ip", &reg_info->vrg_ip);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the vrg_ip value failed, error %!STATUS!", status);
		return status;
	}
	status = RegReadDword(PATH(L""), L"vrg_port", &reg_info->vrg_port);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the vrg_port value failed, error %!STATUS!", status);
		return status;
	}
	status = RegReadDword(PATH(L""), L"vol_num", &reg_info->vol_num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the vol_num value failed, error %!STATUS!", status);
		return status;
	}
	status = RegReadDword(PATH(L""), L"rpo", &reg_info->rpo);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the rpo value failed, error %!STATUS!", status);
		return status;
	}
	uint32_t granularity = 0;
	status = RegReadDword(PATH(""), L"granularity", &granularity);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the granularity value failed, error %!STATUS!", status);
		return status;
	}
	reg_info->granularity = (uint8_t)granularity;
	status = RegReadQword(PATH(""), L"dataset_id", &reg_info->dataset_id);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the dataset_id value failed, error %!STATUS!", status);
		return status;
	}
	status = RegReadQword(PATH(""), L"dataset_id_done", &reg_info->dataset_id_done);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the dataset_id_done value failed, error %!STATUS!", status);
		return status;
	}

	status = RegReadBinary(PATH(L""), L"os_id", reg_info->os_id, VM_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the os_id value failed, error %!STATUS!", status);
		return status;
	}

	status = RegReadBinary(PATH(L""), L"oma_id", reg_info->oma_id, VM_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the oma_id value failed, error %!STATUS!", status);
		return status;
	}
	return status;
}

/*++

函数描述:

    从注册表的特定路径读取分区信息

参数:

	path：分区所在的注册表项路径
	vol_info：分区信息

返回值:

	STATUS_SUCCESS： 成功
	其它: 失败

--*/
NTSTATUS RegReadVol(PWCH path, VolInfo *vol_info)
{
	NTSTATUS status;

	status = RegReadString(path, L"disk_name", &vol_info->disk_name);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the disk_name value failed, error %!STATUS!", status);
		return status;
	}

	status = RegReadBinary(path, L"vol_id", vol_info->vol_id, VOL_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the vol_id value failed, error %!STATUS!", status);
		return status;
	}

	status = RegReadDword(path, L"entire_disk", (uint32_t*)&vol_info->entire_disk);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the entire_disk value failed, error %!STATUS!", status);
		return status;
	}
	
	return status;
}

/*++

函数描述:

    将分区信息写入注册表

参数:

	vol_info：分区信息

返回值:

	STATUS_SUCCESS： 成功
	其它: 失败

--*/
NTSTATUS RegWriteVol(VolInfo *vol_info)
{
	NTSTATUS status;
	PWCH convert_buffer = ConvertString(vol_info);
	if (NULL == convert_buffer)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc buffer failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto error1;
	}
	PWCH name = CreateSubKeyName(convert_buffer);
	if (NULL == convert_buffer)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Allc key name failed");
		ExFreePoolWithTag(convert_buffer, ALLOC_TAG);
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto error1;
	}
	ExFreePoolWithTag(convert_buffer, ALLOC_TAG);
	// 根据name创建相应的注册表项
	status = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, name);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Create the item failed, error %!STATUS!", status);
		goto end;
	}
	// 记录disk_name
	status = RegWriteString(name, L"disk_name", vol_info->pdx->disk_name.Buffer);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the disk_name value failed, error %!STATUS!", status);
		goto error;
	}

	status = RegWriteBinary(name, L"vol_id", vol_info->vol_id, VOL_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vol_id value failed, error %!STATUS!", status);

		goto error;
	}

	status = RegWriteDword(name, L"entire_disk", vol_info->entire_disk);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the entire_disk value failed, error %!STATUS!", status);

		goto error;
	}

end:
	ExFreePoolWithTag(name, ALLOC_TAG);
	return status;
error:
	NTSTATUS delete_status = RegDeleteKey(name);
	if (!NT_SUCCESS(delete_status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Rollback delete %ws failed, error %!STATUS!", name, delete_status);
	}
	ExFreePoolWithTag(name, ALLOC_TAG);
	return status;
error1:
	return status;
}

NTSTATUS RegInitProtect(RegInfo *reg_info)
{
	NTSTATUS status;
	// 记录state
	status = RegWriteDword(PATH(L""), L"state", reg_info->iomirror_state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the state value failed, error %!STATUS!", status);
		return status;
	}
	// 记录vrg_ip
	status = RegWriteDword(PATH(L""), L"vrg_ip", reg_info->vrg_ip);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vrg_ip value failed, error %!STATUS!", status);
		return status;
	}
	// 记录vrg_port
	status = RegWriteDword(PATH(L""), L"vrg_port", reg_info->vrg_port);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vrg_port value failed, error %!STATUS!", status);
		return status;
	}
	// 记录vol_num
	status = RegWriteDword(PATH(L""), L"vol_num", reg_info->vol_num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vol_num value failed, error %!STATUS!", status);
		return status;
	}
	status = RegWriteDword(PATH(L""), L"rpo", reg_info->rpo);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the rpo value failed, error %!STATUS!", status);
		return status;
	}
	status = RegWriteDword(PATH(L""), L"granularity", reg_info->granularity);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the granularity value failed, error %!STATUS!", status);
		return status;
	}
	status = RegWriteQword(PATH(L""), L"dataset_id", reg_info->dataset_id);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the dataset_id value failed, error %!STATUS!", status);
		return status;
	}
	status = RegWriteQword(PATH(L""), L"dataset_id_done", reg_info->dataset_id_done);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the dataset_id_done value failed, error %!STATUS!", status);
		return status;
	}

	status = RegWriteBinary(PATH(L""), L"os_id", reg_info->os_id, VM_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the os_id value failed, error %!STATUS!", status);
		return status;
	}

	status = RegWriteBinary(PATH(L""), L"oma_id", reg_info->oma_id, VM_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the oma_id value failed, error %!STATUS!", status);
		return status;
	}

	return status;
}

NTSTATUS RegWriteDatasetId(uint64_t dataset_id, uint64_t dataset_id_done)
{
	NTSTATUS status = RegWriteQword(PATH(L""), L"dataset_id", dataset_id);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the dataset_id value failed, error %!STATUS!", status);
		return status;
	}

	status = RegWriteQword(PATH(L""), L"dataset_id_done", dataset_id_done);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the dataset_id_done value failed, error %!STATUS!", status);
		return status;
	}

	return status;
}