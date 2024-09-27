#include "persist_file.h"
#include "group_mem.h"

#include "util.h"

#ifdef _DR
#include "kernel_sender.h"
#endif

#include "driver.h"

#include <Ntddvol.h>

#include "wpp_trace.h"
#include "persist_file.tmh"




static volatile PFILE_DISK_EXTENTS file_disk_exts = NULL;

typedef struct tagPER_FILE_INFO
{
	HANDLE file_handle;
	PUCHAR header_buffer;
	PUCHAR descriptor_buffer;
	BOOLEAN load_finish;
} PER_FILE_INFO, *PPER_FILE_INFO;

static PER_FILE_INFO per_file_info = { 0 };



VOID CheckPersistData(GroupInfo *group_info);
VOID ClosePersistFile();
VOID PersistBitmapData(GroupInfo *group_info);
HANDLE GetSystemVolumeHandle();
ULONGLONG GetPersistDataSize(GroupInfo *group_info);
#ifdef _DR
PDEVICE_EXTENSION GetPersistDevice(GroupInfo *group_info, ULONG disk_number);
#else
PDEVICE_EXTENSION GetPersistDevice(GroupInfo *group_info, PDEVICE_OBJECT dev_obj);
#endif
BOOLEAN CheckPerDataHeader(GroupInfo *group_info, PPERS_DATA_HEADER data_header);


VOID InitializePersistFile(IN GroupInfo* group_info)
{
	CheckPersistData(group_info);
}

VOID UnintializePersistFile()
{
	ClosePersistFile();

	if (file_disk_exts)
	{
		ExFreePoolWithTag(file_disk_exts, ALLOC_TAG);
		file_disk_exts = NULL;
	}
}


VOID FirstTimeShutdown(GroupInfo* group_info)
{
#ifdef _DR
	if (DestroySocketThread(group_info))
	{
		if (group_info->state != IM_PG_STATE_STOP && group_info->state != IM_PG_STATE_ERROR)
		{
			NTSTATUS status = RegWriteDatasetId(group_info->dataset_id, group_info->dataset_id_done);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to write dataset id, error %!STATUS!", status);
				LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_DATASET_ID_FAIL, status, NULL, NULL, NULL, 0);
			}
			else
			{
				if (group_info->state == IM_PG_STATE_NORMAL || group_info->state == IM_PG_STATE_CBT)
				{
					ShutdownPrepare(group_info);
				}
				else
				{
					LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_DATA_ABORT_STATE, STATUS_SUCCESS, NULL, NULL, (PVOID)&group_info->state, sizeof(group_info->state));
				}
			}
		}
		else
		{
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_DATA_ABORT_STATE, STATUS_SUCCESS, NULL, NULL, (PVOID)&group_info->state, sizeof(group_info->state));
		}
	}
	else
	{
		LogEvent(group_info->device_obj->DriverObject, LOG_DESTROY_SEND_THREAD_FAIL, STATUS_UNSUCCESSFUL, NULL, NULL, NULL, 0);
	}
#else
	if (group_info->state != IM_PG_STATE_STOP && group_info->state != IM_PG_STATE_ERROR)
	{
		ShutdownPrepare(group_info);
	}
#endif
}

VOID SecondTimeShutdown(GroupInfo* group_info)
{
	if (file_disk_exts)
	{
		if (ShutdownBitmapPrepare(group_info))
		{
			PersistBitmapData(group_info);
		}
		else
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to prepare shutdown bitmap");
		}
	}
}

NTSTATUS RemoveFsCompression(HANDLE data_file_handle)
{
	NTSTATUS status = STATUS_SUCCESS;

	IO_STATUS_BLOCK io_status = { 0 };
	USHORT comp_format = COMPRESSION_FORMAT_NONE;
	status = ZwFsControlFile(data_file_handle, NULL, NULL, NULL, &io_status, FSCTL_GET_COMPRESSION, NULL, 0, &comp_format, sizeof(comp_format));
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get compression format, error %!STATUS!", status);
		return status;
	}

	if (comp_format != COMPRESSION_FORMAT_NONE)
	{
		comp_format = COMPRESSION_FORMAT_NONE;
		status = ZwFsControlFile(data_file_handle, NULL, NULL, NULL, &io_status, FSCTL_SET_COMPRESSION, &comp_format, sizeof(comp_format), NULL, 0);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to set compression format, error %!STATUS!", status);
			return status;
		}
	}

	return status;
}

