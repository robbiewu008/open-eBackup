#include "cdo.h"
#include "group_mem.h"
#include "persist_file.h"

#ifdef _DR
#include "kernel_sender.h"
#endif

#include "ioctl.h"

#include "wpp_trace.h"
#include "cdo.tmh"




static volatile PDEVICE_OBJECT ctl_dev = NULL;

NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status, IN ULONG_PTR info);
NTSTATUS SendToNextDriver(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS ForwardIrpSynchronous(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS CdoDispatchCommon(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS CheckParamByCode(ULONG code, UCHAR* Buffer, ULONG size);
NTSTATUS DeviceCtlByCode(GroupInfo* group_info, ULONG code, UCHAR* Buffer);



NTSTATUS CdoAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT pdo)
{
	NTSTATUS status;
	BOOLEAN Flag = FALSE;
	PDEVICE_OBJECT tmp_dev = NULL;
	PDEVICE_EXTENSION pdx = NULL;

	UNICODE_STRING device_name = { 0 };
	UNICODE_STRING link_name = { 0 };
	BOOLEAN link_create = FALSE;

	do
	{
#ifdef _DR
		RtlInitUnicodeString(&device_name, IOMIRROR_DEVICE_NAME);
#else
		RtlInitUnicodeString(&device_name, IOTRACK_DEVICE_NAME);
#endif

		status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), &device_name, FILE_DEVICE_UNKNOWN, 0, FALSE, &tmp_dev);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to create cdo device, error %!STATUS!", status);
			LogEvent(DriverObject, LOG_CREATE_CDO_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		pdx = (PDEVICE_EXTENSION)tmp_dev->DeviceExtension;
		RtlZeroMemory(pdx, sizeof(DEVICE_EXTENSION));
		pdx->need_clean = FALSE;

		pdx->pending_stop = FALSE;

		IoInitializeRemoveLock(&pdx->RemoveLock, 0, 0, 0);
		KeInitializeEvent(&pdx->PagingPathCountEvent, NotificationEvent, TRUE);
		pdx->DeviceObject = tmp_dev;
		pdx->pdo = pdo;
#ifdef _DR
		pdx->disk_number = 0xffffffff;
#endif
		pdx->first_shutdown = TRUE;

#ifdef _DR
		RtlInitUnicodeString(&link_name, IOMIRROR_DEVICE_LINK_NAME);
#else
		RtlInitUnicodeString(&link_name, IOTRACK_DEVICE_LINK_NAME);
#endif
		status = IoCreateSymbolicLink(&link_name, &device_name);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to create cdo device link, error %!STATUS!", status);
			LogEvent(DriverObject, LOG_CREATE_CDO_SYM_LINK_FAIL, status, &device_name, NULL, NULL, 0);
			break;
		}

		link_create = TRUE;

		status = CreateGroupMem(pdx, tmp_dev, TRUE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::SectionCreateIner failed, error %!STATUS!", status);
			LogEvent(DriverObject, LOG_CREATE_CDO_GROUP_INFO_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		PDEVICE_OBJECT fdo = IoAttachDeviceToDeviceStack(tmp_dev, pdo);
		if (NULL == fdo)
		{
			status = STATUS_DEVICE_REMOVED;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAttachDeviceToDeviceStack failed, error %!STATUS!", status);
			LogEvent(DriverObject, LOG_ATTACH_CDO_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		pdx->hook_device = NULL;
		pdx->LowerDeviceObject = fdo;

		tmp_dev->Flags |= fdo->Flags & (DO_DIRECT_IO | DO_BUFFERED_IO | DO_POWER_PAGABLE);
		tmp_dev->Flags &= ~DO_DEVICE_INITIALIZING;

		ctl_dev = tmp_dev;
	} while (Flag);

	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Add device failed, error %!STATUS!", status);
		if (pdx && pdx->LowerDeviceObject)
		{
			IoDetachDevice(pdx->LowerDeviceObject);
		}

		if (link_create)
		{
			IoDeleteSymbolicLink(&link_name);
		}

		if (NULL != tmp_dev)
		{
			IoDeleteDevice(tmp_dev);
			tmp_dev = NULL;
		}
	}

	return status;
}

NTSTATUS CdoStartDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PAGED_CODE();

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	NTSTATUS status = ForwardIrpSynchronous(DeviceObject, Irp);
	if (!NT_SUCCESS(status))
	{
		LogEvent(DeviceObject->DriverObject, LOG_CDO_START_LOWER_FAIL, status, NULL, NULL, NULL, 0);
		return CompleteRequest(Irp, status, 0);
	}

	DeviceObject->Characteristics |= pdx->LowerDeviceObject->Characteristics;

	BOOLEAN flag = FALSE;
	do
	{
		status = RegInit();
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RegInit failed, error %!STATUS!", status);
			LogEvent(DeviceObject->DriverObject, LOG_CDO_START_INIT_REG_FAIL, status, NULL, NULL, NULL, 0);
			return status;
		}

#ifdef _DR
		status = CreateSocketThread(pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CreateSendThread failed, error %!STATUS!", status);
			LogEvent(DeviceObject->DriverObject, LOG_CDO_START_SEND_THREAD_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}
#endif

		status = IoRegisterShutdownNotification(DeviceObject);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to register shutdown notification, error %!STATUS!", status);
			LogEvent(DeviceObject->DriverObject, LOG_CDO_START_FIRST_SHUTDOWN_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		status = IoRegisterLastChanceShutdownNotification(DeviceObject);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to register last chance shutdown notification, error %!STATUS!", status);
			LogEvent(DeviceObject->DriverObject, LOG_CDO_START_SECOND_SHUTDOWN_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		GroupInfo* group_info = GetGroupInfo(pdx);
		InitializePersistFile(group_info);
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		GroupInfo* group_info = GetGroupInfo(pdx);
		if (group_info)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Stop protection because some Io failed to be handled, error %!STATUS!", status);
			if (!NT_SUCCESS(SendErrorExternCtl(group_info)))
			{
				group_info->state = IM_PG_STATE_ERROR;
			}
		}
	}

	return CompleteRequest(Irp, status, 0);
}

NTSTATUS CdoRemoveDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PAGED_CODE();
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	PDRIVER_OBJECT drv_obj = DeviceObject->DriverObject;

	UndoDriverHookQueue(GetGroupInfo(pdx)->driver_hook_queue);

#ifdef _DR
	if (!DestroySocketThread(GetGroupInfo(pdx)))
	{
		LogEvent(drv_obj, LOG_DESTROY_SEND_THREAD_FAIL, STATUS_UNSUCCESSFUL, NULL, NULL, NULL, 0);
	}
#endif

	IoUnregisterShutdownNotification(DeviceObject);

	IoReleaseRemoveLockAndWait(&pdx->RemoveLock, Irp);

	ReleaseGroupMem(pdx, DeviceObject);

	UNICODE_STRING link_name = { 0 };
	RtlInitUnicodeString(&link_name, IOMIRROR_DEVICE_LINK_NAME);
	IoDeleteSymbolicLink(&link_name);

	IoDetachDevice(pdx->LowerDeviceObject);
	IoDeleteDevice(DeviceObject);
	ctl_dev = NULL;

	UnintializePersistFile();

	LogEvent(drv_obj, LOG_CDO_REMOVED, STATUS_SUCCESS, NULL, NULL, NULL, 0);

	return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}

NTSTATUS CdoCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	return CdoDispatchCommon(DeviceObject, Irp);
}

NTSTATUS CdoClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	return CdoDispatchCommon(DeviceObject, Irp);
}

