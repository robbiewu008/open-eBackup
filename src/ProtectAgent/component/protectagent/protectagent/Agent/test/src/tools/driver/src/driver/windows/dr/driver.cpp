#include "driver.h"
#include "dispatch.h"
#include "kernel_sender.h"
#include "kernel_receiver.h"
#include "group_mem.h"
#include "ioctl.h"
#include "regedit.h"
#include "cdo.h"

#include "wpp_trace.h"
#include "driver.tmh"




VOID StartDevice(IN GroupInfo *group_info, IN PDEVICE_EXTENSION pdx);
VOID RemoveDevice(IN GroupInfo *group_info, IN PDEVICE_EXTENSION pdx);

VOID HookDriver(IN PDRIVER_HOOK drv_hook, IN PDRIVER_OBJECT drv_obj);
PDRIVER_DISPATCH GetOriginalFunction(IN GroupInfo *group_info, IN PDRIVER_OBJECT drv_obj, UCHAR code);
PDEVICE_OBJECT GetPdoByDiskNum(IN ULONG disk_num);
PDEVICE_EXTENSION CreatePdxByPdo(IN PDEVICE_OBJECT pdo);
NTSTATUS GetDiskInfoByPdx(IN PDEVICE_EXTENSION pdx);



extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

	WPP_INIT_TRACING(DriverObject, RegistryPath);

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->DriverExtension->AddDevice = AddDevice;
	for (int i = 0; i < arraysize(DriverObject->MajorFunction); ++i)
		DriverObject->MajorFunction[i] = CdoDispatchAny;

	DriverObject->MajorFunction[IRP_MJ_POWER] = CdoDispatchPower;
	DriverObject->MajorFunction[IRP_MJ_PNP] = CdoDispatchPnp;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = CdoDeviceCtl;
	DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = CdoShutdown;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CdoCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CdoClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = CdoCleanUp;

	LogEvent(DriverObject, LOG_DRIVER_LOADED, STATUS_SUCCESS, NULL, NULL, NULL, 0);

	return STATUS_SUCCESS;
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE();

	WPP_CLEANUP(DriverObject);
}

NTSTATUS HookDevice(IN PDEVICE_EXTENSION pdx)
{
	NTSTATUS  status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	do
	{
		GroupInfo* group_info = GetGroupInfoByCdo();
		PDRIVER_OBJECT drv_obj = pdx->hook_device->DriverObject;
		if (FindHookByDriverObject(group_info->driver_hook_queue, drv_obj))
		{
			break;
		}

		PDRIVER_HOOK_ENTRY hook_entry = (PDRIVER_HOOK_ENTRY)ExAllocatePoolWithTag(NonPagedPool, sizeof(DRIVER_HOOK_ENTRY), ALLOC_TAG);
		if (hook_entry == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc DRIVER_HOOK failed, error %!STATUS!", status);
			break;
		}

		RtlZeroMemory(hook_entry, sizeof(DRIVER_HOOK_ENTRY));

		PDRIVER_HOOK drv_hook = &hook_entry->driver_hook;
		KeInitializeSpinLock(&drv_hook->lock);
		drv_hook->hook_driver = drv_obj;
		HookDriver(drv_hook, drv_obj);

		PushDriverHookQueue(group_info->driver_hook_queue, hook_entry);
	} while (flag);
	
	return status;
}

PDEVICE_OBJECT GetHookDevice(IN PDEVICE_OBJECT pdo)
{
	PDEVICE_OBJECT ret = IoGetAttachedDeviceReference(pdo);
	PDEVICE_OBJECT low_dev = IoGetLowerDeviceObject(ret);
	while (low_dev && low_dev != pdo)
	{
		if (ret)
		{
			ObDereferenceObject(ret);
		}

		ret = low_dev;
		low_dev = IoGetLowerDeviceObject(low_dev);
	}

	if (low_dev)
	{
		ObDereferenceObject(low_dev);
	}

	return ret;
}

