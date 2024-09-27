#include "group_mem.h"
#include "util.h"
#include "persist_file.h"
#include "driver.h"
#include "cdo.h"

#include "wpp_trace.h"
#include "group_mem.tmh"


VOID ImSleep(int time)
{
	LARGE_INTEGER time_out = RtlConvertLongToLargeInteger(time);
	KeDelayExecutionThread(KernelMode, FALSE, &time_out);
}

LARGE_INTEGER GetCurTime()
{
	LARGE_INTEGER cur_time;
	KeQuerySystemTime(&cur_time);
	return cur_time;
}

BOOLEAN MergeBitMap(GroupInfo *group_info, VolInfo *vol_info, unsigned char* data, unsigned int size, uint64_t bit_count)
{
	UNREFERENCED_PARAMETER(group_info);

	BOOLEAN ret = MergeBitmapByBuffer(vol_info->bitmap, data, size);

#ifdef DBG_OM_BITMAP
	if (GetBitmapCount(vol_info->bitmap) != bit_count)
	{
		RaiseException();
	}
#else
	UNREFERENCED_PARAMETER(bit_count);
#endif
	
	return ret;
}

NTSTATUS WaitForExternCtl(uint32_t extern_ctl_type, uint8_t* ctl_buffer, GroupInfo *group_info)
{
	LONGLONG time = 0;
	uint32_t cur_type = IM_PG_IOCTL_STATE_NORMAL;
	while (time <= IOCTL_WAIT_TIME)
	{

		KIRQL old_irql;
		KeAcquireSpinLock(&group_info->extern_ctl_lock, &old_irql);
		cur_type = group_info->extern_ctl_type;
		KeReleaseSpinLock(&group_info->extern_ctl_lock, old_irql);

		if (cur_type == IM_PG_IOCTL_STATE_NORMAL)
		{
			break;
		}

		ImSleep(SECOND_TO_NANOSECOND / 2);
		time += SECOND_TO_NANOSECOND / 2;
	}

	if (time == IOCTL_WAIT_TIME)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Extern ctl is occupied, type %d", cur_type);
		return STATUS_UNSUCCESSFUL;
	}

	KIRQL old_irql;
	KeAcquireSpinLock(&group_info->extern_ctl_lock, &old_irql);
	group_info->extern_ctl_type = extern_ctl_type;
	group_info->extern_ctl_buffer = ctl_buffer;
	KeReleaseSpinLock(&group_info->extern_ctl_lock, old_irql);

	LARGE_INTEGER time_out;
	time_out = RtlConvertLongToLargeInteger(IOCTL_WAIT_TIME);
	if (KeWaitForSingleObject(&group_info->extern_ctl_event, Executive, KernelMode, FALSE, &time_out) != STATUS_SUCCESS)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wait failed for ctl %d", extern_ctl_type);
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		return InterlockedCompareExchange(&group_info->extern_ctl_result, 0, 0);
	}
}

#pragma LOCKEDCODE
NTSTATUS SendErrorExternCtl(GroupInfo *group_info)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&group_info->extern_ctl_lock, &old_irql);
	group_info->extern_ctl_type = IM_PG_IOCTL_STATE_ERROR;
	group_info->extern_ctl_buffer = NULL;
	KeReleaseSpinLock(&group_info->extern_ctl_lock, old_irql);

	return STATUS_SUCCESS;
}

VOID SetEventForExternCtl(GroupInfo *group_info, NTSTATUS status)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&group_info->extern_ctl_lock, &old_irql);
	group_info->extern_ctl_type = IM_PG_IOCTL_STATE_NORMAL;
	group_info->extern_ctl_buffer = NULL;
	KeReleaseSpinLock(&group_info->extern_ctl_lock, old_irql);

	InterlockedExchange(&group_info->extern_ctl_result, status);

	KeSetEvent(&group_info->extern_ctl_event, IO_NO_INCREMENT, FALSE);
}

#pragma LOCKEDCODE
VOID FillDataCmd(IOMirrorCmd *cmd, char *data, uint32_t length, uint32_t alloc_len, uint64_t byte_offset, VolInfo *vol_info, GroupInfo *group_info)
{
	cmd->header.magic = DPP_MAGIC;
	cmd->header.cmd_type = DPP_TYPE_DATA;
	cmd->header.flags = DPP_FLAG_SOURCE_IOMIRROR;
	cmd->header.sequence_num = group_info->sequence_id++;
	cmd->header.body_len = alloc_len;

	PDPP_DATA dpp_data = (PDPP_DATA)cmd->data;
	RtlCopyMemory(dpp_data->vol_id, vol_info->vol_id, VOL_ID_LEN);
	dpp_data->vol_offset = byte_offset;
	dpp_data->data_size = (uint32_t)length - sizeof(DPP_DATA);

	if (data != NULL)
	{
		RtlCopyMemory(dpp_data->data, data, dpp_data->data_size);
	}
}

VOID CopyDataCmdThin(IOMirrorCmd *dest_cmd, IOMirrorCmd *src_cmd)
{
	RtlCopyMemory(dest_cmd, src_cmd, sizeof(IOMirrorCmd) + sizeof(DPP_DATA));
	dest_cmd->header.body_len = sizeof(DPP_DATA);
}

VOID FillDatasetStartCmd(uint32_t work_mode, IOMirrorCmd *cmd, GroupInfo *group_info)
{
	cmd->header.magic = DPP_MAGIC;
	cmd->header.cmd_type = DPP_TYPE_DATASET_START;
	cmd->header.flags = DPP_FLAG_SOURCE_IOMIRROR;
	cmd->header.sequence_num = group_info->sequence_id++;
	cmd->header.body_len = sizeof(DPP_DATASET_START);

	PDPP_DATASET_START dataset_start = (PDPP_DATASET_START)cmd->data;
	dataset_start->dataset_id = group_info->dataset_id++; 
	RtlCopyMemory(dataset_start->vm_id, group_info->protect_strategy.osId, VM_ID_LEN);

	if (work_mode == WORK_MODE_CBT)
	{
		dataset_start->dpp_type = DPP_DATASET_TYPE_CBT;
	}
	else if (work_mode == WORK_MODE_NORMAL)
	{
		dataset_start->dpp_type = DPP_DATASET_TYPE_NORMAL;
	}
}

VOID FillResyncsetStartCmd(IOMirrorCmd *cmd, GroupInfo *group_info, VolInfo *vol_info, uint64_t seg_offset, uint64_t seg_size)
{
	cmd->header.magic = DPP_MAGIC;
	cmd->header.cmd_type = DPP_TYPE_RESYNCSET_START;
	cmd->header.flags = DPP_FLAG_SOURCE_IOMIRROR;
	cmd->header.sequence_num = group_info->sequence_id++;
	cmd->header.body_len = sizeof(DPP_RESYNCSET_START) + sizeof(DPP_VOL_VEC_ENTRY);

	PDPP_RESYNCSET_START resyncset_start = (PDPP_RESYNCSET_START)cmd->data;
	resyncset_start->resyncset_id = group_info->dataset_id++;
	RtlCopyMemory(resyncset_start->vm_id, group_info->protect_strategy.osId, VM_ID_LEN);
	resyncset_start->num_vols = 1;

	RtlCopyMemory(resyncset_start->vol_entry[0].vol_id, vol_info->vol_id, VOL_ID_LEN);
	resyncset_start->vol_entry[0].vol_offset = seg_offset;
	resyncset_start->vol_entry[0].seg_size = seg_size;
}

VOID FillHeartbeatCmd(IOMirrorCmd *cmd, GroupInfo *group_info)
{
	cmd->header.magic = DPP_MAGIC;
	cmd->header.cmd_type = DPP_TYPE_HEARTBEAT;
	cmd->header.flags = DPP_FLAG_SOURCE_IOMIRROR;
	cmd->header.sequence_num = group_info->sequence_id++;
	cmd->header.body_len = 0;
}

VOID FillActivityCmd(IOMirrorCmd *cmd, GroupInfo *group_info, uint64_t cbt_backlog, uint64_t resync_remaining)
{
	cmd->header.magic = DPP_MAGIC;
	cmd->header.cmd_type = DPP_TYPE_ATTENTION;
	cmd->header.flags = DPP_FLAG_SOURCE_IOMIRROR;
	cmd->header.sequence_num = group_info->sequence_id++;
	cmd->header.body_len = sizeof(DPP_ATTENTION) + sizeof(DPP_ATTENTION_PAYLOAD_ACTIVITY);

	PDPP_ATTENTION dpp_attention = (PDPP_ATTENTION)cmd->data;
	memset(dpp_attention, 0, sizeof(DPP_ATTENTION));
	RtlCopyMemory(dpp_attention->vm_id, group_info->protect_strategy.osId, VM_ID_LEN);
	dpp_attention->operation = DPP_ATTENTION_OPERATION_ACTIVITY;
	dpp_attention->payload_len = sizeof(DPP_ATTENTION_PAYLOAD_ACTIVITY);

	PDPP_ATTENTION_PAYLOAD_ACTIVITY att_activity = (PDPP_ATTENTION_PAYLOAD_ACTIVITY)dpp_attention->payload;
	att_activity->cbt_backlog = cbt_backlog;
	att_activity->resync_remaining = resync_remaining;
}

VOID FillSessionLoginCmd(IOMirrorCmd *cmd, GroupInfo *group_info)
{
	cmd->header.magic = DPP_MAGIC;
	cmd->header.cmd_type = DPP_TYPE_SESSION_LOGIN;
	cmd->header.flags = DPP_FLAG_SOURCE_IOMIRROR;
	cmd->header.sequence_num = group_info->sequence_id++;
	cmd->header.body_len = DPP_SESSION_LOGIN_LEN;

	PDPP_SESSION_LOGIN session_login = (PDPP_SESSION_LOGIN)cmd->data;
	memset(session_login, 0, DPP_SESSION_LOGIN_LEN);
	session_login->version = 1;
	RtlCopyMemory(session_login->vm_id, group_info->protect_strategy.osId, VM_ID_LEN);
	session_login->auth_len = DPP_SESSION_LOGIN_AUTH_LEN;
	session_login->aux_login_len = DPP_SESSION_LOGIN_AUX_LEN;

	PDPP_SESSION_LOGIN_AUX login_aux = (PDPP_SESSION_LOGIN_AUX)session_login->payload;
	login_aux->max_dataset_size = IM_DEFAULT_MAX_DATASET_SIZE;
	login_aux->dataset_id_sent = group_info->dataset_id - 1;
	login_aux->dataset_id_done = group_info->dataset_id_done;
}

