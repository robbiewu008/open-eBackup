#include "ioctl.h"
#include "group_mem.h"
#include "regedit.h"
#include "util.h"
#include "cdo.h"

#include "wpp_trace.h"
#include "ioctl.tmh"

NTSTATUS LoadStrategy(GroupInfo *group_info, RegInfo *reg_info)
{
	if (IsInStopState(group_info))
	{
		group_info->bitmap_granularity = reg_info->granularity;
		SetSnapId(group_info, reg_info->snap_id);
		SwitchState(group_info, reg_info->state);
	}

	return STATUS_SUCCESS;
}

NTSTATUS LoadVols(GroupInfo* group_info, PDEVICE_EXTENSION pdx, RegInfo *reg_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	VolInfo *vol_infos = NULL;
	
	do
	{
		uint32_t vol_num = 0;
		status = RegReadAllVol(&vol_infos, &vol_num);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RegReadAllVol failed, error %!STATUS!", status);
			break;
		}

		if (vol_num != reg_info->vol_num)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Vol_num dif, %d, %d", vol_num, reg_info->vol_num);
			break;
		}

		uint32_t i = 0;
		for (i = 0; i < reg_info->vol_num; i++)
		{
			if (wcscmp(vol_infos[i].vol_unique_id.Buffer, pdx->vol_unique_id.Buffer) == 0)
			{
				break;
			}
		}

		if (i == reg_info->vol_num)
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Volume %wZ(%wZ) is not protected", &pdx->vol_name, &pdx->vol_unique_id);
			break;
		}

		VolInfo *vol_info = BuildVolInfo(group_info, pdx, NULL);
		if (vol_info == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc vol info failed, error %!STATUS!", status);
			break;
		}

		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Build vol, name = %wZ", &vol_info->pdx->vol_name);

		if (FindVolInfoByVolInfo(group_info->vol_queue, vol_info))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Vol already exists, name = %wZ", &vol_info->pdx->vol_name);
			FreeVolInfo(vol_info);
			continue;
		}

		PushVolQueue(group_info->vol_queue, vol_info);

		MergePersistBitmap(group_info);
	}while (flag);

	if (NULL != vol_infos)
	{
		RegFreeVolInfo(vol_infos);
		vol_infos = NULL;
	}

	return status;
}

NTSTATUS StartProtectByReg(PDEVICE_EXTENSION pdx)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	GroupInfo* group_info = GetGroupInfo(pdx);

	RegInfo reg_info = {0};

	do
	{
		memset(&reg_info, 0, sizeof(RegInfo));
		status = RegReadProtect(&reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read protect failed, error %!STATUS!", status);
			break;
		}
		else
		{
			if (reg_info.state == IM_PG_STATE_STOP)
			{
				TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::BKDriver is not init");
				break;
			}
		}

		if (reg_info.vol_num == 0)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Protect start, but vol_num == 0");
			break;
		}

		status = LoadStrategy(group_info, &reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Load strategy failed, error %!STATUS!", status);
			break;
		}

		status = LoadVols(group_info, pdx, &reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Load vols failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	return status;
}

NTSTATUS IoctlStartProtection(GroupInfo *group_info, ProtectStrategy *protect_strategy)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Start Protection, mem %d, protect size %llu", protect_strategy->mem_threshold, protect_strategy->protect_size);
	
	do
	{
		if (!IsInStopState(group_info))
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Bk already start, state = %d", group_info->state);
			break;
		}

		group_info->protect_strategy.protect_size = protect_strategy->protect_size;
		if (!is_int_power(protect_strategy->protect_size))
		{
			group_info->protect_strategy.protect_size = (1 << make_log_upper_bound(protect_strategy->protect_size));
			TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::protect_size value %llu is not an integer power, adjust it to %llu", protect_strategy->protect_size, group_info->protect_strategy.protect_size);
		}

		group_info->protect_strategy.mem_threshold = protect_strategy->mem_threshold;
		if (!is_int_power(protect_strategy->mem_threshold))
		{
			group_info->protect_strategy.mem_threshold = (1 << make_log_lower_bound(protect_strategy->mem_threshold));
			TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::mem_threshold value %d is not an integer power, adjust it to %d", protect_strategy->mem_threshold, group_info->protect_strategy.mem_threshold);
		}

		RegInfo reg_info = {0};
		reg_info.state = IM_PG_STATE_CBT;
		reg_info.vol_num = 0;

		if (group_info->protect_strategy.mem_threshold == 0 || group_info->protect_strategy.protect_size == 0)
		{
			reg_info.granularity = DEFAULT_BITMAP_GRANULARITY;
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Use default granularity");
		}
		else
		{
			uint64_t protect_size = (1ULL << 30) * group_info->protect_strategy.protect_size;
			uint64_t granularity = protect_size / (1ULL << 20) / SECTOR_SIZE;
			granularity /= (uint64_t)group_info->protect_strategy.mem_threshold * 8;
			reg_info.granularity = (uint8_t)make_log_upper_bound(granularity);
		}

		group_info->bitmap_granularity = reg_info.granularity;

		status = RegCreateProtect(&reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg create protect failed, error %!STATUS!", status);
			break;
		}

		group_info->per_data_state = PERSIST_DATA_STATE_DISCARD;
		SwitchState(group_info, IM_PG_STATE_CBT);
	} while (flag);

	return status;
}

