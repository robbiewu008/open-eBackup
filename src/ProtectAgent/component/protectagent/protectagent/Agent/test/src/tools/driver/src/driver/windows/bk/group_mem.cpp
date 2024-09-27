#include "group_mem.h"
#include "util.h"

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

BOOLEAN FindVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info)
{
	VolInfo *vol_info = NULL;

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

		if (vol_info->pdx == find_vol_info->pdx)
		{
			break;
		}

		req_entry = temp_req_entry;
	}

	ReleaseResource(&vol_queue->sync_resource);

	return (vol_info != NULL);
}

VolInfo* GetVolInfoByPdx(QueueInfo *vol_queue, PDEVICE_EXTENSION pdx)
{
	VolInfo *vol_info = NULL;

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

		if (vol_info->pdx == pdx)
		{
			break;
		}

		req_entry = temp_req_entry;
	}

	if (vol_info)
	{
		InterlockedIncrement(&vol_info->reference_count);
	}

	ReleaseResource(&vol_queue->sync_resource);

	return vol_info;
}

VolInfo* GetPhyVolInfoBySnapPdx(QueueInfo *vol_queue, PDEVICE_EXTENSION pdx)
{
	VolInfo *vol_info = NULL;

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

		if(vol_info->pdx == pdx->orig_pdx)
		{
			break;
		}

		vol_info = NULL;
		req_entry = temp_req_entry;
	}

	if (vol_info)
	{
		InterlockedIncrement(&vol_info->reference_count);
	}

	ReleaseResource(&vol_queue->sync_resource);

	return vol_info;
}

VolInfo *BuildSimpleVolInfo(GroupInfo* group_info, PDEVICE_EXTENSION pdx)
{
	UNREFERENCED_PARAMETER(group_info);

	VolInfo *vol_info = (VolInfo *)ExAllocatePoolWithTag(NonPagedPool, sizeof(VolInfo), ALLOC_TAG);
	if (NULL == vol_info)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc vol_info failed");
		goto fail;
	}
	memset(vol_info, 0, sizeof(VolInfo));

	vol_info->pdx = pdx;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Build vol, name = %wZ", &vol_info->pdx->vol_name);

	if (!AllocUnicodeString(&vol_info->vol_name, VOL_NAME_LENGTH, NonPagedPool))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc vol_name_buffer failed");
		goto fail;
	}

	if (!AllocUnicodeString(&vol_info->vol_unique_id, VOL_NAME_LENGTH, NonPagedPool))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc vol_unique_id failed");
		goto fail;
	}

	vol_info->sectors = (pdx->vol_size + SECTOR_SIZE - 1) / SECTOR_SIZE;

	RtlCopyUnicodeString(&vol_info->vol_name, &pdx->vol_name);
	RtlCopyUnicodeString(&vol_info->vol_unique_id, &pdx->vol_unique_id);

	vol_info->cbt_bitmap.bit_today = NULL;
	vol_info->cbt_bitmap.bit_tomorrow = NULL;
	vol_info->cbt_bitmap.bit_yesterday = NULL;

	vol_info->cbt_bitmap_copy.bit_today = NULL;
	vol_info->cbt_bitmap_copy.bit_tomorrow = NULL;
	vol_info->cbt_bitmap_copy.bit_yesterday = NULL;

	vol_info->persist_data_apply_state = VOL_PERSIST_DATA_APPLY_STATE_NOT_START;

	vol_info->persist_data = NULL;
	vol_info->persist_len = 0;

	vol_info->vol_type = pdx->vol_type;
	vol_info->phy_vol = NULL;

	KeInitializeGuardedMutex(&vol_info->bitmap_set_lock);

	vol_info->reference_count = 0;

	return vol_info;

fail:
	if (NULL != vol_info)
	{
		FreeUnicodeString(&vol_info->vol_name);
		FreeUnicodeString(&vol_info->vol_unique_id);

		ExFreePoolWithTag(vol_info, ALLOC_TAG);
	}

	return NULL;
}

VolInfo *BuildVolInfo(GroupInfo* group_info, PDEVICE_EXTENSION pdx, VolInfo* phy_vol)
{
	VolInfo* ret = NULL;
	BOOLEAN flag = FALSE;
	BOOLEAN succeed = FALSE;

	do
	{
		ret = BuildSimpleVolInfo(group_info, pdx);
		if (!ret)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to build simple vol info");
			break;
		}

		ret->phy_vol = phy_vol;

		ret->cbt_bitmap.bit_today = BitmapAlloc(ret->sectors, group_info->bitmap_granularity, BitmapAlloc, BitmapFree);
		if (ret->cbt_bitmap.bit_today == NULL)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::BitmapAlloc bit_today failed");
			break;
		}

		succeed = TRUE;
	} while (flag);

	if (!succeed)
	{
		if (ret)
		{
			FreeVolInfo(ret);
			ret = NULL;
		}
	}

	return ret;
}