NTSTATUS FillPersistFile(HANDLE data_file_handle, ULONGLONG data_size)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	BYTE* fill_buffer = NULL;
	ULONG fill_buffer_size = 2 * 1024 * 1024;

	do
	{
		fill_buffer = (BYTE*)ExAllocatePoolWithTag(PagedPool, fill_buffer_size, ALLOC_TAG);
		if (fill_buffer == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for fill buffer");
			break;
		}

		RtlZeroMemory(fill_buffer, fill_buffer_size);

		LARGE_INTEGER offset;
		offset.QuadPart = 0;
		IO_STATUS_BLOCK io_status = { 0 };
		ULONGLONG write_length = 0;
		while (offset.QuadPart < (LONGLONG)data_size)
		{
			write_length = fill_buffer_size;
			if (data_size - offset.QuadPart < fill_buffer_size)
			{
				write_length = data_size - offset.QuadPart;
			}

			status = ZwWriteFile(data_file_handle, NULL, NULL, NULL, &io_status, fill_buffer, (ULONG)write_length, &offset, NULL);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to write persist file at pos %llu, error %!STATUS!", offset.QuadPart, status);
				break;
			}

			offset.QuadPart += write_length;
		}
	} while (flag);

	if (fill_buffer)
	{
		ExFreePoolWithTag(fill_buffer, ALLOC_TAG);
		fill_buffer = NULL;
	}

	return status;
}

NTSTATUS SetPersistFileSize(HANDLE data_file_handle, ULONGLONG data_size)
{
	NTSTATUS status = STATUS_SUCCESS;

	IO_STATUS_BLOCK io_status = { 0 };

	FILE_POSITION_INFORMATION file_pos;
	file_pos.CurrentByteOffset.QuadPart = data_size;
	status = ZwSetInformationFile(data_file_handle, &io_status, &file_pos, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to set file size, error %!STATUS!", status);
		return status;
	}

	FILE_END_OF_FILE_INFORMATION file_end;
	file_end.EndOfFile.QuadPart = data_size;
	status = ZwSetInformationFile(data_file_handle, &io_status, &file_pos, sizeof(FILE_END_OF_FILE_INFORMATION), FileEndOfFileInformation);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to set file end, error %!STATUS!", status);
		return status;
	}

	FILE_VALID_DATA_LENGTH_INFORMATION file_len;
	file_len.ValidDataLength.QuadPart = data_size;
	status = ZwSetInformationFile(data_file_handle, &io_status, &file_pos, sizeof(FILE_VALID_DATA_LENGTH_INFORMATION), FileValidDataLengthInformation);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to set file length, error %!STATUS!", status);
		return status;
	}

	status = FillPersistFile(data_file_handle, data_size);
	if (!NT_SUCCESS(status))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to fill persist file, error %!STATUS!", status);
		return status;
	}

	return status;
}

#ifdef _DR
NTSTATUS FillFileExtent(GroupInfo *group_info, HANDLE sys_vol, HANDLE data_file_handle, PRETRIEVAL_POINTERS_BUFFER poi_buffer, NTFS_VOLUME_DATA_BUFFER* vol_data)
{
	UNREFERENCED_PARAMETER(data_file_handle);

	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	IO_STATUS_BLOCK io_status = { 0 };
	PVOLUME_PHYSICAL_OFFSETS phy_offsets = NULL;
	do
	{
		file_disk_exts->ext_count = poi_buffer->ExtentCount;

		ULONG phy_offset_len = sizeof(VOLUME_PHYSICAL_OFFSETS) + sizeof(VOLUME_PHYSICAL_OFFSET);
		phy_offsets = (PVOLUME_PHYSICAL_OFFSETS)ExAllocatePoolWithTag(PagedPool, phy_offset_len, ALLOC_TAG);
		if (phy_offsets == NULL)
		{
			status = STATUS_NO_MEMORY;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for physical offsets, error %!STATUS!", status);
			break;
		}

		VOLUME_LOGICAL_OFFSET logical_offset;
		ULONGLONG cur_vcn = poi_buffer->StartingVcn.QuadPart;
		for (ULONG i = 0; i < poi_buffer->ExtentCount; i++)
		{
			file_disk_exts->extents[i].vcn = cur_vcn * vol_data->BytesPerCluster;
			file_disk_exts->extents[i].length = (ULONG)((poi_buffer->Extents[i].NextVcn.QuadPart - cur_vcn) * vol_data->BytesPerCluster);
			cur_vcn = poi_buffer->Extents[i].NextVcn.QuadPart;

			logical_offset.LogicalOffset = poi_buffer->Extents[i].Lcn.QuadPart * vol_data->BytesPerCluster;
			status = ZwDeviceIoControlFile(sys_vol, NULL, NULL, NULL, &io_status, IOCTL_VOLUME_LOGICAL_TO_PHYSICAL, &logical_offset, sizeof(VOLUME_LOGICAL_OFFSET), phy_offsets, phy_offset_len);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get physical offset, error %!STATUS!", status);
				break;
			}

			if (phy_offsets->NumberOfPhysicalOffsets != 1)
			{
				status = STATUS_NOT_SUPPORTED;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Don't support dynamic volume, error %!STATUS!", status);
				break;
			}

			file_disk_exts->extents[i].target_device_ext = GetPersistDevice(group_info, phy_offsets->PhysicalOffset[0].DiskNumber);
			if (file_disk_exts->extents[i].target_device_ext == NULL)
			{
				status = STATUS_NOT_FOUND;
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Cannot find target_device_ext for disk %d", phy_offsets->PhysicalOffset[0].DiskNumber);
				break;
			}

			file_disk_exts->extents[i].disk_lcn = phy_offsets->PhysicalOffset[0].Offset;
		}
	} while (flag);

	if (phy_offsets)
	{
		ExFreePoolWithTag(phy_offsets, ALLOC_TAG);
		phy_offsets = NULL;
	}

	return status;
}
#else
NTSTATUS FillFileExtent(GroupInfo *group_info, HANDLE sys_vol, HANDLE data_file_handle, PRETRIEVAL_POINTERS_BUFFER poi_buffer, NTFS_VOLUME_DATA_BUFFER* vol_data)
{
	UNREFERENCED_PARAMETER(sys_vol);

	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	PFILE_OBJECT data_file_obj = NULL;
	IO_STATUS_BLOCK io_status = { 0 };
	do
	{
		status = ObReferenceObjectByHandle(data_file_handle, GENERIC_READ, *IoFileObjectType, KernelMode, (PVOID*)&data_file_obj, NULL);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get object by handle, error %!STATUS!", status);
			break;
		}

		if (data_file_obj->Vpb->RealDevice == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::data_file_obj is invalid");
			break;
		}

		PDEVICE_EXTENSION pdx = GetPersistDevice(group_info, data_file_obj->Vpb->RealDevice);
		if (pdx == NULL)
		{
			status = STATUS_NOT_FOUND;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get device extension");
			break;
		}

		file_disk_exts->ext_count = poi_buffer->ExtentCount;

		ULONGLONG cur_vcn = poi_buffer->StartingVcn.QuadPart;
		for (ULONG i = 0; i < poi_buffer->ExtentCount; i++)
		{
			file_disk_exts->extents[i].vcn = cur_vcn * vol_data->BytesPerCluster;
			file_disk_exts->extents[i].length = (ULONG)((poi_buffer->Extents[i].NextVcn.QuadPart - cur_vcn) * vol_data->BytesPerCluster);
			cur_vcn = poi_buffer->Extents[i].NextVcn.QuadPart;

			file_disk_exts->extents[i].target_device_ext = pdx;
			file_disk_exts->extents[i].disk_lcn = poi_buffer->Extents[i].Lcn.QuadPart * vol_data->BytesPerCluster;
		}
	} while (flag);

	if (data_file_obj)
	{
		ObDereferenceObject(data_file_obj);
		data_file_obj = NULL;
	}


	return status;
}
#endif

