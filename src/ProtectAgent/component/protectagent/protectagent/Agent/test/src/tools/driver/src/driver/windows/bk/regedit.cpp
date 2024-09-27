#include "regedit.h"
#include "Ntstrsafe.h"

#include "wpp_trace.h"
#include "regedit.tmh"




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
	status = RegWriteDword(PATH(L""), L"state", reg_info->state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the state value failed, error %!STATUS!", status);
		goto end;
	}
	// 记录vol_num
	status = RegWriteDword(PATH(L""), L"vol_num", reg_info->vol_num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vol_num value failed, error %!STATUS!", status);
		goto end;
	}
	status = RegWriteDword(PATH(L""), L"granularity", reg_info->granularity);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the granularity value failed, error %!STATUS!", status);
		goto end;
	}

	status = RegWriteBinary(PATH(L""), L"snap_id", reg_info->snap_id, SNAP_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the snap_id value failed, error %!STATUS!", status);

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

	status = RegReadDword(PATH(L""), L"state", &reg_info->state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the state value failed, error %!STATUS!", status);
		return status;
	}
	status = RegReadDword(PATH(L""), L"vol_num", &reg_info->vol_num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the vol_num value failed, error %!STATUS!", status);
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

	status = RegReadBinary(PATH(""), L"snap_id", reg_info->snap_id, SNAP_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the snap_id value failed, error %!STATUS!", status);
		return status;
	}

	return status;
}

NTSTATUS RegReadVol(PWCH path, VolInfo *vol_info)
{
	NTSTATUS status;

	status = RegReadString(path, L"vol_unique_id", &vol_info->vol_unique_id);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read the vol_unique_id value failed, error %!STATUS!", status);
		return status;
	}
	
	return status;
}

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
	// 记录vol_name
	status = RegWriteString(name, L"vol_unique_id", vol_info->pdx->vol_unique_id.Buffer);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vol_unique_id value failed, error %!STATUS!", status);
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
	status = RegWriteDword(PATH(L""), L"state", reg_info->state);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the state value failed, error %!STATUS!", status);
		return status;
	}
	// 记录vol_num
	status = RegWriteDword(PATH(L""), L"vol_num", reg_info->vol_num);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the vol_num value failed, error %!STATUS!", status);
		return status;
	}
	status = RegWriteDword(PATH(L""), L"granularity", reg_info->granularity);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the granularity value failed, error %!STATUS!", status);
		return status;
	}

	status = RegWriteBinary(PATH(L""), L"snap_id", reg_info->snap_id, SNAP_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the snap_id value failed, error %!STATUS!", status);

		return status;
	}

	return status;
}

NTSTATUS RegWriteSnapId(char* snap_id)
{
	NTSTATUS status = RegWriteBinary(PATH(L""), L"snap_id", snap_id, SNAP_ID_LEN);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write the snap_id value failed, error %!STATUS!", status);
	}

	return status;
}