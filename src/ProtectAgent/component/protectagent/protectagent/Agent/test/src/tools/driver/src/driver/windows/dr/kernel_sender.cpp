#include "kernel_sender.h"
#include "kernel_receiver.h"
#include "regedit.h"
#include "group_mem.h"
#include "persist_file.h"
#include "driver.h"

#include "wpp_trace.h"
#include "kernel_sender.tmh"

VOID FlushBitMap(GroupInfo *group_info);
VOID FlushVerifyBitMap(GroupInfo *group_info);
VOID ProcessPausePending(SocketInfo *socket_info, GroupInfo *group_info);

NTSTATUS ProcessIoctlModify(SocketInfo *socket_info, GroupInfo *group_info, ProtectStrategy* protect_strategy)
{
	NTSTATUS status = STATUS_SUCCESS;

	if (group_info->protect_strategy.rpo != protect_strategy->exp_rpo)
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Rpo changed from %d to %d", group_info->protect_strategy.rpo, protect_strategy->exp_rpo);
		group_info->protect_strategy.rpo = protect_strategy->exp_rpo;
	}

	if (memcmp(group_info->protect_strategy.osId, protect_strategy->vm_id, VM_ID_LEN) != 0 ||
		memcmp(group_info->protect_strategy.oma_id, protect_strategy->oma_id, VM_ID_LEN) != 0 || group_info->protect_strategy.vrgIp != protect_strategy->oma_ip[0] || group_info->protect_strategy.vrgPort != protect_strategy->oma_port)
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Connection or login info changed, need to do a reconnect");

		RtlCopyMemory(group_info->protect_strategy.osId, protect_strategy->vm_id, VM_ID_LEN);
		RtlCopyMemory(group_info->protect_strategy.oma_id, protect_strategy->oma_id, VM_ID_LEN);
		group_info->protect_strategy.vrgIp = protect_strategy->oma_ip[0];
		group_info->protect_strategy.vrgPort = protect_strategy->oma_port;

		if (group_info->inter_state != IM_PG_INTER_STATE_CONNECT_STAGE0)
		{
			status = DrDisconnect(&socket_info->end_point);
			if (STATUS_SUCCESS != status)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Disconnect failed, error %!STATUS!", status);
			}
			ProcessDisconnect(group_info);
		}
	}

	ExFreePoolWithTag(protect_strategy, ALLOC_TAG);

	return status;
}

NTSTATUS ProcessIoctlStop(SocketInfo *socket_info, GroupInfo *group_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	uint32_t orig_inter_state = group_info->inter_state;

	group_info->state = IM_PG_STATE_STOP;
	group_info->inter_state = IM_PG_INTER_STATE_STOP;
	group_info->flow_control_pause_flag = FALSE;

	ClearCmdQueue(group_info->cmd_queue, group_info);
	ClearCmdQueue(group_info->temp_cmd_queue, group_info);
	ClearCmdQueue(group_info->pending_cmd_queue, group_info);
	ClearVolQueue(group_info->vol_queue);
	ClearVolQueue(group_info->temp_vol_queue);
	CheckRemoveDelayDeleteQueue(group_info);
	CheckRemoveDelayDeleteVolQueue(group_info);
	group_info->last_heart_time = GetCurTime();
	group_info->heart_num = 0;
	group_info->per_data_state = PERSIST_DATA_STATE_DISCARD;
	group_info->dataset_id = 1;
	group_info->dataset_id_done = 0;
	group_info->sequence_id = 0;
	group_info->set_data_size = 0;
	group_info->queue_set_data_size = 0;

	if (orig_inter_state != IM_PG_INTER_STATE_CONNECT_STAGE0)
	{
		status = DrDisconnect(&socket_info->end_point);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Disconnect failed, error %!STATUS!", status);
		}
		group_info->socket_info.is_receiving = FALSE;
		memset(group_info->socket_info.rev_io_status, 0, sizeof(IO_STATUS_BLOCK));
		KeResetEvent(group_info->socket_info.rev_event);
	}
	
	return status;
}

// 如果正在基线同步或者CBT，则无需改变状态，如果正在一致性校验，则重新开始一致性校验；如果是其他状态，则进入CBT
NTSTATUS ProcessIoctlVolAdd(SocketInfo *socket_info, GroupInfo *group_info, VolInfo* vol_info)
{
	UNREFERENCED_PARAMETER(socket_info);

	//QueueInfo *vol_queue = group_info->vol_queue;
	if (group_info->inter_state == IM_PG_INTER_STATE_VERIFY_DATA)
	{
		if (!InitVolumeVerifyBitmap(vol_info, group_info->bitmap_granularity))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to init verify bitmap for disk %wZ", &vol_info->disk_name);
			FreeVolInfo(vol_info);

			return STATUS_UNSUCCESSFUL;
		}
	}

	PushVolQueue(group_info->vol_queue, vol_info);

	if (group_info->state == IM_PG_STATE_NORMAL)
	{
		group_info->state = IM_PG_STATE_CBT;
		if (   group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE0
			|| group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE1
			|| group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE1_P
			|| group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE2
			|| group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE2_P)
		{
		}
		else
		{
			group_info->inter_state = IM_PG_INTER_STATE_CBT_START;
		}
	}

	return STATUS_SUCCESS;
}

NTSTATUS ProcessIoctlVolDel(SocketInfo *socket_info, GroupInfo *group_info, VolInfo* find_vol_info)
{
	UNREFERENCED_PARAMETER(socket_info);

	NTSTATUS status;

	QueueInfo *vol_queue = group_info->vol_queue;

	BOOL ret = RemoveVolInfoByVolInfo(vol_queue, find_vol_info);
	if (FALSE == ret)
	{
		status = STATUS_UNSUCCESSFUL;
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Remove vol by vol failed, disk %wZ, start_pos %llu, end_pos %llu", &find_vol_info->disk_name, find_vol_info->start_pos, find_vol_info->end_pos);
	}
	else
	{
		status = STATUS_SUCCESS;
	}
	FreeVolInfo(find_vol_info);

	return status;
}

// 修改卷信息只修改volid信息，不需要处理bitmap，并且只允许在未开始传输数据时修改
NTSTATUS ProcessIoctlVolMod(SocketInfo *socket_info, GroupInfo *group_info, VolInfo* vol_info)
{
    UNREFERENCED_PARAMETER(socket_info);

	NTSTATUS status;

	if (ModifyVolInfoByVolInfo(group_info->vol_queue, vol_info)) {
		status = STATUS_SUCCESS;
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::modify vol by vol succ, disk %wZ, start_pos %llu, end_pos %llu.",
			&vol_info->disk_name, vol_info->start_pos, vol_info->end_pos);
	} else {
		status = STATUS_UNSUCCESSFUL;
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::modify vol by vol failed, disk %wZ, start_pos %llu, end_pos %llu.",
			&vol_info->disk_name, vol_info->start_pos, vol_info->end_pos);
	}
	FreeVolInfo(vol_info);

    return status;
}

NTSTATUS ProcessIoctlStart(SocketInfo *socket_info, GroupInfo *group_info, QueueInfo* load_vol_queue)
{
	UNREFERENCED_PARAMETER(socket_info);

	QueueInfo *vol_queue = group_info->boot_flag==TRUE?group_info->vol_queue:group_info->temp_vol_queue;
	PLIST_ENTRY head = &load_vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	VolInfo *vol_info = NULL;

	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		RemoveEntryList(req_entry);
		load_vol_queue->num--;
		PushVolQueue(vol_queue, vol_info);
		req_entry = temp_req_entry;
	}
	FreeVolQueue(load_vol_queue, group_info);

	return STATUS_SUCCESS;
}

