#include "ioctl.h"
#include "group_mem.h"
#include "regedit.h"
#include "util.h"

#include "persist_file.h"

#include "wpp_trace.h"
#include "ioctl.tmh"

NTSTATUS LoadStrategy(GroupInfo *group_info, RegInfo *reg_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	do
	{
		// 如果状态为IM_PG_STATE_STOP，则需要从注册表中读取保护策略信息
		if (IM_PG_STATE_STOP == group_info->state)
		{
			group_info->protect_strategy.vrgIp = reg_info->vrg_ip;
			group_info->protect_strategy.vrgPort = reg_info->vrg_port;
			group_info->protect_strategy.rpo = reg_info->rpo;
			group_info->bitmap_granularity = reg_info->granularity;
			group_info->dataset_id = reg_info->dataset_id;
			group_info->dataset_id_done = reg_info->dataset_id_done;

			RtlZeroMemory(group_info->protect_strategy.osId, VM_ID_LEN);
			RtlCopyMemory(group_info->protect_strategy.osId, reg_info->os_id, VM_ID_LEN);

			RtlZeroMemory(group_info->protect_strategy.oma_id, VM_ID_LEN);
			RtlCopyMemory(group_info->protect_strategy.oma_id, reg_info->oma_id, VM_ID_LEN);

			group_info->state = reg_info->iomirror_state;
			group_info->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
		}
	} while (flag);

	return status;
}

NTSTATUS LoadVols(GroupInfo* group_info, PDEVICE_EXTENSION pdx, RegInfo *reg_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	VolInfo *vol_infos = NULL;
	QueueInfo *load_vol_queue = NULL;
	
	do
	{
		// 如果group_info->boot_flag等于FALSE，则说明网络模块已经启动并连接，即该磁盘加载在网络模块加载之后
		QueueInfo *vol_queue = group_info->boot_flag == TRUE ? group_info->vol_queue : group_info->temp_vol_queue;

		load_vol_queue = InitQueueInfo(group_info);
		if (NULL == load_vol_queue)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init queue failed, error %!STATUS!", status);
			break;
		}

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
		// 把保护的分区信息逐个加载入内存
		for (uint32_t i = 0; i < reg_info->vol_num; i++)
		{
			//如果分区的disk_name跟本驱动所在的disk_name不一样，则分区不属于这个驱动，不需要添加
			if (wcscmp(vol_infos[i].disk_name.Buffer, pdx->disk_name.Buffer) != 0)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Vol does not belong to this driver, %wZ, %wZ", &pdx->disk_name, &vol_infos[i].disk_name);
				continue;
			}

			ProtectVolInter vol_inter;
			memset(&vol_inter, 0, sizeof(ProtectVolInter));
			vol_inter.disk_num = pdx->disk_number;
			vol_inter.need_set_flag = FALSE;
			vol_inter.entire_disk = vol_infos[i].entire_disk;
			RtlCopyMemory(vol_inter.vol_id, vol_infos[i].vol_id, VOL_ID_LEN);

			VolInfo *vol_info = BuildVolInfo(pdx, group_info, &vol_inter);
			if (vol_info == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc vol info failed, error %!STATUS!", status);
				break;
			}

			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Build vol, name = %wZ, start = %llu, end = %llu", &vol_info->pdx->disk_name, vol_info->start_pos, vol_info->end_pos);

			if (FindVolInfoByVolInfo(vol_queue, vol_info))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Vol already exists, name = %wZ, start = %llu, end = %llu", &vol_info->pdx->disk_name, vol_info->start_pos, vol_info->end_pos);
				FreeVolInfo(vol_info);
				continue;
			}
			// 如果没有重复的分区，则插入group_info队列中
			PushVolQueue(load_vol_queue, vol_info);
		}
		
		if (GetQueueNum(load_vol_queue) == 0)
		{
			FreeVolQueue(load_vol_queue, group_info);

			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Vol %wZ(%d) is not being protected", &pdx->disk_name, pdx->disk_number);
			LogEvent(group_info->device_obj->DriverObject, LOG_NO_VOL_PROTECTED, STATUS_SUCCESS, &pdx->disk_name, NULL, &pdx->disk_number, sizeof(pdx->disk_number));
			break;
		}

		if (TRUE == group_info->boot_flag)
		{
			PLIST_ENTRY head = &load_vol_queue->head;
			PLIST_ENTRY req_entry = head->Flink;
			while (req_entry != head)
			{
				PLIST_ENTRY temp_req_entry = req_entry->Flink;
				VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
				RemoveEntryList(req_entry);
				load_vol_queue->num--;
				PushVolQueue(vol_queue, vol_info);
				req_entry = temp_req_entry;
			}
			FreeVolQueue(load_vol_queue, group_info);
		}
		// 该驱动在网络模块加载之后
		else
		{
			status = WaitForExternCtl(IM_PG_IOCTL_STATE_START, (uint8_t*)load_vol_queue, group_info);
			if (STATUS_SUCCESS != status)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Extern ctl execution failed, error %!STATUS!", status);
				break;
			}
		}
	}while (flag);

	if (NULL != vol_infos)
	{
		RegFreeVolInfo(vol_infos);
		vol_infos = NULL;
	}

	return status;
}

