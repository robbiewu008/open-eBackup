#include "driver.h"
#include "dispatch.h"
#include "ioctl.h"
#include "regedit.h"
#include "cdo.h"

#include "wpp_trace.h"
#include "driver.tmh"

#include <Ntdddisk.h>
#include <Mountdev.h>






extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	WPP_INIT_TRACING(DriverObject, RegistryPath);

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->DriverExtension->AddDevice = AddDevice;
	for (int i = 0; i < arraysize(DriverObject->MajorFunction); ++i)
		DriverObject->MajorFunction[i] = DispatchAny;
	DriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
	DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceIOControl;
	DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = DispatchShutdown;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DispatchCleanUp;

	InitializeControlDev(DriverObject);

	return STATUS_SUCCESS;
}

NTSTATUS DispatchShutdown(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp)
{
	if (IsControlDev(DeviceObject))
	{
		return CdoShutdown(DeviceObject, irp);
	}

	NTSTATUS status;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	status = IoAcquireRemoveLock(&pdx->RemoveLock, irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::AcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(irp, status, 0);
	}

	status = SendToNextDriver(DeviceObject, irp);
	IoReleaseRemoveLock(&pdx->RemoveLock, irp);

	return status;
}

NTSTATUS DispatchCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	if (IsControlDev(DeviceObject))
	{
		return CdoCreate(DeviceObject, Irp);
	}

	return DispatchAny(DeviceObject, Irp);
}

NTSTATUS DispatchClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	if (IsControlDev(DeviceObject))
	{
		return CdoClose(DeviceObject, Irp);
	}

	return DispatchAny(DeviceObject, Irp);
}

NTSTATUS DispatchCleanUp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	if (IsControlDev(DeviceObject))
	{
		return CdoCleanUp(DeviceObject, Irp);
	}

	return DispatchAny(DeviceObject, Irp);
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE();

	UninitializeControlDev();

	WPP_CLEANUP(DriverObject);
}

NTSTATUS AddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT pdo)
{
	NTSTATUS status;
	PDEVICE_OBJECT fido = NULL;
	PDEVICE_EXTENSION pdx = NULL;

	BOOLEAN flag = FALSE;

	do
	{
		status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), NULL, GetDeviceTypeToUse(pdo), 0, FALSE, &fido);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoCreateDevice failed, error %!STATUS!", status);
			break;
		}
		pdx = (PDEVICE_EXTENSION)fido->DeviceExtension;
		RtlZeroMemory(pdx, sizeof(DEVICE_EXTENSION));

		IoInitializeRemoveLock(&pdx->RemoveLock, 0, 0, 0);
		pdx->DeviceObject = fido;
		KeInitializeEvent(&pdx->PagingPathCountEvent, NotificationEvent, TRUE);
		pdx->pdo = pdo;

		status = CreateGroupMem(pdx, fido);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::SectionCreateIner failed, error %!STATUS!", status);
			break;
		}

		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::A new volume is adding, type %d", pdx->vol_type);

		PDEVICE_OBJECT fdo = IoAttachDeviceToDeviceStack(fido, pdo);
		if (NULL == fdo)
		{
			status = STATUS_DEVICE_REMOVED;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAttachDeviceToDeviceStack failed, error %!STATUS!", status);
			break;
		}

		pdx->LowerDeviceObject = fdo;
		fido->Flags |= fdo->Flags & (DO_DIRECT_IO | DO_BUFFERED_IO | DO_POWER_PAGABLE);
		fido->Flags &= ~DO_DEVICE_INITIALIZING;
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Add device failed, error %!STATUS!", status);
		if (pdx->LowerDeviceObject)
		{
			IoDetachDevice(pdx->LowerDeviceObject);
		}

		if (NULL != fido)
		{
			IoDeleteDevice(fido);
			fido = NULL;
		}

		return status;
	}

	return STATUS_SUCCESS;
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
NTSTATUS DispatchAny(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fido->DeviceExtension;

	NTSTATUS status;
	status = IoAcquireRemoveLock(&pdx->RemoveLock, Irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(Irp, status, 0);
	}

	status = SendToNextDriver(fido, Irp);
	IoReleaseRemoveLock(&pdx->RemoveLock, Irp);
	return status;
}

