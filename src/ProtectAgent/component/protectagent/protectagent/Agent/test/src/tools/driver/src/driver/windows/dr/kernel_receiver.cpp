#include "kernel_receiver.h"
#include "regedit.h"

#include "persist_file.h"

#include "wpp_trace.h"
#include "kernel_receiver.tmh"



VOID ProcessSessionLoginAck(GroupInfo *group_info, IOMirrorCmd *cmd)
{
	BOOLEAN flag = FALSE;
	BOOLEAN succeed = FALSE;

	do
	{
		if (IM_PG_INTER_STATE_CONNECT_STAGE1_P != group_info->inter_state)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Receive ack, should IM_PG_INTER_STATE_CONNECT_STAGE1_P, but inter_state = %d", group_info->inter_state);
			break;
		}

		if ((cmd->header.flags & DPP_FLAG_FAIL) == DPP_FLAG_FAIL)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Ack failed");
			break;
		}

		PDPP_SESSION_LOGIN login_ack = (PDPP_SESSION_LOGIN)cmd->data;
		if (memcmp(login_ack->vm_id, group_info->protect_strategy.oma_id, VM_ID_LEN) != 0)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::oma_id is not correct, oma id %!HEXDUMP!(%!HEXDUMP!)", LOG_LENSTR(login_ack->vm_id, VM_ID_LEN), LOG_LENSTR(group_info->protect_strategy.oma_id, VM_ID_LEN));
			break;
		}

		PDPP_SESSION_LOGIN_ACK_AUX login_ack_aux = (PDPP_SESSION_LOGIN_ACK_AUX)login_ack->payload;
		if (login_ack_aux->dataset_id_sent < login_ack_aux->dataset_id_done)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Dataset_id %llu is smaller than dataset_id_done %llu", login_ack_aux->dataset_id_sent, login_ack_aux->dataset_id_done);
			break;
		}

		if (group_info->dataset_id_done > login_ack_aux->dataset_id_done)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Dataset_id_done %llu is higher than received one %llu, change to verify mode", group_info->dataset_id_done, login_ack_aux->dataset_id_done);
			group_info->per_data_state = PERSIST_DATA_STATE_FAILED;
		}

		if (group_info->dataset_id <= login_ack_aux->dataset_id_sent)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Dataset_id %llu is less than received one %llu, change to verify mode", group_info->dataset_id, login_ack_aux->dataset_id_sent);
			group_info->dataset_id = login_ack_aux->dataset_id_sent + 1;
			group_info->dataset_id_done = login_ack_aux->dataset_id_done;

			group_info->per_data_state = PERSIST_DATA_STATE_FAILED;
		}

		uint64_t size_low = (uint64_t)1U << IM_MAX_BITMAP_GRANULARITY;
		size_low *= IM_SECTOR_SIZE;
		if (size_low > MAX_BLOCK_SIZE)
		{
			size_low = MAX_BLOCK_SIZE;
		}

		group_info->max_set_data_size = login_ack_aux->max_dataset_size;
		group_info->max_set_data_size &= ~((1 << 20) - 1);
		if (group_info->max_set_data_size < size_low)
		{
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Max dataset size %llu(%llu) is too small", group_info->max_set_data_size, size_low);
			break;
		}

		group_info->seg_size = group_info->max_set_data_size / IM_SEG_SIZE_OPERATOR;
		group_info->seg_size &= ~((1 << 20) - 1);
		if (group_info->seg_size < size_low)
		{
			TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Seg size %llu(%llu) is too small", group_info->seg_size, size_low);
			break;
		}

		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::dataset_id_sent %llu(%llu), dataset_id_done %llu(%llu), Max data set size %llu(%llu), seg size %llu, credit %d", login_ack_aux->dataset_id_sent, group_info->dataset_id, login_ack_aux->dataset_id_done, group_info->dataset_id_done, group_info->max_set_data_size, login_ack_aux->max_dataset_size, group_info->seg_size, login_ack_aux->buf_credit);

		group_info->global_credit = login_ack_aux->buf_credit;

		group_info->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE2;

		succeed = TRUE;
	} while (flag);

	if (!succeed)
	{
		ProcessDisconnect(group_info);
	}
}

VOID ProcessHeartBeatAck(GroupInfo *group_info, IOMirrorCmd *cmd)
{
	UNREFERENCED_PARAMETER(cmd);

	group_info->last_heart_time = GetCurTime();

	if (group_info->heart_num == 0)
	{
		return;
	}
	group_info->heart_num--;
}