PDEVICE_OBJECT GetPdoByDeviceObject(IN PDEVICE_OBJECT dev_obj)
{
	PDEVICE_OBJECT ret = IoGetAttachedDeviceReference(dev_obj);
	PDEVICE_OBJECT low_dev = IoGetLowerDeviceObject(ret);
	while (low_dev)
	{
		if (ret)
		{
			ObDereferenceObject(ret);
		}

		ret = low_dev;
		low_dev = IoGetLowerDeviceObject(low_dev);
	}

	if (low_dev)
	{
		ObDereferenceObject(low_dev);
	}

	return ret;
}

NTSTATUS AddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT pdo)
{
	UNREFERENCED_PARAMETER(DriverObject);

	if (IsControlPdo(pdo))
	{
		return CdoAddDevice(DriverObject, pdo);
	}

	HoldCdoRemoveLock();

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdx = NULL;

	BOOLEAN flag = FALSE;

	GroupInfo *group_info = GetGroupInfoByCdo();

	do
	{
		pdx = CreatePdxByPdo(pdo);
		if (pdx == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to create pdx by pdo");
			break;
		}

		status = HookDevice(pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to hook device %wZ, error %!STATUS!", &pdx->disk_name, status);
			break;
		}

		status = CreateGroupMem(pdx, NULL, FALSE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::SectionCreateIner failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		if (pdx)
		{
			FreeDeviceExtension(pdx);
			pdx = NULL;
		}

		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Stop protection becuae add device failed, error %!STATUS!", status);

		if (!NT_SUCCESS(SendErrorExternCtl(group_info)))
		{
			group_info->state = IM_PG_STATE_ERROR;
		}
	}

	ReleaseCdoRemoveLock();

	return STATUS_SUCCESS;
}

#pragma LOCKEDCODE
NTSTATUS SplitWrite( GroupInfo *group_info, VolInfo *vol_info, LARGE_INTEGER offset, ULONG length, PBYTE sys_buf, PIO_COMP_CONTEXT* comp_context)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	QueueInfo *queue_info = NULL;
	PIO_COMP_CONTEXT temp_context = NULL;
	do
	{
		queue_info = (QueueInfo *)BuildCmdQueue(length, sizeof(DPP_DATA), group_info, vol_info);
		if (NULL == queue_info)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc queue_info failed, error %!STATUS!", status);
			break;
		}

		if (queue_info->num >= MAX_CMD_PER_COMPLETION)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::There are %d cmd in one Io requestor, not support, error %!STATUS!", queue_info->num, status);
			break;
		}

		temp_context = (PIO_COMP_CONTEXT)ExAllocateFromNPagedLookasideList(&group_info->completion_context_npage_list);
		if (NULL == temp_context)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc comp_context failed, error %!STATUS!", status);
			break;
		}

		RtlZeroMemory(temp_context, sizeof(IO_COMP_CONTEXT));
		temp_context->group_info = group_info;

		PLIST_ENTRY head = &queue_info->head;
		char *cur_buf = (char *)sys_buf;
		uint64_t buf_offset = 0;
		PLIST_ENTRY req_entry = head->Flink;
		while (req_entry != head)
		{
			CmdNode *cmd_node = CONTAINING_RECORD(req_entry, CmdNode, list_entry);
			FillDataCmd(cmd_node->cmd, cur_buf, cmd_node->cmd->header.body_len, cmd_node->cmd->header.body_len, (uint64_t)offset.QuadPart + buf_offset, vol_info, group_info);

			PDPP_DATA dpp_data = (PDPP_DATA)cmd_node->cmd->data;
			cur_buf += dpp_data->data_size;
			buf_offset += dpp_data->data_size;
			req_entry = req_entry->Flink;

			temp_context->cmd_node_list[temp_context->node_count++] = cmd_node;
		}

		KIRQL old_irql;
		KeAcquireSpinLock(&group_info->cmd_queue->lock, &old_irql);
		queue_info->head.Flink->Blink = group_info->cmd_queue->head.Blink;
		queue_info->head.Blink->Flink = &group_info->cmd_queue->head;
		group_info->cmd_queue->head.Blink->Flink = queue_info->head.Flink;
		group_info->cmd_queue->head.Blink = queue_info->head.Blink;
		group_info->cmd_queue->num += queue_info->num;
		group_info->cmd_queue->size += queue_info->size;
		InterlockedAdd64(&group_info->queue_set_data_size, queue_info->size);

		InitializeListHead(&queue_info->head);
		queue_info->num = 0;
		queue_info->size = 0;
		KeReleaseSpinLock(&group_info->cmd_queue->lock, old_irql);
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		if (temp_context)
		{
			ExFreeToNPagedLookasideList(&group_info->completion_context_npage_list, temp_context);
			temp_context = NULL;
		}

		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Stop protection because some Io failed to be handled, error %!STATUS!", status);
		SendErrorExternCtl(group_info);
	}

	if (queue_info)
	{
		FreeCmdQueueNLock(queue_info, group_info);
		queue_info = NULL;
	}

	*comp_context = temp_context;

	return status;
}