#pragma PAGEDCODE
NTSTATUS DispatchDeviceIOControl(IN PDEVICE_OBJECT device_object, IN PIRP irp)
{
	if (IsControlDev(device_object))
	{
		return CdoDeviceCtl(device_object, irp);
	}

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)device_object->DeviceExtension;

	NTSTATUS status;
	status = IoAcquireRemoveLock(&pdx->RemoveLock, irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(irp, status, 0);
	}

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

	TracePrint(TRACE_LEVEL_RESERVED6, "%!FUNC!::Received IOCTL code %u, volume %wZ", code, &pdx->vol_name);

	status = SendToNextDriver(device_object, irp);
	IoReleaseRemoveLock(&pdx->RemoveLock, irp);

	return status;
}

typedef struct _BITMAP_SET_CONTEXT
{
	GroupInfo* group_info;
	VolInfo *vol_info;
	uint64_t sector_offset;
	uint64_t sector_length;
	PIO_WORKITEM work_item;
}BITMAP_SET_CONTEXT, *PBITMAP_SET_CONTEXT;
VOID BitmapSetWorkItem(IN PDEVICE_OBJECT DeviceObject, IN PVOID  Context)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PBITMAP_SET_CONTEXT context = (PBITMAP_SET_CONTEXT)Context;

	WriteSetBitmap(context->group_info, context->vol_info, context->sector_offset, context->sector_length);

	IoFreeWorkItem(context->work_item);
	ExFreePoolWithTag(context, ALLOC_TAG);

	InterlockedDecrement(&context->vol_info->reference_count);
}

#pragma LOCKEDCODE
VOID QueueDelayTrackIo(GroupInfo *group_info, VolInfo *vol_info, uint64_t sector_offset, uint64_t sector_length)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	PIO_WORKITEM work_item = NULL;
	PBITMAP_SET_CONTEXT context = NULL;
	do
	{
		work_item = IoAllocateWorkItem(group_info->device_obj);
		if (NULL == work_item)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc work item failed");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		context = (PBITMAP_SET_CONTEXT)ExAllocatePoolWithTag(NonPagedPool, sizeof(BITMAP_SET_CONTEXT), ALLOC_TAG);
		if (!context)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc work item context failed");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		context->group_info = group_info;
		context->vol_info = vol_info;
		context->sector_offset = sector_offset;
		context->sector_length = sector_length;
		context->work_item = work_item;

		IoQueueWorkItem(work_item, BitmapSetWorkItem, DelayedWorkQueue, context);

		InterlockedIncrement(&vol_info->reference_count);
		work_item = NULL;
		context = NULL;
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Stop protection because some Io failed to be handled, error %!STATUS!", status);
		ProcessStop(group_info);
	}

	if (work_item)
	{
		IoFreeWorkItem(work_item);
		work_item = NULL;
	}

	if (context)
	{
		ExFreePoolWithTag(context, ALLOC_TAG);
		context = NULL;
	}
}

#pragma LOCKEDCODE
NTSTATUS DispatchWrite(IN PDEVICE_OBJECT fido, IN PIRP irp)
{
	if (IsControlDev(fido))
	{
		return CdoDispatchAny(fido, irp);
	}

	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fido->DeviceExtension;
	status = IoAcquireRemoveLock(&pdx->RemoveLock, irp);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAcquireRemoveLock failed, error %!STATUS!", status);
		return CompleteRequest(irp, status, 0);
	}

	VolInfo *vol_info = NULL;

	GroupInfo *group_info = GetGroupInfo(pdx);

	do
	{
		if (IsInStopState(group_info))
		{
			break;
		}

		PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);

		ULONG length = 0;
		LARGE_INTEGER offset = { 0 };
		offset = irpsp->Parameters.Write.ByteOffset;
		length = irpsp->Parameters.Write.Length;

		vol_info = NeedProtected(pdx);
		if (vol_info == NULL)
		{
			break;
		}

		if (vol_info->vol_type == VOLUME_TYPE_SW_SNAPSHOT || vol_info->vol_type == VOLUME_TYPE_HW_SNAPSHOT)
		{
			TracePrint(TRACE_LEVEL_RESERVED6, "%!FUNC!::Write happens to snapshot volume");
		}

		uint64_t sector_offset = offset.QuadPart / SECTOR_SIZE;
		uint64_t sector_length = (length + offset.QuadPart - sector_offset * SECTOR_SIZE + SECTOR_SIZE - 1) / SECTOR_SIZE;

		if (KeGetCurrentIrql() <= APC_LEVEL)
		{
			WriteSetBitmap(group_info, vol_info, sector_offset, sector_length);
		}
		else
		{
			QueueDelayTrackIo(group_info, vol_info, sector_offset, sector_length);
		}
	} while (flag);

	if (vol_info)
	{
		InterlockedDecrement(&vol_info->reference_count);
	}

	status = SendToNextDriver(fido, irp);

	IoReleaseRemoveLock(&pdx->RemoveLock, irp);

	return status;
}