NTSTATUS CreateFileExtent(GroupInfo *group_info, HANDLE sys_vol, HANDLE data_file_handle, NTFS_VOLUME_DATA_BUFFER* vol_data)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	PRETRIEVAL_POINTERS_BUFFER poi_buffer = NULL;
	IO_STATUS_BLOCK io_status = { 0 };
	do
	{
		FILE_STANDARD_INFORMATION file_info;
		status = ZwQueryInformationFile(data_file_handle, &io_status, &file_info, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get file standard info, error %!STATUS!", status);
			break;
		}

		RETRIEVAL_POINTERS_BUFFER dummy;
		SIZE_T alloc_size = sizeof(RETRIEVAL_POINTERS_BUFFER) + (file_info.AllocationSize.QuadPart / vol_data->BytesPerCluster) * sizeof(dummy.Extents);
		poi_buffer = (PRETRIEVAL_POINTERS_BUFFER)ExAllocatePoolWithTag(PagedPool, alloc_size, ALLOC_TAG);
		if (poi_buffer == NULL)
		{
			status = STATUS_NO_MEMORY;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc file pointers memory with size 0x%08X, error %!STATUS!", (ULONG)alloc_size, status);
			break;
		}

		SIZE_T ext_size = sizeof(FILE_DISK_EXTENTS) + (file_info.AllocationSize.QuadPart / vol_data->BytesPerCluster) * sizeof(FILE_DISK_EXTENT);
		file_disk_exts = (PFILE_DISK_EXTENTS)ExAllocatePoolWithTag(PagedPool, ext_size, ALLOC_TAG);
		if (file_disk_exts == NULL)
		{
			status = STATUS_NO_MEMORY;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc file extents memory with size 0x%08X, error %!STATUS!", (ULONG)ext_size, status);
			break;
		}

		STARTING_VCN_INPUT_BUFFER input_vcn;
		input_vcn.StartingVcn.QuadPart = 0;
		status = ZwFsControlFile(data_file_handle, NULL, NULL, NULL, &io_status, FSCTL_GET_RETRIEVAL_POINTERS, &input_vcn, sizeof(STARTING_VCN_INPUT_BUFFER), poi_buffer, (ULONG)alloc_size);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get file allocation, error %!STATUS!", status);
			break;
		}

		status = FillFileExtent(group_info, sys_vol, data_file_handle, poi_buffer, vol_data);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to fill file extent, error %!STATUS!", status);
			break;
		}
	} while (flag);

	if (poi_buffer)
	{
		ExFreePoolWithTag(poi_buffer, ALLOC_TAG);
		poi_buffer = NULL;
	}

	return status;
}