VOID FreeVolBitmapNLock(VolInfo* vol_info)
{
	if (NULL != vol_info->cbt_bitmap.bit_today)
	{
		BitmapFree(vol_info->cbt_bitmap.bit_today, BitmapFree);
		vol_info->cbt_bitmap.bit_today = NULL;
	}

	if (NULL != vol_info->cbt_bitmap.bit_tomorrow)
	{
		BitmapFree(vol_info->cbt_bitmap.bit_tomorrow, BitmapFree);
		vol_info->cbt_bitmap.bit_tomorrow = NULL;
	}

	if (NULL != vol_info->cbt_bitmap.bit_yesterday)
	{
		BitmapFree(vol_info->cbt_bitmap.bit_yesterday, BitmapFree);
		vol_info->cbt_bitmap.bit_yesterday = NULL;
	}
}

VOID FreeVolBitmapCopyNLock(VolInfo* vol_info)
{
	if (NULL != vol_info->cbt_bitmap_copy.bit_today)
	{
		BitmapFree(vol_info->cbt_bitmap_copy.bit_today, BitmapFree);
		vol_info->cbt_bitmap_copy.bit_today = NULL;
	}

	if (NULL != vol_info->cbt_bitmap_copy.bit_tomorrow)
	{
		BitmapFree(vol_info->cbt_bitmap_copy.bit_tomorrow, BitmapFree);
		vol_info->cbt_bitmap_copy.bit_tomorrow = NULL;
	}

	if (NULL != vol_info->cbt_bitmap_copy.bit_yesterday)
	{
		BitmapFree(vol_info->cbt_bitmap_copy.bit_yesterday, BitmapFree);
		vol_info->cbt_bitmap_copy.bit_yesterday = NULL;
	}
}

VOID ResetVolBitmapNLock(VolInfo* vol_info)
{
	BitmapResetBit(vol_info->cbt_bitmap.bit_today, 0, vol_info->sectors);

	if (NULL != vol_info->cbt_bitmap.bit_tomorrow)
	{
		BitmapFree(vol_info->cbt_bitmap.bit_tomorrow, BitmapFree);
		vol_info->cbt_bitmap.bit_tomorrow = NULL;
	}

	if (NULL != vol_info->cbt_bitmap.bit_yesterday)
	{
		BitmapFree(vol_info->cbt_bitmap.bit_yesterday, BitmapFree);
		vol_info->cbt_bitmap.bit_yesterday = NULL;
	}
}

VOID FreeVolInfo(VolInfo *vol_info)
{
	if (vol_info->phy_vol)
	{
		InterlockedDecrement(&vol_info->phy_vol->reference_count);
	}
	FreeVolBitmapNLock(vol_info);
	FreeVolBitmapCopyNLock(vol_info);

	FreeUnicodeString(&vol_info->vol_name);
	FreeUnicodeString(&vol_info->vol_unique_id);

	if (vol_info->persist_data != NULL)
	{
		ExFreePoolWithTag(vol_info->persist_data, ALLOC_TAG);
		vol_info->persist_data = NULL;
	}

	ExFreePoolWithTag(vol_info, ALLOC_TAG);
}