NTSTATUS ProcessIoctlPause(SocketInfo *socket_info, GroupInfo *group_info)
{
	UNREFERENCED_PARAMETER(socket_info);

	group_info->custom_point_type = CUSTOM_POINT_TYPE_PAUSE;
	group_info->pause_pending = TRUE;

	return STATUS_SUCCESS;
}

NTSTATUS ProcessIoctlError(SocketInfo *socket_info, GroupInfo *group_info)
{
	UNREFERENCED_PARAMETER(socket_info);
	ProcessErrorState(group_info);

	return STATUS_SUCCESS;
}

NTSTATUS ProcessIoctlDisconnect(SocketInfo *socket_info, GroupInfo *group_info)
{
	UNREFERENCED_PARAMETER(socket_info);

	ProcessDisconnect(group_info);
	
	return STATUS_SUCCESS;
}

VOID ProcessExternCtl(SocketInfo *socket_info, GroupInfo *group_info)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	uint32_t extern_ctl_type;
	uint8_t* extern_ctl_buffer;

	KIRQL old_irql;
	KeAcquireSpinLock(&group_info->extern_ctl_lock, &old_irql);
	extern_ctl_type = group_info->extern_ctl_type;
	extern_ctl_buffer = group_info->extern_ctl_buffer;
	KeReleaseSpinLock(&group_info->extern_ctl_lock, old_irql);

	if (IM_PG_IOCTL_STATE_NORMAL == extern_ctl_type)
	{
		return;
	}

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Enter, type = %d", extern_ctl_type);

	switch (extern_ctl_type)
	{
	case IM_PG_IOCTL_STATE_MODIFY:
		status = ProcessIoctlModify(socket_info, group_info, (ProtectStrategy*)extern_ctl_buffer);
		break;
	case IM_PG_IOCTL_STATE_STOP:
		status = ProcessIoctlStop(socket_info, group_info);
		break;
	case IM_PG_IOCTL_STATE_VOL_ADD:
		status = ProcessIoctlVolAdd(socket_info, group_info, (VolInfo*)extern_ctl_buffer);
		break;
	case IM_PG_IOCTL_STATE_VOL_DEL:
		status = ProcessIoctlVolDel(socket_info, group_info, (VolInfo*)extern_ctl_buffer);
		break;
	case IM_PG_IOCTL_STATE_VOL_MOD:
		status = ProcessIoctlVolMod(socket_info, group_info, (VolInfo*)extern_ctl_buffer);
		break;
	case IM_PG_IOCTL_STATE_START:
		status = ProcessIoctlStart(socket_info, group_info, (QueueInfo*)extern_ctl_buffer);
		break;
	case IM_PG_IOCTL_STATE_PAUSE:
		status = ProcessIoctlPause(socket_info, group_info);
		break;
	case IM_PG_IOCTL_STATE_ERROR:
		status = ProcessIoctlError(socket_info, group_info);
		break;
	case IM_PG_IOCTL_STATE_DISCONNECT:
		status = ProcessIoctlDisconnect(socket_info, group_info);
		break;
	default:
		status = STATUS_UNSUCCESSFUL;
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Unknown external ctl type %d", group_info->extern_ctl_type);
		break;
	}

	CheckRemoveDelayDeleteVolQueue(group_info);

	if (extern_ctl_type == IM_PG_IOCTL_STATE_ERROR)
	{
		SetEventForExternCtl(group_info, STATUS_UNSUCCESSFUL);
	}
	else
	{
		SetEventForExternCtl(group_info, status);
	}
}

VOID RetrieveNormalData(GroupInfo *group_info, uint32_t* data_size, PBOOLEAN stop_at_dataset)
{
	QueueInfo *cmd_queue = group_info->cmd_queue;
	QueueInfo *pending_cmd_queue = group_info->pending_cmd_queue;

	CmdNode *cmd_node = NULL;

	IOMirrorCmd* pending_cmd = NULL;
	IOMirrorCmd* send_cmd = NULL;
	IOMirrorCmd* free_cmd = NULL;

	uint32_t credit_loan = 0;

	RtlZeroMemory(group_info->normal_send_buffer, NORMAL_SEND_BUFFER_SIZE);

	uint32_t send_size = 0;
	BOOLEAN dataset_dpp = FALSE;
	while (send_size < NORMAL_SEND_BUFFER_SIZE)
	{
		if (IsEmptyCmdQueue(cmd_queue))
		{
			break;
		}

		pending_cmd = (IOMirrorCmd*)MallocCmd(0, sizeof(DPP_DATA), group_info);
		if (NULL == pending_cmd)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
			break;
		}

		cmd_node = PopCmdQueueBySize(cmd_queue, group_info->global_credit > (NORMAL_SEND_BUFFER_SIZE - send_size) ? (NORMAL_SEND_BUFFER_SIZE - send_size) : group_info->global_credit);
		if (NULL == cmd_node)
		{
			break;
		}

		send_cmd = cmd_node->cmd;

		if (cmd_node->cmd->header.cmd_type == DPP_TYPE_DATA)
		{
			CopyDataCmdThin(pending_cmd, send_cmd);
			cmd_node->cmd = pending_cmd;
			pending_cmd = NULL;
			free_cmd = send_cmd;
			credit_loan = send_cmd->header.body_len - sizeof(DPP_DATA);
		}
		else
		{
			free_cmd = NULL;
			credit_loan = 0;
			dataset_dpp = TRUE;
		}

		PushCmdQueueNLock(pending_cmd_queue, cmd_node);

		ULONG body_len = send_cmd->header.body_len;
		CmdByteNetworkSwap(send_cmd);
		RtlCopyMemory(group_info->normal_send_buffer + send_size, send_cmd, sizeof(IOMirrorCmd) + body_len);
		CmdByteHostSwap(send_cmd);

		if (group_info->global_credit != 0)
		{
			group_info->global_credit -= credit_loan;
		}

		send_size += sizeof(IOMirrorCmd) + body_len;

		CheckFreeCmd(free_cmd, group_info);
		free_cmd = NULL;

		CheckFreeCmd(pending_cmd, group_info);
		pending_cmd = NULL;

		if (dataset_dpp)
		{
			break;
		}
	}

	CheckFreeCmd(free_cmd, group_info);
	free_cmd = NULL;

	CheckFreeCmd(pending_cmd, group_info);
	pending_cmd = NULL;

	*data_size = send_size;
	*stop_at_dataset = dataset_dpp;
}

BOOLEAN SendNormalData(SocketInfo *socket_info, GroupInfo *group_info)
{
	uint32_t send_size = 0;
	BOOLEAN dataset_dpp = FALSE;
	RetrieveNormalData(group_info, &send_size, &dataset_dpp);
	if (send_size == 0)
	{
		return FALSE;
	}

	NTSTATUS status = ImSendBufferSync(socket_info, group_info->normal_send_buffer, send_size);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);

		ProcessDisconnect(group_info);
		return FALSE;
	}

	if (dataset_dpp)
	{
		if (group_info->pause_pending)
		{
			ProcessPausePending(socket_info, group_info);
		}
	}

	return TRUE;
}

BOOLEAN BuildConnect(SocketInfo *socket_info, GroupInfo *group_info)
{
	NTSTATUS status;
	status = DrConnect(&socket_info->end_point, group_info->protect_strategy.vrgIp, group_info->protect_strategy.vrgPort);
	if (status == STATUS_CONNECTION_ACTIVE)
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Connect is active, disconnect it");
		DrDisconnect(&socket_info->end_point);
		return TRUE;
	}

	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Connection failed, error %!STATUS!", status);
		return FALSE;
	}

	group_info->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE1;

	KeSetTimer(&group_info->heart_beat_timer, RtlConvertLongToLargeInteger(-1*IM_PG_HEARTBEAT_INTERVAL), NULL);
	KeSetTimer(&group_info->activity_timer, RtlConvertLongToLargeInteger(-1 * IM_PG_ACTIVITY_INTERVAL), NULL);
	
	group_info->heart_num = 0;
	group_info->last_heart_time = GetCurTime();

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Connection created");

	return TRUE;
}