NTSTATUS DispatchPower(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
	if (IsControlDev(fido))
	{
		return CdoDispatchAny(fido, Irp);
	}
	
#if (NTDDI_VERSION < NTDDI_VISTA)
	PoStartNextPowerIrp(Irp);
	IoSkipCurrentIrpStackLocation(Irp);

	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	return PoCallDriver(deviceExtension->TargetDeviceObject, Irp);
#else
	IoSkipCurrentIrpStackLocation(Irp);

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fido->DeviceExtension;
	return IoCallDriver(pdx->LowerDeviceObject, Irp);
#endif
}


NTSTATUS DispatchPnp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PAGED_CODE();

	if (IsControlDev(DeviceObject))
	{
		return CdoDispatchAny(DeviceObject, Irp);
	}

	PIO_STACK_LOCATION  StackLocation = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS            status = Irp->IoStatus.Status;
	PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);

	if (!NT_SUCCESS(status))
	{
		return CompleteRequest(Irp, status, 0);
	}

	if (StackLocation->MinorFunction == IRP_MN_START_DEVICE)
	{
		status = StartDevice(DeviceObject, Irp);
		IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

		return status;
	}
	
	if (StackLocation->MinorFunction == IRP_MN_REMOVE_DEVICE)
	{
		status = RemoveDevice(DeviceObject, Irp);
		return status;
	}

	if (StackLocation->MinorFunction == IRP_MN_DEVICE_USAGE_NOTIFICATION)
	{
		PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
		if (irpStack->Parameters.UsageNotification.Type != DeviceUsageTypePaging)
		{
			status = SendToNextDriver(DeviceObject, Irp);
			IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
			return status;
		}

		deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

		status = KeWaitForSingleObject(&deviceExtension->PagingPathCountEvent, Executive, KernelMode, FALSE, NULL);

		BOOLEAN setPagable = FALSE;
		if (!irpStack->Parameters.UsageNotification.InPath && deviceExtension->PagingPathCount == 1)
		{
			if ((DeviceObject->Flags & DO_POWER_INRUSH) == 0)
			{
				DeviceObject->Flags |= DO_POWER_PAGABLE;
				setPagable = TRUE;
			}
		}

		status = ForwardIrpSynchronous(DeviceObject, Irp);
		if (NT_SUCCESS(status))
		{
			IoAdjustPagingPathCount(&deviceExtension->PagingPathCount, irpStack->Parameters.UsageNotification.InPath);

			if (irpStack->Parameters.UsageNotification.InPath)
			{
				if (deviceExtension->PagingPathCount == 1)
				{
					DeviceObject->Flags &= ~DO_POWER_PAGABLE;
				}
			}
		}
		else
		{
			if (setPagable == TRUE)
			{
				DeviceObject->Flags &= ~DO_POWER_PAGABLE;
				setPagable = FALSE;
			}
		}

		KeSetEvent(&deviceExtension->PagingPathCountEvent, IO_NO_INCREMENT, FALSE);

		CompleteRequest(Irp, status, 0);
		IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

		return status;
	}

	status = SendToNextDriver(DeviceObject, Irp);
	IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

	return status;
}



#pragma PAGEDCODE
ULONG GetDeviceTypeToUse(PDEVICE_OBJECT pdo)
{
	PDEVICE_OBJECT ldo = IoGetAttachedDeviceReference(pdo);
	if (!ldo)
		return FILE_DEVICE_UNKNOWN;
	ULONG devtype = ldo->DeviceType;
	ObDereferenceObject(ldo);
	return devtype;
}

BOOLEAN CompareHwSnapshotPdx(PDEVICE_EXTENSION pdx_org, PDEVICE_EXTENSION pdx_snap)
{
	return (pdx_org->vol_serial_num == pdx_snap->vol_serial_num);
}