VOID ShutdownPrepare(GroupInfo *group_info)
{
	BOOL Flag = FALSE;
	NTSTATUS status = STATUS_SUCCESS;

	HANDLE sys_vol = NULL;
	HANDLE data_file_handle = NULL;

	do
	{
		ClosePersistFile();

		sys_vol = GetSystemVolumeHandle();
		if (sys_vol == NULL)
		{
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_GET_SYS_VOL_FAIL, STATUS_UNSUCCESSFUL, NULL, NULL, NULL, 0);
			break;
		}

		IO_STATUS_BLOCK io_status = { 0 };

		NTFS_VOLUME_DATA_BUFFER vol_data;
		status = ZwFsControlFile(sys_vol, NULL, NULL, NULL, &io_status, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, &vol_data, sizeof(NTFS_VOLUME_DATA_BUFFER));
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get volume data, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_GET_NTFS_VOL_DATA_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		UNICODE_STRING data_file_path = { 0 };
		WCHAR data_file_buffer[64] = { 0 };
		RtlStringCbPrintfW(data_file_buffer, sizeof(data_file_buffer), L"%s\\%s", L"\\SystemRoot", PERSIST_DATA_FILE);
		RtlInitUnicodeString(&data_file_path, data_file_buffer);

		OBJECT_ATTRIBUTES data_file_attr;
		InitializeObjectAttributes(&data_file_attr, &data_file_path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		ZwDeleteFile(&data_file_attr);

		status = RegWritePersistState(PERSIST_DATA_STATE_FAILED);
		if (!NT_SUCCESS(status))
		{
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_WRITE_PERSIST_STATE_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		status = ZwCreateFile(&data_file_handle, GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE, &data_file_attr, &io_status, NULL, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN, FILE_SHARE_READ, FILE_CREATE, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to create data file, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_CREATE_PERSIST_FILE_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		status = RemoveFsCompression(data_file_handle);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to remove fs compression, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_REMOVE_COMPRESSION_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		ULONGLONG data_size = GetPersistDataSize(group_info);
		status = SetPersistFileSize(data_file_handle, data_size);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to set persist file size, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_SET_PERSIST_FILE_SIZE_FAIL, status, NULL, NULL, &data_size, sizeof(data_size));
			break;
		}

		status = CreateFileExtent(group_info, sys_vol, data_file_handle, &vol_data);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to create file extent, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_CREATE_FILE_EXTENT_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		status = RegWritePersistState(PERSIST_DATA_STATE_SUCCEED);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to set persist state, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_WRITE_PERSIST_STATE_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_SHUTDOWN_PREPARE_FINISH, STATUS_SUCCESS, NULL, NULL, NULL, 0);
	} while (Flag);

	if (!NT_SUCCESS(status))
	{
		if (file_disk_exts)
		{
			ExFreePoolWithTag(file_disk_exts, ALLOC_TAG);
			file_disk_exts = NULL;
		}
	}

	if (sys_vol)
	{
		ZwClose(sys_vol);
		sys_vol = NULL;
	}

	if (data_file_handle)
	{
		ZwClose(data_file_handle);
		data_file_handle = NULL;
	}
}

NTSTATUS GetPersistBitmapByVol(GroupInfo* group_info, VolInfo *vol_info, UCHAR* bit_data, ULONG size, PULONGLONG bit_count)
{
	NTSTATUS status;
	BOOLEAN flag = FALSE;

	UNREFERENCED_PARAMETER(group_info);

	ANSI_STRING ani_string = { 0 };

	do
	{
		if (per_file_info.file_handle == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		PPERS_DATA_HEADER data_header = (PPERS_DATA_HEADER)per_file_info.header_buffer;
		PPERS_VOL_BITMAP_DESC descriptor = (PPERS_VOL_BITMAP_DESC)per_file_info.descriptor_buffer;

#ifdef _DR
		status = RtlUnicodeStringToAnsiString(&ani_string, &vol_info->disk_name, TRUE);
#else
		status = RtlUnicodeStringToAnsiString(&ani_string, &vol_info->vol_unique_id, TRUE);
#endif
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RtlUnicodeStringToAnsiString failed, error %!STATUS!", status);
			break;
		}

		PPER_VOL_BITMAP_INFO bitmap_info = NULL;
		for (ULONG i = 0; i < descriptor->disk_count; i++)
		{
			if (strncmp(descriptor->bitmap_info[i].vol_info.disk_name, ani_string.Buffer, ani_string.Length) == 0)
			{
				bitmap_info = &descriptor->bitmap_info[i];
				break;
			}
		}

		if (bitmap_info == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
#ifdef _DR
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find bitmap for volume %wZ, error %!STATUS!", &vol_info->disk_name, status);
#else
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find bitmap for volume %wZ, volume Id %wZ, error %!STATUS!", &vol_info->vol_name, &vol_info->vol_unique_id, status);
#endif
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_NO_VOL_BITMAP, status, &vol_info->disk_name, NULL, &vol_info->pdx->disk_number, sizeof(vol_info->pdx->disk_number));
			break;
		}

		*bit_count = bitmap_info->bitmap_count;

		IO_STATUS_BLOCK io_status;
		LARGE_INTEGER byte_offset;
		byte_offset.QuadPart = data_header->bitmap_offset + bitmap_info->logical_offset;
		status = ZwReadFile(per_file_info.file_handle, NULL, NULL, NULL, &io_status, bit_data, size, &byte_offset, NULL);
		if (!NT_SUCCESS(status))
		{
#ifdef _DR
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to bitmap data for volume %wZ, error %!STATUS!", &vol_info->disk_name, status);
#else
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to bitmap data for volume %wZ, volume Id %wZ, error %!STATUS!", &vol_info->vol_name, &vol_info->vol_unique_id, status);
#endif
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_READ_VOL_BITMAP_FAIL, status, &vol_info->disk_name, NULL, &vol_info->pdx->disk_number, sizeof(vol_info->pdx->disk_number));
			break;
		}

		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_READ_VOL_BITMAP_FINISH, status, &vol_info->disk_name, NULL, &vol_info->pdx->disk_number, sizeof(vol_info->pdx->disk_number));
	} while (flag);

	if (ani_string.Buffer != NULL)
	{
		RtlFreeAnsiString(&ani_string);
	}

	return status;
}