BOOLEAN SendSessionLogin(SocketInfo *socket_info, GroupInfo *group_info)
{
	IOMirrorCmd *cmd = (IOMirrorCmd *)MallocCmd(0, DPP_SESSION_LOGIN_LEN, group_info);
	if (NULL == cmd)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
		return FALSE;
	}

	FillSessionLoginCmd(cmd, group_info);

	NTSTATUS status = ImSendSync(group_info, socket_info, cmd, cmd);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
		ProcessDisconnect(group_info);
		return FALSE;
	}

	group_info->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE1_P;

	return TRUE;
}


NTSTATUS
	ReadSectorsCompletion(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context
	)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	if (Irp->PendingReturned && NULL != Context) 
	{
		KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);
	}
	return STATUS_MORE_PROCESSING_REQUIRED;
}

// start:相对分区的起始位置，size：大小
NTSTATUS ReadBlock(VolInfo *vol_info, uint8_t *data, uint64_t start, uint32_t size)
{
	NTSTATUS status;
	KEVENT event;
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	IO_STATUS_BLOCK io_block;
	// 相对整个磁盘的偏移值
	uint64_t offset = start+vol_info->start_pos;
	PDEVICE_OBJECT io_device = GetIoDevice(vol_info->pdx);
	PIRP irp = IoBuildSynchronousFsdRequest(IRP_MJ_READ, io_device, data, size,
		(PLARGE_INTEGER)&offset, &event, &io_block);
	if (NULL == irp)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Build irp failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	IoSetCompletionRoutine(irp, ReadSectorsCompletion, &event, TRUE, TRUE, TRUE);

	status = IoCallDriver(io_device, irp);
	if (status == STATUS_PENDING)
	{
		status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
		status = irp->IoStatus.Status;		
	}

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

BOOLEAN StartCbt(SocketInfo *socket_info, GroupInfo *group_info)
{
	BOOLEAN ret = FALSE;

	BOOLEAN flag = FALSE;

	CmdNode *cmd_node = NULL;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Starting CBT");

	do
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Send data set start for beginning of CBT, id %llu", group_info->dataset_id);

		cmd_node = (CmdNode *)MallocCmdNode(0, sizeof(DPP_DATASET_START), group_info, NULL, IO_COMPLETION_STATUS_ON_GOING);
		if (NULL == cmd_node)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
			break;
		}

		FillDatasetStartCmd(WORK_MODE_CBT, cmd_node->cmd, group_info);

		NTSTATUS status = RegWriteIomirrorState(IM_PG_STATE_CBT);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write iomirror state failed, error %!STATUS!", status);
			break;
		}

		status = ImSendSync(group_info, socket_info, cmd_node->cmd, NULL);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
			ProcessDisconnect(group_info);
			break;
		}

		FlushBitMap(group_info);

		group_info->inter_state = IM_PG_INTER_STATE_CBT_DATA;

		PushCmdQueueNLock(group_info->pending_cmd_queue, cmd_node);
		cmd_node = NULL;

		group_info->set_data_size = 0;

		KeCancelTimer(&group_info->dataset_timer);

		ret = TRUE;
	} while (flag);

	if (cmd_node)
	{
		FreeCmdNode(cmd_node, group_info);
		cmd_node = NULL;
	}

	return ret;
}

BOOLEAN FinishCbt(SocketInfo *socket_info, GroupInfo *group_info)
{
	BOOLEAN ret = FALSE;

	UNREFERENCED_PARAMETER(socket_info);

	BOOLEAN flag = FALSE;

	CmdNode* cmd_node = NULL;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::CBT finished");

	do
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Insert data set start to cmd_queue at ending of CBT, id %llu", group_info->dataset_id);

		cmd_node = (CmdNode *)MallocCmdNode(0, sizeof(DPP_DATASET_START), group_info, NULL, IO_COMPLETION_STATUS_ON_GOING);
		if (NULL == cmd_node)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
			break;
		}

		NTSTATUS status = RegWriteIomirrorState(IM_PG_STATE_NORMAL);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write iomirror state failed, error %!STATUS!", status);
			break;
		}

		FillDatasetStartCmd(WORK_MODE_NORMAL, cmd_node->cmd, group_info);

		PushCmdQueue(group_info->cmd_queue, cmd_node);
		cmd_node = NULL;

		LARGE_INTEGER time_out;
		time_out.QuadPart = group_info->protect_strategy.rpo / 3;
		time_out.QuadPart *= SECOND_TO_NANOSECOND;
		KeSetTimer(&group_info->dataset_timer, time_out, NULL);

		group_info->state = IM_PG_STATE_NORMAL;
		group_info->inter_state = IM_PG_INTER_STATE_NORMAL;

		InterlockedExchange64(&group_info->queue_set_data_size, 0);

		ret = TRUE;
	} while (flag);

	if (cmd_node)
	{
		FreeCmdNode(cmd_node, group_info);
		cmd_node = NULL;
	}

	return ret;
}

BOOLEAN SendCbtDatasetData(SocketInfo *socket_info, GroupInfo *group_info, uint64_t size)
{
	BOOLEAN ret = FALSE;
	BOOLEAN flag = FALSE;

	NTSTATUS status;
	CmdNode *cmd_node = NULL;
	do
	{
		if (group_info->set_data_size + size > group_info->seg_size)
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Send data set start for CBT, id %llu, set data size %llu", group_info->dataset_id, group_info->set_data_size);

			cmd_node = (CmdNode *)MallocCmdNode(0, sizeof(DPP_DATASET_START), group_info, NULL, IO_COMPLETION_STATUS_ON_GOING);
			if (NULL == cmd_node)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
				break;
			}

			FillDatasetStartCmd(WORK_MODE_CBT, cmd_node->cmd, group_info);

			status = ImSendSync(group_info, socket_info, cmd_node->cmd, NULL);
			if (STATUS_SUCCESS != status)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
				ProcessDisconnect(group_info);
				break;
			}

			PushCmdQueueNLock(group_info->pending_cmd_queue, cmd_node);
			cmd_node = NULL;

			group_info->set_data_size = 0;

			if (group_info->pause_pending)
			{
				ProcessPausePending(socket_info, group_info);
			}
		}

		ret = TRUE;
	} while (flag);

	if (cmd_node)
	{
		FreeCmdNode(cmd_node, group_info);
		cmd_node = NULL;
	}

	return ret;
}