/*++

函数描述:

    当OS启动时，判断如果注册表内有容灾属性，则根据该容灾属性启动容灾保护

参数:

	pdx：驱动设备对象扩展信息

返回值:

	STATUS_SUCCESS： 成功
	其它: 失败

--*/
NTSTATUS StartProtectByReg(PDEVICE_EXTENSION pdx)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	GroupInfo* group_info = GetGroupInfo(pdx);

	RegInfo reg_info = {0};

	do
	{
		// 首先读取注册表的内容
		memset(&reg_info, 0, sizeof(RegInfo));
		status = RegReadProtect(&reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read protect failed, error %!STATUS!", status);
			break;
		}
		else
		{
			// 如果注册表内容中的init字段为FALSE，则直接返回
			if (reg_info.iomirror_state == IM_PG_STATE_STOP)
			{
				TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::DR is not init");
				break;
			}
		}

		// 再次从注册表读出需要保护的分区
		if (reg_info.vol_num == 0)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Protect start, but vol_num == 0");
			break;
		}
		// 注册表里有容灾的属性，表示上次关机之前容灾已经启动，将容灾属性加载入内存
		// load strategy
		status = LoadStrategy(group_info, &reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Load strategy failed, error %!STATUS!", status);
			break;
		}
		// load vol
		status = LoadVols(group_info, pdx, &reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Load vols failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	return status;
}