VOID FillFlushCmd(IOMirrorCmd *cmd, GroupInfo *group_info)
{
	cmd->header.magic = DPP_MAGIC;
	cmd->header.cmd_type = DPP_TYPE_FLUSH;
	cmd->header.flags = DPP_FLAG_SOURCE_IOMIRROR;
	cmd->header.sequence_num = group_info->sequence_id++;
	cmd->header.body_len = sizeof(DPP_FLUSH);

	PDPP_FLUSH  dpp_flush = (PDPP_FLUSH)cmd->data;
	RtlCopyMemory(dpp_flush->vm_id, group_info->protect_strategy.osId, VM_ID_LEN);
}

#pragma LOCKEDCODE
VOID* MallocCmdInList(NPAGED_LOOKASIDE_LIST* list, uint32_t length)
{
	VOID *cmd = NULL;

	if (length > MAX_BLOCK_SIZE)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Big length %d", length);
	}
	else
	{
		cmd = ExAllocateFromNPagedLookasideList(list);
		if (NULL == cmd)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc data failed, size %d", length);
		}
	}

	return cmd;
}

#pragma LOCKEDCODE
VOID* MallocCmd(uint32_t length, uint32_t msg_len, GroupInfo *group_info)
{
	IOMirrorCmd *cmd = NULL;

	if (length > NINTH_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->max_page_list, length);
	}
	else if (length > EIGHTH_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->ninth_level_page_list, length);
	}
	else if (length > SEVENTH_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->eighth_level_page_list, length);
	}
	else if (length > SIXTH_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->seventh_level_page_list, length);
	}
	else if (length > FIFTH_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->sixth_level_page_list, length);
	}
	else if (length > FOURTH_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->fifth_level_page_list, length);
	}
	else if (length > THIRD_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->fourth_level_page_list, length);
	}
	else if (length > SECOND_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->third_level_page_list, length);
	}
	else if (length > FIRST_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->second_level_page_list, length);
	}
	else if (length > GROUND_LEVEL_BLOCK_SIZE)
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->first_level_page_list, length);
	}
	else
	{
		cmd = (IOMirrorCmd *)MallocCmdInList(&group_info->ground_level_page_list, length);
	}

	if (cmd)
	{
		cmd->header.body_len = length + msg_len;
	}

	return cmd;
}

/*++

函数描述:

    为I/O报文分配空间

参数:

    length: 数据大小
    group_info : 保护组信息
    pdx: 截获I/O驱动的扩展对象
	vol_info：分区信息

返回值:

    分配的空间首地址

--*/
#pragma LOCKEDCODE
VOID* MallocCmdNode(uint32_t length, uint32_t msg_len, GroupInfo *group_info, VolInfo *vol_info, LONG io_status)
{
	CmdNode *cmd_node = (CmdNode *)ExAllocateFromNPagedLookasideList(&group_info->node_npage_list);
	if (NULL == cmd_node)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd_node failed");
		return NULL;
	}
	IOMirrorCmd *cmd = (IOMirrorCmd *)MallocCmd(length, msg_len, group_info);
	if (NULL == cmd)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd_node failed");
		goto error;
	}
	cmd_node->cmd = cmd;
	cmd_node->vol_info = vol_info;
	if (vol_info)
	{
		InterlockedIncrement(&vol_info->reference_count);
	}
	cmd_node->io_status = io_status;
	cmd_node->group_info = group_info;

	return cmd_node;

error:
	ExFreeToNPagedLookasideList(&group_info->node_npage_list, cmd_node);
	return NULL;
}

#pragma LOCKEDCODE
VOID FreeCmd(IOMirrorCmd *cmd, GroupInfo *group_info)
{
	if (cmd->header.body_len > MAX_PAGED_ALLOC_SIZE)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Big body_len %d", cmd->header.body_len);
	}
	else if (cmd->header.body_len > NINTH_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->max_page_list, cmd);
	}
	else if (cmd->header.body_len > EIGHTH_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->ninth_level_page_list, cmd);
	}
	else if (cmd->header.body_len > SEVENTH_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->eighth_level_page_list, cmd);
	}
	else if (cmd->header.body_len > SIXTH_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->seventh_level_page_list, cmd);
	}
	else if (cmd->header.body_len > FIFTH_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->sixth_level_page_list, cmd);
	}
	else if (cmd->header.body_len > FOURTH_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->fifth_level_page_list, cmd);
	}
	else if (cmd->header.body_len > THIRD_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->fourth_level_page_list, cmd);
	}
	else if (cmd->header.body_len > SECOND_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->third_level_page_list, cmd);
	}
	else if (cmd->header.body_len > FIRST_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->second_level_page_list, cmd);
	}
	else if(cmd->header.body_len > GROUND_PAGE_ALLOC_SIZE)
	{
		ExFreeToNPagedLookasideList(&group_info->first_level_page_list, cmd);
	}
	else
	{
		ExFreeToNPagedLookasideList(&group_info->ground_level_page_list, cmd);
	}
}

#pragma LOCKEDCODE
VOID CheckFreeCmd(IOMirrorCmd *cmd, GroupInfo *group_info)
{
	if (cmd)
	{
		FreeCmd(cmd, group_info);
	}
}

/*++

函数描述:

    释放为I/O报文空间

参数:

    cmd_node: I/O报文内存首地址
    group_info : 保护组信息

返回值:

    分配的空间首地址

--*/
#pragma LOCKEDCODE
VOID FreeCmdNode(CmdNode *cmd_node, GroupInfo *group_info)
{
	if (cmd_node->vol_info)
	{
		InterlockedDecrement(&cmd_node->vol_info->reference_count);
	}
	FreeCmd(cmd_node->cmd, group_info);
	ExFreeToNPagedLookasideList(&group_info->node_npage_list, cmd_node);
}

/*++

函数描述:

    为截获的I/O分配空间，数据结构为queue

参数:

    length: 数据大小
    group_info : 保护组信息
    pdx: 截获I/O驱动的扩展对象
	vol_info：分区信息

返回值:

    分配的空间首地址

--*/
#pragma LOCKEDCODE
QueueInfo* BuildCmdQueue(uint32_t length, uint32_t msg_len, GroupInfo *group_info, VolInfo *vol_info)
{
	QueueInfo *queue_info = InitQueueInfo(group_info);
	if (NULL == queue_info)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init queue_info failed");
		return NULL;
	}

	uint32_t residual = length;
	while (residual > 0)
	{
		CmdNode *cmd_node = NULL;
		// 如果剩余长度大于2M，则直接分配2M空间，如果小于2M那么，大于4K，则分配2M空间；小于等于4K，则分配4K空间
		if (residual > MAX_BLOCK_SIZE)
		{
			cmd_node = (CmdNode *)MallocCmdNode(MAX_BLOCK_SIZE, msg_len, group_info, vol_info, IO_COMPLETION_STATUS_ON_GOING);
			if (NULL == cmd_node)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd_node failed, size %d", MAX_BLOCK_SIZE);
				goto error;
			}
			residual -= MAX_BLOCK_SIZE;
		}
		else
		{
			cmd_node = (CmdNode *)MallocCmdNode(residual, msg_len, group_info, vol_info, IO_COMPLETION_STATUS_ON_GOING);
			if (NULL == cmd_node)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd_node failed, size %d", residual);
				goto error;
			}
			residual = 0;
		}
		InsertTailList(&queue_info->head, &cmd_node->list_entry);
		queue_info->num++;
		queue_info->size += (cmd_node->cmd->header.body_len);
	}
	return queue_info;

error:
	FreeCmdQueueNLock(queue_info, group_info);
	return NULL;
}

BOOLEAN FindVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info)
{
	VolInfo *vol_info = NULL;

	QueueInfo* delay_del_queue = vol_queue->group_info->delay_del_vol_queue;

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->end_pos == find_vol_info->end_pos 
			&& vol_info->start_pos == find_vol_info->start_pos)
		{
			if (vol_info->pdx == find_vol_info->pdx)
			{
				break;
			}

			if (wcscmp(vol_info->pdx->disk_name.Buffer, find_vol_info->pdx->disk_name.Buffer) == 0)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::PDX dif, name same, name = %wZ", &vol_info->pdx->disk_name);
				RemoveEntryList(req_entry);
				vol_queue->num--;
				if (InterlockedCompareExchange(&vol_info->reference_count, 0, 0) != 0)
				{
					PushVolQueue(delay_del_queue, vol_info);
					TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::vol_info reference count %d is not 0, delay delete it", vol_info->reference_count);
				}
				else
				{
					FreeVolInfo(vol_info);
				}
			}
		}

		vol_info = NULL;
		req_entry = temp_req_entry;
	}

	ReleaseResource(&vol_queue->sync_resource);

	return (vol_info != NULL);
}

/*++

函数描述:

    创建分区信息结构体

参数:

    pdx: 分区所在驱动的扩展对象
    group_info : 保护组信息
	strategy：保护策略

返回值:

    分区信息结构体地址

--*/
VolInfo *BuildVolInfo(PDEVICE_EXTENSION pdx, GroupInfo *group_info, ProtectVolInter *vol)
{
	VolInfo *vol_info = (VolInfo *)ExAllocatePoolWithTag(NonPagedPool, sizeof(VolInfo), ALLOC_TAG);
	if (NULL == vol_info)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc vol_info failed");
		goto fail;
	}
	memset(vol_info, 0, sizeof(VolInfo));

	vol_info->pdx = pdx;
	vol_info->entire_disk = vol->entire_disk;
	RtlCopyMemory(vol_info->vol_id, vol->vol_id, VOL_ID_LEN);

	if (vol_info->entire_disk)
	{
		vol_info->start_pos = 0;
		vol_info->end_pos = pdx->disk_size;
	}
	else
	{
	}

	vol_info->sectors = (vol_info->end_pos - vol_info->start_pos) / IM_SECTOR_SIZE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Build vol, name = %wZ, start = %llu, end = %llu, sectors = %d, entire_disk %d", &vol_info->pdx->disk_name, vol_info->start_pos, vol_info->end_pos, (ULONG)vol_info->sectors, vol_info->entire_disk);
	
	vol_info->disk_name.MaximumLength = DISK_NAME_LENGTH;
	vol_info->disk_name.Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPool, DISK_NAME_LENGTH, ALLOC_TAG);
	if (NULL == vol_info->disk_name.Buffer)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc disk_name_buffer failed");
		goto fail;
	}
	RtlZeroMemory(vol_info->disk_name.Buffer, DISK_NAME_LENGTH);
	RtlCopyUnicodeString(&vol_info->disk_name, &pdx->disk_name);

	/* 初始化bitmap */
	vol_info->bitmap = BitmapAlloc(vol_info->sectors, group_info->bitmap_granularity, BitmapAlloc, BitmapFree);
	if (NULL == vol_info->bitmap)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::BitmapAlloc failed");
		goto fail;
	}
	vol_info->hbi = (OM_BITMAP_IT *)ExAllocatePoolWithTag(NonPagedPool, sizeof(OM_BITMAP_IT), ALLOC_TAG);
	if (NULL == vol_info->hbi)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc hbi failed");
		goto fail;
	}
	memset(vol_info->hbi, 0, sizeof(OM_BITMAP_IT));
	BitmapItInit(vol_info->hbi, vol_info->bitmap, 0);

	// 新加的卷将cbt_bitmap全置为1
	if (vol->need_set_flag == TRUE)
	{
		BitmapSetBit(vol_info->bitmap, 0, vol_info->sectors);
		vol_info->persist_data_apply_state = VOL_PERSIST_DATA_APPLY_STATE_DONE;
	}
	else
	{
		vol_info->persist_data_apply_state = VOL_PERSIST_DATA_APPLY_STATE_NOT_START;
	}

	vol_info->persist_data = NULL;
	vol_info->persist_len = 0;

	vol_info->reference_count = 0;

	return vol_info;