BOOLEAN SendCbtIoData(SocketInfo *socket_info, GroupInfo *group_info, VolInfo *vol_info, int64_t sector, uint64_t size)
{
	BOOLEAN ret = FALSE;
	BOOLEAN flag = FALSE;

	NTSTATUS status = STATUS_SUCCESS;
	IOMirrorCmd *cmd = NULL;
	CmdNode *cmd_node = NULL;
	do
	{
		cmd = (IOMirrorCmd *)MallocCmd((uint32_t)size, sizeof(DPP_DATA), group_info);
		if (NULL == cmd)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
			break;
		}

		cmd_node = (CmdNode *)MallocCmdNode(0, sizeof(DPP_DATA), group_info, vol_info, IO_COMPLETION_STATUS_SUCCEED);
		if (NULL == cmd_node)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd_node failed");
			break;
		}

		FillDataCmd(cmd, NULL, cmd->header.body_len, cmd->header.body_len, sector * IM_SECTOR_SIZE, vol_info, group_info);

		PDPP_DATA dpp_data = (PDPP_DATA)cmd->data;
		status = ReadBlock(vol_info, (uint8_t *)dpp_data->data, sector*IM_SECTOR_SIZE, (uint32_t)size);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read_block failed, error %!STATUS!", status);
			break;
		}

		CopyDataCmdThin(cmd_node->cmd, cmd);

		status = ImSendSync(group_info, socket_info, cmd, cmd);
		cmd = NULL;
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
			ProcessDisconnect(group_info);
			break;
		}

		PushCmdQueueNLock(group_info->pending_cmd_queue, cmd_node);
		cmd_node = NULL;

		group_info->set_data_size += size;

		ret = TRUE;
	} while (flag);

	if (cmd)
	{
		FreeCmd(cmd, group_info);
		cmd = NULL;
	}

	if (cmd_node)
	{
		FreeCmdNode(cmd_node, group_info);
		cmd_node = NULL;
	}

	return ret;
}

BOOLEAN SendCbtData(SocketInfo *socket_info, GroupInfo *group_info)
{
	VolInfo *vol_info = FindVolInfoByCbtMap(group_info->vol_queue);
	if (NULL == vol_info)
	{
		group_info->inter_state = IM_PG_INTER_STATE_CBT_END;
		return TRUE;
	}

	NTSTATUS status = IoAcquireRemoveLock(&vol_info->pdx->RemoveLock, vol_info->pdx);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}

	BOOLEAN ret = FALSE;

	int64_t sector = 0;
	uint64_t size = 0;
	uint64_t max_require = ((uint64_t)1U << IM_MAX_BITMAP_GRANULARITY) * IM_SECTOR_SIZE;
	uint64_t nb_sectors = (uint64_t)1U << group_info->bitmap_granularity;
	for (int i = 0; i < MAX_PROCESS_NUM; i++)
	{
		if (max_require > group_info->global_credit)
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Credit restrain, credit %d, required %d", group_info->global_credit, (uint32_t)max_require);
			max_require = group_info->global_credit;
		}

		max_require /= IM_SECTOR_SIZE;

		max_require = (max_require / nb_sectors) * nb_sectors;
		if (max_require == 0)
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::The restrain size is too small for a bit");
			break;
		}

		sector = BitmapItNextSuccessive(vol_info->hbi, max_require, &size);
		if (sector < 0 || sector >= (int64_t)vol_info->sectors)
		{		
			BitmapItInit(vol_info->hbi, vol_info->bitmap, 0);
			break;
		}

		size *= IM_SECTOR_SIZE;

		if (!SendCbtDatasetData(socket_info, group_info, size))
		{
			break;
		}

		if (!SendCbtIoData(socket_info, group_info, vol_info, sector, size))
		{
			break;
		}

		if (group_info->global_credit != 0)
		{
			group_info->global_credit -= (uint32_t)size;
		}

		BitmapResetBit(vol_info->bitmap, sector, size / IM_SECTOR_SIZE);

		ret = TRUE;
	}

	IoReleaseRemoveLock(&vol_info->pdx->RemoveLock, vol_info->pdx);

	InterlockedDecrement(&vol_info->reference_count);

	return ret;
}

BOOLEAN StartVerify(SocketInfo *socket_info, GroupInfo *group_info)
{
	UNREFERENCED_PARAMETER(socket_info);

	BOOLEAN ret = FALSE;

	BOOLEAN flag = FALSE;


	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Starting verify");

	do
	{
		if (!InitAllVolVerifyMap(group_info->vol_queue, group_info->bitmap_granularity))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::InitAllVolMap failed");
			break;
		}

		NTSTATUS status = RegWriteIomirrorState(IM_PG_STATE_VERIFY);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Write iomirror state failed, error %!STATUS!", status);
			break;
		}

		FlushVerifyBitMap(group_info);

		group_info->inter_state = IM_PG_INTER_STATE_VERIFY_DATA;

		group_info->set_data_size = 0;

		KeCancelTimer(&group_info->dataset_timer);

		ret = TRUE;
	} while (flag);

	return ret;
}

BOOLEAN SendVerifyDatasetData(SocketInfo *socket_info, GroupInfo *group_info, VolInfo *vol_info, int64_t sector)
{
	BOOLEAN ret = FALSE;
	BOOLEAN flag = FALSE;

	NTSTATUS status;
	CmdNode *cmd_node = NULL;
	do
	{
		if (vol_info->verify_countdown == 0)
		{
			cmd_node = (CmdNode *)MallocCmdNode(sizeof(DPP_VOL_VEC_ENTRY), sizeof(DPP_RESYNCSET_START), group_info, NULL, IO_COMPLETION_STATUS_ON_GOING);
			if (NULL == cmd_node)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
				break;
			}

			uint64_t seg_offset = sector * IM_SECTOR_SIZE;
			uint64_t seg_size = group_info->seg_size;
			if (seg_offset + seg_size > vol_info->sectors * IM_SECTOR_SIZE)
			{
				seg_size = vol_info->sectors * IM_SECTOR_SIZE - seg_offset;
			}

			FillResyncsetStartCmd(cmd_node->cmd, group_info, vol_info, seg_offset, seg_size);

			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Send resync set start for verify, id %llu, seg_offset %llu, seg_size %llu, vol(disk name %wZ, start_pos %llu, end_pos %llu)", group_info->dataset_id, seg_offset, seg_size, &vol_info->disk_name, vol_info->start_pos, vol_info->end_pos);

			status = ImSendSync(group_info, socket_info, cmd_node->cmd, NULL);
			if (STATUS_SUCCESS != status)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
				ProcessDisconnect(group_info);
				break;
			}

			PushCmdQueueNLock(group_info->pending_cmd_queue, cmd_node);
			cmd_node = NULL;

			group_info->set_data_size = 0;
			vol_info->verify_countdown = seg_size;

			if (group_info->pause_pending)
			{
				ProcessPausePending(socket_info, group_info);
			}
		}

		ret = TRUE;
	} while (flag);

	if (cmd_node)
	{
		FreeCmdNode(cmd_node, group_info);
		cmd_node = NULL;
	}

	return ret;
}

BOOLEAN SendVerifyIoData(SocketInfo *socket_info, GroupInfo *group_info, VolInfo *vol_info, int64_t sector, uint64_t size)
{
	BOOLEAN ret = FALSE;
	BOOLEAN flag = FALSE;

	NTSTATUS status;
	IOMirrorCmd *cmd = NULL;
	CmdNode *cmd_node = NULL;
	do
	{
		cmd = (IOMirrorCmd *)MallocCmd((uint32_t)size, sizeof(DPP_DATA), group_info);
		if (NULL == cmd)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
			break;
		}

		cmd_node = (CmdNode *)MallocCmdNode(0, sizeof(DPP_DATA), group_info, vol_info, IO_COMPLETION_STATUS_SUCCEED);
		if (NULL == cmd_node)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd_node failed");
			break;
		}

		FillDataCmd(cmd, NULL, cmd->header.body_len, cmd->header.body_len, sector * IM_SECTOR_SIZE, vol_info, group_info);

		PDPP_DATA dpp_data = (PDPP_DATA)cmd->data;
		status = ReadBlock(vol_info, (uint8_t *)dpp_data->data, sector*IM_SECTOR_SIZE, (uint32_t)size);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Read_block failed, error %!STATUS!", status);
			break;
		}

		CopyDataCmdThin(cmd_node->cmd, cmd);

		status = ImSendSync(group_info, socket_info, cmd, cmd);
		cmd = NULL;
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
			ProcessDisconnect(group_info);
			break;
		}

		PushCmdQueueNLock(group_info->pending_cmd_queue, cmd_node);
		cmd_node = NULL;

		group_info->set_data_size += size;
		vol_info->verify_countdown -= size;

		ret = TRUE;
	} while (flag);

	if (cmd)
	{
		FreeCmd(cmd, group_info);
		cmd = NULL;
	}

	if (cmd_node)
	{
		FreeCmdNode(cmd_node, group_info);
		cmd_node = NULL;
	}

	return ret;
}

