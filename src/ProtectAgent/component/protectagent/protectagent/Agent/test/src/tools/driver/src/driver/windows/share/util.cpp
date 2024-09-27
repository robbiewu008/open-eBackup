#include "util.h"

#include <ntstrsafe.h>

#include "wpp_trace.h"
#include "util.tmh"



BOOLEAN AllocUnicodeString(IN PUNICODE_STRING str, IN USHORT size, IN POOL_TYPE type)
{
	str->Length = str->MaximumLength = size;
	str->Buffer = (PWCH)ExAllocatePoolWithTag(type, size, ALLOC_TAG);
	if (str->Buffer == NULL)
	{
		return FALSE;
	}

	RtlZeroMemory(str->Buffer, size);

	return TRUE;
}

VOID FreeUnicodeString(IN PUNICODE_STRING str)
{
	if (str->Buffer)
	{
		ExFreePoolWithTag(str->Buffer, ALLOC_TAG);
	}

	RtlZeroMemory(str, sizeof(UNICODE_STRING));
}

BOOLEAN IsUnicodeStringEmpty(IN PUNICODE_STRING str)
{
	return (str->Buffer == NULL || str->Length == 0);
}

VOID GetParentPath(IN PUNICODE_STRING str)
{
	if (str->Buffer == NULL || str->Length == 0)
	{
		return;
	}

	USHORT  pos = str->Length - 1;
	while (pos > 0)
	{
		if (str->Buffer[pos] == L'\\')
		{
			break;
		}

		pos--;
	}

	str->Length = pos + 1;
}

VOID AcquireExclusiveResource(PERESOURCE res)
{
	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(res, TRUE);
}

VOID AcquireShareResource(PERESOURCE res)
{
	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(res, TRUE);
}

VOID ReleaseResource(PERESOURCE res)
{
	ExReleaseResourceLite(res);
	KeLeaveCriticalRegion();
}

uint32_t make_log_lower_bound(uint64_t value)
{
	uint32_t ret = 0;
	while (value >= 2)
	{
		ret++;
		value = value >> 1;
	}

	return ret;
}

uint32_t make_log_upper_bound(uint64_t value)
{
	uint32_t ret = 0;
	while (value != 0)
	{
		ret++;
		value = value >> 1;
	}

	return ret;
}

bool is_int_power(uint64_t value)
{
	bool ret = true;

	while (value != 0)
	{
		if (value == 2)
		{
			ret = true;
			break;
		}
		else if (value < 2)
		{
			ret = false;
			break;
		}

		value = value >> 1;
	}

	return ret;
}


VOID LogEvent(IN PDRIVER_OBJECT drv_obj, IN NTSTATUS error_code, IN NTSTATUS final_status, IN PUNICODE_STRING param1, IN PUNICODE_STRING param2, IN PVOID dump_data, IN USHORT dump_size)
{
	BOOLEAN flag = FALSE;
	PIO_ERROR_LOG_PACKET log_packet = NULL;
	do
	{
		if (!drv_obj)
		{
			break;
		}

		ULONG param1_len = 0;
		ULONG param2_len = 0;

		ULONG pack_len = sizeof(IO_ERROR_LOG_PACKET) + dump_size;
		if (param1)
		{
			param1_len = param1->Length + sizeof(WCHAR);

			if (param2)
			{
				param2_len = param2->Length + sizeof(WCHAR);
			}
		}

		pack_len += param1_len + param2_len;

		log_packet = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(drv_obj, (UCHAR)pack_len);
		if (!log_packet)
		{
			break;
		}

		RtlZeroMemory(log_packet, pack_len);
		log_packet->ErrorCode = error_code;
		log_packet->FinalStatus = final_status;
		log_packet->DumpDataSize = dump_size;
		log_packet->StringOffset = sizeof(IO_ERROR_LOG_PACKET) + log_packet->DumpDataSize;

		if (dump_data && log_packet->DumpDataSize)
		{
			RtlCopyMemory(log_packet->DumpData, dump_data, log_packet->DumpDataSize);
		}

		PBYTE string_pos = (PBYTE)log_packet + log_packet->StringOffset;
		if (param1_len > 0)
		{
			RtlCopyMemory(string_pos, param1->Buffer, param1->Length);
			string_pos += param1_len;
			log_packet->NumberOfStrings++;
		}

		if (param2_len > 0)
		{
			RtlCopyMemory(string_pos, param2->Buffer, param2->Length);
			string_pos += param2_len;
			log_packet->NumberOfStrings++;
		}

		IoWriteErrorLogEntry(log_packet);
	} while (flag);
}