/*++

函数描述:

    为该OS创建保护策略

参数:

	pdx：驱动设备对象扩展信息
	protect_strategy：保护策略

返回值:

	TRUE： 成功
	FALSE: 失败

--*/
NTSTATUS IomirrorStart(GroupInfo *group_info, ProtectStrategy *protect_strategy)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;
	
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Start iomirror, ip %d, port %d, rpo %d, mem %d, protect size %llu, vm id %!HEXDUMP!, soma id %!HEXDUMP!", protect_strategy->oma_ip[0], protect_strategy->oma_port, protect_strategy->exp_rpo, protect_strategy->mem_threshold, protect_strategy->protect_size, LOG_LENSTR(protect_strategy->vm_id, VM_ID_LEN), LOG_LENSTR(protect_strategy->oma_id, VM_ID_LEN));
	
	do
	{
		if (group_info->state != IM_PG_STATE_STOP)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror already start, state = %d", group_info->state);
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

		group_info->protect_strategy.rpo = protect_strategy->exp_rpo;
		if (group_info->protect_strategy.rpo < IM_MIN_RPO)
		{
			group_info->protect_strategy.rpo = IM_MIN_RPO;
			TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::RPO value %d is too small, adjust it to %d", protect_strategy->exp_rpo, group_info->protect_strategy.rpo);
		}

		RtlCopyMemory(group_info->protect_strategy.osId, protect_strategy->vm_id, VM_ID_LEN);
		RtlCopyMemory(group_info->protect_strategy.oma_id, protect_strategy->oma_id, VM_ID_LEN);
		group_info->protect_strategy.vrgIp = protect_strategy->oma_ip[0];
		group_info->protect_strategy.vrgPort = protect_strategy->oma_port;

		// 注册表添加保护策略
		RegInfo reg_info = {0};
		reg_info.iomirror_state = IM_PG_STATE_CBT;
		reg_info.vol_num = 0;
		reg_info.vrg_ip = group_info->protect_strategy.vrgIp;
		reg_info.vrg_port = group_info->protect_strategy.vrgPort;
		reg_info.rpo = group_info->protect_strategy.rpo;
		reg_info.dataset_id = 1;
		reg_info.dataset_id_done = 0;

		if (group_info->protect_strategy.mem_threshold == 0 || group_info->protect_strategy.protect_size == 0)
		{
			reg_info.granularity = IM_BITMAP_GRANULARITY;
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Use default granularity");
		}
		else
		{
			uint64_t protect_size = (1ULL << 30) * group_info->protect_strategy.protect_size;
			uint64_t granularity = protect_size / (1ULL << 20) / IM_SECTOR_SIZE;
			granularity /= (uint64_t)group_info->protect_strategy.mem_threshold * 8 / 2;
			reg_info.granularity = (uint8_t)make_log_upper_bound(granularity);

			if (reg_info.granularity < IM_BITMAP_GRANULARITY)
			{
				TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::Given guranularity %d is too small, use the default one", reg_info.granularity);
				reg_info.granularity = IM_BITMAP_GRANULARITY;
			}
			else if (reg_info.granularity > IM_MAX_BITMAP_GRANULARITY)
			{
				TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::Given guranularity %d is too large, use the largest one", reg_info.granularity);
				reg_info.granularity = IM_MAX_BITMAP_GRANULARITY;
			}
		}

		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Final granularity %d", reg_info.granularity);

		group_info->bitmap_granularity = reg_info.granularity;

		RtlCopyMemory(reg_info.os_id, group_info->protect_strategy.osId, VM_ID_LEN);
		RtlCopyMemory(reg_info.oma_id, group_info->protect_strategy.oma_id, VM_ID_LEN);

		status = RegCreateProtect(&reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg create protect failed, error %!STATUS!", status);
			break;
		}

		ClearCmdQueue(group_info->cmd_queue, group_info);

		group_info->boot_flag = FALSE;
		group_info->per_data_state = PERSIST_DATA_STATE_DISCARD;
		group_info->state = IM_PG_STATE_CBT;
		group_info->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
	} while (flag);

	return status;
}