BOOLEAN SendVerifyData(SocketInfo *socket_info, GroupInfo *group_info)
{
	VolInfo *vol_info = FindVolInfoByVerifyMap(group_info->vol_queue);
	if (NULL == vol_info)
	{
		group_info->inter_state = IM_PG_INTER_STATE_VERIFY_END;
		return TRUE;
	}

	NTSTATUS status = IoAcquireRemoveLock(&vol_info->pdx->RemoveLock, vol_info->pdx);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}

	BOOLEAN ret = FALSE;

	int64_t sector           = 0;//相对分区的偏移
	uint64_t size            = 0;
	uint64_t max_require = ((uint64_t)1U << IM_MAX_BITMAP_GRANULARITY) * IM_SECTOR_SIZE;
	uint64_t nb_sectors = (uint64_t)1U << group_info->bitmap_granularity;
	OM_BITMAP_IT temp_hbi = *vol_info->hbi_verify;
	for (int i = 0; i < MAX_PROCESS_NUM; i++)
	{
		if (max_require > group_info->global_credit)
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Credit restrain, credit %d, required %d", group_info->global_credit, (uint32_t)max_require);
			max_require = group_info->global_credit;
		}

		if (vol_info->verify_countdown != 0 && max_require > vol_info->verify_countdown)
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Verify count down restrain, count down %d, required %d", (uint32_t)vol_info->verify_countdown, (uint32_t)max_require);
			max_require = vol_info->verify_countdown;
		}

		max_require /= IM_SECTOR_SIZE;

		max_require = (max_require / nb_sectors) * nb_sectors;
		if (max_require == 0)
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::The restrain size is too small for a bit");
			break;
		}

		sector = BitmapItNextSuccessive(&temp_hbi, max_require, &size);
		if (sector < 0 || sector >= (int64_t)vol_info->sectors)
		{		
			BitmapItInit(vol_info->hbi_verify, vol_info->bitmap_verify, 0);
			break;
		}

		size *= IM_SECTOR_SIZE;

		if (!SendVerifyDatasetData(socket_info, group_info, vol_info, sector))
		{
			break;
		}

		if (!SendVerifyIoData(socket_info, group_info, vol_info, sector, size))
		{
			break;
		}

		if (group_info->global_credit != 0)
		{
			group_info->global_credit -= (uint32_t)size;
		}

		BitmapResetBit(vol_info->bitmap_verify, sector, size / IM_SECTOR_SIZE);

		ret = TRUE;
	}

	if (ret)
	{
		*vol_info->hbi_verify = temp_hbi;
	}

	IoReleaseRemoveLock(&vol_info->pdx->RemoveLock, vol_info->pdx);

	InterlockedDecrement(&vol_info->reference_count);

	return ret;
}

BOOLEAN FinishVerify(SocketInfo *socket_info, GroupInfo *group_info)
{
	UNREFERENCED_PARAMETER(socket_info);

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Verify finished");

	ClearAllVolVerifyMap(group_info->vol_queue);

	group_info->state = IM_PG_STATE_CBT;
	group_info->inter_state = IM_PG_INTER_STATE_CBT_START;

	return TRUE;
}

#define IM_PG_BIT_MASK(nr)        (1UL << ((nr) % IM_PG_BITS_PER_BYTE))
#define IM_PG_BIT_WORD(nr)        ((nr) / IM_PG_BITS_PER_BYTE)
VOID SetBit(uint64_t nr, unsigned char *addr)
{
	unsigned char mask = IM_PG_BIT_MASK(nr);
	unsigned char *p = addr + IM_PG_BIT_WORD(nr);
	*p |= mask;
}

BOOLEAN ShutdownBitmapPrepare(GroupInfo *group_info)
{
	BOOLEAN ret = TRUE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Prepare shutdown bitmap");

	uint64_t nb_sectors = (uint64_t)1U << group_info->bitmap_granularity;

	FlushBitMap(group_info);

	AcquireShareResource(&group_info->vol_queue->sync_resource);

	PLIST_ENTRY head = &group_info->vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		ULONGLONG len = (vol_info->sectors + nb_sectors - 1) / nb_sectors;
		len = (len + IM_PG_BITS_PER_BYTE - 1) / IM_PG_BITS_PER_BYTE;
		len = (len + PER_DATA_ALIGN - 1) / PER_DATA_ALIGN * PER_DATA_ALIGN;

		vol_info->persist_len = (ULONG)len;
		vol_info->persist_data = ExAllocatePoolWithTag(NonPagedPool, vol_info->persist_len, ALLOC_TAG);
		if (NULL == vol_info->persist_data)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc persist data buffer failed, length %d", vol_info->persist_len);
			ret = FALSE;
			break;
		}
		memset(vol_info->persist_data, 0, vol_info->persist_len);

		if (!GetBitmapBuffer(vol_info->bitmap, (unsigned char*)vol_info->persist_data, vol_info->persist_len))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Get bitmap buffer failed");
			ret = FALSE;
			break;
		}

#ifdef DBG_OM_BITMAP
		int64_t sector = 0;
		PVOID test_buffer = ExAllocatePoolWithTag(PagedPool, vol_info->persist_len, ALLOC_TAG);
		memset(test_buffer, 0, vol_info->persist_len);

		BitmapItInit(vol_info->hbi, vol_info->bitmap, 0);
		while (1)
		{
			sector = BitmapItNext(vol_info->hbi);
			if (sector < 0 || sector >= (int64_t)vol_info->sectors)
			{
				break;
			}
			SetBit(sector / nb_sectors, (unsigned char *)test_buffer);
		}

		if (memcmp(vol_info->persist_data, test_buffer, vol_info->persist_len) != 0)
		{
			RaiseException();
		}

		ExFreePoolWithTag(test_buffer, ALLOC_TAG);
#endif

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&group_info->vol_queue->sync_resource);

	return ret;
}

BOOLEAN InsertDatasetCmd(QueueInfo* queue, GroupInfo *group_info)
{
	BOOLEAN ret = FALSE;
	BOOLEAN flag = FALSE;

	CmdNode* cmd_node = NULL;
	do
	{
		cmd_node = (CmdNode *)MallocCmdNode(0, sizeof(DPP_DATASET_START), group_info, NULL, IO_COMPLETION_STATUS_ON_GOING);
		if (NULL == cmd_node)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
			break;
		}

		FillDatasetStartCmd(WORK_MODE_NORMAL, cmd_node->cmd, group_info);

		PushCmdQueue(queue, cmd_node);

		ret = TRUE;
	} while (flag);

	return ret;
}