#pragma LOCKEDCODE
NTSTATUS DispatchWrite(IN PDEVICE_OBJECT fido, IN PIRP irp)
{
	HoldCdoRemoveLock();

	NTSTATUS status;
	BOOLEAN flag = FALSE;

	GroupInfo *group_info = GetGroupInfoByCdo();
	BOOLEAN protecting = FALSE;
	PIO_COMP_CONTEXT comp_context = NULL;
	VolInfo *vol_info = NULL;
	PDEVICE_EXTENSION pdx = NULL;
	BOOLEAN remove_locked = FALSE;

	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);

	do
	{
		if (group_info->state == IM_PG_STATE_STOP || group_info->state == IM_PG_STATE_ERROR)
		{
			break;
		}

		pdx = FindDeviceExtensionByHookDevice(group_info->dev_ext_queue, fido);
		if (pdx == NULL)
		{
			break;
		}

		status = IoAcquireRemoveLock(&pdx->RemoveLock, pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to acquire remove lock, error %!STATUS!", status);
			break;
		}

		remove_locked = TRUE;

		ULONG length = 0;
		LARGE_INTEGER offset = { 0 };
		offset = irpsp->Parameters.Write.ByteOffset;
		length = irpsp->Parameters.Write.Length;

		vol_info = NeedProtected(pdx, (uint64_t)offset.QuadPart);
		if (vol_info == NULL)
		{
			break;
		}

		offset.QuadPart -= vol_info->start_pos;

		PBYTE sys_buf = NULL;
		if (NULL == irp->MdlAddress)
		{
			sys_buf = (PBYTE)irp->UserBuffer;
		}
		else
		{
			sys_buf = (PBYTE)MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority | MdlMappingNoExecute);
		}

		status = SplitWrite(group_info, vol_info, offset, length, sys_buf, &comp_context);
		if (!NT_SUCCESS(status))
		{
			break;
		}

		protecting = TRUE;
	} while (flag);

	if (!protecting && remove_locked)
	{
		IoReleaseRemoveLock(&pdx->RemoveLock, pdx);
	}

	if (vol_info)
	{
		InterlockedDecrement(&vol_info->reference_count);
	}

	if (protecting)
	{
		comp_context->comp_routine = irpsp->CompletionRoutine;
		comp_context->real_context = irpsp->Context;
		comp_context->comp_control = irpsp->Control;
		comp_context->pdx = pdx;

		irpsp->CompletionRoutine = IoCompletion;
		irpsp->Context = comp_context;
		irpsp->Control = SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL;
	}

	PDRIVER_DISPATCH disp_write = GetOriginalFunction(group_info, fido->DriverObject, IRP_MJ_WRITE);
	status = disp_write(fido, irp);

	if (!protecting)
	{
		ReleaseCdoRemoveLock();
	}

	return status;
}