fail:
	if (NULL != vol_info)
	{
		if (NULL != vol_info->bitmap)
		{
			BitmapFree(vol_info->bitmap, BitmapFree);
		}
		if (NULL != vol_info->hbi)
		{
			ExFreePoolWithTag(vol_info->hbi, ALLOC_TAG);
		}
		if (NULL != vol_info->disk_name.Buffer)
		{
			ExFreePoolWithTag(vol_info->disk_name.Buffer, ALLOC_TAG);
		}
		ExFreePoolWithTag(vol_info, ALLOC_TAG);
	}

	return NULL;
}

/*++

函数描述:

    释放分区信息，包括里面的bitmap和hbi

参数:

	info：分区信息内存首地址

返回值:

    无

--*/
VOID FreeVolInfo(VolInfo *vol_info)
{
	if (vol_info->bitmap)
	{
		BitmapFree(vol_info->bitmap, BitmapFree);
		vol_info->bitmap = NULL;
	}
	if (vol_info->hbi)
	{
		ExFreePoolWithTag(vol_info->hbi, ALLOC_TAG);
		vol_info->hbi = NULL;
	}
	if (vol_info->bitmap_verify)
	{
		BitmapFree(vol_info->bitmap_verify, BitmapFree);
	}
	if (vol_info->hbi_verify)
	{
		ExFreePoolWithTag(vol_info->hbi_verify, ALLOC_TAG);
		vol_info->hbi_verify = NULL;
	}
	if (NULL != vol_info->disk_name.Buffer)
	{
		ExFreePoolWithTag(vol_info->disk_name.Buffer, ALLOC_TAG);
		vol_info->disk_name.Buffer = NULL;
		vol_info->disk_name.Length = vol_info->disk_name.MaximumLength = 0;
	}

	if (vol_info->persist_data != NULL)
	{
		ExFreePoolWithTag(vol_info->persist_data, ALLOC_TAG);
		vol_info->persist_data = NULL;
	}

	ExFreePoolWithTag(vol_info, ALLOC_TAG);
}

VOID FreeDeviceExtension(PDEVICE_EXTENSION pdx)
{
	FreeUnicodeString(&pdx->disk_name);
	if (pdx->hook_device)
	{
		ObDereferenceObject(pdx->hook_device);
	}

	if (pdx->need_clean)
	{
		ExFreePoolWithTag(pdx, ALLOC_TAG);
	}
}

VOID FreeDevExtInfo(PDeviceExtInfo dev_ext_info)
{
	FreeDeviceExtension(dev_ext_info->pdx);
	ExFreePoolWithTag(dev_ext_info, ALLOC_TAG);
}

VOID FreeDriverHookInfo(PDRIVER_HOOK_ENTRY driver_hook_info)
{
	ExFreePoolWithTag(driver_hook_info, ALLOC_TAG);
}

/*++

函数描述:

    将报文结点插入队列的尾部

参数:

	queue_info：节点所要插入的队列
	cmd_node： 所要插入的节点

返回值:

    无

--*/
VOID PushCmdQueue(QueueInfo *cmd_queue, CmdNode *cmd_node)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&cmd_queue->lock, &old_irql);
	InsertTailList(&cmd_queue->head, &cmd_node->list_entry);
	cmd_queue->num++;
	cmd_queue->size += (cmd_node->cmd->header.body_len);
	KeReleaseSpinLock(&cmd_queue->lock, old_irql);
}

VOID PushCmdQueueNLock(QueueInfo *cmd_queue, CmdNode *cmd_node)
{
	InsertTailList(&cmd_queue->head, &cmd_node->list_entry);
	cmd_queue->num++;
	cmd_queue->size += (cmd_node->cmd->header.body_len);
}

/*++

函数描述:

    将报文结点从队列的头部取出

参数:

	queue_info：节点所要插入的队列

返回值:

    取出的节点

--*/
CmdNode *PopCmdQueue(QueueInfo *cmd_queue)
{
	PLIST_ENTRY req_entry = NULL;
	CmdNode *cmd_node = NULL;
	
	KIRQL old_irql;
	KeAcquireSpinLock(&cmd_queue->lock, &old_irql);
	if (!IsListEmpty(&cmd_queue->head))
	{	
		req_entry = RemoveHeadList(&cmd_queue->head);
		cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);
		cmd_queue->num--;
		cmd_queue->size -= (cmd_node->cmd->header.body_len);
	}
	KeReleaseSpinLock(&cmd_queue->lock, old_irql);

	return cmd_node;
}

CmdNode *PopCmdQueueBySize(QueueInfo *cmd_queue, uint32_t size)
{
	CmdNode *cmd_node = NULL;
	BOOLEAN flag = FALSE;

	KIRQL old_irql;
	KeAcquireSpinLock(&cmd_queue->lock, &old_irql);

	do
	{
		PLIST_ENTRY req_entry = NULL;
		if (!IsListEmpty(&cmd_queue->head))
		{
			req_entry = cmd_queue->head.Flink;
			cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);
			if (cmd_node->cmd->header.cmd_type == DPP_TYPE_DATA)
			{
				if (cmd_node->cmd->header.body_len - sizeof(DPP_DATA) > size)
				{
					TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Credit restrain, credit %d, required %d", size, cmd_node->cmd->header.body_len - sizeof(DPP_DATA));
					cmd_node = NULL;
					break;
				}

			}

			req_entry = RemoveHeadList(&cmd_queue->head);
			cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);
			cmd_queue->num--;
			cmd_queue->size -= (cmd_node->cmd->header.body_len);
		}
	} while (flag);

	KeReleaseSpinLock(&cmd_queue->lock, old_irql);

	return cmd_node;
}

BOOL IsEmptyCmdQueueNLock(QueueInfo *cmd_queue)
{
	BOOL ret = IsListEmpty(&cmd_queue->head);
	if (ret)
	{
		if (cmd_queue->num != 0)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Cmd queue is empty but queue_num = %d", cmd_queue->num);
			cmd_queue->num = 0;
		}
	}

	return ret;
}

BOOL IsEmptyCmdQueue(QueueInfo *cmd_queue)
{
	BOOL ret = FALSE;

	KIRQL old_irql;
	KeAcquireSpinLock(&cmd_queue->lock, &old_irql);
	ret = IsEmptyCmdQueueNLock(cmd_queue);
	KeReleaseSpinLock(&cmd_queue->lock, old_irql);

	return ret;
}

uint64_t GetCmdQueueSize(QueueInfo *cmd_queue)
{
	uint64_t size = 0;

	KIRQL old_irql;
	KeAcquireSpinLock(&cmd_queue->lock, &old_irql);
	size = cmd_queue->size;
	KeReleaseSpinLock(&cmd_queue->lock, old_irql);
	return size;
}

CmdNode *PopCmdQueueNLock(QueueInfo *cmd_queue)
{
	PLIST_ENTRY req_entry = NULL;
	CmdNode *cmd_node = NULL;

	if (TRUE == IsListEmpty(&cmd_queue->head))
	{
		return NULL;
	}
	req_entry = RemoveHeadList(&cmd_queue->head);
	cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);
	cmd_queue->num--;
	cmd_queue->size -= (cmd_node->cmd->header.body_len);

	return cmd_node;
}

BOOL IsEmptyVolQueue(QueueInfo *vol_queue)
{
	BOOLEAN ret = FALSE;

	AcquireShareResource(&vol_queue->sync_resource);

	if (TRUE == IsListEmpty(&vol_queue->head))
	{
		if (vol_queue->num != 0)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Vol queue is empty but queue_num = %d", vol_queue->num);
			vol_queue->num = 0;
		}
		ret = TRUE;
	}

	ReleaseResource(&vol_queue->sync_resource);

	return ret;
}

BOOL ModifyVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info)
{
    BOOLEAN ret = FALSE;

    AcquireExclusiveResource(&vol_queue->sync_resource);
    KIRQL old_irql;
    KeAcquireSpinLock(&vol_queue->lock, &old_irql);

    PLIST_ENTRY head = &vol_queue->head;
    PLIST_ENTRY req_entry = head->Flink;
    while (req_entry != head)
    {
        PLIST_ENTRY temp_req_entry = req_entry->Flink;
        VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
        if (vol_info->end_pos == find_vol_info->end_pos
            && vol_info->start_pos == find_vol_info->start_pos
            && vol_info->pdx == find_vol_info->pdx)
        {
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::modify vol id, old volid %!HEXDUMP!, new vol id %!HEXDUMP!.",
				LOG_LENSTR(vol_info->vol_id, VM_ID_LEN), LOG_LENSTR(find_vol_info->vol_id, VM_ID_LEN));
            // 当前只支持修改vol id
            RtlCopyMemory(vol_info->vol_id, find_vol_info->vol_id, VOL_ID_LEN);
            ret = TRUE;
            break;
        }
        req_entry = temp_req_entry;
    }

    KeReleaseSpinLock(&vol_queue->lock, old_irql);
    ReleaseResource(&vol_queue->sync_resource);

    return ret;
}