VOID FreeDevExtInfo(PDeviceExtInfo dev_ext_info)
{
	ExFreePoolWithTag(dev_ext_info, ALLOC_TAG);
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

uint32_t GetQueueNum(QueueInfo *queue_info)
{
	uint32_t num = queue_info->num;
	return num;
}

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

VOID UninitQueueInfo(QueueInfo* queue_info)
{
	if (KeGetCurrentIrql() <= APC_LEVEL)
	{
		ExDeleteResourceLite(&queue_info->sync_resource);
	}
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

PDEVICE_EXTENSION FindDeviceExtensionByVolName(QueueInfo *dev_ext_queue, PUNICODE_STRING vol_name)
{
	PDEVICE_EXTENSION pdx = NULL;

	AcquireShareResource(&dev_ext_queue->sync_resource);

	PLIST_ENTRY head = &dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	PDeviceExtInfo dev_ext_info = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;

		dev_ext_info = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if(dev_ext_info->pdx->vol_name.Length == vol_name->Length && memcmp(dev_ext_info->pdx->vol_name.Buffer, vol_name->Buffer, dev_ext_info->pdx->vol_name.Length) == 0)
		{
				pdx = dev_ext_info->pdx;
				break;
		}

		req_entry = temp_req_entry;
	}

	ReleaseResource(&dev_ext_queue->sync_resource);

	return pdx;
}

PDEVICE_EXTENSION FindDeviceExtensionByVolDev(QueueInfo *dev_ext_queue, PDEVICE_OBJECT dev_obj)
{
	PDEVICE_EXTENSION pdx = NULL;

	AcquireShareResource(&dev_ext_queue->sync_resource);

	PLIST_ENTRY head = &dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	PLIST_ENTRY temp_req_entry = NULL;
	PDeviceExtInfo dev_ext_info = NULL;
	while (req_entry != head)
	{
		temp_req_entry = req_entry->Flink;

		dev_ext_info = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if (dev_ext_info->pdx->pdo == dev_obj)
		{
			pdx = dev_ext_info->pdx;
			break;
		}
		req_entry = temp_req_entry;
	}

	ReleaseResource(&dev_ext_queue->sync_resource);

	return pdx;
}

NTSTATUS InitGroupInfo(GroupInfo *group_info, PDEVICE_OBJECT device_obj)
{
	ExInitializeNPagedLookasideList(&group_info->queue_info_npage_list, NULL, NULL, 0, sizeof(QueueInfo), NOPAGED_LS_QU_ALLOC_TAG, 0);

	group_info->vol_queue = InitQueueInfo(group_info);
	if (NULL == group_info->vol_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol_info failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->delay_del_vol_queue = InitQueueInfo(group_info);
	if (NULL == group_info->delay_del_vol_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init delay_del_vol_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->snap_vol_queue = InitQueueInfo(group_info);
	if (NULL == group_info->snap_vol_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init snap_vol_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->dev_ext_queue = InitQueueInfo(group_info);
	if (NULL == group_info->dev_ext_queue)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init dev_ext_queue failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	group_info->bitmap_granularity = DEFAULT_BITMAP_GRANULARITY;
	group_info->state = IM_PG_STATE_STOP;
	group_info->reg_select_cur = 0;

	group_info->device_obj = device_obj;

	group_info->per_data_state = PERSIST_DATA_STATE_UNKNOWN;

	KeInitializeGuardedMutex(&group_info->group_lock);

	ExInitializeResourceLite(&group_info->bitmap_lock);

	return STATUS_SUCCESS;
}

VOID DestroyGroupInfo(GroupInfo *group_info)
{
	FreeVolQueue(group_info->vol_queue, group_info);
	FreeVolQueue(group_info->delay_del_vol_queue, group_info);
	FreeVolQueue(group_info->snap_vol_queue, group_info);
	FreeDeviceExtQueue(group_info->dev_ext_queue, group_info);
	ExDeleteNPagedLookasideList(&group_info->queue_info_npage_list);

	ExDeleteResourceLite(&group_info->bitmap_lock);
}

#pragma LOCKEDCODE
GroupInfo* GetGroupInfo(PDEVICE_EXTENSION pdx)
{
	GroupMemInfo *mem_info = (GroupMemInfo *)pdx->mem_info;
	GroupInfo *group_info = mem_info->group_info;

	return group_info;
}

NTSTATUS CreateGroupMem(PDEVICE_EXTENSION pdx, PDEVICE_OBJECT device_obj)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	GroupMemInfo *mem_info_create = NULL;
	PDeviceExtInfo dev_ext_info = NULL;

	do
	{
		BOOL open_flag = FALSE;
		pdx->protect_group_memory_info.nSecSize = sizeof(GroupMemInfo *);
		status = SectionOpen(PROTECT_GROUP_MEMORY_NAME, pdx->protect_group_memory_info.nSecSize,
			&pdx->protect_group_memory_info.hSection, &pdx->protect_group_memory_info.ppkoCB,
			&pdx->protect_group_memory_info.pSecKMapping, &open_flag);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::SectionCreateIner failed, error %!STATUS!", status);
			break;
		}

		// 当open_flag为false时，表示该内存是第一次创建，需要进行初始化	
		if (open_flag == FALSE)
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
			status = InitGroupInfo(mem_info_create->group_info, device_obj);//根据注册表保护组的个数来初始化内存
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::InitGroupInfo failed, error %!STATUS!", status);
				break;
			}
			*(GroupMemInfo **)(pdx->protect_group_memory_info.pSecKMapping) = mem_info_create;
			pdx->mem_info = mem_info_create;
		}
		// 此处需要做地址映射，为什么两个驱动Mapping后的地址不一样？驱动的地址空间应该是一样的。
		else
		{
			pdx->mem_info = *(GroupMemInfo **)(pdx->protect_group_memory_info.pSecKMapping);
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
		SectionClose(pdx->protect_group_memory_info.hSection, pdx->protect_group_memory_info.ppkoCB, pdx->protect_group_memory_info.pSecKMapping);

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
	if (pdx->mem_info)
	{
		if (((GroupMemInfo*)pdx->mem_info)->group_info->device_obj == device_obj)
		{
			DestroyGroupInfo(((GroupMemInfo*)pdx->mem_info)->group_info);

			ExFreePoolWithTag(pdx->mem_info, ALLOC_TAG);
		}
	}

	pdx->mem_info = NULL;

	SectionClose(pdx->protect_group_memory_info.hSection, pdx->protect_group_memory_info.ppkoCB, pdx->protect_group_memory_info.pSecKMapping);
}

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
	AcquireExclusiveResource(&dev_ext_queue->sync_resource);
	InsertTailList(&dev_ext_queue->head, &dev_ext_info->list_entry);
	dev_ext_queue->num++;
	ReleaseResource(&dev_ext_queue->sync_resource);
}

VOID ClearDeviceExtQueue(QueueInfo *dev_ext_queue)
{
	AcquireExclusiveResource(&dev_ext_queue->sync_resource);
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
	ReleaseResource(&dev_ext_queue->sync_resource);
}

BOOL RemoveVolRefByPdx(QueueInfo *vol_queue, PDEVICE_EXTENSION pdx)
{
	BOOLEAN ret = FALSE;

	AcquireExclusiveResource(&vol_queue->sync_resource);
	KIRQL old_irql;
	KeAcquireSpinLock(&vol_queue->lock, &old_irql);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->pdx == pdx)
		{
			vol_info->pdx = NULL;
			ret = TRUE;
		}
		req_entry = req_entry->Flink;
	}

	KeReleaseSpinLock(&vol_queue->lock, old_irql);
	ReleaseResource(&vol_queue->sync_resource);

	return ret;
}