#pragma LOCKEDCODE
BOOL NeedRedirect(IN PIRP irp, IN UCHAR org_control, IN PIO_COMPLETION_ROUTINE org_routine)
{
	BOOLEAN redirect = FALSE;
	if ((NT_SUCCESS(irp->IoStatus.Status) && org_control & SL_INVOKE_ON_SUCCESS) ||
		(!NT_SUCCESS(irp->IoStatus.Status) && org_control & SL_INVOKE_ON_ERROR) ||
		(irp->Cancel && org_control & SL_INVOKE_ON_CANCEL))
	{
		redirect = TRUE;
	}

	if (!org_routine)
	{
		redirect = FALSE;
	}

	return redirect;
}

#pragma LOCKEDCODE
LONG GetCompletionStatus(IN PIRP irp)
{
	LONG io_status = (STATUS_SUCCESS == irp->IoStatus.Status) ? IO_COMPLETION_STATUS_SUCCEED : IO_COMPLETION_STATUS_FAILED;
	if (irp->Cancel)
	{
		io_status = IO_COMPLETION_STATUS_FAILED;
	}

	return io_status;
}

#pragma LOCKEDCODE
NTSTATUS IoCompletion(IN PDEVICE_OBJECT device_object, IN PIRP irp, IN PVOID context)
{
	UNREFERENCED_PARAMETER(device_object);

	PIO_COMPLETION_ROUTINE org_routine = NULL;
	PVOID org_context = NULL;
	UCHAR org_control = 0;
	PDEVICE_EXTENSION pdx = NULL;

	PIO_COMP_CONTEXT comp_context = (PIO_COMP_CONTEXT)context;
	if (comp_context)
	{
		LONG io_status = GetCompletionStatus(irp);

		for (uint32_t i = 0; i < comp_context->node_count; i++)
		{
			InterlockedExchange(&comp_context->cmd_node_list[i]->io_status, io_status);
		}

		org_routine = comp_context->comp_routine;
		org_context = comp_context->real_context;
		org_control = comp_context->comp_control;
		pdx = comp_context->pdx;
		
		GroupInfo* group_info = comp_context->group_info;
		ExFreeToNPagedLookasideList(&group_info->completion_context_npage_list, comp_context);
		comp_context = NULL;
	}

	PIO_STACK_LOCATION irpsp = IoGetNextIrpStackLocation(irp);
	if (irpsp && irpsp->CompletionRoutine == IoCompletion)
	{
		irpsp->CompletionRoutine = org_routine;
		irpsp->Context = org_context;
	}

	NTSTATUS status;
	if (NeedRedirect(irp, org_control, org_routine))
	{
		status = org_routine(device_object, irp, org_context);
	}
	else
	{
		if (irp->PendingReturned)
		{
			IoMarkIrpPending(irp);
		}

		status = STATUS_CONTINUE_COMPLETION;
	}

	if (pdx)
	{
		IoReleaseRemoveLock(&pdx->RemoveLock, pdx);
	}

	ReleaseCdoRemoveLock();

	return status;
}