/*++

函数描述:

    获取队列内的节点个数

参数:

	queue_info：节点所要插入的队列

返回值:

    队列内的节点个数

--*/
uint32_t GetQueueNum(QueueInfo *queue_info)
{
	uint32_t num = queue_info->num;
	return num;
}

#pragma LOCKEDCODE
VOID ClearCmdQueueNLock(QueueInfo *cmd_queue, GroupInfo *group_info)
{
	PLIST_ENTRY head = &cmd_queue->head;
	PLIST_ENTRY req_entry = NULL;

	req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		CmdNode *cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);
		RemoveEntryList(req_entry);
		cmd_queue->num--;
		cmd_queue->size -= (cmd_node->cmd->header.body_len);
		FreeCmdNode(cmd_node, group_info);
		req_entry = temp_req_entry;
	}
}

/*++

函数描述:

    清空队列

参数:

	queue_info：所要清空的队列
	group_info：保护组

返回值:

    无

--*/
BOOLEAN ClearCmdQueueDropBack(QueueInfo *cmd_queue, GroupInfo *group_info)
{
	BOOL drop_back = FALSE;

	QueueInfo* delay_del_queue = group_info->delay_del_cmd_queue;

	KIRQL old_irql;
	KeAcquireSpinLock(&cmd_queue->lock, &old_irql);
	uint32_t queue_num = GetQueueNum(cmd_queue);
	for (uint32_t i = 0; i < queue_num; i++)
	{
		CmdNode *cmd_node = PopCmdQueueNLock(cmd_queue);
		if (NULL == cmd_node)
		{
			break;
		}

		if (cmd_node->cmd->header.cmd_type == DPP_TYPE_DATA)
		{
			if (InterlockedCompareExchange(&cmd_node->io_status, 0, 0) == IO_COMPLETION_STATUS_ON_GOING)
			{
				if (delay_del_queue == cmd_queue)
				{
					PushCmdQueueNLock(delay_del_queue, cmd_node);
					drop_back = TRUE;
				}
				else
				{
					PushCmdQueue(delay_del_queue, cmd_node);
				}

				cmd_node = NULL;
			}
		}

		if (cmd_node)
		{
			FreeCmdNode(cmd_node, group_info);
		}
	}

	KeReleaseSpinLock(&cmd_queue->lock, old_irql);

	return drop_back;
}

VOID ClearCmdQueue(QueueInfo *cmd_queue, GroupInfo *group_info)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Clear cmd queue");

	BOOL drop_back = FALSE;
	do
	{
		drop_back = ClearCmdQueueDropBack(cmd_queue, group_info);
		if (drop_back)
		{
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Io for cmd is still pending finish, wait for a while");
			ImSleep(RETRY_TIME);
		}		
	} while (drop_back);
}

// 插入到尾部
VOID FlushCmdQueue(QueueInfo *src_queue, QueueInfo *dst_queue)
{
	KIRQL old_irql_src;
	KeAcquireSpinLock(&src_queue->lock, &old_irql_src);

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Enter FlushCmdQueue, src_queue_num = %d, dst_queue_num = %d", src_queue->num, dst_queue->num);

	if (src_queue->num == 0 || src_queue == dst_queue)
	{
		KeReleaseSpinLock(&src_queue->lock, old_irql_src);
		return;
	}

	KIRQL old_irql_dst;
	KeAcquireSpinLock(&dst_queue->lock, &old_irql_dst);
	src_queue->head.Flink->Blink = dst_queue->head.Blink;
	dst_queue->head.Blink->Flink = src_queue->head.Flink;
	src_queue->head.Blink->Flink = &dst_queue->head;	
	dst_queue->head.Blink = src_queue->head.Blink;
	dst_queue->num += src_queue->num;
	dst_queue->size += src_queue->size;
	src_queue->num = 0;
	src_queue->size = 0;
	InitializeListHead(&src_queue->head);
	KeReleaseSpinLock(&dst_queue->lock, old_irql_dst);
	KeReleaseSpinLock(&src_queue->lock, old_irql_src);
}

BOOLEAN RemovePendingCmd(GroupInfo* group_info, uint64_t dataset_id)
{
	BOOLEAN ret = TRUE;

	QueueInfo* queue_info = group_info->pending_cmd_queue;

	if (GetQueueNum(queue_info) == 0)
	{
		return ret;
	}

	TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Begin remove pending cmd queue, queue_num = %d", queue_info->num);

	PLIST_ENTRY head = &queue_info->head;
	PLIST_ENTRY req_entry = NULL;
	PLIST_ENTRY end_entry = NULL;
	PLIST_ENTRY set_entry = NULL;

	req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		CmdNode *cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);

		if (InterlockedCompareExchange(&cmd_node->io_status, 0, 0) == IO_COMPLETION_STATUS_FAILED)
		{
			ret = FALSE;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::The cmd %llu failed", cmd_node->cmd->header.sequence_num);
			break;
		}

		if (cmd_node->cmd->header.cmd_type == DPP_TYPE_DATA)
		{
			if (InterlockedCompareExchange(&cmd_node->io_status, 0, 0) == IO_COMPLETION_STATUS_ON_GOING)
			{
				end_entry = set_entry;
			}
		}
		else
		{
			set_entry = req_entry;

			PDPP_DATASET_START dataset_start = (PDPP_DATASET_START)cmd_node->cmd->data;
			if (dataset_start->dataset_id == dataset_id)
			{
				InterlockedExchange(&cmd_node->io_status, IO_COMPLETION_STATUS_SUCCEED);
				if (end_entry != NULL)
				{
					break;
				}
			}
			else if (dataset_start->dataset_id > dataset_id)
			{
				if (end_entry == NULL)
				{
					end_entry = set_entry;
				}

				break;
			}

			if (InterlockedCompareExchange(&cmd_node->io_status, 0, 0) != IO_COMPLETION_STATUS_SUCCEED)
			{
				ret = FALSE;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::The dataset start cmd %llu has not finished yet", cmd_node->cmd->header.sequence_num);
				break;
			}
		}

		req_entry = temp_req_entry;
	}

	if (!ret)
	{
		return ret;
	}

	if (end_entry == NULL)
	{
		return ret;
	}

	req_entry = head->Flink;
	while (req_entry != end_entry)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		CmdNode *cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);

		RemoveEntryList(req_entry);
		queue_info->num--;
		queue_info->size -= (cmd_node->cmd->header.body_len);

		FreeCmdNode(cmd_node, group_info);

		req_entry = temp_req_entry;
	}

	TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::End remove pending cmd queue, queue_num = %d, queue_size = %llu", queue_info->num, queue_info->size);

	return ret;
}

/*++

函数描述:

    初始化队列，包括分配内存，初始化头，锁

参数:

	无

返回值:

    分配好的队列

--*/
#pragma LOCKEDCODE
QueueInfo *InitQueueInfo(GroupInfo *group_info)
{
	// 初始化队列数据
	QueueInfo *queue_info = (QueueInfo *)ExAllocateFromNPagedLookasideList(&group_info->queue_info_npage_list);
	if (NULL == queue_info)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc queue_info failed");
		return NULL;
	}
	InitializeListHead(&queue_info->head);
	KeInitializeSpinLock(&queue_info->lock);//使用后irql提升到DISPATCH级别

	if (KeGetCurrentIrql() <= APC_LEVEL)
	{
		ExInitializeResourceLite(&queue_info->sync_resource);
	}

	queue_info->num = 0;
	queue_info->size = 0;
	queue_info->group_info = group_info;

	return queue_info;
}

#pragma LOCKEDCODE
VOID UninitQueueInfo(QueueInfo* queue_info)
{
	if (KeGetCurrentIrql() <= APC_LEVEL)
	{
		ExDeleteResourceLite(&queue_info->sync_resource);
	}
}

#pragma LOCKEDCODE
VOID FreeCmdQueueNLock(QueueInfo *cmd_queue, GroupInfo *group_info)
{
	ClearCmdQueueNLock(cmd_queue, group_info);
	UninitQueueInfo(cmd_queue);
	ExFreeToNPagedLookasideList(&group_info->queue_info_npage_list, cmd_queue);
}

VOID FreeCmdQueue(QueueInfo *cmd_queue, GroupInfo *group_info)
{
	ClearCmdQueue(cmd_queue, group_info);
	UninitQueueInfo(cmd_queue);
	ExFreeToNPagedLookasideList(&group_info->queue_info_npage_list, cmd_queue);
}

VOID FreeVolQueue(QueueInfo *vol_queue, GroupInfo *group_info)
{
	ClearVolQueue(vol_queue);
	UninitQueueInfo(vol_queue);
	ExFreeToNPagedLookasideList(&group_info->queue_info_npage_list, vol_queue);
}

VOID FreeDeviceExtQueue(QueueInfo *dev_ext_queue, GroupInfo *group_info)
{
	ClearDeviceExtQueue(dev_ext_queue);
	UninitQueueInfo(dev_ext_queue);
	ExFreeToNPagedLookasideList(&group_info->queue_info_npage_list, dev_ext_queue);
}

VOID FreeDriverHookQueue(QueueInfo *drv_hook_queue, GroupInfo *group_info)
{
	ClearDriverHookQueue(drv_hook_queue);
	UninitQueueInfo(drv_hook_queue);
	ExFreeToNPagedLookasideList(&group_info->queue_info_npage_list, drv_hook_queue);
}

PDEVICE_EXTENSION FindDeviceExtensionByDiskNum(QueueInfo *dev_ext_queue, uint32_t disk_num)
{
	PDEVICE_EXTENSION pdx = NULL;

	KIRQL old_irql;
	KeAcquireSpinLock(&dev_ext_queue->lock, &old_irql);

	PLIST_ENTRY head = &dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	PDeviceExtInfo dev_ext_info = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;

		dev_ext_info = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if (dev_ext_info->pdx->disk_number == (ULONG)disk_num)
		{
			pdx = dev_ext_info->pdx;
			break;
		}
		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&dev_ext_queue->lock, old_irql);

	return pdx;
}