NTSTATUS CdoCleanUp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	return CdoDispatchCommon(DeviceObject, Irp);
}

#pragma PAGEDCODE
NTSTATUS CdoDeviceCtl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	status = IoAcquireRemoveLock(&pdx->RemoveLock, Irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(Irp, status, 0);
	}

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

	UCHAR* Buffer = (UCHAR*)Irp->AssociatedIrp.SystemBuffer;

	GroupInfo* group_info = GetGroupInfo(pdx);

	BOOLEAN flag = FALSE;

	do
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Message type = %d", code);
	
		if (InterlockedCompareExchange((volatile LONG*)&pdx->pending_stop, 0, 0) == TRUE)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Device is pending stop");
			break;
		}

		if (stack->Parameters.DeviceIoControl.OutputBufferLength != 0)
		{
			if (stack->Parameters.DeviceIoControl.InputBufferLength != stack->Parameters.DeviceIoControl.OutputBufferLength)
			{
				status = STATUS_INVALID_PARAMETER;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Output buffer size %d is not valid(%d)", stack->Parameters.DeviceIoControl.OutputBufferLength, stack->Parameters.DeviceIoControl.InputBufferLength);
				break;
			}
		}

		status = CheckParamByCode(code, Buffer, stack->Parameters.DeviceIoControl.InputBufferLength);
		if (!NT_SUCCESS(status))
		{
			break;
		}

		status = DeviceCtlByCode(group_info, code, Buffer);
		if (!NT_SUCCESS(status))
		{
			break;
		}
	} while (flag);

