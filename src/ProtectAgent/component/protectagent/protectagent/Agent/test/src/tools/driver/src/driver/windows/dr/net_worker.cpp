#include "net_worker.h"

#include "wpp_trace.h"
#include "net_worker.tmh"


#define TCP_DEVICE L"\\Device\\Tcp"

//内核态发送需要的资源对象，创建和绑定Socket
VOID CreateSocket(SocketInfo *socket_info, GroupInfo *group_info)
{
	while (1)
	{
		NTSTATUS status;
		if (socket_info->address.h_address == NULL)
		{
			status = DrOpenTransportAddress(TCP_DEVICE, &socket_info->address);
			if (STATUS_SUCCESS != status)
			{
				if (STATUS_OBJECT_NAME_NOT_FOUND != status)
				{
					TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Net work is closed, error %!STATUS!", status);
				}

				goto end;
			}
		}
		status = DrOpenConnectionEndpoint(TCP_DEVICE, &socket_info->address, &socket_info->end_point, NULL);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Open Endpoint failed, error %!STATUS!", status);
		}
		else
		{
			return;
		}
	end:
		if (!group_info->send_thread_run_flag)
		{
			return;
		}

		ImSleep(CREATE_SOCKET_TIME);
	}
}

NTSTATUS ImReceiveSync(SocketInfo *socket_info, char *data, uint32_t size)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMDL mdl = NULL;
	uint32_t residual = size;
	while (residual != 0)
	{
		uint32_t rev_size = 0;
		mdl = IoAllocateMdl(data, residual, FALSE, FALSE, NULL);
		if(NULL == mdl)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc mdl");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		__try
		{
			MmProbeAndLockPages(mdl, KernelMode, IoModifyAccess);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			IoFreeMdl(mdl);
			mdl = NULL;

			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to lock page");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		status = DrReceiveSync(&socket_info->end_point, mdl, 0, (PULONG)&rev_size);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrReceiveSync failed, error %!STATUS!", status);
			break;
		}

		if (rev_size == 0)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Receive size is zero");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		residual -= rev_size;
		data += rev_size;

		DrUnlockAndFreeMdl(mdl);
		mdl = NULL;
	}

	if (mdl)
	{
		DrUnlockAndFreeMdl(mdl);
		mdl = NULL;
	}

	return status;
}

NTSTATUS ImReceiveAsyncEx(SocketInfo *socket_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	IOMirrorCmd *cmd = NULL;
	PIO_STATUS_BLOCK rev_io_status = socket_info->rev_io_status;
	if (0 == rev_io_status->Information)
	{
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}
	if (rev_io_status->Information < CMD_HEAD)
	{
		status = ImReceiveSync(socket_info, socket_info->rev_buf+rev_io_status->Information, (uint32_t)(CMD_HEAD - rev_io_status->Information));
		if(status != STATUS_SUCCESS) 
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ImReceiveSync failed, error %!STATUS!", status);
			goto out;
		}
	}
	else if (rev_io_status->Information > CMD_HEAD)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::need head size %d, got more(%d)", CMD_HEAD, (ULONG)rev_io_status->Information);
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}

	cmd = (IOMirrorCmd *)socket_info->rev_buf;
	CmdHeaderByteSwap(&cmd->header);

	if(cmd->header.magic != DPP_MAGIC)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Magic failed, %x", cmd->header.magic);
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}

	if (0 == cmd->header.body_len)
	{
		goto out;
	}

	status = ImReceiveSync(socket_info, socket_info->rev_buf+CMD_HEAD, cmd->header.body_len);
	if(status != STATUS_SUCCESS) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ImReceiveSync failed, error %!STATUS!", status);
		goto out;
	}

	CmdBodyByteHostSwap(cmd);

out:
	return status;
}