PDEVICE_EXTENSION FindDeviceExtensionByHookDevice(QueueInfo *dev_ext_queue, PDEVICE_OBJECT hook_dev)
{
	PDEVICE_EXTENSION pdx = NULL;

	KIRQL old_irql;
	KeAcquireSpinLock(&dev_ext_queue->lock, &old_irql);

	PLIST_ENTRY head = &dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	PDeviceExtInfo dev_ext_info = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;

		dev_ext_info = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if (dev_ext_info->pdx->hook_device == hook_dev)
		{
			pdx = dev_ext_info->pdx;
			break;
		}
		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&dev_ext_queue->lock, old_irql);

	return pdx;
}

VOID LockDriverHookQueue(QueueInfo* driver_hook_queue)
{
	AcquireShareResource(&driver_hook_queue->sync_resource);
}

VOID ReleaseDriverHookQueue(QueueInfo* driver_hook_queue)
{
	ReleaseResource(&driver_hook_queue->sync_resource);
}

PDRIVER_HOOK FindHookByDriverObjectNLock(QueueInfo *driver_hook_queue, PDRIVER_OBJECT drv_obj)
{
	PDRIVER_HOOK drv_hook = NULL;

	PLIST_ENTRY head = &driver_hook_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	PDRIVER_HOOK_ENTRY drv_hook_info = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;

		drv_hook_info = CONTAINING_RECORD(req_entry, DRIVER_HOOK_ENTRY, list_entry);
		if (drv_hook_info->driver_hook.hook_driver == drv_obj)
		{
			drv_hook = &drv_hook_info->driver_hook;
			break;
		}
		req_entry = temp_req_entry;
	}

	return drv_hook;
}

BOOL FindHookByDriverObject(QueueInfo *driver_hook_queue, PDRIVER_OBJECT drv_obj)
{
	BOOLEAN ret = FALSE;

	LockDriverHookQueue(driver_hook_queue);
	PDRIVER_HOOK driver_hook = FindHookByDriverObjectNLock(driver_hook_queue, drv_obj);
	if (driver_hook)
	{
		ret = TRUE;
	}
	ReleaseDriverHookQueue(driver_hook_queue);

	return ret;
}

NTSTATUS InitSocketInfo(SocketInfo *socket_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	socket_info->rev_buf = (char *)ExAllocatePoolWithTag(NonPagedPool, REV_BUF_SIZE, ALLOC_TAG);
	if (NULL == socket_info->rev_buf)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc socket_info->rev_buf failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto fail;
	}
	memset(socket_info->rev_buf, 0, REV_BUF_SIZE);
	socket_info->flags = 0;
	socket_info->rev_event = (PKEVENT)ExAllocatePoolWithTag(NonPagedPool, sizeof(KEVENT), ALLOC_TAG);
	if (NULL == socket_info->rev_event)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc socket_info->rev_event failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto fail;
	}
	KeInitializeEvent(socket_info->rev_event, SynchronizationEvent, FALSE);
	socket_info->rev_io_status = (PIO_STATUS_BLOCK)ExAllocatePoolWithTag(NonPagedPool, sizeof(IO_STATUS_BLOCK), ALLOC_TAG);
	if (NULL == socket_info->rev_io_status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc socket_info->rev_io_status failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto fail;
	}
	memset(socket_info->rev_io_status, 0, sizeof(IO_STATUS_BLOCK));
	socket_info->is_receiving = FALSE;
	socket_info->rev_head_mdl = IoAllocateMdl(socket_info->rev_buf, CMD_HEAD, FALSE, FALSE, NULL);
	if(NULL == socket_info->rev_head_mdl)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc mdl failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto fail;
	}

	__try
	{
		MmProbeAndLockPages(socket_info->rev_head_mdl, KernelMode, IoModifyAccess);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		IoFreeMdl(socket_info->rev_head_mdl);
		socket_info->rev_head_mdl = NULL;

		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Lock page failed");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto fail;
	}

	return status;

fail:

	if (socket_info->rev_head_mdl)
	{
		MmUnlockPages(socket_info->rev_head_mdl);
		IoFreeMdl(socket_info->rev_head_mdl);
		socket_info->rev_head_mdl = NULL;
	}

	if (NULL != socket_info->rev_buf)
	{
		ExFreePoolWithTag(socket_info->rev_buf, ALLOC_TAG);
		socket_info->rev_buf = NULL;
	}
	if (NULL != socket_info->rev_event)
	{
		ExFreePoolWithTag(socket_info->rev_event, ALLOC_TAG);
		socket_info->rev_event = NULL;
	}
	if (NULL != socket_info->rev_io_status)
	{
		ExFreePoolWithTag(socket_info->rev_io_status, ALLOC_TAG);
		socket_info->rev_io_status = NULL;
	}
	return status;
}

VOID UnInitSocketInfo(SocketInfo *socket_info)
{
	if (socket_info->rev_head_mdl)
	{
		MmUnlockPages(socket_info->rev_head_mdl);
		IoFreeMdl(socket_info->rev_head_mdl);
		socket_info->rev_head_mdl = NULL;
	}

	if (NULL != socket_info->rev_buf)
	{
		ExFreePoolWithTag(socket_info->rev_buf, ALLOC_TAG);
		socket_info->rev_buf = NULL;
	}
	if (NULL != socket_info->rev_event)
	{
		ExFreePoolWithTag(socket_info->rev_event, ALLOC_TAG);
		socket_info->rev_event = NULL;
	}
	if (NULL != socket_info->rev_io_status)
	{
		ExFreePoolWithTag(socket_info->rev_io_status, ALLOC_TAG);
		socket_info->rev_io_status = NULL;
	}
}