BOOL RemoveDeviceExtByPdx(QueueInfo *dev_ext_queue, PDEVICE_EXTENSION pdx)
{
	BOOLEAN ret = FALSE;

	AcquireExclusiveResource(&dev_ext_queue->sync_resource);

	PLIST_ENTRY head = &dev_ext_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		DeviceExtInfo *dev_ext = CONTAINING_RECORD(req_entry, DeviceExtInfo, list_entry);
		if (dev_ext->pdx == pdx)
		{
			RemoveEntryList(req_entry);
			dev_ext_queue->num--;

			FreeDevExtInfo(dev_ext);
			ret = TRUE;
		}
		req_entry = temp_req_entry;
	}

	ReleaseResource(&dev_ext_queue->sync_resource);

	if (pdx->vol_type == VOLUME_TYPE_PHYSICAL)
	{
		RemoveVolRefByPdx(dev_ext_queue->group_info->vol_queue, pdx);
	}
	else
	{
		RemoveVolRefByPdx(dev_ext_queue->group_info->snap_vol_queue, pdx);
	}

	return ret;
}

#pragma LOCKEDCODE
VolInfo *NeedProtected(PDEVICE_EXTENSION pdx)
{
	VolInfo *vol_info = NULL;

	GroupInfo *group_info = GetGroupInfo(pdx);

	QueueInfo *vol_queue = NULL;

	if (pdx->vol_type == VOLUME_TYPE_SW_SNAPSHOT || pdx->vol_type == VOLUME_TYPE_HW_SNAPSHOT)
	{
		vol_queue = group_info->snap_vol_queue;
	}
	else
	{
		vol_queue = group_info->vol_queue;
	}

	KIRQL old_irql;
	KeAcquireSpinLock(&vol_queue->lock, &old_irql);

	PLIST_ENTRY head = &vol_queue->head;

	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (pdx == vol_info->pdx)
		{
			break;
		}

		vol_info = NULL;
		req_entry = req_entry->Flink;
	}

	if (vol_info)
	{
		InterlockedIncrement(&vol_info->reference_count);
	}

	KeReleaseSpinLock(&vol_queue->lock, old_irql);

	return vol_info;
}

BOOL RemoveSnapVolByPhyVolOnDpc(QueueInfo* vol_queue, VolInfo* phy_vol)
{
	BOOLEAN ret = FALSE;

	QueueInfo* delay_del_queue = vol_queue->group_info->delay_del_vol_queue;

	KIRQL old_irql;
	KeAcquireSpinLock(&vol_queue->lock, &old_irql);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->phy_vol == phy_vol)
		{
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
			ret = TRUE;
		}
		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&vol_queue->lock, old_irql);

	return ret;
}

BOOL RemoveVolInfoByPdx(QueueInfo *vol_queue, PDEVICE_EXTENSION pdx)
{
	BOOLEAN ret = FALSE;

	QueueInfo* delay_del_queue = vol_queue->group_info->delay_del_vol_queue;
	QueueInfo* snap_vol_queue = vol_queue->group_info->snap_vol_queue;

	AcquireExclusiveResource(&vol_queue->sync_resource);
	AcquireExclusiveResource(&delay_del_queue->sync_resource);
	AcquireExclusiveResource(&snap_vol_queue->sync_resource);

	KIRQL old_irql;
	KeAcquireSpinLock(&vol_queue->lock, &old_irql);

	PLIST_ENTRY head = &vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		PLIST_ENTRY temp_req_entry = req_entry->Flink;
		VolInfo *vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (vol_info->pdx == pdx)
		{
			if (vol_info->vol_type == VOLUME_TYPE_PHYSICAL)
			{
				RemoveSnapVolByPhyVolOnDpc(snap_vol_queue, vol_info);
			}

			RemoveEntryList(req_entry);
			vol_queue->num--;
			if (InterlockedCompareExchange(&vol_info->reference_count, 0, 0) != 0)
			{
				PushVolQueueNLock(delay_del_queue, vol_info);
				TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::vol_info reference count %d is not 0, delay delete it", vol_info->reference_count);
			}
			else
			{
				FreeVolInfo(vol_info);
			}
			ret = TRUE;
		}
		req_entry = temp_req_entry;
	}

	KeReleaseSpinLock(&vol_queue->lock, old_irql);

	ReleaseResource(&snap_vol_queue->sync_resource);
	ReleaseResource(&delay_del_queue->sync_resource);
	ReleaseResource(&vol_queue->sync_resource);

	return ret;
}

BOOL RemoveVolInfoByVolInfo(QueueInfo *vol_queue, VolInfo *find_vol_info)
{	
	return RemoveVolInfoByPdx(vol_queue, find_vol_info->pdx);
}



VolInfo* GetSnapVolByPhyVol(QueueInfo* vol_queue, VolInfo* phy_vol)
{
	VolInfo* snap_vol = NULL;

	AcquireShareResource(&vol_queue->sync_resource);

	PLIST_ENTRY head = &vol_queue->head;

	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		snap_vol = CONTAINING_RECORD(req_entry, VolInfo, list_entry);
		if (snap_vol->phy_vol == phy_vol)
		{
			break;
		}

		snap_vol = NULL;
		req_entry = req_entry->Flink;
	}

	if (snap_vol)
	{
		InterlockedIncrement(&snap_vol->reference_count);
	}

	ReleaseResource(&vol_queue->sync_resource);

	return snap_vol;
}