VOID InsertNormalCheckpoint(GroupInfo *group_info)
{
	BOOLEAN flag = FALSE;

	QueueInfo *cmd_queue = group_info->cmd_queue;
	do
	{
		BOOLEAN insert_dataset = FALSE;
		uint32_t reason = 0;

		if (IM_PG_INTER_STATE_NORMAL == group_info->inter_state)
		{
			insert_dataset = (InterlockedCompareExchange64(&group_info->queue_set_data_size, 0, 0) >= (LONG64)group_info->max_set_data_size);
			if (!insert_dataset)
			{
				LARGE_INTEGER time_out = RtlConvertLongToLargeInteger(0);
				NTSTATUS status = KeWaitForSingleObject(&group_info->dataset_timer, Executive, KernelMode, FALSE, &time_out);
				insert_dataset = (STATUS_SUCCESS == status);
				reason = 1;
			}

			if (!insert_dataset)
			{
				insert_dataset = (group_info->custom_point_type != CUSTOM_POINT_TYPE_UNKNOWN);
				reason = group_info->custom_point_type;
			}
		}

		if (!insert_dataset)
		{
			break;
		}

		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Insert data set start to cmd queue, id %llu, reason %d", group_info->dataset_id, reason);

		LARGE_INTEGER time_out;
		time_out.QuadPart = group_info->protect_strategy.rpo / 3;
		time_out.QuadPart *= SECOND_TO_NANOSECOND;
		KeSetTimer(&group_info->dataset_timer, time_out, NULL);

		if (!InsertDatasetCmd(cmd_queue, group_info))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Insert dataset cmd failed");
			break;
		}

		InterlockedExchange64(&group_info->queue_set_data_size, 0);

		group_info->custom_point_type = CUSTOM_POINT_TYPE_UNKNOWN;
	} while (flag);
}

VOID FlushCbtFromNormalQueue(GroupInfo *group_info)
{
	BOOLEAN flag = FALSE;

	QueueInfo *cmd_queue = group_info->cmd_queue;
	QueueInfo *temp_cmd_queue = group_info->temp_cmd_queue;
	QueueInfo* delay_del_queue = group_info->delay_del_cmd_queue;
	do
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Flush cmd queue to CBT, state = %d", group_info->state);

		/* 若当前状态为cbt或verify，则无需切换至cbt状态 */
		if (IM_PG_STATE_CBT == group_info->state || IM_PG_STATE_VERIFY == group_info->state)
		{
		}
		else
		{
			group_info->state = IM_PG_STATE_CBT;
			if (IM_PG_INTER_STATE_CONNECT_STAGE0 != group_info->inter_state
				&& IM_PG_INTER_STATE_CONNECT_STAGE1 != group_info->inter_state
				&& IM_PG_INTER_STATE_CONNECT_STAGE1_P != group_info->inter_state
				&& IM_PG_INTER_STATE_CONNECT_STAGE2 != group_info->inter_state
				&& IM_PG_INTER_STATE_CONNECT_STAGE2_P != group_info->inter_state)
			{
				group_info->inter_state = IM_PG_INTER_STATE_CBT_START;
			}
		}

		FlushCmdQueue(cmd_queue, temp_cmd_queue);
		while (1)
		{
			CmdNode *cmd_node = PopCmdQueueNLock(temp_cmd_queue);
			if (NULL == cmd_node)
			{
				break;
			}
			if (cmd_node->cmd->header.cmd_type == DPP_TYPE_DATA)
			{
				PDPP_DATA dpp_data = (PDPP_DATA)cmd_node->cmd->data;
				BitmapSetBit(cmd_node->vol_info->bitmap, dpp_data->vol_offset / IM_SECTOR_SIZE, dpp_data->data_size / IM_SECTOR_SIZE);

				if (InterlockedCompareExchange(&cmd_node->io_status, 0, 0) == IO_COMPLETION_STATUS_ON_GOING)
				{
					PushCmdQueue(delay_del_queue, cmd_node);
					cmd_node = NULL;
				}
			}

			if (cmd_node)
			{
				FreeCmdNode(cmd_node, group_info);
			}
		}

		CheckRemoveDelayDeleteQueue(group_info);
	} while (flag);
}

VOID CheckAndFlush(GroupInfo *group_info)
{
	QueueInfo *cmd_queue = group_info->cmd_queue;

	uint64_t queue_size = GetCmdQueueSize(cmd_queue);

	if (queue_size == 0)
	{
		return;
	}

	if (queue_size < QUEUE_MAX_SIZE)
	{
		InsertNormalCheckpoint(group_info);
	}
	else
	{
		FlushCbtFromNormalQueue(group_info);
	}
}

VOID FlushBitMap(GroupInfo *group_info)
{
	QueueInfo *cmd_queue = group_info->cmd_queue;
	QueueInfo* pending_queue = group_info->pending_cmd_queue;
	QueueInfo *temp_cmd_queue = group_info->temp_cmd_queue;
	QueueInfo* delay_del_queue = group_info->delay_del_cmd_queue;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::cmd_queue number %d, pending_queue number %d", GetQueueNum(cmd_queue), GetQueueNum(pending_queue));

	FlushCmdQueue(cmd_queue, temp_cmd_queue);
	FlushCmdQueue(pending_queue, temp_cmd_queue);

	while (1)
	{
		CmdNode *cmd_node = PopCmdQueueNLock(temp_cmd_queue);
		if (NULL == cmd_node)
		{
			break;
		}

		if (cmd_node->cmd->header.cmd_type == DPP_TYPE_DATA)
		{
			PDPP_DATA dpp_data = (PDPP_DATA)cmd_node->cmd->data;
			BitmapSetBit(cmd_node->vol_info->bitmap, dpp_data->vol_offset / IM_SECTOR_SIZE, dpp_data->data_size / IM_SECTOR_SIZE);

			if (InterlockedCompareExchange(&cmd_node->io_status, 0, 0) == IO_COMPLETION_STATUS_ON_GOING)
			{
				PushCmdQueue(delay_del_queue, cmd_node);
				cmd_node = NULL;
			}
		}

		if (cmd_node)
		{
			FreeCmdNode(cmd_node, group_info);
		}
	}
}

VOID FlushVerifyBitMap(GroupInfo *group_info)
{
	QueueInfo* pending_queue = group_info->pending_cmd_queue;
	QueueInfo *temp_cmd_queue = group_info->temp_cmd_queue;
	QueueInfo* delay_del_queue = group_info->delay_del_cmd_queue;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::pending_queue number %d", GetQueueNum(pending_queue));

	FlushCmdQueue(pending_queue, temp_cmd_queue);

	while (1)
	{
		CmdNode *cmd_node = PopCmdQueueNLock(temp_cmd_queue);
		if (NULL == cmd_node)
		{
			break;
		}

		if (cmd_node->cmd->header.cmd_type == DPP_TYPE_DATA)
		{
			PDPP_DATA dpp_data = (PDPP_DATA)cmd_node->cmd->data;
			BitmapSetBit(cmd_node->vol_info->bitmap_verify, dpp_data->vol_offset / IM_SECTOR_SIZE, dpp_data->data_size / IM_SECTOR_SIZE);

			if (InterlockedCompareExchange(&cmd_node->io_status, 0, 0) == IO_COMPLETION_STATUS_ON_GOING)
			{
				PushCmdQueue(delay_del_queue, cmd_node);
				cmd_node = NULL;
			}
		}

		if (cmd_node)
		{
			FreeCmdNode(cmd_node, group_info);
		}
	}
}

VOID SendHeartBeat(SocketInfo *socket_info, GroupInfo *group_info)
{
	NTSTATUS status;
	LARGE_INTEGER time_out;

	if (group_info->inter_state == IM_PG_INTER_STATE_STOP
		|| group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE0)
	{
		return;
	}

	time_out = RtlConvertLongToLargeInteger(0);
	status = KeWaitForSingleObject(&group_info->heart_beat_timer, Executive, KernelMode, FALSE, &time_out);
	if (STATUS_TIMEOUT == status)
	{
		return;
	}
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wait hear_beat_time failed, error %!STATUS!", status);
		return;
	}
	IOMirrorCmd *cmd = (IOMirrorCmd *)MallocCmd(0, 0, group_info);
	if (NULL == cmd)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
		return;
	}

	FillHeartbeatCmd(cmd, group_info);

	status = ImSendSync(group_info, socket_info, cmd, cmd);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
		ProcessDisconnect(group_info);
		return;
	}
	time_out = RtlConvertLongToLargeInteger(-1*IM_PG_HEARTBEAT_INTERVAL);
	KeSetTimer(&group_info->heart_beat_timer, time_out, NULL);
	group_info->heart_num++;
}