/*++

函数描述:

    初始化保护组，包括初始化队列，初始化内存池，设置数据复制的状态等

参数:

	保护组

返回值:

    STATUS_SUCCESS： 成功
	其它： 失败

--*/
NTSTATUS InitGroupInfo(GroupInfo *group_info, PDEVICE_OBJECT device_obj)
{
	NTSTATUS status;

	// 初始化内存池
	ExInitializeNPagedLookasideList(&group_info->node_npage_list, NULL, NULL, POOL_NX_ALLOCATION, sizeof(CmdNode), NOPAGED_LS_ND_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->queue_info_npage_list, NULL, NULL, POOL_NX_ALLOCATION, sizeof(QueueInfo), NOPAGED_LS_QU_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->first_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, FIRST_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->second_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, SECOND_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->third_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, THIRD_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->fourth_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, FOURTH_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->fifth_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, FIFTH_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->sixth_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, SIXTH_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->seventh_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, SEVENTH_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->eighth_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, EIGHTH_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->ninth_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, NINTH_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->max_page_list, NULL, NULL, POOL_NX_ALLOCATION, MAX_PAGED_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->ground_level_page_list, NULL, NULL, POOL_NX_ALLOCATION, GROUND_PAGE_ALLOC_SIZE, NOPAGED_LS_CD_ALLOC_TAG, 0);
	ExInitializeNPagedLookasideList(&group_info->completion_context_npage_list, NULL, NULL, POOL_NX_ALLOCATION, sizeof(IO_COMP_CONTEXT), NOPAGED_LS_CC_ALLOC_TAG, 0);

	// 初始化队列数据
	group_info->cmd_queue = InitQueueInfo(group_info);
	if (NULL == group_info->cmd_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init queue_info failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->temp_cmd_queue = InitQueueInfo(group_info);
	if (NULL == group_info->temp_cmd_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init temp_queue_info failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->pending_cmd_queue = InitQueueInfo(group_info);
	if (NULL == group_info->pending_cmd_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init pending_cmd_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->delay_del_cmd_queue = InitQueueInfo(group_info);
	if (NULL == group_info->delay_del_cmd_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init delay_del_cmd_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->vol_queue = InitQueueInfo(group_info);
	if (NULL == group_info->vol_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol_info failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->temp_vol_queue = InitQueueInfo(group_info);
	if (NULL == group_info->temp_vol_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init temp_vol_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->delay_del_vol_queue = InitQueueInfo(group_info);
	if (NULL == group_info->delay_del_vol_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init delay_del_vol_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->dev_ext_queue = InitQueueInfo(group_info);
	if (NULL == group_info->dev_ext_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init dev_ext_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->driver_hook_queue = InitQueueInfo(group_info);
	if (NULL == group_info->driver_hook_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init driver_hook_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = InitSocketInfo(&group_info->socket_info);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init socket_info failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KeInitializeTimerEx(&group_info->heart_beat_timer, SynchronizationTimer);
	KeInitializeTimerEx(&group_info->activity_timer, SynchronizationTimer);
	KeInitializeTimerEx(&group_info->dataset_timer, SynchronizationTimer);

	group_info->extern_ctl_buffer = NULL;
	KeInitializeEvent(&group_info->extern_ctl_event, SynchronizationEvent, FALSE);
	group_info->extern_ctl_type = IM_PG_IOCTL_STATE_NORMAL;
	KeInitializeSpinLock(&group_info->extern_ctl_lock);

	group_info->normal_send_buffer = (uint8_t *)ExAllocatePoolWithTag(PagedPool, NORMAL_SEND_BUFFER_SIZE, ALLOC_TAG);
	if (NULL == group_info->normal_send_buffer)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc normal_send_buffer failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->bitmap_granularity = IM_BITMAP_GRANULARITY;
	group_info->state = IM_PG_STATE_STOP;
	group_info->inter_state = IM_PG_INTER_STATE_STOP;
	group_info->boot_flag = TRUE;

	group_info->send_thread_run_flag = FALSE;
	group_info->send_thread_handle = NULL;

	group_info->flow_control_pause_flag = 0;
	group_info->pause_pending = FALSE;
	group_info->resume_pending = FALSE;

	group_info->custom_point_type = CUSTOM_POINT_TYPE_UNKNOWN;

	group_info->send_queue_depth = 0;
	group_info->send_queue_size = 0;
	ExInitializeFastMutex(&group_info->send_queue_mutex);

	group_info->device_obj = device_obj;

	group_info->sequence_id = 0;
	group_info->dataset_id = 1;
	group_info->dataset_id_done = 0;
	group_info->set_data_size = 0;
	group_info->queue_set_data_size = 0;
	group_info->max_set_data_size = 0;
	group_info->seg_size = 0;
	group_info->global_credit = 0;
	group_info->per_data_state = PERSIST_DATA_STATE_UNKNOWN;
	group_info->reg_select_cur = 0;

	group_info->speed_pause_flag = 0;
	group_info->maxspeed = MAX_SEND_QUEUE_SIZE;

	RtlCopyMemory(group_info->group_id, "im_pg", 5);//im_pg长度为5

	KeInitializeGuardedMutex(&group_info->group_lock);

	return STATUS_SUCCESS;
}

VOID DestroyGroupInfo(GroupInfo *group_info)
{
	FreeCmdQueue(group_info->cmd_queue, group_info);
	FreeCmdQueue(group_info->temp_cmd_queue, group_info);
	FreeCmdQueue(group_info->pending_cmd_queue, group_info);
	FreeCmdQueue(group_info->delay_del_cmd_queue, group_info);

	FreeVolQueue(group_info->vol_queue, group_info);
	FreeVolQueue(group_info->temp_vol_queue, group_info);
	FreeVolQueue(group_info->delay_del_vol_queue, group_info);

	FreeDeviceExtQueue(group_info->dev_ext_queue, group_info);
	FreeDriverHookQueue(group_info->driver_hook_queue, group_info);

	ExFreePoolWithTag(group_info->normal_send_buffer, ALLOC_TAG);
	
	ExDeleteNPagedLookasideList(&group_info->node_npage_list);
	ExDeleteNPagedLookasideList(&group_info->queue_info_npage_list);
	ExDeleteNPagedLookasideList(&group_info->first_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->second_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->third_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->fourth_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->fifth_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->sixth_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->seventh_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->eighth_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->ninth_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->max_page_list);
	ExDeleteNPagedLookasideList(&group_info->ground_level_page_list);
	ExDeleteNPagedLookasideList(&group_info->completion_context_npage_list);

	UnInitSocketInfo(&group_info->socket_info);
}

#pragma LOCKEDCODE
GroupInfo* GetGroupInfo(PDEVICE_EXTENSION pdx)
{
	GroupMemInfo *mem_info = (GroupMemInfo *)pdx->mem_info;
	GroupInfo *group_info = mem_info->group_info;

	return group_info;
}


/*++

函数描述:

    分配保护组的内存

参数:

	pdx：驱动设备对象扩展信息

返回值:

    STATUS_SUCCESS： 成功
	其它： 失败

--*/
NTSTATUS CreateGroupMem(PDEVICE_EXTENSION pdx, PDEVICE_OBJECT device_obj, BOOLEAN create_new)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	GroupMemInfo *mem_info_create = NULL;
	PDeviceExtInfo dev_ext_info = NULL;

	do
	{
		if (create_new)
		{
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Alloc and init group info");
			mem_info_create = (GroupMemInfo *)ExAllocatePoolWithTag(NonPagedPool, sizeof(GroupMemInfo), ALLOC_TAG);
			if (mem_info_create == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc mem_info failed, error %!STATUS!", status);
				break;
			}
			memset(mem_info_create, 0, sizeof(GroupMemInfo));
			status = InitGroupInfo(mem_info_create->group_info, device_obj);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::InitGroupInfo failed, error %!STATUS!", status);
				break;
			}
			pdx->mem_info = mem_info_create;
		}
		else
		{
			pdx->mem_info = GetCdoMemInfo();
		}

		dev_ext_info = (PDeviceExtInfo)ExAllocatePoolWithTag(NonPagedPool, sizeof(DeviceExtInfo), ALLOC_TAG);
		if (dev_ext_info == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc dev_ext_info failed, error %!STATUS!", status);
			break;
		}

		dev_ext_info->pdx = pdx;
		PushDeviceExtQueue(((GroupMemInfo *)pdx->mem_info)->group_info->dev_ext_queue, dev_ext_info);
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		if (mem_info_create)
		{
			ExFreePoolWithTag(mem_info_create, ALLOC_TAG);
			mem_info_create = NULL;
		}
	}
	
	return status;
}

VOID ReleaseGroupMem(PDEVICE_EXTENSION pdx, PDEVICE_OBJECT device_obj)
{
	GroupMemInfo* mem_info = (GroupMemInfo*)pdx->mem_info;
	if (mem_info)
	{	
		GroupInfo* group_info = mem_info->group_info;
		if (group_info->device_obj == device_obj)
		{
			DestroyGroupInfo(group_info);
			ExFreePoolWithTag(mem_info, ALLOC_TAG);
		}
	}
	else
	{
		pdx->mem_info = NULL;
	}
}

/**************************************** vol *******************************************/
VOID PushVolQueueNLock(QueueInfo *vol_queue, VolInfo *vol_info)
{
	InsertTailList(&vol_queue->head, &vol_info->list_entry);
	vol_queue->num++;
}

VOID PushVolQueue(QueueInfo *vol_queue, VolInfo *vol_info)
{
	AcquireExclusiveResource(&vol_queue->sync_resource);
	KIRQL old_irql;
	KeAcquireSpinLock(&vol_queue->lock, &old_irql);
	PushVolQueueNLock(vol_queue, vol_info);
	KeReleaseSpinLock(&vol_queue->lock, old_irql);
	ReleaseResource(&vol_queue->sync_resource);
}

VOID ClearVolQueue(QueueInfo *vol_queue)
{
	QueueInfo* delay_del_queue = vol_queue->group_info->delay_del_vol_queue;

	AcquireExclusiveResource(&vol_queue->sync_resource);
	AcquireExclusiveResource(&delay_del_queue->sync_resource);
	KIRQL old_irql;
	KeAcquireSpinLock(&vol_queue->lock, &old_irql);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		RemoveEntryList(req_entry);
		vol_queue->num--;

		if (InterlockedCompareExchange(&vol_info->reference_count, 0, 0) != 0)
		{
			if (delay_del_queue == vol_queue)
			{
				TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::vol_info reference count %d doesn't come to 0 but remove it anyway", vol_info->reference_count);
			}
			else
			{
				PushVolQueueNLock(delay_del_queue, vol_info);
				vol_info = NULL;
			}
		}
		
		if(vol_info)
		{
			FreeVolInfo(vol_info);
		}

		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&vol_queue->lock, old_irql);

	ReleaseResource(&delay_del_queue->sync_resource);
	ReleaseResource(&vol_queue->sync_resource);
}

VOID CheckRemoveDelayDeleteVolQueue(GroupInfo *group_info)
{
	AcquireExclusiveResource(&group_info->delay_del_vol_queue->sync_resource);

	if (GetQueueNum(group_info->delay_del_vol_queue) == 0)
	{
		ReleaseResource(&group_info->delay_del_vol_queue->sync_resource);
		return;
	}

	QueueInfo* queue_info = group_info->delay_del_vol_queue;

	PLIST_ENTRY head = &queue_info->head;
	VolInfo *vol_info = NULL;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (InterlockedCompareExchange(&vol_info->reference_count, 0, 0) == 0)
		{
			RemoveEntryList(&vol_info->list_entry);
			queue_info->num--;
			FreeVolInfo(vol_info);
		}
		vol_info = NULL;
		req_entry = temp_req_entry;
	}

	ReleaseResource(&group_info->delay_del_vol_queue->sync_resource);
}

VOID PushDeviceExtQueue(QueueInfo *dev_ext_queue, PDeviceExtInfo dev_ext_info)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&dev_ext_queue->lock, &old_irql);
	InsertTailList(&dev_ext_queue->head, &dev_ext_info->list_entry);
	dev_ext_queue->num++;
	KeReleaseSpinLock(&dev_ext_queue->lock, old_irql);
}

VOID RemoveDeviceExtByPdx(QueueInfo *dev_ext_queue, PDEVICE_EXTENSION pdx)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&dev_ext_queue->lock, &old_irql);

	DeviceExtInfo *dev_ext_info = NULL;
	PLIST_ENTRY head = &dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		dev_ext_info = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if (dev_ext_info->pdx == pdx)
		{
			break;
		}
		req_entry = req_entry->Flink;
	}

	if (req_entry != head)
	{
		RemoveEntryList(req_entry);
		dev_ext_queue->num--;
		FreeDevExtInfo(dev_ext_info);
	}

	KeReleaseSpinLock(&dev_ext_queue->lock, old_irql);
}

VOID ClearDeviceExtQueue(QueueInfo *dev_ext_queue)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&dev_ext_queue->lock, &old_irql);

	PLIST_ENTRY head = &dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		DeviceExtInfo *dev_ext_info = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		RemoveEntryList(req_entry);
		dev_ext_queue->num--;
		FreeDevExtInfo(dev_ext_info);
		
		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&dev_ext_queue->lock, old_irql);
}

VOID PushDriverHookQueue(QueueInfo* driver_hook_queue, PDRIVER_HOOK_ENTRY hook_entry)
{
	AcquireExclusiveResource(&driver_hook_queue->sync_resource);
	InsertTailList(&driver_hook_queue->head, &hook_entry->list_entry);
	driver_hook_queue->num++;
	ReleaseResource(&driver_hook_queue->sync_resource);
}

VOID ClearDriverHookQueue(QueueInfo *driver_hook_queue)
{
	AcquireExclusiveResource(&driver_hook_queue->sync_resource);
	PLIST_ENTRY head = &driver_hook_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		DRIVER_HOOK_ENTRY *driver_hook = CONTAINING_RECORD(req_entry, DRIVER_HOOK_ENTRY, list_entry);
		RemoveEntryList(req_entry);
		driver_hook_queue->num--;
		FreeDriverHookInfo(driver_hook);
		req_entry = temp_req_entry;
	}
	ReleaseResource(&driver_hook_queue->sync_resource);
}

VOID UndoDriverHookQueue(QueueInfo *driver_hook_queue)
{
	AcquireExclusiveResource(&driver_hook_queue->sync_resource);
	PLIST_ENTRY head = &driver_hook_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		DRIVER_HOOK_ENTRY *driver_hook = CONTAINING_RECORD(req_entry, DRIVER_HOOK_ENTRY, list_entry);
		UnhookDriver(&driver_hook->driver_hook);
		req_entry = temp_req_entry;
	}
	ReleaseResource(&driver_hook_queue->sync_resource);

	LARGE_INTEGER time_out = RtlConvertLongToLargeInteger(HOOK_RELEASE_TIME);
	KeDelayExecutionThread(KernelMode, FALSE, &time_out);
}