#ifdef DBG_OM_BITMAP
#define IM_PG_BIT_MASK(nr)        (1UL << ((nr) % IM_PG_BITS_PER_BYTE))
#define IM_PG_BIT_WORD(nr)        ((nr) / IM_PG_BITS_PER_BYTE)
VOID SetBit(uint64_t nr, unsigned char *addr)
{
	unsigned char mask = IM_PG_BIT_MASK(nr);
	unsigned char *p = addr + IM_PG_BIT_WORD(nr);
	*p |= mask;
}
#endif

BOOLEAN ShutdownBitmapPrepare(GroupInfo *group_info)
{
	BOOLEAN ret = TRUE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!::Prepare shutdown bitmap");

	uint64_t nb_sectors = (uint64_t)1U << group_info->bitmap_granularity;

	AcquireShareResource(&group_info->bitmap_lock);
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
		vol_info->persist_data = ExAllocatePoolWithTag(PagedPool, vol_info->persist_len, ALLOC_TAG);
		if (NULL == vol_info->persist_data)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Malloc persist data buffer failed, length %d", vol_info->persist_len);
			ret = FALSE;
			break;
		}
		memset(vol_info->persist_data, 0, vol_info->persist_len);

		if (vol_info->cbt_bitmap.bit_yesterday)
		{
			MergeBitmap(vol_info->cbt_bitmap.bit_today, vol_info->cbt_bitmap.bit_yesterday, BitmapAlloc, BitmapFree);
		}

		if (!GetBitmapBuffer(vol_info->cbt_bitmap.bit_today, (unsigned char*)vol_info->persist_data, vol_info->persist_len))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Get bitmap buffer failed");
			ret = FALSE;
			break;
		}

#ifdef DBG_OM_BITMAP
		int64_t sector = 0;
		PVOID test_buffer = ExAllocatePoolWithTag(PagedPool, vol_info->persist_len, ALLOC_TAG);
		memset(test_buffer, 0, vol_info->persist_len);

		OM_BITMAP_IT* hbi = (OM_BITMAP_IT *)ExAllocatePoolWithTag(NonPagedPool, sizeof(OM_BITMAP_IT), ALLOC_TAG);
		BitmapItInit(hbi, vol_info->cbt_bitmap.bit_today, 0);

		while (1)
		{
			sector = BitmapItNext(hbi);
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
	ReleaseResource(&group_info->bitmap_lock);

	return ret;
}


typedef VOID FREE_BITMAP_FUN(VolInfo *);
typedef FREE_BITMAP_FUN* PFREE_BITMAP_FUN;
VOID DropBitmapNLock(GroupInfo *group_info, PFREE_BITMAP_FUN free_fun)
{
	PLIST_ENTRY head = &group_info->vol_queue->head;
	VolInfo *vol_info = NULL;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

		free_fun(vol_info);

		req_entry = req_entry->Flink;
	}
}

typedef BOOLEAN CHECK_BITMAP_FUN(VolInfo*);
typedef CHECK_BITMAP_FUN* PCHECK_BITMAP_FUN;
BOOLEAN CheckBitmap(GroupInfo *group_info, PCHECK_BITMAP_FUN check_fun)
{
	BOOLEAN ret = TRUE;

	AcquireShareResource(&group_info->bitmap_lock);
	AcquireShareResource(&group_info->vol_queue->sync_resource);

	PLIST_ENTRY head = &group_info->vol_queue->head;
	VolInfo *vol_info = NULL;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

		ret = check_fun(vol_info);
		if (!ret)
		{
			break;
		}

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&group_info->vol_queue->sync_resource);
	ReleaseResource(&group_info->bitmap_lock);

	return ret;
}

NTSTATUS CreateBitmapCopy(GroupInfo *group_info)
{
	NTSTATUS status = STATUS_SUCCESS;

	AcquireShareResource(&group_info->vol_queue->sync_resource);

	VolInfo *vol_info = NULL;
	PLIST_ENTRY head = &group_info->vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

		vol_info->cbt_bitmap_copy.bit_tomorrow = BitmapAlloc(vol_info->sectors, group_info->bitmap_granularity, BitmapAlloc, BitmapFree);
		if (vol_info->cbt_bitmap_copy.bit_tomorrow == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::BitmapAlloc bit_tomorrow failed");

			break;
		}

		vol_info->cbt_bitmap_copy.bit_today = NULL;
		vol_info->cbt_bitmap_copy.bit_yesterday = NULL;

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&group_info->vol_queue->sync_resource);

	return status;
}

BOOLEAN CheckBitmapOnSunset(VolInfo *vol_info)
{
	if (vol_info->cbt_bitmap.bit_yesterday != NULL)
	{
		TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::bit_yesterday is invalid for vol %wZ", &vol_info->vol_name);
		return FALSE;
	}

	if (vol_info->cbt_bitmap.bit_tomorrow != NULL)
	{
		TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::bit_tomorrow is invalid for vol %wZ", &vol_info->vol_name);
		return FALSE;
	}

	return TRUE;
}