#define VOL_SNAP_DRIVER_NAME				L"\\Driver\\volsnap"
BOOLEAN CompareSwSnapshotPdx(PDEVICE_EXTENSION pdx_org, PDEVICE_EXTENSION pdx_snap)
{
	BOOLEAN ret = FALSE;
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	UNICODE_STRING vol_snap_driver = { 0 };
	RtlInitUnicodeString(&vol_snap_driver, VOL_SNAP_DRIVER_NAME);
	PDEVICE_OBJECT pdo = NULL;
	do
	{
		PDEVICE_OBJECT dev_obj = IoGetAttachedDevice(pdx_org->DeviceObject);
		while (dev_obj)
		{
			if (RtlEqualUnicodeString(&dev_obj->DriverObject->DriverName, &vol_snap_driver, FALSE))
			{
				break;
			}

			dev_obj = IoGetLowerDeviceObject(dev_obj);
		}

		if (dev_obj == NULL)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find volsnap device for volume %wZ", &pdx_org->vol_name);
			break;
		}

		status = DirectGetPdo(dev_obj, &pdo);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get Pdo for volume %wZ, error %!STATUS!", &pdx_org->vol_name, status);
			break;
		}

		if (pdo == pdx_snap->pdo)
		{
			ret = TRUE;
		}
	} while (flag);

	return ret;
}

BOOLEAN GetOriginalVolume(GroupInfo* group_info, PDEVICE_EXTENSION pdx)
{
	BOOLEAN ret = FALSE;
	QueueInfo* dev_ext_queue = group_info->dev_ext_queue;

	AcquireShareResource(&dev_ext_queue->sync_resource);
	PLIST_ENTRY head = &dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	PDeviceExtInfo dev_ext_info = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;

		dev_ext_info = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if (IsControlDev(dev_ext_info->pdx->DeviceObject))
		{
			req_entry = temp_req_entry;
			continue;
		}

		if (dev_ext_info->pdx->vol_type != VOLUME_TYPE_PHYSICAL)
		{
			req_entry = temp_req_entry;
			continue;
		}

		BOOLEAN found = FALSE;
		if (pdx->vol_type == VOLUME_TYPE_HW_SNAPSHOT)
		{
			found = CompareHwSnapshotPdx(dev_ext_info->pdx, pdx);
		}
		else
		{
			found = CompareSwSnapshotPdx(dev_ext_info->pdx, pdx);
		}

		if (found)
		{
			pdx->orig_pdx = dev_ext_info->pdx;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Found original volume %wZ for snapshot volume %wZ", &dev_ext_info->pdx->vol_name, &pdx->vol_name);

			ret = TRUE;
			break;
		}

		req_entry = temp_req_entry;
	}

	ReleaseResource(&dev_ext_queue->sync_resource);

	return ret;
}

NTSTATUS GetVolumeInfo(PDEVICE_EXTENSION pdx, GroupInfo* group_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	do
	{
		if (pdx->vol_type == VOLUME_TYPE_PHYSICAL)
		{
			if (!AllocUnicodeString(&pdx->vol_unique_id, VOL_NAME_LENGTH, NonPagedPool))
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for vol_unique_id");
				break;
			}

			GUID vol_id = { 0 };
			status = GetVolumeGuid(pdx, &vol_id);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get volume guid");
				break;
			}

			status = RtlStringFromGUID(vol_id, &pdx->vol_unique_id);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to convert vol_unique_id string");
				break;
			}

			GET_LENGTH_INFORMATION len_info = { 0 };
			status = DirectIoControl(pdx->pdo, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &len_info, sizeof(GET_LENGTH_INFORMATION));
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get volume length for device %wZ, error %!STATUS!", &pdx->vol_name, status);
				break;
			}

			pdx->vol_size = len_info.Length.QuadPart;
		}

		if (pdx->vol_type == VOLUME_TYPE_PHYSICAL || pdx->vol_type == VOLUME_TYPE_HW_SNAPSHOT)
		{
			ULONG disk_num = 0;
			LONGLONG disk_offset = 0;
			status = GetVolumeBeginSectorOffset(pdx, &disk_num, &disk_offset);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get volume beginning sector offset");
				break;
			}

			pdx->vol_serial_num = GetVolumeSerialNumber(disk_num, disk_offset, &pdx->vol_name);
		}

		if (pdx->vol_type == VOLUME_TYPE_SW_SNAPSHOT || pdx->vol_type == VOLUME_TYPE_HW_SNAPSHOT)
		{
			if (!GetOriginalVolume(group_info, pdx))
			{
				TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Original volume was not found for snapshot volume %wZ, will not protect it", &pdx->vol_name);
				break;
			}

			pdx->vol_size = pdx->orig_pdx->vol_size;
		}
	} while (flag);

	return status;
}