/*++

函数描述:

    查找该I/O报文的起始点是否落在所要保护的分区范围之内

参数:

	pdx：驱动设备对象扩展信息
	start_pos： I/O报文的起始位置

返回值:

	分区信息，如果没找到，则返回NULL

--*/
#pragma LOCKEDCODE
VolInfo *NeedProtected(PDEVICE_EXTENSION pdx, uint64_t start_pos)
{
	VolInfo *vol_info = NULL;

	GroupInfo *group_info = GetGroupInfo(pdx);

	KIRQL old_irql;
	KeAcquireSpinLock(&group_info->vol_queue->lock, &old_irql);

	PLIST_ENTRY head = &group_info->vol_queue->head;

	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (pdx == vol_info->pdx)
		{
			if (start_pos >= vol_info->start_pos && start_pos < vol_info->end_pos)
			{
				break;
			}
		}

		vol_info = NULL;
		req_entry = req_entry->Flink;
	}

	if (vol_info)
	{
		InterlockedIncrement(&vol_info->reference_count);
	}

	KeReleaseSpinLock(&group_info->vol_queue->lock, old_irql);

	return vol_info;
}


BOOL RemoveVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info)
{	
	BOOLEAN ret = FALSE;

	QueueInfo* delay_del_queue = vol_queue->group_info->delay_del_vol_queue;

	AcquireExclusiveResource(&vol_queue->sync_resource);
	KIRQL old_irql;
	KeAcquireSpinLock(&vol_queue->lock, &old_irql);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	VolInfo *vol_info = NULL;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->end_pos == find_vol_info->end_pos
			&& vol_info->start_pos == find_vol_info->start_pos
			&& vol_info->pdx == find_vol_info->pdx)
		{
			RemoveEntryList(req_entry);
			vol_queue->num--;
			break;
		}

		vol_info = NULL;
		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&vol_queue->lock, old_irql);
	ReleaseResource(&vol_queue->sync_resource);

	if (vol_info)
	{
		if (InterlockedCompareExchange(&vol_info->reference_count, 0, 0) != 0)
		{
			PushVolQueue(delay_del_queue, vol_info);
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::vol_info reference count %d is not 0, delay delete it", vol_info->reference_count);
		}
		else
		{
			FreeVolInfo(vol_info);
		}

		ret = TRUE;
	}

	return ret;
}

BOOL RemoveVolInfoByPdx(GroupInfo* group_info, PDEVICE_EXTENSION pdx)
{
	BOOLEAN ret = FALSE;

	QueueInfo *vol_queue = group_info->vol_queue;
	QueueInfo* delay_del_queue = group_info->delay_del_vol_queue;

	AcquireExclusiveResource(&vol_queue->sync_resource);
	KIRQL old_irql;
	KeAcquireSpinLock(&vol_queue->lock, &old_irql);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	VolInfo *vol_info = NULL;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->pdx == pdx)
		{
			RemoveEntryList(req_entry);
			vol_queue->num--;
			break;
		}

		vol_info = NULL;
		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&vol_queue->lock, old_irql);
	ReleaseResource(&vol_queue->sync_resource);

	if (vol_info)
	{
		if (InterlockedCompareExchange(&vol_info->reference_count, 0, 0) != 0)
		{
			PushVolQueue(delay_del_queue, vol_info);
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::vol_info reference count %d is not 0, delay delete it", vol_info->reference_count);
		}
		else
		{
			FreeVolInfo(vol_info);
		}

		ret = TRUE;
	}

	return ret;
}

VolInfo *FindVolInfoByVerifyMap(QueueInfo *vol_queue)
{
	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	VolInfo *vol_info = NULL;

	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (IsBitmapEmpty(vol_info->bitmap_verify))
		{
			vol_info = NULL;
			req_entry = req_entry->Flink;
			continue;
		}
		else
		{
			break;
		}
	}

	if (vol_info)
	{
		InterlockedIncrement(&vol_info->reference_count);
	}

	ReleaseResource(&vol_queue->sync_resource);

	return vol_info;
}

VolInfo *FindVolInfoByCbtMap(QueueInfo *vol_queue)
{
	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	VolInfo *vol_info = NULL;

	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (IsBitmapEmpty(vol_info->bitmap))
		{
			vol_info = NULL;
			req_entry = req_entry->Flink;
			continue;
		}
		else
		{
			break;
		}
	}

	if (vol_info)
	{
		InterlockedIncrement(&vol_info->reference_count);
	}

	ReleaseResource(&vol_queue->sync_resource);

	return vol_info;
}

VOID GetAllVolBitmapSize(QueueInfo *vol_queue, uint64_t* cbt_size, uint64_t* verify_size)
{
	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	VolInfo *vol_info = NULL;

	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		*cbt_size += GetBitmapCount(vol_info->bitmap);

		if (vol_info->bitmap_verify)
		{
			*verify_size += GetBitmapCount(vol_info->bitmap_verify);
		}

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&vol_queue->sync_resource);

	*cbt_size *= IM_SECTOR_SIZE;
	*verify_size *= IM_SECTOR_SIZE;
}

VOID ClearAllVolVerifyMap(QueueInfo *vol_queue)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Clear verify bitmap");

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	VolInfo *vol_info = NULL;

	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->bitmap_verify)
		{
			BitmapFree(vol_info->bitmap_verify, BitmapFree);
			vol_info->bitmap_verify = NULL;
		}
		if (vol_info->hbi_verify)
		{
			ExFreePoolWithTag(vol_info->hbi_verify, ALLOC_TAG);
			vol_info->hbi_verify = NULL;
		}

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&vol_queue->sync_resource);
}

BOOL InitAllVolVerifyMap(QueueInfo *vol_queue, uint8_t bitmap_granularity)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Init verify bitmap");

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	VolInfo *vol_info = NULL;

	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->bitmap_verify == NULL)
		{
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Create a new bitmap_verify");

			vol_info->bitmap_verify = BitmapAlloc(vol_info->sectors, bitmap_granularity, BitmapAlloc, BitmapFree);
			if (NULL == vol_info->bitmap_verify)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::BitmapAlloc failed");
				goto fail;
			}
			vol_info->hbi_verify = (OM_BITMAP_IT *)ExAllocatePoolWithTag(NonPagedPool, sizeof(OM_BITMAP_IT), ALLOC_TAG);
			if (NULL == vol_info->hbi_verify)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc hbi failed");
				goto fail;
			}

			memset(vol_info->hbi_verify, 0, sizeof(OM_BITMAP_IT));
			BitmapSetBit(vol_info->bitmap_verify, 0, vol_info->sectors);
			BitmapItInit(vol_info->hbi_verify, vol_info->bitmap_verify, 0);

			BitmapResetBit(vol_info->bitmap, 0, vol_info->sectors);
			BitmapItInit(vol_info->hbi, vol_info->bitmap, 0);
			vol_info->persist_data_apply_state = VOL_PERSIST_DATA_APPLY_STATE_DONE;
		}

		vol_info->verify_countdown = 0;

		req_entry = req_entry->Flink;	
	}

	ReleaseResource(&vol_queue->sync_resource);

	return TRUE;

fail:
	ReleaseResource(&vol_queue->sync_resource);

	ClearAllVolVerifyMap(vol_queue);

	return FALSE;
}

BOOL InitVolumeVerifyBitmap(VolInfo *vol_info, uint8_t bitmap_granularity)
{
	if (vol_info->bitmap_verify == NULL)
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Create a new bitmap_verify for volume %wZ", &vol_info->pdx->disk_name);

		vol_info->bitmap_verify = BitmapAlloc(vol_info->sectors, bitmap_granularity, BitmapAlloc, BitmapFree);
		if (NULL == vol_info->bitmap_verify)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::BitmapAlloc failed");
			goto fail;
		}

		vol_info->hbi_verify = (OM_BITMAP_IT *)ExAllocatePoolWithTag(NonPagedPool, sizeof(OM_BITMAP_IT), ALLOC_TAG);
		if (NULL == vol_info->hbi_verify)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc hbi failed");
			goto fail;
		}

		memset(vol_info->hbi_verify, 0, sizeof(OM_BITMAP_IT));
		BitmapSetBit(vol_info->bitmap_verify, 0, vol_info->sectors);
		BitmapItInit(vol_info->hbi_verify, vol_info->bitmap_verify, 0);

		BitmapResetBit(vol_info->bitmap, 0, vol_info->sectors);
		BitmapItInit(vol_info->hbi, vol_info->bitmap, 0);
		vol_info->persist_data_apply_state = VOL_PERSIST_DATA_APPLY_STATE_DONE;
	}

	vol_info->verify_countdown = 0;

	return TRUE;

fail:
	if (vol_info->bitmap_verify)
	{
		BitmapFree(vol_info->bitmap_verify, BitmapFree);
		vol_info->bitmap_verify = NULL;
	}

	if (vol_info->hbi_verify)
	{
		ExFreePoolWithTag(vol_info->hbi_verify, ALLOC_TAG);
		vol_info->hbi_verify = NULL;
	}

	return FALSE;
}

BOOL IsAllVolVerifyCease(QueueInfo *vol_queue)
{
	BOOL Ret = TRUE;

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	VolInfo *vol_info = NULL;

	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->verify_countdown != 0)
		{
			Ret = FALSE;
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Verify for volume %wZ is not ceased", &vol_info->pdx->disk_name);
			break;
		}

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&vol_queue->sync_resource);

	return Ret;
}

VOID CheckRemoveDelayDeleteQueue(GroupInfo *group_info)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&group_info->delay_del_cmd_queue->lock, &old_irql);

	if (GetQueueNum(group_info->delay_del_cmd_queue) == 0)
	{
		KeReleaseSpinLock(&group_info->delay_del_cmd_queue->lock, old_irql);
		return;
	}

	QueueInfo* queue_info = group_info->delay_del_cmd_queue;

	PLIST_ENTRY head = &queue_info->head;
	CmdNode *cmd_node = NULL;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);
		if (InterlockedCompareExchange(&cmd_node->io_status, 0, 0) != IO_COMPLETION_STATUS_ON_GOING)
		{
			RemoveEntryList(&cmd_node->list_entry);
			queue_info->num--;
			queue_info->size -= (cmd_node->cmd->header.body_len);
			FreeCmdNode(cmd_node, group_info);
		}
		cmd_node = NULL;
		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&group_info->delay_del_cmd_queue->lock, old_irql);
}



#pragma LOCKEDCODE
VOID DataCmdByteSwap(PDPP_DATA dpp_data)
{
	dpp_data->vol_offset = RtlUlonglongByteSwap(dpp_data->vol_offset);
	dpp_data->data_size = RtlUlongByteSwap(dpp_data->data_size);
}

#pragma LOCKEDCODE
VOID DatasetCmdByteSwap(PDPP_DATASET_START dataset_start)
{
	dataset_start->dataset_id = RtlUlonglongByteSwap(dataset_start->dataset_id);
	dataset_start->dpp_type = RtlUlongByteSwap(dataset_start->dpp_type);
}