NTSTATUS DispatchPnp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PAGED_CODE();

	HoldCdoRemoveLock();

	NTSTATUS status;
	BOOLEAN flag = FALSE;
	BOOLEAN remove_locked = FALSE;
	PDEVICE_EXTENSION pdx = NULL;
	do
	{
		GroupInfo *group_info = GetGroupInfoByCdo();

		PDRIVER_DISPATCH disp_pnp = GetOriginalFunction(group_info, DeviceObject->DriverObject, IRP_MJ_PNP);
		PIO_STACK_LOCATION  StackLocation = IoGetCurrentIrpStackLocation(Irp);
		UCHAR minor_code = StackLocation->MinorFunction;

		status = disp_pnp(DeviceObject, Irp);
		if (!NT_SUCCESS(status))
		{
			break;
		}

		pdx = FindDeviceExtensionByHookDevice(group_info->dev_ext_queue, DeviceObject);
		if (!pdx)
		{
			break;
		}

		NTSTATUS status_inter = IoAcquireRemoveLock(&pdx->RemoveLock, pdx);
		if (!NT_SUCCESS(status_inter))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to acquire remove lock, error %!STATUS!", status_inter);
			break;
		}

		remove_locked = TRUE;

		if (minor_code == IRP_MN_START_DEVICE)
		{
			StartDevice(group_info, pdx);
		}
		else if (minor_code == IRP_MN_REMOVE_DEVICE)
		{
			RemoveDevice(group_info, pdx);
			remove_locked = FALSE;
		}
	} while (flag);

	if (remove_locked)
	{
		IoReleaseRemoveLock(&pdx->RemoveLock, pdx);
	}	
	
	ReleaseCdoRemoveLock();

	return status;
}



VOID StartDevice(IN GroupInfo *group_info, IN PDEVICE_EXTENSION pdx)
{
	PAGED_CODE();

	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;
	do
	{
		status = GetDiskInfoByPdx(pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get disk info for %wZ", &pdx->disk_name);
			break;
		}

		status = StartProtectByReg(pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Start protect by reg failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Stop protection because some Io failed to be handled, error %!STATUS!", status);
		if (!NT_SUCCESS(SendErrorExternCtl(group_info)))
		{
			group_info->state = IM_PG_STATE_ERROR;
		}
	}
}

VOID RemoveDevice(IN GroupInfo *group_info, IN PDEVICE_EXTENSION pdx)
{
	PAGED_CODE();

	RemoveVolInfoByPdx(group_info, pdx);
	IoReleaseRemoveLockAndWait(&pdx->RemoveLock, pdx);
	ReleaseGroupMem(pdx, pdx->hook_device);
	RemoveDeviceExtByPdx(group_info->dev_ext_queue, pdx);
}

NTSTATUS DirectIoControl(PDEVICE_OBJECT DevObj, ULONG CtlCode, PVOID Buffer, ULONG Size)
{
	KEVENT event;
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	IO_STATUS_BLOCK io_block;
	PIRP irp = IoBuildDeviceIoControlRequest(CtlCode, DevObj, NULL, 0, Buffer, Size, FALSE, &event, &io_block);
	if (NULL == irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	NTSTATUS status;
	status = IoCallDriver(DevObj, irp);
	if (status == STATUS_PENDING)
	{
		status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
		status = irp->IoStatus.Status;
	}

	return status;
}

PDRIVER_DISPATCH GetOriginalFunction(IN GroupInfo *group_info, IN PDRIVER_OBJECT drv_obj, UCHAR code)
{
	PDRIVER_DISPATCH ret = NULL;

	LockDriverHookQueue(group_info->driver_hook_queue);

	PDRIVER_HOOK drv_hook = FindHookByDriverObjectNLock(group_info->driver_hook_queue, drv_obj);

	KIRQL old_irql;
	KeAcquireSpinLock(&drv_hook->lock, &old_irql);
	ret = drv_hook->hook_fun[code].org_fun;
	KeReleaseSpinLock(&drv_hook->lock, old_irql);

	ReleaseDriverHookQueue(group_info->driver_hook_queue);

	return ret;
}

VOID HookDriver(IN PDRIVER_HOOK drv_hook, IN PDRIVER_OBJECT drv_obj)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&drv_hook->lock, &old_irql);
	if (drv_obj->MajorFunction[IRP_MJ_WRITE] != DispatchWrite)
	{
		drv_hook->hook_fun[IRP_MJ_WRITE].org_fun = drv_obj->MajorFunction[IRP_MJ_WRITE];
		drv_hook->hook_fun[IRP_MJ_WRITE].new_fun = DispatchWrite;
		drv_obj->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
	}

	if (drv_obj->MajorFunction[IRP_MJ_PNP] != DispatchPnp)
	{
		drv_hook->hook_fun[IRP_MJ_PNP].org_fun = drv_obj->MajorFunction[IRP_MJ_PNP];
		drv_hook->hook_fun[IRP_MJ_PNP].new_fun = DispatchPnp;
		drv_obj->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
	}

	KeReleaseSpinLock(&drv_hook->lock, old_irql);
}