#define VOL_SNAP_CLASS_ID			L"{533c5b84-ec70-11d2-9505-00c04f79deaf}"
NTSTATUS GetVolumeType(PDEVICE_EXTENSION pdx)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	do
	{
		WCHAR class_id[VOL_NAME_LENGTH] = { 0 };
		ULONG  size = 0;
		status = IoGetDeviceProperty(pdx->pdo, DevicePropertyClassGuid, VOL_NAME_LENGTH, class_id, &size);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Get device property, error %!STATUS!", status);
			break;
		}

		if (_wcsicmp(class_id, VOL_SNAP_CLASS_ID) == 0)
		{
			pdx->vol_type = VOLUME_TYPE_SW_SNAPSHOT;
		}
		else
		{
			VOLUME_GET_GPT_ATTRIBUTES_INFORMATION attr_info = { 0 };
			status = DirectIoControl(pdx->pdo, IOCTL_VOLUME_GET_GPT_ATTRIBUTES, NULL, 0, &attr_info, sizeof(VOLUME_GET_GPT_ATTRIBUTES_INFORMATION));
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get volume attribute info for device %wZ, error %!STATUS!", &pdx->vol_name, status);
				break;
			}

			if ((attr_info.GptAttributes & GPT_BASIC_DATA_ATTRIBUTE_SHADOW_COPY) != 0)
			{
				pdx->vol_type = VOLUME_TYPE_HW_SNAPSHOT;
			}
			else
			{
				pdx->vol_type = VOLUME_TYPE_PHYSICAL;
			}
		}
	} while (flag);

	return status;
}

NTSTATUS StartVolumeProtection(IN PDEVICE_EXTENSION pdx, IN GroupInfo* group_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	POBJECT_NAME_INFORMATION obj_name = NULL;

	do
	{
		obj_name = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, VOL_NAME_LENGTH + sizeof(OBJECT_NAME_INFORMATION), ALLOC_TAG);
		if (obj_name == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc root name buffer, error %!STATUS!", status);
			break;
		}

		ULONG ulRet;
		status = ObQueryNameString(pdx->pdo, obj_name, VOL_NAME_LENGTH + sizeof(OBJECT_NAME_INFORMATION), &ulRet);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ObQueryNameString failed, error %!STATUS!", status);
			break;
		}

		if (!AllocUnicodeString(&pdx->vol_name, VOL_NAME_LENGTH, NonPagedPool))
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for vol_name");
			break;
		}

		RtlCopyUnicodeString(&pdx->vol_name, &obj_name->Name);

		TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::A new volume is started, name %wZ", &pdx->vol_name);

		status = GetVolumeType(pdx);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get volume type for device %wZ, error %!STATUS!", &pdx->vol_name, status);
			break;
		}

		status = GetVolumeInfo(pdx, group_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get volume info for device %wZ, error %!STATUS!", &pdx->vol_name, status);
			break;
		}

		if (pdx->vol_type == VOLUME_TYPE_PHYSICAL)
		{
			status = StartProtectByReg(pdx);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Start protect by reg failed, error %!STATUS!", status);
				break;
			}
		}
		else
		{
			status = StartProtectByPhysicalVol(group_info, pdx);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Start protect by physical vol failed, error %!STATUS!", status);
				break;
			}
		}
	} while (flag);

	if (obj_name)
	{
		ExFreePoolWithTag(obj_name, ALLOC_TAG);
		obj_name = NULL;
	}

	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Stop protection because fail to handle start for volume %wZ, error %!STATUS!", &pdx->vol_name, status);
		ProcessStop(group_info);
	}

	return status;
}