/*++

函数描述:

    为该OS创建保护策略

参数:

	pdx：驱动设备对象扩展信息
	protect_strategy：保护策略

返回值:

	TRUE： 成功
	FALSE: 失败

--*/
NTSTATUS IomirrorStartWithVerify(GroupInfo *group_info, ProtectStrategy *protect_strategy)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Start iomirror with verify, ip %d, port %d, rpo %d, mem %d, protect size %llu, vm id %!HEXDUMP!, soma id %!HEXDUMP!", protect_strategy->oma_ip[0], protect_strategy->oma_port, protect_strategy->exp_rpo, protect_strategy->mem_threshold, protect_strategy->protect_size, LOG_LENSTR(protect_strategy->vm_id, VM_ID_LEN), LOG_LENSTR(protect_strategy->oma_id, VM_ID_LEN));

	do
	{
		if (group_info->state != IM_PG_STATE_STOP)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror already start, state = %d", group_info->state);
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

		group_info->protect_strategy.rpo = protect_strategy->exp_rpo;
		if (group_info->protect_strategy.rpo < IM_MIN_RPO)
		{
			group_info->protect_strategy.rpo = IM_MIN_RPO;
			TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::RPO value %d is too small, adjust it to %d", protect_strategy->exp_rpo, group_info->protect_strategy.rpo);
		}
		
		RtlCopyMemory(group_info->protect_strategy.osId, protect_strategy->vm_id, VM_ID_LEN);
		RtlCopyMemory(group_info->protect_strategy.oma_id, protect_strategy->oma_id, VM_ID_LEN);
		group_info->protect_strategy.vrgIp = protect_strategy->oma_ip[0];
		group_info->protect_strategy.vrgPort = protect_strategy->oma_port;

		// 注册表添加保护策略
		RegInfo reg_info = {0};
		reg_info.iomirror_state = IM_PG_STATE_VERIFY;
		reg_info.rpo = group_info->protect_strategy.rpo;
		reg_info.vol_num = 0;
		reg_info.vrg_ip = group_info->protect_strategy.vrgIp;
		reg_info.vrg_port = group_info->protect_strategy.vrgPort;

		if (group_info->protect_strategy.mem_threshold == 0 || group_info->protect_strategy.protect_size == 0)
		{
			reg_info.granularity = IM_BITMAP_GRANULARITY;
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Use default granularity");
		}
		else
		{
			uint64_t protect_size = (1ULL << 30) * group_info->protect_strategy.protect_size;
			uint64_t granularity = protect_size / (1ULL << 20) / IM_SECTOR_SIZE;
			granularity /= (uint64_t)group_info->protect_strategy.mem_threshold * 8 / 2;
			reg_info.granularity = (uint8_t)make_log_upper_bound(granularity);

			if (reg_info.granularity < IM_BITMAP_GRANULARITY)
			{
				TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::Given guranularity %d is too small, use the default one", reg_info.granularity);
				reg_info.granularity = IM_BITMAP_GRANULARITY;
			}
			else if (reg_info.granularity > IM_MAX_BITMAP_GRANULARITY)
			{
				TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::Given guranularity %d is too large, use the largest one", reg_info.granularity);
				reg_info.granularity = IM_MAX_BITMAP_GRANULARITY;
			}
		}

		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Final granularity %d", reg_info.granularity);

		group_info->bitmap_granularity = reg_info.granularity;

		reg_info.dataset_id = 1;
		reg_info.dataset_id_done = 0;

		RtlCopyMemory(reg_info.os_id, group_info->protect_strategy.osId, VM_ID_LEN);
		RtlCopyMemory(reg_info.oma_id, group_info->protect_strategy.oma_id, VM_ID_LEN);

		status = RegCreateProtect(&reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg create protect failed, error %!STATUS!", status);
			break;
		}

		ClearCmdQueue(group_info->cmd_queue, group_info);

		group_info->boot_flag = FALSE;
		group_info->state = IM_PG_STATE_VERIFY;
		group_info->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
	} while (flag);

	return status;
}

/*++

函数描述:

    为该OS修改保护策略

参数:

	pdx：驱动设备对象扩展信息
	protect_strategy：保护策略

返回值:

	TRUE： 成功
	FALSE: 失败

--*/
NTSTATUS IomirrorModify(GroupInfo *group_info, ProtectStrategy *protect_strategy)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Modify iomirror, ip %d, port %d, rpo %d, mem %d, protect size %llu, vm id %!HEXDUMP!, soma id %!HEXDUMP!", protect_strategy->oma_ip[0], protect_strategy->oma_port, protect_strategy->exp_rpo, protect_strategy->mem_threshold, protect_strategy->protect_size, LOG_LENSTR(protect_strategy->vm_id, VM_ID_LEN), LOG_LENSTR(protect_strategy->oma_id, VM_ID_LEN));

	do
	{
		if (group_info->state == IM_PG_STATE_STOP)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror is in stop state");
			break;
		}

		// 保存修改之前注册表的内容
		RegInfo reg_info_orig;
		memset(&reg_info_orig, 0, sizeof(RegInfo));
		status = RegReadProtect(&reg_info_orig);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read protect failed, error %!STATUS!", status);
			break;
		}

		RegInfo reg_info = reg_info_orig;
		reg_info.rpo = protect_strategy->exp_rpo;
		if (reg_info.rpo < IM_MIN_RPO)
		{
			reg_info.rpo = IM_MIN_RPO;
			TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::RPO value %d is too small, adjust it to %d", protect_strategy->exp_rpo, reg_info.rpo);
		}

		reg_info.vrg_ip = protect_strategy->oma_ip[0];
		reg_info.vrg_port = protect_strategy->oma_port;

		RtlCopyMemory(reg_info.os_id, protect_strategy->vm_id, VM_ID_LEN);
		RtlCopyMemory(reg_info.oma_id, protect_strategy->oma_id, VM_ID_LEN);

		status = RegModifyProtect(&reg_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg create protect failed, error %!STATUS!", status);
			break;
		}

		ProtectStrategy* protect_info = (ProtectStrategy*)ExAllocatePoolWithTag(NonPagedPool, sizeof(ProtectStrategy), ALLOC_TAG);
		if (NULL == protect_info)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc ProtectStrategy failed");
			break;
		}

		RtlCopyMemory(protect_info, protect_strategy, sizeof(ProtectStrategy));

		status = WaitForExternCtl(IM_PG_IOCTL_STATE_MODIFY, (uint8_t*)protect_info, group_info);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Extern ctl execution failed, error %!STATUS!", status);
			break;
		}
	} while (flag);


	return status;
}