NTSTATUS IoctlStopProtection(GroupInfo *group_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;
	
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Delete protect group status = %d", group_info->state);

	do
	{
		if (IsInStopState(group_info))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror already stops");
			break;
		}

		status = RegDeleteProtect();
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg delete protect failed, error %!STATUS!", status);
			break;
		}

		ProcessStop(group_info);
	} while (flag);

	return status;
}

NTSTATUS IoctlVolAdd(GroupInfo *group_info, ProtectVol *vol_outer)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Volume name %s", vol_outer->disk_path);

	VolInfo *vol_info = NULL;
	UNICODE_STRING vol_name = { 0 };

	do
	{
		if (IsInStopState(group_info))
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is in stop state");
			break;
		}

		if (IsInSnapshotState(group_info))
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is in snapshot state");
			break;
		}

		if (!OpenPersistFile(group_info))
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to open persist file");
			break;
		}

		ANSI_STRING ansi_string;
		RtlInitAnsiString(&ansi_string, vol_outer->disk_path);
		status = RtlAnsiStringToUnicodeString(&vol_name, &ansi_string, TRUE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Convert string failed, error %!STATUS!", status);
			break;
		}

		PDEVICE_EXTENSION pdx = FindDeviceExtensionByVolName(group_info->dev_ext_queue, &vol_name);
		if (pdx == NULL)
		{
			status = STATUS_NOT_FOUND;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find volume with vol name %wZ", &vol_name);
			break;
		}

		vol_info = BuildVolInfo(group_info, pdx, NULL);
		if (NULL == vol_info)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol failed");
			break;
		}

		QueueInfo *vol_queue = group_info->vol_queue;
		if (FindVolInfoByVolInfo(vol_queue, vol_info))
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Vol already exists, name = %wZ", &vol_name);
			break;
		}

		status = RegWriteVolAdd(vol_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg add vol info failed");
			break;
		}

		PushVolQueue(group_info->vol_queue, vol_info);
		vol_info = NULL;

		MergePersistBitmap(group_info);
	} while (flag);

	RtlFreeUnicodeString(&vol_name);

	if (vol_info)
	{
		FreeVolInfo(vol_info);
		vol_info = NULL;
	}

	return status;
}

NTSTATUS IoctlVolDel(GroupInfo *group_info, ProtectVol *vol_outer)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Volume name %s", vol_outer->disk_path);

	VolInfo *find_vol_info = NULL;
	UNICODE_STRING vol_name = { 0 };

	do
	{
		if (IsInStopState(group_info))
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is in stop state");
			break;
		}

		if (IsInSnapshotState(group_info))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is in snapshot state");
			return STATUS_UNSUCCESSFUL;
		}

		ANSI_STRING ansi_string;
		RtlInitAnsiString(&ansi_string, vol_outer->disk_path);
		status = RtlAnsiStringToUnicodeString(&vol_name, &ansi_string, TRUE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Convert string failed, error %!STATUS!", status);
			break;
		}

		PDEVICE_EXTENSION pdx = FindDeviceExtensionByVolName(group_info->dev_ext_queue, &vol_name);
		if (pdx == NULL)
		{
			status = STATUS_NOT_FOUND;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find volume with vol name %wZ", &vol_name);
			break;
		}

		find_vol_info = BuildSimpleVolInfo(group_info, pdx);
		if (NULL == find_vol_info)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol failed");
			break;
		}

		QueueInfo *vol_queue = group_info->vol_queue;
		if (!FindVolInfoByVolInfo(vol_queue, find_vol_info))
		{
			status = STATUS_NOT_FOUND;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Find vol by vol failed, name = %wZ", &vol_name);
			break;
		}

		status = RegWriteVolDel(find_vol_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg add vol info failed, error %!STATUS!", status);
			break;
		}

		RemoveVolInfoByVolInfo(group_info->vol_queue, find_vol_info);
	} while (flag);
	
	RtlFreeUnicodeString(&vol_name);
	
	if (find_vol_info)
	{
		FreeVolInfo(find_vol_info);
		find_vol_info = NULL;
	}

	return status;
}