VOID UnhookDriver(IN PDRIVER_HOOK drv_hook)
{
	KIRQL old_irql;
	KeAcquireSpinLock(&drv_hook->lock, &old_irql);
	if (drv_hook->hook_driver->MajorFunction[IRP_MJ_WRITE] != drv_hook->hook_fun[IRP_MJ_WRITE].org_fun)
	{
		drv_hook->hook_driver->MajorFunction[IRP_MJ_WRITE] = drv_hook->hook_fun[IRP_MJ_WRITE].org_fun;
	}

	if (drv_hook->hook_driver->MajorFunction[IRP_MJ_PNP] != drv_hook->hook_fun[IRP_MJ_PNP].org_fun)
	{
		drv_hook->hook_driver->MajorFunction[IRP_MJ_PNP] = drv_hook->hook_fun[IRP_MJ_PNP].org_fun;
	}

	KeReleaseSpinLock(&drv_hook->lock, old_irql);
}


PDEVICE_OBJECT GetIoDevice(IN PDEVICE_EXTENSION pdx)
{
	return pdx->hook_device;
}

PDEVICE_EXTENSION CreateDeviceExtensionByDiskNumAndHook(IN ULONG disk_num)
{
	PDEVICE_EXTENSION pdx = NULL;
	BOOLEAN flag = FALSE;
	NTSTATUS status = STATUS_SUCCESS;;

	PDEVICE_OBJECT device_object = NULL;

	do
	{
		device_object = GetPdoByDiskNum(disk_num);
		if (device_object == NULL)
		{
			status = STATUS_NOT_FOUND;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get Pdo for disk %d", disk_num);
			break;
		}

		pdx = CreatePdxByPdo(device_object);
		if (pdx == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to create pdx by pdo");
			break;
		}

		status = GetDiskInfoByPdx(pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get disk info for %wZ", &pdx->disk_name);
			break;
		}

		status = HookDevice(pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to hook device %wZ, error %!STATUS!", &pdx->disk_name, status);
		}

		status = CreateGroupMem(pdx, NULL, FALSE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::SectionCreateIner failed, error %!STATUS!", status);
			break;
		}
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		if (pdx)
		{
			FreeDeviceExtension(pdx);
			pdx = NULL;
		}
	}

	return pdx;
}

PDEVICE_OBJECT GetPdoByDiskNum(IN ULONG disk_num)
{
	BOOLEAN flag = FALSE;
	NTSTATUS status = STATUS_SUCCESS;;

	UNICODE_STRING disk_obj_name = { 0 };
	PFILE_OBJECT file_object = NULL;
	PDEVICE_OBJECT device_object = NULL;

	do
	{
		if (!AllocUnicodeString(&disk_obj_name, DISK_NAME_LENGTH, NonPagedPool))
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc disk obj name string");
			break;
		}

		status = RtlUnicodeStringPrintf(&disk_obj_name, DISK_DEVICE_SL_NAME_FORMAT, disk_num);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to format disk obj name string");
			break;
		}

		status = IoGetDeviceObjectPointer(&disk_obj_name, FILE_READ_ATTRIBUTES, &file_object, &device_object);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get device by name %wZ", &disk_obj_name);
			break;
		}

		device_object = GetPdoByDeviceObject(device_object);
		if (device_object == NULL)
		{
			status = STATUS_NOT_FOUND;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find Pdo for device %wZ", &disk_obj_name);
			break;
		}
	} while (flag);

	if (file_object)
	{
		ObDereferenceObject(file_object);
	}

	FreeUnicodeString(&disk_obj_name);

	return device_object;
}