/*++

函数描述:

    从该OS删除保护策略

参数:

	pdx：驱动设备对象扩展信息
	protect_strategy：保护策略

返回值:

	TRUE： 成功
	FALSE: 失败

--*/
NTSTATUS IomirrorStop(GroupInfo *group_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;
	
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Delete protect group status = %d", group_info->state);

	do
	{
		if (group_info->state == IM_PG_STATE_STOP)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror already stops");
			break;
		}

		// 注册表删除保护策略
		status = RegDeleteProtect();
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg delete protect failed, error %!STATUS!", status);
			break;
		}

		status = WaitForExternCtl(IM_PG_IOCTL_STATE_STOP, NULL, group_info);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Extern ctl execution failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	return status;
}

VolInfo* BuildVolInfoFromDiskNum(GroupInfo *group_info, ProtectVolInter* vol_inter, BOOLEAN create_new)
{
	VolInfo* vol_info = NULL;
	BOOLEAN flag = FALSE;

	do
	{
		PDEVICE_EXTENSION pdx = FindDeviceExtensionByDiskNum(group_info->dev_ext_queue, vol_inter->disk_num);
		if (pdx == NULL && create_new)
		{
				TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Pdx for disk %d is not found, create a new one", vol_inter->disk_num);
				pdx = CreateDeviceExtensionByDiskNumAndHook(vol_inter->disk_num);
		}

		if (pdx == NULL)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find or create pdx with disk number %d", vol_inter->disk_num);
			break;
		}

		NTSTATUS status = IoAcquireRemoveLock(&pdx->RemoveLock, pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to acquire remove lock, disk number %d, error %!STATUS!", vol_inter->disk_num, status);
			break;
		}

		vol_info = BuildVolInfo(pdx, group_info, vol_inter);
		if (NULL == vol_info)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol failed");
		}

		IoReleaseRemoveLock(&pdx->RemoveLock, pdx);
	} while (flag);

	return vol_info;
}