#ifdef _BK
	CheckRemoveDelayDeleteVolQueue(group_info);
#endif

	if (NT_SUCCESS(status))
	{
		status = CompleteRequest(Irp, status, stack->Parameters.DeviceIoControl.OutputBufferLength);
	}
	else
	{
		status = CompleteRequest(Irp, status, 0);
	}

	IoReleaseRemoveLock(&pdx->RemoveLock, Irp);

	return status;
}

NTSTATUS CdoShutdown(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	NTSTATUS status = IoAcquireRemoveLock(&pdx->RemoveLock, Irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(Irp, status, 0);
	}

	GroupInfo* group_info = GetGroupInfo(pdx);

	if (pdx->first_shutdown)
	{
		pdx->first_shutdown = FALSE;

		FirstTimeShutdown(group_info);
	}

	status = CompleteRequest(Irp, STATUS_SUCCESS, 0);

	IoReleaseRemoveLock(&pdx->RemoveLock, Irp);

	return status;
}

#pragma LOCKEDCODE
NTSTATUS CdoDispatchAny(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	NTSTATUS status;
	status = IoAcquireRemoveLock(&pdx->RemoveLock, Irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(Irp, status, 0);
	}

	status = SendToNextDriver(DeviceObject, Irp);
	IoReleaseRemoveLock(&pdx->RemoveLock, Irp);
	return status;
}

NTSTATUS CdoQueryStop(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	GroupInfo* group_info = GetGroupInfoByCdo();
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	BOOLEAN block_stop = FALSE;

	if (group_info->state != IM_PG_STATE_STOP && group_info->state != IM_PG_STATE_ERROR)
	{
		InterlockedExchange((volatile LONG*)&pdx->pending_stop, TRUE);
		block_stop = TRUE;
	}

	if (block_stop)
	{
		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Device cannot stop, state %d", group_info->state);
		LogEvent(DeviceObject->DriverObject, LOG_QUERY_STOP_DRIVER_IN_USE, STATUS_SUCCESS, NULL, NULL, (PVOID)&group_info->state, sizeof(group_info->state));

		return CompleteRequest(Irp, STATUS_UNSUCCESSFUL, 0);
	}
	else
	{
		NTSTATUS  status = SendToNextDriver(DeviceObject, Irp);
		if (!NT_SUCCESS(status))
		{
			LogEvent(DeviceObject->DriverObject, LOG_QUERY_STOP_LOWER_IN_USE, status, NULL, NULL, NULL, 0);
		}
		else
		{
			LogEvent(DeviceObject->DriverObject, LOG_QUERY_STOP_SUCCESS, STATUS_SUCCESS, NULL, NULL, NULL, 0);
		}

		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Stop query lower result %!STATUS!", status);

		return status;
	}
}