PDEVICE_EXTENSION CreatePdxByPdo(IN PDEVICE_OBJECT pdo)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdx = NULL;
	POBJECT_NAME_INFORMATION obj_name = NULL;

	BOOLEAN flag = FALSE;

	do
	{
		pdx = (PDEVICE_EXTENSION)ExAllocatePoolWithTag(NonPagedPool, sizeof(DEVICE_EXTENSION), ALLOC_TAG);
		if (pdx == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc pdx buffer, error %!STATUS!", status);
			break;
		}

		RtlZeroMemory(pdx, sizeof(DEVICE_EXTENSION));
		pdx->need_clean = TRUE;
		pdx->pending_stop = FALSE;
		IoInitializeRemoveLock(&pdx->RemoveLock, 0, 0, 0);

		obj_name = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, DISK_NAME_LENGTH + sizeof(OBJECT_NAME_INFORMATION), ALLOC_TAG);
		if (obj_name == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc root name buffer, error %!STATUS!", status);
			break;
		}

		ULONG ulRet;
		status = ObQueryNameString(pdo, obj_name, DISK_NAME_LENGTH + sizeof(OBJECT_NAME_INFORMATION), &ulRet);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ObQueryNameString failed, error %!STATUS!", status);
			break;
		}

		pdx->disk_name.MaximumLength = DISK_NAME_LENGTH;
		pdx->disk_name.Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPool, DISK_NAME_LENGTH, ALLOC_TAG);
		if (NULL == pdx->disk_name.Buffer)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc disk_name_buffer failed, error %!STATUS!", status);
			break;
		}
		RtlZeroMemory(pdx->disk_name.Buffer, DISK_NAME_LENGTH);
		RtlCopyUnicodeString(&pdx->disk_name, &obj_name->Name);

		pdx->DeviceObject = NULL;
		pdx->LowerDeviceObject = NULL;
		KeInitializeEvent(&pdx->PagingPathCountEvent, NotificationEvent, TRUE);
		pdx->pdo = pdo;
		pdx->hook_device = GetHookDevice(pdo);
	} while (flag);

	if (obj_name)
	{
		ExFreePoolWithTag(obj_name, ALLOC_TAG);
		obj_name = NULL;
	}

	if (!NT_SUCCESS(status))
	{
		if (pdx)
		{
			FreeDeviceExtension(pdx);
			pdx = NULL;
		}
	}

	return pdx;
}

NTSTATUS GetDiskInfoByPdx(IN PDEVICE_EXTENSION pdx)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;
	do
	{
		STORAGE_DEVICE_NUMBER st_device_number = { 0 };
		status = DirectIoControl(pdx->hook_device, IOCTL_STORAGE_GET_DEVICE_NUMBER, &st_device_number, sizeof(STORAGE_DEVICE_NUMBER));
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get disk number for device %wZ, error %!STATUS!", &pdx->disk_name, status);
			break;
		}

		pdx->disk_number = st_device_number.DeviceNumber;

		//if (!AllocUnicodeString(&pdx->disk_name, DISK_NAME_LENGTH, NonPagedPool))
		//{
		//	status = STATUS_INSUFFICIENT_RESOURCES;
		//	TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc disk_name_buffer failed, error %!STATUS!", status);
		//	break;
		//}

		//RtlUnicodeStringPrintf(&pdx->disk_name, L"DEVICE\\%08x", pdx->disk_number);

		DISK_GEOMETRY_EX dsk_gem = { 0 };
		status = DirectIoControl(pdx->hook_device, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, &dsk_gem, sizeof(DISK_GEOMETRY_EX));
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get disk geometry for device %wZ, error %!STATUS!", &pdx->disk_name, status);
			break;
		}

		pdx->sector_size = dsk_gem.Geometry.BytesPerSector;
		pdx->disk_size = dsk_gem.DiskSize.QuadPart;
	} while (flag);

	return status;
}