NTSTATUS BitmapSunset(GroupInfo *group_info, char* snap_id)
{
	NTSTATUS status = STATUS_SUCCESS;
	
	BOOLEAN drop_bitmap = FALSE;
	if (!CompareSnapId(group_info, snap_id))
	{
		drop_bitmap = TRUE;
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Group snap GUID %!GUID! doesn't match to %!GUID!, clear today bitmap", (GUID*)group_info->snap_id, (GUID*)snap_id);
	}

	if (!CheckBitmap(group_info, CheckBitmapOnSunset))
	{
		status = STATUS_UNSUCCESSFUL;
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CheckBitmap failed");
	}
	else
	{
		status = CreateBitmapCopy(group_info);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::CreateBitmapCopy failed");
		}
	}

	AcquireExclusiveResource(&group_info->bitmap_lock);
	AcquireShareResource(&group_info->vol_queue->sync_resource);

	if (drop_bitmap)
	{
		DropBitmapNLock(group_info, ResetVolBitmapNLock);
	}

	ClearVolQueue(group_info->snap_vol_queue);

	if (NT_SUCCESS(status))
	{
		VolInfo *vol_info = NULL;
		PLIST_ENTRY head = &group_info->vol_queue->head;
		PLIST_ENTRY req_entry = head->Flink;
		while (req_entry != head)
		{
			vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

			vol_info->cbt_bitmap.bit_tomorrow = vol_info->cbt_bitmap_copy.bit_tomorrow;
			vol_info->cbt_bitmap_copy.bit_tomorrow = NULL;

			req_entry = req_entry->Flink;
		}
	}

	ReleaseResource(&group_info->vol_queue->sync_resource);
	ReleaseResource(&group_info->bitmap_lock);

	DropBitmapNLock(group_info, FreeVolBitmapCopyNLock);

	return status;
}

BOOLEAN CheckBitmapOnMidnight(VolInfo *vol_info)
{
	if (vol_info->cbt_bitmap.bit_yesterday != NULL)
	{
		TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::bit_yesterday is invalid for vol %wZ", &vol_info->vol_name);
		return FALSE;
	}

	if (vol_info->cbt_bitmap.bit_tomorrow == NULL)
	{
		TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::bit_tomorrow is invalid for vol %wZ", &vol_info->vol_name);
		return FALSE;
	}

	return TRUE;
}

NTSTATUS BitmapMidnight(GroupInfo *group_info)
{
	NTSTATUS status = STATUS_SUCCESS;

	if(!CheckBitmap(group_info, CheckBitmapOnMidnight))
	{
		status = STATUS_UNSUCCESSFUL;
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Drop bitmap because CheckBitmap failed");
		return status;
	}

	AcquireExclusiveResource(&group_info->bitmap_lock);
	AcquireShareResource(&group_info->vol_queue->sync_resource);

	VolInfo *vol_info = NULL;
	PLIST_ENTRY head = &group_info->vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

		vol_info->cbt_bitmap.bit_yesterday = vol_info->cbt_bitmap.bit_today;
		vol_info->cbt_bitmap.bit_today = vol_info->cbt_bitmap.bit_tomorrow;
		vol_info->cbt_bitmap.bit_tomorrow = NULL;

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&group_info->vol_queue->sync_resource);
	ReleaseResource(&group_info->bitmap_lock);

	return status;
}

NTSTATUS BitmapSunrise(GroupInfo *group_info, char* snap_id, BOOLEAN failed)
{
	NTSTATUS status = STATUS_SUCCESS;

	AcquireExclusiveResource(&group_info->bitmap_lock);
	AcquireShareResource(&group_info->vol_queue->sync_resource);

	VolInfo *vol_info = NULL;
	PLIST_ENTRY head = &group_info->vol_queue->head;
	PLIST_ENTRY req_entry = head->Flink;
	while (req_entry != head)
	{
		vol_info = CONTAINING_RECORD(req_entry, VolInfo, list_entry);

		if (vol_info->cbt_bitmap.bit_yesterday)
		{
			if (failed)
			{
				MergeBitmap(vol_info->cbt_bitmap.bit_today, vol_info->cbt_bitmap.bit_yesterday, BitmapAlloc, BitmapFree);
			}
			else
			{
				SetSnapId(group_info, snap_id);
			}

			BitmapFree(vol_info->cbt_bitmap.bit_yesterday, BitmapFree);
			vol_info->cbt_bitmap.bit_yesterday = NULL;
		}

		///call Sunrise without Midnight ahead
		if (vol_info->cbt_bitmap.bit_tomorrow)
		{
			BitmapFree(vol_info->cbt_bitmap.bit_tomorrow, BitmapFree);
			vol_info->cbt_bitmap.bit_tomorrow = NULL;
		}

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&group_info->vol_queue->sync_resource);

	ClearVolQueue(group_info->snap_vol_queue);
	ReleaseResource(&group_info->bitmap_lock);

	return status;
}