NTSTATUS ImReceiveAsync(SocketInfo *socket_info)
{
	NTSTATUS status = STATUS_SUCCESS;

	status = DrReceiveAsync(
		&socket_info->end_point,
		socket_info->rev_head_mdl,
		socket_info->flags,
		socket_info->rev_event,
		socket_info->rev_io_status
		);
	if (status != STATUS_SUCCESS) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrReceiveAsync failed, error %!STATUS!", status);
		status = STATUS_NET_ERROR;
	}

	return status;
}

NTSTATUS ImReceive(SocketInfo *socket_info)
{
	NTSTATUS status;
	LARGE_INTEGER time_out = RtlConvertLongToLargeInteger(0);
	if (FALSE == socket_info->is_receiving)
	{
		status = ImReceiveAsync(socket_info);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Recv failed, error %!STATUS!", status);
			return status;
		}
		socket_info->is_receiving = TRUE;
	}	
	status = KeWaitForSingleObject(socket_info->rev_event, Executive, KernelMode, FALSE, &time_out);
	if (STATUS_TIMEOUT == status)
	{
		return status;
	}
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wait failed, error %!STATUS!", status);
		return status;
	}
	status = ImReceiveAsyncEx(socket_info);
	if (STATUS_SUCCESS != status)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Recv failed, error %!STATUS!", status);
		return status;
	}
	socket_info->is_receiving = FALSE;
	return status;
}

NTSTATUS ImSendBufferSync(SocketInfo *socket_info, PVOID buffer, uint32_t size)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	PMDL mdl = NULL;
	do
	{
		mdl = IoAllocateMdl(buffer, size, FALSE, FALSE, NULL);
		if (NULL == mdl)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc mdl failed!");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		//MmBuildMdlForNonPagedPool(mdl); //非分页内存初始化
		__try
		{
			MmProbeAndLockPages(mdl, KernelMode, IoModifyAccess);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			IoFreeMdl(mdl);
			mdl = NULL;

			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Lock page failed");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		status = DrSendSync(&socket_info->end_point, mdl, 0);
		if (STATUS_SUCCESS != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Send failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	if (mdl)
	{
		DrUnlockAndFreeMdl(mdl);
		mdl = NULL;
	}

	return status;
}

NTSTATUS ImSendSync(GroupInfo *group_info, SocketInfo *socket_info, IOMirrorCmd *cmd, PVOID free_data)
{
	NTSTATUS status = STATUS_SUCCESS;;

	ULONG body_len = cmd->header.body_len;
	CmdByteNetworkSwap(cmd);

	status = ImSendBufferSync(socket_info, (char *)cmd, sizeof(IOMirrorCmd) + body_len);

	CmdByteHostSwap(cmd);

	if (free_data)
	{
		FreeCmd((IOMirrorCmd *)free_data, group_info);
	}

	return status;
}

VOID ProcessErrorState(GroupInfo *group_info)
{
	if (group_info->state == IM_PG_STATE_ERROR || group_info->state == IM_PG_STATE_STOP)
	{
		return;
	}

	group_info->state = IM_PG_STATE_ERROR;
	ClearVolQueue(group_info->vol_queue);
	ClearVolQueue(group_info->temp_vol_queue);
	ClearCmdQueue(group_info->cmd_queue, group_info);
	ClearCmdQueue(group_info->temp_cmd_queue, group_info);
	ClearCmdQueue(group_info->pending_cmd_queue, group_info);
	CheckRemoveDelayDeleteQueue(group_info);
	CheckRemoveDelayDeleteVolQueue(group_info);
}

VOID ProcessDisconnect(GroupInfo *group_info)
{
	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Enter ProcessDisconnect, inter_state = %d", group_info->inter_state);

	if (group_info->state == IM_PG_STATE_ERROR || group_info->state == IM_PG_STATE_STOP || group_info->inter_state == IM_PG_INTER_STATE_CONNECT_STAGE0)
	{
		return;
	}

	group_info->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;

	group_info->socket_info.is_receiving = FALSE;
	memset(group_info->socket_info.rev_io_status, 0, sizeof(IO_STATUS_BLOCK));
	KeResetEvent(group_info->socket_info.rev_event);

	ImSleep(RECONNECT_TIME);
}