VOID SendActivity(SocketInfo *socket_info, GroupInfo *group_info)
{
	if (group_info->state != IM_PG_STATE_CBT && group_info->state != IM_PG_STATE_VERIFY)
	{
		return;
	}

	if (group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE0)
	{
		return;
	}

	LARGE_INTEGER time_out = RtlConvertLongToLargeInteger(0);
	NTSTATUS status = KeWaitForSingleObject(&group_info->activity_timer, Executive, KernelMode, FALSE, &time_out);
	if (STATUS_TIMEOUT == status)
	{
		return;
	}
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wait activity_timer failed, error %!STATUS!", status);
		return;
	}

	IOMirrorCmd *cmd = (IOMirrorCmd *)MallocCmd(0, sizeof(DPP_ATTENTION) + sizeof(DPP_ATTENTION_PAYLOAD_ACTIVITY), group_info);
	if (NULL == cmd)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
		return;
	}

	uint64_t cbt_backlog = 0, resync_remaining = 0;
	GetAllVolBitmapSize(group_info->vol_queue, &cbt_backlog, &resync_remaining);

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::cbt_backlog %llu, resync_remaining %llu", cbt_backlog, resync_remaining);

	FillActivityCmd(cmd, group_info, cbt_backlog, resync_remaining);

	status = ImSendSync(group_info, socket_info, cmd, cmd);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
		ProcessDisconnect(group_info);
		return;
	}

	time_out = RtlConvertLongToLargeInteger(-1 * IM_PG_ACTIVITY_INTERVAL);
	KeSetTimer(&group_info->activity_timer, time_out, NULL);
}

VOID MergePersistBitmap(GroupInfo *group_info, QueueInfo *vol_queue)
{
	if (TRUE == IsEmptyVolQueue(vol_queue))
	{
		return;
	}

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	UCHAR* bit_data = NULL;
	ULONGLONG bit_count = 0;
	uint64_t size = 0;
	uint64_t nb_sectors = (uint64_t)1U << group_info->bitmap_granularity;
	NTSTATUS status;
	while (req_entry != head)
	{
		VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->persist_data_apply_state == VOL_PERSIST_DATA_APPLY_STATE_DONE)
		{
			req_entry = req_entry->Flink;
			continue;
		}

		if (group_info->per_data_state == PERSIST_DATA_STATE_SUCCEED)
		{
			size = (vol_info->sectors + nb_sectors - 1) / nb_sectors;
			size = (size + IM_PG_BITS_PER_BYTE - 1) / IM_PG_BITS_PER_BYTE;
			bit_data = (UCHAR*)ExAllocatePoolWithTag(PagedPool, size, ALLOC_TAG);
			if (bit_data != NULL)
			{
				status = GetPersistBitmapByVol(group_info, vol_info, bit_data, (uint32_t)size, &bit_count);
				if (!NT_SUCCESS(status))
				{
					ExFreePoolWithTag(bit_data, ALLOC_TAG);
					bit_data = NULL;
				}
			}
		}
		else
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::No persist bitmap for volume, disk %wZ, start_pos %llu, end_pos %llu", &vol_info->disk_name, vol_info->start_pos, vol_info->end_pos);
		}

		if (bit_data == NULL)
		{
			BitmapSetBit(vol_info->bitmap, 0, vol_info->sectors);
		}
		else
		{
			if (!MergeBitMap(group_info, vol_info, bit_data, (uint32_t)size, bit_count))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Merge bitmap failed for volume, disk %wZ, start_pos %llu, end_pos %llu", &vol_info->disk_name, vol_info->start_pos, vol_info->end_pos);
				BitmapSetBit(vol_info->bitmap, 0, vol_info->sectors);
			}
		}

		if (bit_data)
		{
			ExFreePoolWithTag(bit_data, ALLOC_TAG);
			bit_data = NULL;
		}

		vol_info->persist_data_apply_state = VOL_PERSIST_DATA_APPLY_STATE_DONE;


		req_entry = req_entry->Flink;
	}

	ReleaseResource(&vol_queue->sync_resource);
}


VOID CheckBitmap(GroupInfo *group_info)
{
	QueueInfo *temp_vol_queue = group_info->temp_vol_queue;

	if (TRUE == IsEmptyVolQueue(temp_vol_queue))
	{
		return;
	}

	if (group_info->inter_state != IM_PG_INTER_STATE_CBT_DATA && group_info->inter_state != IM_PG_INTER_STATE_NORMAL)
	{
		return;
	}

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::temp_vol_queue number %d", temp_vol_queue->num);

	QueueInfo *vol_queue = group_info->temp_vol_queue;
	AcquireExclusiveResource(&vol_queue->sync_resource);
	MergePersistBitmap(group_info, vol_queue);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		RemoveEntryList(req_entry);
		vol_queue->num--;
		PushVolQueue(group_info->vol_queue, vol_info);
		req_entry = temp_req_entry;
	}
	ReleaseResource(&vol_queue->sync_resource);

	if (group_info->inter_state == IM_PG_INTER_STATE_NORMAL)
	{
		group_info->state = IM_PG_STATE_CBT;
		group_info->inter_state = IM_PG_INTER_STATE_CBT_START;
	}
}

BOOLEAN StartPrepare(SocketInfo *socket_info, GroupInfo *group_info)
{
	UNREFERENCED_PARAMETER(socket_info);

	if (IsEmptyVolQueue(group_info->vol_queue))
	{
		LogEvent(group_info->device_obj->DriverObject, LOG_EMPTY_VOL_QUEUE, STATUS_SUCCESS, NULL, NULL, NULL, 0);
		return FALSE;
	}

	if (!OpenPersistFile(group_info))
	{
		LogEvent(group_info->device_obj->DriverObject, LOG_OPEN_PERIST_FILE_FAIL, STATUS_SUCCESS, NULL, NULL, NULL, 0);
		return FALSE;
	}

	if (group_info->state == IM_PG_STATE_VERIFY)
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Verify to verify");

		group_info->inter_state = IM_PG_INTER_STATE_VERIFY_START;
		group_info->per_data_state = PERSIST_DATA_STATE_DISCARD;
	}
	else if (group_info->per_data_state == PERSIST_DATA_STATE_FAILED)
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Change to verify due to failure of persist data, previous state %d", group_info->state);

		group_info->state = IM_PG_STATE_VERIFY;
		group_info->inter_state = IM_PG_INTER_STATE_VERIFY_START;
		group_info->per_data_state = PERSIST_DATA_STATE_DISCARD;
	}
	else
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Change to CBT, previous state %d", group_info->state);

		MergePersistBitmap(group_info, group_info->vol_queue);

		group_info->state = IM_PG_STATE_CBT;
		group_info->inter_state = IM_PG_INTER_STATE_CBT_START;
	}

	LogEvent(group_info->device_obj->DriverObject, LOG_SYNC_START_UNDER_STATE, STATUS_SUCCESS, NULL, NULL, (PVOID)&group_info->state, sizeof(group_info->state));

	return TRUE;
}


VOID SpeedControl(SocketInfo *socket_info, GroupInfo *group_info)
{
	UNREFERENCED_PARAMETER(socket_info);

	if(InterlockedCompareExchange(&group_info->send_queue_depth, 0, 0) > MAX_SEND_QUEUE_SIZE)
	{
		group_info->speed_pause_flag = TRUE;
	}
	else
	{
		group_info->speed_pause_flag = FALSE;
	}
}