#define FILTER_DEVICE_PROPOGATE_CHARACTERISTICS (FILE_REMOVABLE_MEDIA |  FILE_READ_ONLY_DEVICE |  FILE_FLOPPY_DISKETTE)
#define MAX_VOLUME_EXTENTS			256
NTSTATUS StartDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PAGED_CODE();

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	
	GroupInfo* group_info = GetGroupInfo(pdx);

	NTSTATUS status = ForwardIrpSynchronous(DeviceObject, Irp);
	if (!NT_SUCCESS(status))
	{
		return CompleteRequest(Irp, status, 0);
	}

	ULONG Flags = pdx->LowerDeviceObject->Characteristics & FILTER_DEVICE_PROPOGATE_CHARACTERISTICS;
	DeviceObject->Characteristics |= Flags;
	
	StartVolumeProtection(pdx, group_info);

	return CompleteRequest(Irp, status, 0);
}

NTSTATUS RemoveDevice(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
	PAGED_CODE();
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fido->DeviceExtension;

	IoReleaseRemoveLockAndWait(&pdx->RemoveLock, Irp);

	GroupInfo* group_info = GetGroupInfo(pdx);
	if (pdx->vol_type == VOLUME_TYPE_PHYSICAL)
	{
		RemoveVolInfoByPdx(group_info->vol_queue, pdx);
	}
	else
	{
		RemoveVolInfoByPdx(group_info->snap_vol_queue, pdx);
	}
	RemoveDeviceExtByPdx(group_info->dev_ext_queue, pdx);

	ReleaseGroupMem(pdx, fido);

	FreeUnicodeString(&pdx->vol_name);
	FreeUnicodeString(&pdx->vol_unique_id);

	NTSTATUS status = SendToNextDriver(fido, Irp);

	IoDetachDevice(pdx->LowerDeviceObject);
	IoDeleteDevice(fido);

	return status;
}

NTSTATUS SendToNextDriver(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
}

NTSTATUS SendToNextDriverAndPend(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	IoMarkIrpPending(Irp);
	IoSkipCurrentIrpStackLocation(Irp);

	IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

	return STATUS_PENDING;
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

NTSTATUS DirectIoControl(PDEVICE_OBJECT DevObj, ULONG CtlCode, PVOID InputBuffer, ULONG InputSize, PVOID OutputBuffer, ULONG OutputSize)
{
	KEVENT event;
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	IO_STATUS_BLOCK io_block;
	PIRP irp = IoBuildDeviceIoControlRequest(CtlCode, DevObj, InputBuffer, InputSize, OutputBuffer, OutputSize, FALSE, &event, &io_block);
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

ULONG GetVolumeSerialNumber(ULONG disk_num, ULONGLONG vol_offset, PUNICODE_STRING vol_name)
{
	ULONG ret = 0;

	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	HANDLE disk_handle = NULL;
	OBJECT_ATTRIBUTES obj_attr;
	UNICODE_STRING disk_name = { 0 };
	PUCHAR buffer = NULL;
	do
	{
		if (!AllocUnicodeString(&disk_name, VOL_NAME_LENGTH, PagedPool))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc disk name string");
			break;
		}

		buffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, SECTOR_SIZE, ALLOC_TAG);
		if (buffer == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc read buffer, error %!STATUS!", status);
			break;
		}

		RtlUnicodeStringPrintf(&disk_name, L"\\Device\\Harddisk%d\\Partition0", disk_num);
		InitializeObjectAttributes(&obj_attr, &disk_name, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		IO_STATUS_BLOCK io_status = { 0 };
		status = ZwCreateFile(&disk_handle, GENERIC_READ | SYNCHRONIZE, &obj_attr, &io_status, NULL, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to open disk handle of disk %d for volume %wZ, error %!STATUS!", disk_num, vol_name, status);
			break;
		}

		LARGE_INTEGER read_offset;
		read_offset.QuadPart = vol_offset;
		ZwReadFile(disk_handle, NULL, NULL, NULL, &io_status, buffer, SECTOR_SIZE, &read_offset, NULL);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to  read disk %d for volume %wZ, error %!STATUS!", disk_num, vol_name, status);
			break;
		}

		CHAR fs_type[16] = { 0 };
		memcpy_s(fs_type, sizeof(fs_type), buffer + VOLUME_FS_TYPE_OFFSET, 4);

		if (strcmp(fs_type, VOLUME_FS_TYPE_NTFS) == 0)
		{
			ret = *(PDWORD)(buffer + VOLUME_NTFS_SERIAL_NUM_OFFSET);
		}
		else if (strcmp(fs_type, VOLUME_FS_TYPE_REFS) == 0)
		{
			ret = *(PDWORD)(buffer + VOLUME_REFS_SERIAL_NUM_OFFSET);
		}
		else
		{
			ret = 0;
			TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::Volume %wZ is not with a supported FS by VSS, FS type %s", vol_name, fs_type);
		}
	} while (flag);

	if (disk_handle)
	{
		ZwClose(disk_handle);
		disk_handle = NULL;
	}

	FreeUnicodeString(&disk_name);

	return ret;
}