NTSTATUS IoctlStartSnapshot(GroupInfo *group_info, TakeSnapshot* take_snapshot)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Snap id %!GUID!", (GUID*)take_snapshot->snap_id);

	if (IsInStopState(group_info))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is in stop state");
		return STATUS_UNSUCCESSFUL;
	}

	if (IsInSnapshotState(group_info))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is already in snapshot state");
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS status = BitmapSunset(group_info, take_snapshot->snap_id);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::BitmapSunset failed, error %!STATUS!", status);
		return status;
	}

	take_snapshot->bitmap_granularity = group_info->bitmap_granularity;

	SwitchState(group_info, IM_PG_STATE_SNAPSHOT);

	return status;
}

NTSTATUS IoctlFinishSnapshot(GroupInfo *group_info)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Finish snapshot");

	if (IsInStopState(group_info))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is in stop state");
		return STATUS_UNSUCCESSFUL;
	}

	if (!IsInSnapshotState(group_info))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is not in snapshot state");
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS status = BitmapMidnight(group_info);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::BitmapMidnight failed, error %!STATUS!", status);
	}

	return status;
}

NTSTATUS IoctlGetBitmap(GroupInfo *group_info, GetBitmap* get_bitmap)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Get snapshot for vol %s", get_bitmap->vol_path);

	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	VolInfo* vol_info = NULL;
	UNICODE_STRING vol_name = { 0 };
	UNICODE_STRING vol_snap_name = { 0 };

	do
	{
		if (IsInStopState(group_info))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is in stop state");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		if (!IsInSnapshotState(group_info))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is not in snapshot state");
			return STATUS_UNSUCCESSFUL;
		}

		ANSI_STRING ansi_string;
		RtlInitAnsiString(&ansi_string, get_bitmap->vol_path);
		status = RtlAnsiStringToUnicodeString(&vol_name, &ansi_string, TRUE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Convert vol_name string failed, error %!STATUS!", status);
			break;
		}

		RtlInitAnsiString(&ansi_string, get_bitmap->vol_snap_path);
		status = RtlAnsiStringToUnicodeString(&vol_snap_name, &ansi_string, TRUE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Convert vol_snap_name string failed, error %!STATUS!", status);
			break;
		}

		PDEVICE_EXTENSION pdx = FindDeviceExtensionByVolName(group_info->dev_ext_queue, &vol_name);
		if (pdx == NULL)
		{
			status = STATUS_NOT_FOUND;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find volume with name %wZ", &vol_name);
			break;
		}

		BOOLEAN need_snap = FALSE;
		if (!IsUnicodeStringEmpty(&vol_snap_name))
		{
			PDEVICE_EXTENSION snap_pdx = FindDeviceExtensionByVolName(group_info->dev_ext_queue, &vol_snap_name);
			if (pdx == NULL)
			{
				status = STATUS_NOT_FOUND;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find snap volume with name %wZ", &vol_snap_name);
				break;
			}

			if (snap_pdx->orig_pdx != pdx)
			{
				status = STATUS_UNSUCCESSFUL;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Snap volume %wZ is expected for volume %wZ, but not %wZ", &snap_pdx->vol_name, &vol_name, &snap_pdx->orig_pdx->vol_name);
				break;
			}

			need_snap = TRUE;
		}

		vol_info = GetVolInfoByPdx(group_info->vol_queue, pdx);
		if(vol_info == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Vol doesn't exist, name = %wZ", &vol_name);
			break;
		}

		status = GetYesterdayBitmap(group_info, vol_info, need_snap, get_bitmap->data, get_bitmap->bitmap_size);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::GetTodayBitmap failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	RtlFreeUnicodeString(&vol_name);
	RtlFreeUnicodeString(&vol_snap_name);

	if (vol_info)
	{
		InterlockedDecrement(&vol_info->reference_count);
	}

	return status;
}

NTSTATUS IoctlRemoveSnapshot(GroupInfo *group_info, RemoveSnapshot* remove_snapshot)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Remove snapshot %!GUID!, failed %d", (GUID*)remove_snapshot->snap_id, remove_snapshot->is_failed);

	if (IsInStopState(group_info))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is in stop state");
		return STATUS_UNSUCCESSFUL;
	}

	if (!IsInSnapshotState(group_info))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CBT driver is not in snapshot state");
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS status = BitmapSunrise(group_info, remove_snapshot->snap_id, remove_snapshot->is_failed);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::BitmapSunraise failed, error %!STATUS!", status);
	}

	SwitchState(group_info, IM_PG_STATE_CBT);

	return status;
}