VOID ProcessResyncSetDone(GroupInfo *group_info, IOMirrorCmd *cmd)
{
	BOOLEAN flag = FALSE;
	BOOLEAN succeed = FALSE;

	do
	{
		PDPP_DATASET_DONE dataset_done = (PDPP_DATASET_DONE)cmd->data;

		TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Received resync set done, id %llu", dataset_done->dataset_id);

		if (memcmp(dataset_done->vm_id, group_info->protect_strategy.osId, VM_ID_LEN) != 0)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong vm_id");
			break;
		}

		if (cmd->header.cmd_type != DPP_TYPE_RESYNCSET_DONE)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong cmd_type %d", cmd->header.cmd_type);
			break;
		}

		if ((cmd->header.flags & DPP_FLAG_FAIL) == DPP_FLAG_FAIL)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Dataset failed");
			break;
		}

		if (group_info->dataset_id < dataset_done->dataset_id)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Dataset id %llu is less than the received one %llu.Change to verify mode", group_info->dataset_id_done, dataset_done->dataset_id);
			break;
		}

		group_info->dataset_id_done = dataset_done->dataset_id;

		uint32_t queue_num = group_info->pending_cmd_queue->num;

		if (!RemovePendingCmd(group_info, dataset_done->dataset_id))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Remove pending cmd failed");
			break;
		}

		TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Removed pending queue, current number %d, previous %d", group_info->pending_cmd_queue->num, queue_num);

		succeed = TRUE;
	} while (flag);

	CheckRemoveDelayDeleteQueue(group_info);

	if (!succeed)
	{
		if (group_info->state == IM_PG_STATE_VERIFY)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Process resync set done failed, go to verify mode");
		}
		else
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Process resync set done failed, go to CBT mode");
		}

		ProcessDisconnect(group_info);
	}
}

VOID ProcessDataSetDone(GroupInfo *group_info, IOMirrorCmd *cmd)
{
	BOOLEAN flag = FALSE;
	BOOLEAN succeed = FALSE;

	do
	{
		PDPP_DATASET_DONE dataset_done = (PDPP_DATASET_DONE)cmd->data;

		TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Received data set done, id %llu", dataset_done->dataset_id);

		if (memcmp(dataset_done->vm_id, group_info->protect_strategy.osId, VM_ID_LEN) != 0)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong vm_id");
			break;
		}

		if (cmd->header.cmd_type != DPP_TYPE_DATASET_DONE)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong cmd type %d", cmd->header.cmd_type);
			break;
		}

		if ((cmd->header.flags & DPP_FLAG_FAIL) == DPP_FLAG_FAIL)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Dataset failed");
			break;
		}

		if (group_info->dataset_id < dataset_done->dataset_id)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Dataset done id %llu is less than the received one %llu. Change to verify mode", group_info->dataset_id_done, dataset_done->dataset_id);
			break;
		}

		group_info->dataset_id_done = dataset_done->dataset_id;

		uint32_t queue_num = group_info->pending_cmd_queue->num;

		if (!RemovePendingCmd(group_info, dataset_done->dataset_id))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Remove pending cmd failed");
			break;
		}

		TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Removed pending queue, current number %d, previous %d", group_info->pending_cmd_queue->num, queue_num);

		succeed = TRUE;
	} while (flag);

	CheckRemoveDelayDeleteQueue(group_info);

	if (!succeed)
	{
		if (group_info->state == IM_PG_STATE_VERIFY)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Process resync set done failed, go to verify mode");
		}
		else
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Process resync set done failed, go to CBT mode");
		}

		ProcessDisconnect(group_info);
	}
}

VOID ProcessCredit(GroupInfo *group_info, IOMirrorCmd *cmd)
{
	UNREFERENCED_PARAMETER(group_info);

	BOOLEAN flag = FALSE;
	BOOLEAN succeed = FALSE;

	do
	{
		PDPP_CREDIT_BUFFER credit_buffer = (PDPP_CREDIT_BUFFER)cmd->data;

		TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Received credit, buffer %d", credit_buffer->buf_credit);

		if (memcmp(credit_buffer->vm_id, group_info->protect_strategy.osId, VM_ID_LEN) != 0)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong vm_id");
			break;
		}

		if (cmd->header.cmd_type != DPP_TYPE_CREDIT)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong cmd type %d", cmd->header.cmd_type);
			break;
		}

		uint32_t credit = group_info->global_credit;

		group_info->global_credit += credit_buffer->buf_credit;

		TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Set credit, current size %d, previous %d", group_info->global_credit, credit);

		succeed = TRUE;
	} while (flag);

	if (!succeed)
	{
		ProcessDisconnect(group_info);
	}
}

VOID ReceiveAndProcessCmd(SocketInfo *socket_info, GroupInfo *group_info)
{
	NTSTATUS status;
	IOMirrorCmd *cmd = NULL;
	int i = 0;

	if (group_info->inter_state == IM_PG_INTER_STATE_STOP
		|| group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE0
		|| group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE1)
	{
		return;
	}
	for (i = 0; i < MAX_PROCESS_NUM; i++)
	{
		status = ImReceive(socket_info);
		if (STATUS_TIMEOUT == status)
		{
			return;
		}
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Receive failed, error %!STATUS!", status);
			ProcessDisconnect(group_info);
			return;
		}

		cmd = (IOMirrorCmd *)socket_info->rev_buf;
		switch (cmd->header.cmd_type)
		{
		case DPP_TYPE_SESSION_LOGIN_ACK:
			ProcessSessionLoginAck(group_info, cmd);
			break;
		case DPP_TYPE_HEARTBEAT_ACK:
			ProcessHeartBeatAck(group_info, cmd);
			break;
		case DPP_TYPE_CREDIT:
			ProcessCredit(group_info, cmd);
			break;
		case DPP_TYPE_RESYNCSET_DONE:
			ProcessResyncSetDone(group_info, cmd);
			break;
		case DPP_TYPE_DATASET_DONE:
			ProcessDataSetDone(group_info, cmd);
			break;
		default:
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Unknown cmd_type = %d", cmd->header.cmd_type);
			break;
		}
	}
}