NTSTATUS GetVolumeGuid(PDEVICE_EXTENSION pdx, GUID* vol_guid)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	PMOUNTDEV_UNIQUE_ID unique_id = NULL;
	do
	{
		unique_id = (PMOUNTDEV_UNIQUE_ID)ExAllocatePoolWithTag(PagedPool, sizeof(PMOUNTDEV_UNIQUE_ID) + VOL_NAME_LENGTH, ALLOC_TAG);
		if (unique_id == NULL)
		{
			status = STATUS_NO_MEMORY;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for unique_id, error %!STATUS!", status);
			break;
		}

		RtlZeroMemory(unique_id, sizeof(PMOUNTDEV_UNIQUE_ID) + VOL_NAME_LENGTH);
		unique_id->UniqueIdLength = VOL_NAME_LENGTH;

		status = DirectIoControl(pdx->pdo, IOCTL_MOUNTDEV_QUERY_UNIQUE_ID, NULL, 0, unique_id, sizeof(PMOUNTDEV_UNIQUE_ID) + VOL_NAME_LENGTH);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get volume unique id for device %wZ, error %!STATUS!", &pdx->vol_name, status);
			break;
		}

		*vol_guid = *(GUID*)unique_id->UniqueId;
	} while (flag);

	if (unique_id)
	{
		ExFreePoolWithTag(unique_id, ALLOC_TAG);
		unique_id = NULL;
	}

	return status;
}

NTSTATUS GetVolumeBeginSectorOffset(PDEVICE_EXTENSION pdx, ULONG* disk_num, LONGLONG* offset)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	PVOLUME_PHYSICAL_OFFSETS phy_offsets = NULL;
	do
	{
		ULONG phy_offset_len = sizeof(VOLUME_PHYSICAL_OFFSETS) + sizeof(VOLUME_PHYSICAL_OFFSET) * 2;
		phy_offsets = (PVOLUME_PHYSICAL_OFFSETS)ExAllocatePoolWithTag(PagedPool, phy_offset_len, ALLOC_TAG);
		if (phy_offsets == NULL)
		{
			status = STATUS_NO_MEMORY;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for physical offsets, error %!STATUS!", status);
			break;
		}

		VOLUME_LOGICAL_OFFSET logical_offset;
		logical_offset.LogicalOffset = 0;
		status = DirectIoControl(pdx->pdo, IOCTL_VOLUME_LOGICAL_TO_PHYSICAL, &logical_offset, sizeof(VOLUME_LOGICAL_OFFSET), phy_offsets, phy_offset_len);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get physical offset, error %!STATUS!", status);
			break;
		}

		*disk_num = phy_offsets->PhysicalOffset[0].DiskNumber;
		*offset = phy_offsets->PhysicalOffset[0].Offset;
	} while (flag);

	if (phy_offsets)
	{
		ExFreePoolWithTag(phy_offsets, ALLOC_TAG);
		phy_offsets = NULL;
	}

	return status;
}

NTSTATUS DirectGetPdo(PDEVICE_OBJECT DevObj, PDEVICE_OBJECT* pdo)
{
	KEVENT event;
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	IO_STATUS_BLOCK io_block;
	PIRP irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP, DevObj, NULL, 0,NULL, &event, &io_block);
	if (NULL == irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

	PIO_STACK_LOCATION  io_stack = IoGetNextIrpStackLocation(irp);
	io_stack->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
	io_stack->Parameters.QueryDeviceRelations.Type = BusRelations;

	NTSTATUS status;
	status = IoCallDriver(DevObj, irp);
	if (status == STATUS_PENDING)
	{
		status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
		status = irp->IoStatus.Status;
	}

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	PDEVICE_RELATIONS dev_relation = ((PDEVICE_RELATIONS)io_block.Information);
	*pdo = dev_relation->Objects[0];
	ExFreePool(dev_relation);

	return status;
}