NTSTATUS CdoCancelStop(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	if (InterlockedCompareExchange((volatile LONG*)&pdx->pending_stop, 0, 0) == FALSE)
	{
		return SendToNextDriver(DeviceObject, Irp);
	}

	NTSTATUS status = ForwardIrpSynchronous(DeviceObject, Irp);
	if (NT_SUCCESS(status))
	{
		InterlockedExchange((volatile LONG*)&pdx->pending_stop, FALSE);
	}

	return CompleteRequest(Irp, status, NULL);
}

NTSTATUS CdoStopDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	PDRIVER_OBJECT drv_obj = DeviceObject->DriverObject;

	UndoDriverHookQueue(GetGroupInfo(pdx)->driver_hook_queue);

#ifdef _DR
	if(!DestroySocketThread(GetGroupInfo(pdx)))
	{
		LogEvent(drv_obj, LOG_DESTROY_SEND_THREAD_FAIL, STATUS_UNSUCCESSFUL, NULL, NULL, NULL, 0);
	}
#endif

	IoUnregisterShutdownNotification(DeviceObject);

	ReleaseGroupMem(pdx, DeviceObject);

	UnintializePersistFile();

	InterlockedExchange((volatile LONG*)&pdx->pending_stop, FALSE);

	LogEvent(drv_obj, LOG_CDO_STOPPED, STATUS_SUCCESS, NULL, NULL, NULL, 0);

	return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}

NTSTATUS CdoDispatchPnp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PAGED_CODE();

	PIO_STACK_LOCATION  StackLocation = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS            status = Irp->IoStatus.Status;
	PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
	if (!NT_SUCCESS(status))
	{
		return CompleteRequest(Irp, status, 0);
	}

	if (StackLocation->MinorFunction == IRP_MN_REMOVE_DEVICE)
	{
		status = CdoRemoveDevice(DeviceObject, Irp);
		return status;
	}

	switch (StackLocation->MinorFunction)
	{
	case IRP_MN_START_DEVICE:
		status = CdoStartDevice(DeviceObject, Irp);
		break;
	case IRP_MN_STOP_DEVICE:
		status = CdoStopDevice(DeviceObject, Irp);
	case IRP_MN_QUERY_STOP_DEVICE:
	case IRP_MN_QUERY_REMOVE_DEVICE:
		status = CdoQueryStop(DeviceObject,  Irp);
		break;
	case IRP_MN_CANCEL_STOP_DEVICE:
	case IRP_MN_CANCEL_REMOVE_DEVICE:
		status = CdoCancelStop(DeviceObject, Irp);
		break;
	default:
		status = SendToNextDriver(DeviceObject, Irp);
		break;
	}

	IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

	return status;
}

NTSTATUS CdoDispatchPower(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	NTSTATUS status = IoAcquireRemoveLock(&pdx->RemoveLock, Irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(Irp, status, 0);
	}

	PIO_STACK_LOCATION  StackLocation = IoGetCurrentIrpStackLocation(Irp);
	if (StackLocation->MinorFunction == IRP_MN_SET_POWER)
	{
		if (StackLocation->Parameters.Power.Type == SystemPowerState)
		{
			if (StackLocation->Parameters.Power.State.SystemState == PowerSystemShutdown)
			{
				GroupInfo* group_info = GetGroupInfo(pdx);
				SecondTimeShutdown(group_info);
			}
		}
	}

	IoSkipCurrentIrpStackLocation(Irp);
	status = IoCallDriver(pdx->LowerDeviceObject, Irp);

	IoReleaseRemoveLock(&pdx->RemoveLock, Irp);

	return status;
}

BOOLEAN IsControlDev(IN PDEVICE_OBJECT DeviceObject)
{
	return (ctl_dev == DeviceObject);
}