NTSTATUS GetYesterdayBitmap(GroupInfo *group_info, VolInfo* vol_info, BOOLEAN need_snap, unsigned char* data, uint32_t size)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	VolInfo* snap_vol = NULL;
	do
	{
		if (vol_info->cbt_bitmap.bit_yesterday == NULL)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::bit_yesterday is empty");
			status = STATUS_NOT_FOUND;
			break;
		}

		if (need_snap)
		{
			snap_vol = GetSnapVolByPhyVol(group_info->snap_vol_queue, vol_info);
			if (snap_vol)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to find the snapshot vol");
				status = STATUS_NOT_FOUND;
				break;
			}

			if (snap_vol->cbt_bitmap.bit_today == NULL)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::snap_vol bitmap is empty");
				status = STATUS_NOT_FOUND;
				break;
			}

			if (!MergeBitmap(vol_info->cbt_bitmap.bit_yesterday, snap_vol->cbt_bitmap.bit_today, BitmapAlloc, BitmapFree))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to merge snapshot bitmap");
				status = STATUS_UNSUCCESSFUL;
				break;
			}
		}
		else
		{
			TracePrint(TRACE_LEVEL_WARNING, "%!FUNC!::Snap bitmap is not required, this is crash consistent");
		}

		if (!GetBitmapBuffer(vol_info->cbt_bitmap.bit_yesterday, data, size))
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}
	} while (flag);

	if (snap_vol)
	{
		InterlockedDecrement(&snap_vol->reference_count);
	}

	return STATUS_SUCCESS;
}

VOID ProcessStop(GroupInfo *group_info)
{
	SwitchState(group_info, IM_PG_STATE_STOP);

	ClearVolQueue(group_info->vol_queue);
	ClearVolQueue(group_info->snap_vol_queue);
	CheckRemoveDelayDeleteVolQueue(group_info);

	group_info->per_data_state = PERSIST_DATA_STATE_DISCARD;
}

VOID SwitchState(GroupInfo *group_info, LONG state)
{
	InterlockedExchange(&group_info->state, state);
}

BOOLEAN IsInStopState(GroupInfo *group_info)
{
	return (InterlockedCompareExchange(&group_info->state, 0, 0) == IM_PG_STATE_STOP);
}

BOOLEAN IsInSnapshotState(GroupInfo *group_info)
{
	return (InterlockedCompareExchange(&group_info->state, 0, 0) == IM_PG_STATE_SNAPSHOT);
}

VOID SetSnapId(GroupInfo *group_info, char* snap_id)
{
	KeAcquireGuardedMutex(&group_info->group_lock);
	memcpy(group_info->snap_id, snap_id, SNAP_ID_LEN);
	RegWriteSnapId(group_info->snap_id);
	KeReleaseGuardedMutex(&group_info->group_lock);
}

VOID ResetSnapId(GroupInfo *group_info)
{
	KeAcquireGuardedMutex(&group_info->group_lock);
	memset(group_info->snap_id, 0, SNAP_ID_LEN);
	RegWriteSnapId(group_info->snap_id);
	KeReleaseGuardedMutex(&group_info->group_lock);
}

BOOLEAN CompareSnapId(GroupInfo *group_info, char* snap_id)
{
	BOOLEAN ret = FALSE;

	KeAcquireGuardedMutex(&group_info->group_lock);
	ret = (memcmp(group_info->snap_id, snap_id, SNAP_ID_LEN) == 0);
	KeReleaseGuardedMutex(&group_info->group_lock);

	return ret;
}

int GetNextBit(unsigned char c)
{
	if (0 == c)
	{
		return -1;
	}
	if (c & 0x01)
	{
		return 0;
	}
	if (c & 0x02)
	{
		return 1;
	}
	if (c & 0x04)
	{
		return 2;
	}
	if (c & 0x08)
	{
		return 3;
	}
	if (c & 0x10)
	{
		return 4;
	}
	if (c & 0x20)
	{
		return 5;
	}
	if (c & 0x40)
	{
		return 6;
	}
	if (c & 0x80)
	{
		return 7;
	}
	return -1;
}

static inline void ResetBit(int nr, unsigned char* addr)
{
	uint8_t* p = addr;
	uint8_t mask = ~(1U << nr);
	*p &= mask;
}

BOOLEAN MergeBitMap(GroupInfo *group_info, VolInfo *vol_info, unsigned char* data, unsigned int size, uint64_t bit_count)
{
	BOOLEAN ret = MergeBitmapByBuffer(vol_info->cbt_bitmap.bit_today, data, size);

#ifdef DBG_OM_BITMAP

	OM_BITMAP* test_bitmap = BitmapAlloc(vol_info->sectors, group_info->bitmap_granularity, BitmapAlloc, BitmapFree);

	int bit = 0;
	unsigned char *cur = data;
	unsigned int len = size;
	unsigned int pos = 0;
	uint64_t sector_num = 0;
	uint64_t nb_sectors = (uint64_t)1U << group_info->bitmap_granularity;
	while (len > 0)
	{
		while (1)
		{
			bit = GetNextBit(*cur);
			if (-1 == bit)
			{
				break;
			}

			sector_num = nb_sectors * (8 * pos + bit);
			if (vol_info->sectors > sector_num + nb_sectors)
			{
				BitmapSetBit(test_bitmap, sector_num, nb_sectors);
			}
			else
			{
				BitmapSetBit(test_bitmap, sector_num, vol_info->sectors - sector_num);
				break;
			}
			ResetBit(bit, cur);
		}
		pos++;
		cur++;
		len--;
	}

	if (!CompareBitmap(vol_info->cbt_bitmap.bit_today, test_bitmap))
	{
		RaiseException();
	}

	if (GetBitmapCount(vol_info->cbt_bitmap.bit_today) != bit_count)
	{
		RaiseException();
	}

	BitmapFree(test_bitmap, BitmapFree);
#else
	UNREFERENCED_PARAMETER(group_info);
	UNREFERENCED_PARAMETER(bit_count);
#endif

	return ret;
}