VOID LogPersistStateResult(GroupInfo *group_info)
{
	RegInfo reg_info = { 0 };
	NTSTATUS status = RegReadProtect(&reg_info);
	if (!NT_SUCCESS(status))
	{
		return;
	}

	if (reg_info.iomirror_state != IM_PG_STATE_NORMAL && reg_info.iomirror_state != IM_PG_STATE_CBT)
	{
		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_STATE_ABANDON, STATUS_SUCCESS, NULL, NULL, &reg_info.iomirror_state, sizeof(reg_info.iomirror_state));
	}
	else if (group_info->per_data_state != PERSIST_DATA_STATE_SUCCEED)
	{
		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_STATE_FAIL, STATUS_SUCCESS, NULL, NULL, (PVOID)&group_info->per_data_state, sizeof(group_info->per_data_state));
	}
	else
	{
		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_STATE_SUCCEED, STATUS_SUCCESS, NULL, NULL, NULL, 0);
	}
}

VOID CheckPersistData(GroupInfo *group_info)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	BOOLEAN flag = FALSE;

	ULONG state = PERSIST_DATA_STATE_UNKNOWN;
	do
	{
		status = RegReadPersistState(&state, PERSIST_DATA_STATE_UNKNOWN);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to read persist state, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_READ_PERSIST_STATE_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		ULONG select = 0;
		status = RegReadSelect(&select);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to read select, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_READ_REG_SELECTION_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		group_info->reg_select_cur = select;
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		group_info->per_data_state = PERSIST_DATA_STATE_FAILED;
	}
	else
	{
		group_info->per_data_state = state;
	}

	LogPersistStateResult(group_info);

	do
	{
		status = RegWritePersistState(PERSIST_DATA_STATE_FAILED);
	} while (!NT_SUCCESS(status));
}

HANDLE GetSystemVolumeHandle()
{
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE sys_volume = NULL;
	BOOL flag = FALSE;

	do
	{
		WCHAR sys_vol_buffer[64] = { 0 };
		UNICODE_STRING sys_vol = { 0 };
		RtlStringCbPrintfW(sys_vol_buffer, sizeof(sys_vol_buffer), L"%s", L"\\Device\\BootDevice");
		RtlInitUnicodeString(&sys_vol, sys_vol_buffer);

		OBJECT_ATTRIBUTES obj_attr;
		InitializeObjectAttributes(&obj_attr, &sys_vol, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		IO_STATUS_BLOCK io_status = { 0 };
		status = ZwCreateFile(&sys_volume, GENERIC_READ | SYNCHRONIZE, &obj_attr, &io_status, NULL, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to create data file, error %!STATUS!", status);
			break;
		}
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		if (sys_volume)
		{
			ZwClose(sys_volume);
			sys_volume = NULL;
		}
	}

	return sys_volume;
}

ULONGLONG GetVolBitmapSize(GroupInfo *group_info, VolInfo* vol_info)
{
	uint64_t nb_sectors = (uint64_t)1U << group_info->bitmap_granularity;
	uint64_t size = (vol_info->sectors + nb_sectors - 1) / nb_sectors;
	size = (size + IM_PG_BITS_PER_BYTE - 1) / IM_PG_BITS_PER_BYTE;

	return size;
}

ULONGLONG GetPersistDataSize(GroupInfo *group_info)
{
	ULONGLONG ret = 0;

	ULONGLONG bitmap_size = 0;
	AcquireShareResource(&group_info->vol_queue->sync_resource);
	PLIST_ENTRY head = &group_info->vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		req_entry = req_entry->Flink;
		bitmap_size = GetVolBitmapSize(group_info, vol_info);
		ret += (bitmap_size + PER_DATA_ALIGN - 1) / PER_DATA_ALIGN * PER_DATA_ALIGN;
	}
	ReleaseResource(&group_info->vol_queue->sync_resource);

	ret += PER_HEADER_SECTION_SIZE + PER_DESCRIPTOR_SECTION_SIZE;

	return ret;
}