#define PNP_MGR_NAME				L"\\Driver\\PnpManager"
BOOLEAN IsControlPdo(IN PDEVICE_OBJECT DeviceObject)
{
	PDEVICE_OBJECT cur_dev = IoGetAttachedDevice(DeviceObject);
	while (cur_dev)
	{
		if (_wcsnicmp(cur_dev->DriverObject->DriverName.Buffer, PNP_MGR_NAME, sizeof(PNP_MGR_NAME) / sizeof(WCHAR)) == 0)
		{
			return TRUE;
		}

		cur_dev = IoGetLowerDeviceObject(cur_dev);
	}

	return FALSE;
}

GroupInfo* GetGroupInfoByCdo()
{
	return GetGroupInfo((PDEVICE_EXTENSION)ctl_dev->DeviceExtension);
}

PVOID GetCdoMemInfo()
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)ctl_dev->DeviceExtension;
	return pdx->mem_info;
}

#pragma LOCKEDCODE
VOID HoldCdoRemoveLock()
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)ctl_dev->DeviceExtension;
	IoAcquireRemoveLock(&pdx->RemoveLock, ctl_dev);
}

#pragma LOCKEDCODE
VOID ReleaseCdoRemoveLock()
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)ctl_dev->DeviceExtension;
	IoReleaseRemoveLock(&pdx->RemoveLock, ctl_dev);
}

PDRIVER_OBJECT GetCdoDriverObject()
{
	if (ctl_dev)
	{
		return ctl_dev->DriverObject;
	}

	return NULL;
}



#pragma LOCKEDCODE
NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status, IN ULONG_PTR info)
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

#pragma LOCKEDCODE
NTSTATUS SendToNextDriver(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
}