#pragma LOCKEDCODE
VOID ResyncsetCmdByteNetworkSwap(PDPP_RESYNCSET_START resyncset_start)
{
	uint32_t vol_num = resyncset_start->num_vols;

	resyncset_start->resyncset_id = RtlUlonglongByteSwap(resyncset_start->resyncset_id);
	resyncset_start->num_vols = RtlUlongByteSwap(resyncset_start->num_vols);

	for (uint32_t i = 0; i < vol_num; i++)
	{
		resyncset_start->vol_entry[i].vol_offset = RtlUlonglongByteSwap(resyncset_start->vol_entry[i].vol_offset);
		resyncset_start->vol_entry[i].seg_size = RtlUlonglongByteSwap(resyncset_start->vol_entry[i].seg_size);
	}
}

#pragma LOCKEDCODE
VOID ResyncsetCmdByteHostSwap(PDPP_RESYNCSET_START resyncset_start)
{
	resyncset_start->resyncset_id = RtlUlonglongByteSwap(resyncset_start->resyncset_id);
	resyncset_start->num_vols = RtlUlongByteSwap(resyncset_start->num_vols);

	for (uint32_t i = 0; i < resyncset_start->num_vols; i++)
	{
		resyncset_start->vol_entry[i].vol_offset = RtlUlonglongByteSwap(resyncset_start->vol_entry[i].vol_offset);
		resyncset_start->vol_entry[i].seg_size = RtlUlonglongByteSwap(resyncset_start->vol_entry[i].seg_size);
	}
}

#pragma LOCKEDCODE
VOID AttentionCmdByteNetworkSwap(PDPP_ATTENTION dpp_attention)
{
	if (dpp_attention->operation == DPP_ATTENTION_OPERATION_DISCOVERY)
	{
	}
	else if (dpp_attention->operation == DPP_ATTENTION_OPERATION_ALERT)
	{
		PDPP_ATTENTION_PAYLOAD_ALERT payload_alert = (PDPP_ATTENTION_PAYLOAD_ALERT)dpp_attention->payload;
		payload_alert->description = RtlUlongByteSwap(payload_alert->description);
		payload_alert->level = RtlUlongByteSwap(payload_alert->level);
	}
	else if (dpp_attention->operation == DPP_ATTENTION_OPERATION_ACTIVITY)
	{
		PDPP_ATTENTION_PAYLOAD_ACTIVITY payload_activity = (PDPP_ATTENTION_PAYLOAD_ACTIVITY)dpp_attention->payload;
		payload_activity->cbt_backlog = RtlUlonglongByteSwap(payload_activity->cbt_backlog);
		payload_activity->resync_remaining = RtlUlonglongByteSwap(payload_activity->resync_remaining);
	}

	dpp_attention->operation = RtlUlongByteSwap(dpp_attention->operation);
	dpp_attention->payload_len = RtlUlongByteSwap(dpp_attention->payload_len);
}

#pragma LOCKEDCODE
VOID AttentionCmdByteHostSwap(PDPP_ATTENTION dpp_attention)
{
	dpp_attention->operation = RtlUlongByteSwap(dpp_attention->operation);
	dpp_attention->payload_len = RtlUlongByteSwap(dpp_attention->payload_len);

	if (dpp_attention->operation == DPP_ATTENTION_OPERATION_DISCOVERY)
	{
	}
	else if (dpp_attention->operation == DPP_ATTENTION_OPERATION_ALERT)
	{
		PDPP_ATTENTION_PAYLOAD_ALERT payload_alert = (PDPP_ATTENTION_PAYLOAD_ALERT)dpp_attention->payload;
		payload_alert->description = RtlUlongByteSwap(payload_alert->description);
		payload_alert->level = RtlUlongByteSwap(payload_alert->level);
	}
	else if (dpp_attention->operation == DPP_ATTENTION_OPERATION_ACTIVITY)
	{
		PDPP_ATTENTION_PAYLOAD_ACTIVITY payload_activity = (PDPP_ATTENTION_PAYLOAD_ACTIVITY)dpp_attention->payload;
		payload_activity->cbt_backlog = RtlUlonglongByteSwap(payload_activity->cbt_backlog);
		payload_activity->resync_remaining = RtlUlonglongByteSwap(payload_activity->resync_remaining);
	}
}

#pragma LOCKEDCODE
VOID SessionLoginByteSwap(PDPP_SESSION_LOGIN session_login)
{
	PDPP_SESSION_LOGIN_AUX login_aux = (PDPP_SESSION_LOGIN_AUX)session_login->payload;
	login_aux->max_dataset_size = RtlUlonglongByteSwap(login_aux->max_dataset_size);
	login_aux->dataset_id_sent = RtlUlonglongByteSwap(login_aux->dataset_id_sent);
	login_aux->dataset_id_done = RtlUlonglongByteSwap(login_aux->dataset_id_done);

	session_login->aux_login_len = RtlUshortByteSwap(session_login->aux_login_len);
	session_login->auth_len = RtlUshortByteSwap(session_login->auth_len);
}

#pragma LOCKEDCODE
VOID SessionLoginAckByteSwap(PDPP_SESSION_LOGIN session_login)
{
	session_login->aux_login_len = RtlUshortByteSwap(session_login->aux_login_len);
	session_login->auth_len = RtlUshortByteSwap(session_login->auth_len);

	PDPP_SESSION_LOGIN_ACK_AUX login_ack_aux = (PDPP_SESSION_LOGIN_ACK_AUX)session_login->payload;
	login_ack_aux->max_dataset_size = RtlUlonglongByteSwap(login_ack_aux->max_dataset_size);
	login_ack_aux->dataset_id_sent = RtlUlonglongByteSwap(login_ack_aux->dataset_id_sent);
	login_ack_aux->dataset_id_done = RtlUlonglongByteSwap(login_ack_aux->dataset_id_done);
	login_ack_aux->buf_credit = RtlUlongByteSwap(login_ack_aux->buf_credit);
}

#pragma LOCKEDCODE
VOID DatasetDoneCmdByteSwap(PDPP_DATASET_DONE dataset_done)
{
	dataset_done->dataset_id = RtlUlonglongByteSwap(dataset_done->dataset_id);
}

#pragma LOCKEDCODE
VOID CreditAckByteSwap(PDPP_CREDIT_BUFFER credit_buffer)
{
	credit_buffer->buf_credit = RtlUlongByteSwap(credit_buffer->buf_credit);
}

#pragma LOCKEDCODE
VOID CmdHeaderByteSwap(DPP_HEADER* cmd_header)
{
	cmd_header->magic = RtlUlongByteSwap(cmd_header->magic);
	cmd_header->cmd_type = RtlUshortByteSwap(cmd_header->cmd_type);
	cmd_header->flags = RtlUshortByteSwap(cmd_header->flags);
	cmd_header->sequence_num = RtlUlonglongByteSwap(cmd_header->sequence_num);
	cmd_header->body_len = RtlUlongByteSwap(cmd_header->body_len);
	cmd_header->reserved = RtlUlongByteSwap(cmd_header->reserved);
}

#pragma LOCKEDCODE
VOID CmdBodyByteNetworkSwap(IOMirrorCmd* cmd)
{
	switch (cmd->header.cmd_type)
	{
	case DPP_TYPE_SESSION_LOGIN:
		SessionLoginByteSwap((PDPP_SESSION_LOGIN)cmd->data);
		break;
	case DPP_TYPE_DATA:
		DataCmdByteSwap((PDPP_DATA)cmd->data);
		break;
	case DPP_TYPE_RESYNCSET_START:
		ResyncsetCmdByteNetworkSwap((PDPP_RESYNCSET_START)cmd->data);
		break;
	case DPP_TYPE_DATASET_START:
		DatasetCmdByteSwap((PDPP_DATASET_START)cmd->data);
		break;
	case DPP_TYPE_ATTENTION:
		AttentionCmdByteNetworkSwap((PDPP_ATTENTION)cmd->data);
		break;
	case DPP_TYPE_SESSION_LOGIN_ACK:
		SessionLoginAckByteSwap((PDPP_SESSION_LOGIN)cmd->data);
		break;
	case DPP_TYPE_DATASET_DONE:
	case DPP_TYPE_RESYNCSET_DONE:
		DatasetDoneCmdByteSwap((PDPP_DATASET_DONE)cmd->data);
		break;
	case DPP_TYPE_CREDIT:
		CreditAckByteSwap((PDPP_CREDIT_BUFFER)cmd->data);
		break;
	default:
		break;
	}
}

#pragma LOCKEDCODE
VOID CmdBodyByteHostSwap(IOMirrorCmd* cmd)
{
	switch (cmd->header.cmd_type)
	{
	case DPP_TYPE_SESSION_LOGIN:
		SessionLoginByteSwap((PDPP_SESSION_LOGIN)cmd->data);
		break;
	case DPP_TYPE_DATA:
		DataCmdByteSwap((PDPP_DATA)cmd->data);
		break;
	case DPP_TYPE_RESYNCSET_START:
		ResyncsetCmdByteHostSwap((PDPP_RESYNCSET_START)cmd->data);
		break;
	case DPP_TYPE_DATASET_START:
		DatasetCmdByteSwap((PDPP_DATASET_START)cmd->data);
		break;
	case DPP_TYPE_ATTENTION:
		AttentionCmdByteHostSwap((PDPP_ATTENTION)cmd->data);
		break;
	case DPP_TYPE_SESSION_LOGIN_ACK:
		SessionLoginAckByteSwap((PDPP_SESSION_LOGIN)cmd->data);
		break;
	case DPP_TYPE_DATASET_DONE:
	case DPP_TYPE_RESYNCSET_DONE:
		DatasetDoneCmdByteSwap((PDPP_DATASET_DONE)cmd->data);
		break;
	case DPP_TYPE_CREDIT:
		CreditAckByteSwap((PDPP_CREDIT_BUFFER)cmd->data);
		break;
	default:
		break;
	}
}


#pragma LOCKEDCODE
VOID CmdByteNetworkSwap(IOMirrorCmd* cmd)
{
	CmdBodyByteNetworkSwap(cmd);
	CmdHeaderByteSwap(&cmd->header);
}

#pragma LOCKEDCODE
VOID CmdByteHostSwap(IOMirrorCmd* cmd)
{
	CmdHeaderByteSwap(&cmd->header);
	CmdBodyByteHostSwap(cmd);
}