#ifdef _DR
PDEVICE_EXTENSION GetPersistDevice(GroupInfo *group_info, ULONG disk_number)
{
	PDEVICE_EXTENSION ret = NULL;

	KIRQL old_irql;
	KeAcquireSpinLock(&group_info->dev_ext_queue->lock, &old_irql);
	PLIST_ENTRY head = &group_info->dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PDeviceExtInfo device_ext = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if (device_ext->pdx->disk_number == disk_number)
		{
			ret = device_ext->pdx;
			break;
		}
		req_entry = req_entry->Flink;
	}
	KeReleaseSpinLock(&group_info->dev_ext_queue->lock, old_irql);

	return ret;
}
#else
PDEVICE_EXTENSION GetPersistDevice(GroupInfo *group_info, PDEVICE_OBJECT dev_obj)
{
	PDEVICE_EXTENSION ret = NULL;

	KIRQL old_irql;
	KeAcquireSpinLock(&group_info->dev_ext_queue->lock, &old_irql);
	PLIST_ENTRY head = &group_info->dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PDeviceExtInfo device_ext = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if (device_ext->pdx->pdo == dev_obj)
		{
			ret = device_ext->pdx;
			break;
		}
		req_entry = req_entry->Flink;
	}
	KeReleaseSpinLock(&group_info->dev_ext_queue->lock, old_irql);

	return ret;
}
#endif



NTSTATUS WriteCompletion(IN PDEVICE_OBJECT device_obj, IN PIRP irp, IN PVOID context)
{
	UNREFERENCED_PARAMETER(device_obj);

	if (irp->AssociatedIrp.SystemBuffer && (irp->Flags & IRP_DEALLOCATE_BUFFER))
	{
		ExFreePool(irp->AssociatedIrp.SystemBuffer);
	}

	PMDL mdl;
	while (irp->MdlAddress)
	{
		mdl = irp->MdlAddress;
		irp->MdlAddress = mdl->Next;

		MmUnlockPages(mdl);
		IoFreeMdl(mdl);
	}

	if (irp->PendingReturned && context)
	{
		*irp->UserIosb = irp->IoStatus;
		KeSetEvent((PKEVENT)context, IO_DISK_INCREMENT, FALSE);
	}

	IoFreeIrp(irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS WritePhysical(PDEVICE_EXTENSION pdx, ULONGLONG offset, ULONG size, PVOID buffer)
{
	NTSTATUS status;
	BOOLEAN flag = FALSE;
	PIRP irp = NULL;
	PDEVICE_OBJECT io_device = NULL;
	do
	{
		io_device = GetIoDevice(pdx);
		if (io_device == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		ObReferenceObject(io_device);

		LARGE_INTEGER write_offset;
		write_offset.QuadPart = offset;
		IO_STATUS_BLOCK io_status = { 0 };
		irp = IoBuildAsynchronousFsdRequest(IRP_MJ_WRITE, io_device, buffer, size, &write_offset, &io_status);
		if (!irp)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to build write irp, error %!STATUS!", status);
			break;
		}

		IoGetNextIrpStackLocation(irp)->Flags |= SL_FORCE_DIRECT_WRITE;

		KEVENT write_event;
		KeInitializeEvent(&write_event, NotificationEvent, FALSE);
		IoSetCompletionRoutine(irp, WriteCompletion, &write_event, TRUE, TRUE, TRUE);

		status = IoCallDriver(io_device, irp);
		if (status == STATUS_PENDING)
		{
			KeWaitForSingleObject(&write_event, Executive, KernelMode, FALSE, NULL);
			status = io_status.Status;
		}
		else
		{
			break;
		}
	} while (flag);

	if (io_device != NULL)
	{
		ObDereferenceObject(io_device);
	}

	return status;
}

NTSTATUS DiskWriteFile(ULONGLONG offset, PVOID buffer, ULONG size)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	do
	{
		if (file_disk_exts == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		ULONG start_ext = 0xffffffff;
		ULONG end_ext = 0xffffffff;
		for (ULONG i = 0; i < file_disk_exts->ext_count; i++)
		{
			if (file_disk_exts->extents[i].vcn <= offset && file_disk_exts->extents[i].vcn + file_disk_exts->extents[i].length > offset)
			{
				start_ext = i;
			}

			if (file_disk_exts->extents[i].vcn <= offset + size - 1 && file_disk_exts->extents[i].vcn + file_disk_exts->extents[i].length > offset + size - 1)
			{
				end_ext = i;
			}
		}

		if (start_ext == 0xffffffff || end_ext == 0xffffffff)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find the disk extent, error %!STATUS!", status);
			break;
		}

		ULONG start_adjust = 0;
		ULONG end_adjust = 0;
		ULONG written = 0;
		ULONG write_len = 0;
		for (ULONG i = start_ext; i <= end_ext; i++)
		{
			start_adjust = 0;
			if (i == start_ext)
			{
				start_adjust = (ULONG)(offset - file_disk_exts->extents[i].vcn);
			}

			end_adjust = 0;
			if (i == end_ext)
			{
				end_adjust = (ULONG)(file_disk_exts->extents[i].vcn + file_disk_exts->extents[i].length - offset - size);
			}

			write_len = file_disk_exts->extents[i].length - start_adjust - end_adjust;

			status = WritePhysical(file_disk_exts->extents[i].target_device_ext, file_disk_exts->extents[i].disk_lcn + start_adjust, write_len, (PUCHAR)buffer + written);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to write extent, error %!STATUS!", status);
				break;
			}

			written += write_len;
		}

		if (!NT_SUCCESS(status))
		{
			break;
		}
	} while (flag);

	return status;
}