VOID MergePersistBitmap(GroupInfo *group_info)
{
	if (!OpenPersistFile(group_info))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reset snap_id because open persist file failed");
		ResetSnapId(group_info);

		return;
	}

	if (group_info->per_data_state != PERSIST_DATA_STATE_SUCCEED)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Reset snap_id because persist data state is %d", group_info->per_data_state);
		ResetSnapId(group_info);

		return;
	}

	QueueInfo *vol_queue = group_info->vol_queue;

	if (TRUE == IsEmptyVolQueue(vol_queue))
	{
		return;
	}

	AcquireExclusiveResource(&group_info->bitmap_lock);
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

		if (vol_info->persist_data_apply_state == VOL_PERSIST_DATA_APPLY_STATE_NOT_START)
		{
			size = (vol_info->sectors + nb_sectors - 1) / nb_sectors;
			size = (size + IM_PG_BITS_PER_BYTE - 1) / IM_PG_BITS_PER_BYTE;
			bit_data = (UCHAR*)ExAllocatePoolWithTag(PagedPool, size, ALLOC_TAG);
			if (bit_data != NULL)
			{
				status = GetPersistBitmapByVol(vol_info, bit_data, (uint32_t)size, &bit_count);
				if (!NT_SUCCESS(status))
				{
					ExFreePoolWithTag(bit_data, ALLOC_TAG);
					bit_data = NULL;

					TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to get persist bit data for volume %wZ", &vol_info->vol_name);
				}
			}
			else
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Failed to alloc bit_data for volume %wZ", &vol_info->vol_name);
			}

			if (bit_data == NULL)
			{
				BitmapSetBit(vol_info->cbt_bitmap.bit_today, 0, vol_info->sectors);
			}
			else
			{
				if (!MergeBitMap(group_info, vol_info, bit_data, (uint32_t)size, bit_count))
				{
					TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Merge bitmap failed for volume %wZ", &vol_info->vol_name);
					BitmapSetBit(vol_info->cbt_bitmap.bit_today, 0, vol_info->sectors);
				}
			}

			if (bit_data)
			{
				ExFreePoolWithTag(bit_data, ALLOC_TAG);
				bit_data = NULL;
			}
		}

		vol_info->persist_data_apply_state = VOL_PERSIST_DATA_APPLY_STATE_DONE;

		req_entry = req_entry->Flink;
	}

	ReleaseResource(&vol_queue->sync_resource);
	ReleaseResource(&group_info->bitmap_lock);
}

VOID WriteSetBitmap(GroupInfo* group_info, VolInfo *vol_info, uint64_t sector_offset, uint64_t sector_length)
{
	AcquireShareResource(&group_info->bitmap_lock);
	KeAcquireGuardedMutex(&vol_info->bitmap_set_lock);
	BitmapSetBit(vol_info->cbt_bitmap.bit_today, sector_offset, sector_length);

	if (vol_info->cbt_bitmap.bit_tomorrow)
	{
		BitmapSetBit(vol_info->cbt_bitmap.bit_tomorrow, sector_offset, sector_length);
	}
	KeReleaseGuardedMutex(&vol_info->bitmap_set_lock);
	ReleaseResource(&group_info->bitmap_lock);
}

NTSTATUS StartProtectByPhysicalVol(GroupInfo *group_info, PDEVICE_EXTENSION pdx)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN flag = FALSE;

	TracePrint(TRACE_LEVEL_INFORMATION, "%!FUNC!:Snapshot volume name %wZ", &pdx->vol_name);

	VolInfo *vol_info = NULL;

	do
	{
#ifndef LESS_STATE_CHECK
		if (!IsInSnapshotState(group_info))
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::CBT driver is in snapshot state");
			break;
		}
#endif

		if (pdx->orig_pdx == NULL)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::orig_pdx is empty for snapshot volume");
			status = STATUS_NOT_FOUND;
			break;
		}

		VolInfo* phy_vol = GetPhyVolInfoBySnapPdx(group_info->vol_queue, pdx);
		if (phy_vol == NULL)
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Cannot find the physcial volume for snapshot volume %wZ", &pdx->vol_name);
			break;
		}

		vol_info = BuildVolInfo(group_info, pdx, phy_vol);
		if (NULL == vol_info)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Init vol failed");
			break;
		}

		QueueInfo *vol_queue = group_info->snap_vol_queue;
		if (FindVolInfoByVolInfo(vol_queue, vol_info))
		{
			TracePrint(TRACE_LEVEL_VERBOSE, "%!FUNC!::Vol already exists, name = %wZ", &pdx->vol_name);
			break;
		}

		PushVolQueue(vol_queue, vol_info);
		vol_info = NULL;
	} while (flag);

	if (vol_info)
	{
		FreeVolInfo(vol_info);
		vol_info = NULL;
	}

	return status;
}