VOID ProcessPausePending(SocketInfo *socket_info, GroupInfo *group_info)
{
	if (!group_info->pause_pending)
	{
		return;
	}

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Process pause under state %d", group_info->state);

	IOMirrorCmd *cmd = (IOMirrorCmd *)MallocCmd(0, sizeof(DPP_FLUSH), group_info);
	if (NULL == cmd)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc cmd failed");
		return;
	}

	FillFlushCmd(cmd, group_info);

	NTSTATUS status = ImSendSync(group_info, socket_info, cmd, cmd);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
		ProcessDisconnect(group_info);
		return;
	}

	group_info->flow_control_pause_flag = TRUE;
	group_info->pause_pending = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Pause succeeded");
}


BOOLEAN ProcessInternalCmd(SocketInfo *socket_info, GroupInfo *group_info)
{
	BOOLEAN ret = FALSE;
	switch (group_info->inter_state)
	{
	case IM_PG_INTER_STATE_NORMAL:
		ret = SendNormalData(socket_info, group_info);
		break;
	case IM_PG_INTER_STATE_CBT_START:
		ret = StartCbt(socket_info, group_info);
		break;
	case IM_PG_INTER_STATE_CBT_END:
		ret = FinishCbt(socket_info, group_info);
		break;
	case IM_PG_INTER_STATE_CBT_DATA:
		ret = SendCbtData(socket_info, group_info);
		break;
	case IM_PG_INTER_STATE_VERIFY_START:
		ret = StartVerify(socket_info, group_info);
		break;
	case IM_PG_INTER_STATE_VERIFY_END:
		ret = FinishVerify(socket_info, group_info);
		break;
	case IM_PG_INTER_STATE_VERIFY_DATA:
		ret = SendVerifyData(socket_info, group_info);
		break;
	case IM_PG_INTER_STATE_CONNECT_STAGE0:
		ret = BuildConnect(socket_info, group_info);
		break;

	case IM_PG_INTER_STATE_CONNECT_STAGE1:
		ret = SendSessionLogin(socket_info, group_info);
		break;

	case IM_PG_INTER_STATE_CONNECT_STAGE2:
		ret = StartPrepare(socket_info, group_info);
		break;
	case IM_PG_INTER_STATE_STOP:
		break;
	case IM_PG_INTER_STATE_CONNECT_STAGE1_P:
	case IM_PG_INTER_STATE_CONNECT_STAGE2_P:
		ImSleep(SEND_MIN_WAIT_TIME);
		break;

	default:
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Unknown internal state %d.", group_info->inter_state);
		break;
	}

	return ret;
}

VOID ProcessAllCmd(SocketInfo *socket_info, GroupInfo *group_info)
{
	ProcessExternCtl(socket_info, group_info);

	if (group_info->state == IM_PG_STATE_ERROR)
	{
		ImSleep(ERROR_PAUSE_TIME);
		return;
	}

	CheckAndFlush(group_info);
	SendHeartBeat(socket_info, group_info);
	SendActivity(socket_info, group_info);

	if (group_info->resume_pending)
	{
		group_info->pause_pending = FALSE;
		group_info->flow_control_pause_flag = FALSE;
		group_info->resume_pending = FALSE;
	}

	if (TRUE == group_info->flow_control_pause_flag)
	{
		TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!:Flow control pause");
		ImSleep(PAUSE_TIME);
		return;
	}

	if (TRUE == group_info->speed_pause_flag)
	{
		TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!:Speed control pause");
		ImSleep(SEND_MIN_WAIT_TIME);
		return;
	}

	BOOLEAN ret = ProcessInternalCmd(socket_info, group_info);

	if (group_info->inter_state == IM_PG_INTER_STATE_STOP
		|| group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE0)
	{
		ImSleep(RECONNECT_TIME);
		return;
	}

	if (!ret)
	{
		ImSleep(SEND_MIN_WAIT_TIME);
	}

	CheckBitmap(group_info); //当网络模块加载完后有新的卷加载时的处理
}

VOID CleanUpThread(SocketInfo *socket_info, GroupInfo *group_info)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Exit write thread");
	
	while (InterlockedCompareExchange(&group_info->send_queue_depth, 0, 0) != 0)
	{
		ImSleep(PAUSE_TIME);
	}

	KeCancelTimer(&group_info->heart_beat_timer);
	KeCancelTimer(&group_info->activity_timer);
	KeCancelTimer(&group_info->dataset_timer);

	DrCloseConnectionEndpoint(&socket_info->end_point);
	DrCloseTransportAddress(&socket_info->address);
	PsTerminateSystemThread(STATUS_SUCCESS);

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Send thread end");
}

VOID SocketThread(PVOID context)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Enter SocketThread");

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)context;

	GroupInfo *group_info = GetGroupInfo(pdx);

	//设置这个线程的优先级
	KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);
	CreateSocket(&group_info->socket_info, group_info);
	while (TRUE == group_info->send_thread_run_flag)
	{
		ProcessAllCmd(&group_info->socket_info, group_info);
		ReceiveAndProcessCmd(&group_info->socket_info, group_info);
	}

	CleanUpThread(&group_info->socket_info, group_info);

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Send thread exit");
}

NTSTATUS CreateSocketThread(PDEVICE_EXTENSION pdx)
{
	NTSTATUS status;
	HANDLE thread_handle = NULL;

	GroupInfo *group_info = GetGroupInfo(pdx);

	if (TRUE == group_info->send_thread_run_flag)
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Send thread already starts");
		return STATUS_SUCCESS; 
	}

	group_info->send_thread_run_flag = TRUE;
	status = PsCreateSystemThread(
		&thread_handle,
		(ACCESS_MASK)0L,
		NULL,
		NULL,
		NULL,
		SocketThread,
		pdx
		);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Create SocketThread failed, error %!STATUS!", status);
		group_info->send_thread_run_flag = FALSE;
	}
	else
	{
		InterlockedExchangePointer(&group_info->send_thread_handle, thread_handle);
	}

	return status;
}

BOOLEAN DestroySocketThread(GroupInfo *group_info)
{
	BOOLEAN ret = FALSE;
	BOOLEAN flag = FALSE;

	PVOID thread_object = NULL;

	do
	{
		group_info->send_thread_run_flag = FALSE;

		HANDLE thread_handle = (HANDLE)InterlockedCompareExchangePointer(&group_info->send_thread_handle, NULL, NULL);
		if (thread_handle == NULL)
		{
			break;
		}

		thread_handle = (HANDLE)InterlockedCompareExchangePointer(&group_info->send_thread_handle, NULL, thread_handle);
		if (thread_handle == NULL)
		{
			break;
		}

		NTSTATUS  status = ObReferenceObjectByHandle(thread_handle, SYNCHRONIZE, *PsThreadType, KernelMode, &thread_object, NULL);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get the thread object, error %!STATUS!", status);
			InterlockedExchangePointer(&group_info->send_thread_handle, thread_handle);
			break;
		}

		status = KeWaitForSingleObject(thread_object, Executive, KernelMode, FALSE, NULL);
		if (status != STATUS_SUCCESS)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to wait the thread object, error %!STATUS!", status);
			InterlockedExchangePointer(&group_info->send_thread_handle, thread_handle);
			break;
		}

		ZwClose(thread_handle);
		thread_handle = NULL;

		ret = TRUE;
	} while (flag);

	if (thread_object)
	{
		ObDereferenceObject(thread_object);
		thread_object = NULL;
	}
	
	return ret;
}