VOID PersistBitmapData(GroupInfo *group_info)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	PUCHAR header_buffer = NULL;
	PUCHAR descriptor_buffer = NULL;
	do
	{
		header_buffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, PER_HEADER_SECTION_SIZE, ALLOC_TAG);
		if (header_buffer == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for data header, error %!STATUS!", status);
			break;
		}

		RtlZeroMemory(header_buffer, PER_HEADER_SECTION_SIZE);

		descriptor_buffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, PER_DESCRIPTOR_SECTION_SIZE, ALLOC_TAG);
		if (descriptor_buffer == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for descriptor, error %!STATUS!", status);
			break;
		}

		RtlZeroMemory(descriptor_buffer, PER_DESCRIPTOR_SECTION_SIZE);

		PPERS_DATA_HEADER data_header = (PPERS_DATA_HEADER)header_buffer;
		RtlCopyMemory(data_header->magic, PER_DATA_MAGIC, sizeof(data_header->magic));
		data_header->start_safe_flag = 1;
		data_header->reg_select = group_info->reg_select_cur;
		data_header->desc_offset = PER_HEADER_SECTION_SIZE;
		data_header->bitmap_offset = PER_HEADER_SECTION_SIZE + PER_DESCRIPTOR_SECTION_SIZE;

		PPERS_VOL_BITMAP_DESC desc_data = (PPERS_VOL_BITMAP_DESC)descriptor_buffer;
		desc_data->disk_count = group_info->vol_queue->num;
		desc_data->granularity = group_info->bitmap_granularity;

		ULONG vol_count = 0;
		ULONG bitmap_offset = 0;
		AcquireShareResource(&group_info->vol_queue->sync_resource);
		PLIST_ENTRY head = &group_info->vol_queue->head;
		PLIST_ENTRY req_entry = head->Flink;
		while (req_entry != head)
		{
			VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
			req_entry = req_entry->Flink;

			ANSI_STRING ani_string;
#ifdef _DR
			status = RtlUnicodeStringToAnsiString(&ani_string, &vol_info->disk_name, TRUE);
#else
			status = RtlUnicodeStringToAnsiString(&ani_string, &vol_info->vol_unique_id, TRUE);
#endif
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::RtlUnicodeStringToAnsiString failed, error %!STATUS!", status);
				break;
			}

			memset(desc_data->bitmap_info[vol_count].vol_info.disk_name, 0, PER_DISK_NAME_LEN);
			RtlCopyMemory(desc_data->bitmap_info[vol_count].vol_info.disk_name, ani_string.Buffer, ani_string.Length);
			RtlFreeAnsiString(&ani_string);

			RtlCopyMemory(desc_data->bitmap_info[vol_count].vol_info.vol_id, vol_info->vol_id, VOL_ID_LEN);
			desc_data->bitmap_info[vol_count].bitmap_size = vol_info->persist_len;
			desc_data->bitmap_info[vol_count].logical_offset = bitmap_offset;
#ifdef _DR
			desc_data->bitmap_info[vol_count].bitmap_count = GetBitmapCount(vol_info->bitmap);
#else
			desc_data->bitmap_info[vol_count].bitmap_count = GetBitmapCount(vol_info->cbt_bitmap.bit_today);
#endif

			bitmap_offset += desc_data->bitmap_info[vol_count].bitmap_size;

			status = DiskWriteFile(data_header->bitmap_offset + desc_data->bitmap_info[vol_count].logical_offset, vol_info->persist_data, vol_info->persist_len);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to write bitmap, error %!STATUS!", status);
				break;
			}

			vol_count++;
		}
		ReleaseResource(&group_info->vol_queue->sync_resource);

		if (!NT_SUCCESS(status))
		{
			break;
		}

		status = DiskWriteFile(data_header->desc_offset, descriptor_buffer, PER_DESCRIPTOR_SECTION_SIZE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to write descriptor, error %!STATUS!", status);
			break;
		}

		status = DiskWriteFile(0, header_buffer, PER_HEADER_SECTION_SIZE);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to write header, error %!STATUS!", status);
			break;
		}
	} while (flag);

	if (descriptor_buffer)
	{
		ExFreePoolWithTag(descriptor_buffer, ALLOC_TAG);
		descriptor_buffer = NULL;
	}

	if (header_buffer)
	{
		ExFreePoolWithTag(header_buffer, ALLOC_TAG);
		header_buffer = NULL;
	}
}