/*++

函数描述:

    为保护组添加一个分区信息

参数:

	pdx：驱动设备对象扩展信息
	strategy： 所要添加的分区信息

返回值:

    TRUE： 成功
	FALSE：失败

--*/
NTSTATUS IomirrorVolAdd(GroupInfo *group_info, ProtectVol *vol_outer)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Disk number %d, vol id %!HEXDUMP!", vol_outer->disk_num, LOG_LENSTR(vol_outer->vol_id, VM_ID_LEN));

	VolInfo *vol_info = NULL;

	do
	{
		if (group_info->state == IM_PG_STATE_STOP)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror is in stop state, state %d", group_info->state);
			break;
		}

		ProtectVolInter vol_inter = { 0 };
		vol_inter.disk_num = vol_outer->disk_num;
		vol_inter.need_set_flag = TRUE ;
		vol_inter.entire_disk = TRUE;
		RtlCopyMemory(vol_inter.vol_id, vol_outer->vol_id, VOL_ID_LEN);

		vol_info = BuildVolInfoFromDiskNum(group_info, &vol_inter, TRUE);
		if (NULL == vol_info)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol failed");
			break;
		}

		// 判断是否已经有重复的策略（分区）
		QueueInfo *vol_queue = group_info->vol_queue;
		if (FindVolInfoByVolInfo(vol_queue, vol_info))
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Vol already exists, name = %wZ, start = %llu, end = %llu", &vol_info->pdx->disk_name, vol_info->start_pos, vol_info->end_pos);
			FreeVolInfo(vol_info);
			break;
		}

		status = RegWriteVolAdd(vol_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg add vol info failed");
			FreeVolInfo(vol_info);
			break;
		}

		status = WaitForExternCtl(IM_PG_IOCTL_STATE_VOL_ADD, (uint8_t*)vol_info, group_info);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Extern ctl execution failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	return status;
}

/*++

函数描述:

    从保护组删除一个分区信息

参数:

	pdx：驱动设备对象扩展信息
	strategy： 所要删除的分区信息

返回值:

    TRUE： 成功
	FALSE：失败

--*/
NTSTATUS IomirrorVolDel(GroupInfo *group_info, ProtectVol *vol_outer)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Disk number %d, vol id %!HEXDUMP!", vol_outer->disk_num, LOG_LENSTR(vol_outer->vol_id, VM_ID_LEN));

	VolInfo *find_vol_info = NULL;

	do
	{
		if (group_info->state == IM_PG_STATE_STOP)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror is in stop state, state %d", group_info->state);
			break;
		}

		ProtectVolInter vol_inter = { 0 };
		vol_inter.disk_num = vol_outer->disk_num;
		vol_inter.need_set_flag = TRUE;
		vol_inter.entire_disk = TRUE;
		RtlCopyMemory(vol_inter.vol_id, vol_outer->vol_id, VOL_ID_LEN);

		find_vol_info = BuildVolInfoFromDiskNum(group_info, &vol_inter, FALSE);
		if (NULL == find_vol_info)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol failed");
			break;
		}

		QueueInfo *vol_queue = group_info->vol_queue;
		if (!FindVolInfoByVolInfo(vol_queue, find_vol_info))
		{
			status = STATUS_NOT_FOUND;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Find vol by vol failed, name = %wZ, start = %llu, end = %llu", &find_vol_info->pdx->disk_name, find_vol_info->start_pos, find_vol_info->end_pos);
			FreeVolInfo(find_vol_info);
			break;
		}

		//从注册表中删除该卷
		status = RegWriteVolDel(find_vol_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg add vol info failed, error %!STATUS!", status);
			FreeVolInfo(find_vol_info);
			break;
		}

		//从内存中删除该卷
		status = WaitForExternCtl(IM_PG_IOCTL_STATE_VOL_DEL, (uint8_t*)find_vol_info, group_info);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Extern ctl execution failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	return status;
}

BOOL CheckGroupState(GroupInfo *group_info)
{
	if (group_info->state == IM_PG_STATE_STOP) {
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::modify vol by vol failed, Iomirror is in stop state, state %d", 
			group_info->state);
		return FALSE;
	}

	if (group_info->inter_state != IM_PG_INTER_STATE_CONNECT_STAGE0
		&& group_info->inter_state != IM_PG_INTER_STATE_CONNECT_STAGE1
		&& group_info->inter_state != IM_PG_INTER_STATE_CONNECT_STAGE1_P
		&& group_info->inter_state != IM_PG_INTER_STATE_CONNECT_STAGE2
		&& group_info->inter_state != IM_PG_INTER_STATE_CONNECT_STAGE2_P) {
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::modify vol by vol failed, group inter_stat is not expected, state %d.",
			group_info->inter_state);
		return FALSE;
	}

	return TRUE;
}