NTSTATUS IrpCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	PKEVENT Event = (PKEVENT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	if (Event != NULL)
	{
		KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
	}

	return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS ForwardIrpSynchronous(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	KEVENT Event;
	KeInitializeEvent(&Event, NotificationEvent, FALSE);

	IoCopyCurrentIrpStackLocationToNext(Irp);

	IoSetCompletionRoutine(Irp, IrpCompletion, &Event, TRUE, TRUE, TRUE);

	NTSTATUS status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

	__analysis_assume(status != STATUS_PENDING);
	__analysis_assume(IoGetCurrentIrpStackLocation(Irp)->MinorFunction != IRP_MN_START_DEVICE);
	if (status == STATUS_PENDING)
	{
		KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
		status = Irp->IoStatus.Status;
	}

	return status;
}

#pragma LOCKEDCODE
NTSTATUS CdoDispatchCommon(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	NTSTATUS status;
	status = IoAcquireRemoveLock(&pdx->RemoveLock, Irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(Irp, status, 0);
	}

	status = CompleteRequest(Irp, status, NULL);
	IoReleaseRemoveLock(&pdx->RemoveLock, Irp);

	return status;
}

#ifdef _DR
NTSTATUS CheckParamByCode(ULONG code, UCHAR* Buffer, ULONG size)
{
	UNREFERENCED_PARAMETER(Buffer);

	NTSTATUS status = STATUS_SUCCESS;
	if (code == IOCTL_IOMIRROR_START || code == IOCTL_IOMIRROR_START_WITH_VERIFY || code == IOCTL_IOMIRROR_MODIFY)
	{
		if (size < sizeof(ProtectStrategy))
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else if (code == IOCTL_IOMIRROR_VOL_ADD || code == IOCTL_IOMIRROR_VOL_DELETE)
	{
		if (size < sizeof(ProtectVol))
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else if (code == IOCTL_IOMIRROR_NOTIFY_CHANGE)
	{
		if (size < sizeof(NotifyChange))
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}

	return status;
}

NTSTATUS DeviceCtlByCode(GroupInfo* group_info, ULONG code, UCHAR* Buffer)
{
	NTSTATUS status = STATUS_SUCCESS;

	switch (code)
	{
	case IOCTL_IOMIRROR_START:
		status = IomirrorStart(group_info, (ProtectStrategy*)Buffer);
		break;
	case IOCTL_IOMIRROR_START_WITH_VERIFY:
		status = IomirrorStartWithVerify(group_info, (ProtectStrategy*)Buffer);
		break;
	case IOCTL_IOMIRROR_MODIFY:
		status = IomirrorModify(group_info, (ProtectStrategy*)Buffer);
		break;
	case IOCTL_IOMIRROR_STOP:
		status = IomirrorStop(group_info);
		break;
	case IOCTL_IOMIRROR_VOL_ADD:
		status = IomirrorVolAdd(group_info, (ProtectVol*)Buffer);
		break;
	case IOCTL_IOMIRROR_VOL_DELETE:
		status = IomirrorVolDel(group_info, (ProtectVol*)Buffer);
		break;
	case IOCTL_IOMIRROR_VOL_MODIFY:
		status = IomirrorVolMod(group_info, (ProtectVol*)Buffer);
		break;
	case IOCTL_IOMIRROR_NOTIFY_CHANGE:
		status = IomirrorNotifyChange(group_info, (NotifyChange*)Buffer);
		break;
	case IOCTL_IOMIRROR_PAUSE:
		status = IomirrorPause(group_info);
		break;
	case IOCTL_IOMIRROR_RESUME:
		status = IomirrorResume(group_info);
		break;
	case IOCTL_IOMIRROR_QUERY_START:
		status = IomirrorQueryStart(group_info, (QueryStart*)Buffer);
		break;
	default:
		status = STATUS_UNSUCCESSFUL;
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Unsupported ctl code %d, error %!STATUS!", code, status);
		break;
	}

	return status;
}
#else
NTSTATUS CheckParamByCode(ULONG code, UCHAR* Buffer, ULONG size)
{
	UNREFERENCED_PARAMETER(Buffer);

	NTSTATUS status = STATUS_SUCCESS;
	if (code == IOCTL_IOMIRROR_START)
	{
		if (size < sizeof(ProtectStrategy))
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else if (code == IOCTL_IOMIRROR_VOL_ADD || code == IOCTL_IOMIRROR_VOL_DELETE)
	{
		if (size < sizeof(ProtectVol))
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else if (code == IOCTL_BK_TAKE_SNAPSHOT)
	{
		if (size < sizeof(TakeSnapshot))
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else if (code == IOCTL_BK_REMOVE_SNAPSHOT)
	{
		if (size < sizeof(RemoveSnapshot))
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else if (code == IOCTL_BK_GET_BITMAP)
	{
		if (size < sizeof(GetBitmap))
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}

	return status;
}

NTSTATUS DeviceCtlByCode(GroupInfo* group_info, ULONG code, UCHAR* Buffer)
{
	NTSTATUS status = STATUS_SUCCESS;

	switch (code)
	{
	case IOCTL_IOMIRROR_START:
		status = IoctlStartProtection(group_info, (ProtectStrategy*)Buffer);
		break;
	case IOCTL_IOMIRROR_STOP:
		status = IoctlStopProtection(group_info);
		break;
	case IOCTL_IOMIRROR_VOL_ADD:
		status = IoctlVolAdd(group_info, (ProtectVol*)Buffer);
		break;
	case IOCTL_IOMIRROR_VOL_DELETE:
		status = IoctlVolDel(group_info, (ProtectVol*)Buffer);
		break;
	case IOCTL_BK_TAKE_SNAPSHOT:
		status = IoctlStartSnapshot(group_info, (TakeSnapshot*)Buffer);
		break;
	case IOCTL_BK_FINISH_SNAPSHOT:
		status = IoctlFinishSnapshot(group_info);
		break;
	case IOCTL_BK_REMOVE_SNAPSHOT:
		status = IoctlRemoveSnapshot(group_info, (RemoveSnapshot*)Buffer);
		break;
	case IOCTL_BK_GET_BITMAP:
		status = IoctlGetBitmap(group_info, (GetBitmap*)Buffer);
		break;
	default:
		status = STATUS_UNSUCCESSFUL;
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Unsupported ctl code %d, error %!STATUS!", code, status);
		break;
	}

	return status;
}
#endif