BOOLEAN CheckPerDataHeader(GroupInfo* group_info, PPERS_DATA_HEADER data_header)
{
	if (!data_header)
	{
		return FALSE;
	}

	if (memcmp(data_header->magic, PER_DATA_MAGIC, sizeof(data_header->magic)) != 0)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong magic number");
		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_WRONG_HEADER_MAGIC, STATUS_UNSUCCESSFUL, NULL, NULL, data_header->magic, sizeof(data_header->magic));
		return FALSE;
	}

	if (data_header->start_safe_flag != 1)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong start safe flag %d", data_header->start_safe_flag);
		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_WRONG_HEADER_START_SAFE, STATUS_UNSUCCESSFUL, NULL, NULL, &data_header->start_safe_flag, sizeof(data_header->start_safe_flag));
		return FALSE;
	}

	if (data_header->reg_select != group_info->reg_select_cur)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!:Reg select changed %d, %d", data_header->reg_select, group_info->reg_select_cur);
		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_WRONG_HEADER_SELECTION, STATUS_UNSUCCESSFUL, NULL, NULL, &data_header->reg_select, sizeof(data_header->reg_select));
		return FALSE;
	}

	return TRUE;
}

BOOLEAN OpenPersistFile(GroupInfo* group_info)
{
	BOOLEAN ret = TRUE;

	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	do
	{
		if (per_file_info.load_finish)
		{
			break;
		}

		if (group_info->per_data_state != PERSIST_DATA_STATE_SUCCEED)
		{
			per_file_info.load_finish = TRUE;
			break;
		}

		int retry_count = 5;

		HANDLE sys_vol = NULL;
		for (int i = 0; i < retry_count; i++)
		{
			sys_vol = GetSystemVolumeHandle();
			if (sys_vol != NULL)
			{
				break;
			}

			ImSleep(SECOND_TO_NANOSECOND);
		}

		if (!sys_vol)
		{
			ret = FALSE;
			break;
		}

		ZwClose(sys_vol);
		sys_vol = NULL;

		per_file_info.load_finish = TRUE;

		UNICODE_STRING data_file_path = { 0 };
		WCHAR data_file_buffer[64] = { 0 };
		RtlStringCbPrintfW(data_file_buffer, sizeof(data_file_buffer), L"%s\\%s", L"\\SystemRoot", PERSIST_DATA_FILE);
		RtlInitUnicodeString(&data_file_path, data_file_buffer);

		OBJECT_ATTRIBUTES data_file_attr;
		InitializeObjectAttributes(&data_file_attr, &data_file_path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		IO_STATUS_BLOCK io_status;
		status = ZwCreateFile(&per_file_info.file_handle, GENERIC_READ | SYNCHRONIZE, &data_file_attr, &io_status, NULL, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN, FILE_SHARE_WRITE, FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to open data file, error %!STATUS!", status);
			LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_OPEN_PERSIST_FILE_FAIL, status, NULL, NULL, NULL, 0);
			break;
		}

		per_file_info.header_buffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, PER_HEADER_SECTION_SIZE, ALLOC_TAG);
		if (per_file_info.header_buffer == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for header buffer, error %!STATUS!", status);
			break;
		}

		RtlZeroMemory(per_file_info.header_buffer, PER_HEADER_SECTION_SIZE);

		per_file_info.descriptor_buffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, PER_DESCRIPTOR_SECTION_SIZE, ALLOC_TAG);
		if (per_file_info.descriptor_buffer == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc memory for descriptor buffer, error %!STATUS!", status);
			break;
		}

		RtlZeroMemory(per_file_info.descriptor_buffer, PER_DESCRIPTOR_SECTION_SIZE);

		LARGE_INTEGER byte_offset;
		byte_offset.QuadPart = 0;
		status = ZwReadFile(per_file_info.file_handle, NULL, NULL, NULL, &io_status, per_file_info.header_buffer, PER_HEADER_SECTION_SIZE, &byte_offset, NULL);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to read header, error %!STATUS!", status);
			break;
		}

		byte_offset.QuadPart = PER_HEADER_SECTION_SIZE;
		status = ZwReadFile(per_file_info.file_handle, NULL, NULL, NULL, &io_status, per_file_info.descriptor_buffer, PER_DESCRIPTOR_SECTION_SIZE, &byte_offset, NULL);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to read descriptor, error %!STATUS!", status);
			break;
		}

		if (!CheckPerDataHeader(group_info, (PPERS_DATA_HEADER)per_file_info.header_buffer))
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Wrong header, error %!STATUS!", status);
			break;
		}

		LogEvent(group_info->device_obj->DriverObject, LOG_PERSIST_OPEN_PERSIST_FINISHED, STATUS_SUCCESS, NULL, NULL, NULL, 0);
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		group_info->per_data_state = PERSIST_DATA_STATE_FAILED;
		ClosePersistFile();
	}

	return ret;
}

VOID ClosePersistFile()
{
	if (per_file_info.file_handle)
	{
		ZwClose(per_file_info.file_handle);
		per_file_info.file_handle = NULL;
	}

	if (per_file_info.header_buffer)
	{
		ExFreePoolWithTag(per_file_info.header_buffer, ALLOC_TAG);
		per_file_info.header_buffer = NULL;
	}

	if (per_file_info.descriptor_buffer)
	{
		ExFreePoolWithTag(per_file_info.descriptor_buffer, ALLOC_TAG);
		per_file_info.descriptor_buffer = NULL;
	}
}