/*++
函数描述:
修改保护组一个分区信息信息
参数:
pdx：驱动设备对象扩展信息
strategy： 所要删除的分区信息
返回值:
TRUE： 成功
FALSE：失败
--*/
NTSTATUS IomirrorVolMod(GroupInfo *group_info, ProtectVol *vol_outer)
{
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN flag = FALSE;

    TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Disk number %d, vol id %!HEXDUMP!, old vol id %!HEXDUMP!.", 
        vol_outer->disk_num, LOG_LENSTR(vol_outer->vol_id, VM_ID_LEN), LOG_LENSTR(vol_outer->old_vol_id, VM_ID_LEN));

    VolInfo *vol_info = NULL;

    do {
        if (!CheckGroupState(group_info)) {
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        ProtectVolInter vol_inter = { 0 };
        vol_inter.disk_num = vol_outer->disk_num;
        vol_inter.need_set_flag = TRUE;
        vol_inter.entire_disk = TRUE;
        RtlCopyMemory(vol_inter.vol_id, vol_outer->vol_id, VOL_ID_LEN);

		vol_info = BuildVolInfoFromDiskNum(group_info, &vol_inter, FALSE);
		if (NULL == vol_info)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol failed");
			break;
		}

        // 判断是否已经有重复的策略（分区）
        QueueInfo *vol_queue = group_info->vol_queue;
        if (!FindVolInfoByVolInfo(vol_queue, vol_info)) {
            status = STATUS_NOT_FOUND;
            TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Find vol by vol failed, name = %wZ, start = %llu, end = %llu", 
                &vol_info->pdx->disk_name, vol_info->start_pos, vol_info->end_pos);
            FreeVolInfo(vol_info);
            break;
        }

        status = RegWriteVolMod(vol_info);
        if (!NT_SUCCESS(status)) {
            TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reg add vol info failed");
            FreeVolInfo(vol_info);
            break;
        }
		
        // 设置失败后的回滚由用户态触发
        group_info->extern_ctl_buffer = (uint8_t*)vol_info;
        status = WaitForExternCtl(IM_PG_IOCTL_STATE_VOL_MOD, (uint8_t*)vol_info, group_info);
        if (!NT_SUCCESS(status)) {
            TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Extern ctl execution failed, error %!STATUS!", status);
            FreeVolInfo(vol_info);
            break;
        }
    } while (flag);

    return status;
}

NTSTATUS IomirrorNotifyChange(GroupInfo *group_info, NotifyChange* change)
{
	UNREFERENCED_PARAMETER(group_info);
	UNREFERENCED_PARAMETER(change);

	return FALSE;
}

NTSTATUS IomirrorPause(GroupInfo *group_info)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Do pause group status = %d", group_info->state);

	if (group_info->state == IM_PG_STATE_STOP || group_info->state == IM_PG_STATE_ERROR)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror is in wrong state");
		return STATUS_UNSUCCESSFUL;
	}

	if (group_info->flow_control_pause_flag || group_info->pause_pending)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Pause is ongoing, pause flag %d, pause pending %d", group_info->flow_control_pause_flag, group_info->pause_pending);
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS status = WaitForExternCtl(IM_PG_IOCTL_STATE_PAUSE, NULL, group_info);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Extern ctl execution failed, error %!STATUS!", status);
		return status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS IomirrorResume(GroupInfo *group_info)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Do resume group status = %d", group_info->state);

	if (!group_info->flow_control_pause_flag && !group_info->pause_pending)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Iomirror is in wrong state");
		return STATUS_UNSUCCESSFUL;
	}

	group_info->resume_pending = TRUE;

	return STATUS_SUCCESS;
}

NTSTATUS IomirrorQueryStart(GroupInfo *group_info, QueryStart* query_start)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::QueryStart status = %d", group_info->state);

	if (group_info->state == IM_PG_STATE_STOP)
	{
		query_start->start = FALSE;
	}
	else
	{
		query_start->start = TRUE;
	}

	return STATUS_SUCCESS;
}