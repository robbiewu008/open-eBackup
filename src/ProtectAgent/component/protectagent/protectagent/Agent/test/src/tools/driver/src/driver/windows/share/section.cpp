#include "Section.h"


#include "wpp_trace.h"
#include "section.tmh"


extern "C"
{
POBJECT_TYPE MmSectionObjectType;	//ddk中的全局变量
}

void *ReferenceByHandle(HANDLE hObject)
{
	void * m_pObject = NULL;
	if (!NT_SUCCESS(ObReferenceObjectByHandle(
		hObject,
		SECTION_ALL_ACCESS,
		NULL,
		KernelMode,
		(VOID**)&m_pObject,
		NULL)))
	{
		return NULL;
	}
	return m_pObject;
}

VOID SectionInit(PVOID pSecKMapping)
{
	SectionConfig *config = (SectionConfig *)(pSecKMapping);
	memset(config, 0, sizeof(SectionConfig));
	config->max_pos = MAX_USE_POS;
}

// 创建section内存，open_flag表示是第一次创建还是打开已有的
NTSTATUS SectionOpen(
	PWCH name,
	SIZE_T nSecSize,
	HANDLE* hSection,
	PVOID* ppkoCB,
	PVOID* pSecKMapping,
	BOOL *open_flag)
{
	ASSERT(ppkoCB);
	ASSERT(pSecKMapping);
	ASSERT(hSection);

	*ppkoCB = NULL;
	*pSecKMapping = NULL;
	*hSection = NULL;

	NTSTATUS status = STATUS_SUCCESS;
	PVOID	ob_section = NULL;
	HANDLE SectionHandle = NULL;
	PVOID SecMapping = NULL;

	BOOLEAN Flag = FALSE;

	LARGE_INTEGER ssize;
	SIZE_T vwsize;

	OBJECT_ATTRIBUTES o_a;
	RtlZeroMemory(&o_a, sizeof(OBJECT_ATTRIBUTES));

	UNICODE_STRING object_name;
	RtlInitUnicodeString(&object_name, name);
	InitializeObjectAttributes(&o_a, &object_name, OBJ_KERNEL_HANDLE, NULL, NULL);

	do
	{
		status = ZwOpenSection(&SectionHandle, SECTION_ALL_ACCESS, &o_a);
		if (!NT_SUCCESS(status))
		{
			ssize.QuadPart = nSecSize;
			status = ZwCreateSection(
				&SectionHandle,
				SECTION_ALL_ACCESS,
				&o_a,
				&ssize,
				PAGE_READWRITE,
				SEC_COMMIT,
				0);
			if (!NT_SUCCESS(status))
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ZwCreateSection failed, error %!STATUS!", status);
				break;
			}
			*open_flag = FALSE;
		}
		else
		{
			*open_flag = TRUE;
		}

		if (NULL == SectionHandle)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		ob_section = ReferenceByHandle(SectionHandle);
		if (!ob_section)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ReferenceByHandle failed");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		vwsize = nSecSize;
		status = MmMapViewInSystemSpace(ob_section, &SecMapping, &vwsize);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::MmMapViewInSystemSpace failed, error %!STATUS!", status);
			break;
		}

		if (vwsize != nSecSize)
		{
			status = STATUS_UNSUCCESSFUL;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::View size does not equal to desired size, view size %d, desired %d", (ULONG)vwsize, (ULONG)nSecSize);
			break;
		}
	} while (Flag);

	if (!NT_SUCCESS(status))
	{
		SectionReleaseIner(ob_section, SectionHandle, SecMapping);
	}
	else
	{
		*ppkoCB = ob_section;
		*pSecKMapping = SecMapping;
		*hSection = SectionHandle;
	}

	return status;
}

VOID SectionClose(HANDLE hSection, PVOID pkoCB, PVOID pSecKMapping)
{
	SectionReleaseIner(pkoCB, hSection, pSecKMapping);
}

NTSTATUS SectionCreateIner(
						   PWCH name,
						   SIZE_T nSecSize,
						   HANDLE* hSection,
						   PVOID* ppkoCB,
						   PVOID* pSecKMapping)
{
	NTSTATUS status = STATUS_SUCCESS;
	void*	ob_section;
	//HANDLE	hSection;
	ASSERT(ppkoCB);
	ASSERT(pSecKMapping);
	*ppkoCB = NULL;
	*pSecKMapping = NULL;

	LARGE_INTEGER ssize;
	NTSTATUS callstatus;
	SIZE_T vwsize;

	OBJECT_ATTRIBUTES o_a;
	RtlZeroMemory(&o_a, sizeof(OBJECT_ATTRIBUTES));

	UNICODE_STRING object_name;
	RtlInitUnicodeString(&object_name, name);
	InitializeObjectAttributes(&o_a, &object_name, OBJ_KERNEL_HANDLE, NULL, NULL);

	callstatus = ZwOpenSection(hSection, SECTION_ALL_ACCESS, &o_a);
	if (!NT_SUCCESS(callstatus))
	{
		ssize.QuadPart = nSecSize;
		callstatus = ZwCreateSection(
			hSection,
			SECTION_ALL_ACCESS,
			&o_a,
			&ssize,
			PAGE_READWRITE,
			SEC_COMMIT,
			0);
		if (!NT_SUCCESS(callstatus))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ZwCreateSection failed, error %!STATUS!", callstatus);
			return STATUS_UNSUCCESSFUL;
		}
	}

	if( NULL == *hSection )
	{
		return STATUS_UNSUCCESSFUL;
	}

	ob_section = ReferenceByHandle(*hSection);
	if (!ob_section)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ReferenceByHandle failed");
		return STATUS_UNSUCCESSFUL;
	}

	vwsize = nSecSize;
	if (!NT_SUCCESS(MmMapViewInSystemSpace(ob_section, pSecKMapping, &vwsize)))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::MmMapViewInSystemSpace failed");
		return STATUS_UNSUCCESSFUL;
	}
	if(vwsize != nSecSize)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::View size does not equal to desired size, view size %d, desired %d", (ULONG)vwsize, (ULONG)nSecSize);
	}
	*ppkoCB = ob_section;
	
	return status;
}

NTSTATUS SectionCreate(
					   SIZE_T nSecSize,
					   PVOID* ppkoCB,
					   PVOID* pSecKMapping)
{
	HANDLE hSection = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	//HANDLE	hSection;
	ASSERT(ppkoCB);
	ASSERT(pSecKMapping);
	*ppkoCB = NULL;
	*pSecKMapping = NULL;

	LARGE_INTEGER ssize;
	NTSTATUS callstatus;
	SIZE_T viewSize = 0;

	OBJECT_ATTRIBUTES	o_a;
	RtlZeroMemory(&o_a, sizeof(OBJECT_ATTRIBUTES));

	UNICODE_STRING object_name;
	RtlInitUnicodeString(&object_name, L"DP_SECTION");
	InitializeObjectAttributes(&o_a, &object_name, OBJ_KERNEL_HANDLE, NULL, NULL);

	callstatus = ZwOpenSection(&hSection, SECTION_ALL_ACCESS, &o_a);
	if (!NT_SUCCESS(callstatus))
	{
		ssize.QuadPart = nSecSize;
		callstatus = ZwCreateSection(
			&hSection,
			SECTION_ALL_ACCESS,
			&o_a,
			&ssize,
			PAGE_READWRITE,
			SEC_COMMIT,
			0);
		if (!NT_SUCCESS(callstatus))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ZwCreateSection failed, error %!STATUS!", callstatus);
			return STATUS_UNSUCCESSFUL;
		}
	}

	if( NULL == hSection )
	{
		return STATUS_UNSUCCESSFUL;
	}

	callstatus = ZwMapViewOfSection(
		hSection,
		(HANDLE)-1,
		pSecKMapping,
		0,
		nSecSize,
		0,
		&viewSize,
		ViewUnmap,
		0,
		PAGE_READWRITE);
	if (!NT_SUCCESS(callstatus))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ZwMapViewOfSection failed, error %!STATUS!", callstatus);
		ZwClose(hSection);   //添加申请出错时，句柄的关闭处理
		return ERROR_INVALID_FUNCTION;
	}
	ZwClose(hSection);

	return status;
}

NTSTATUS SectionRelease(PVOID pSecKMapping)
{
	ASSERT(pSecKMapping);

	ZwUnmapViewOfSection((HANDLE)-1, pSecKMapping);

	return STATUS_SUCCESS;
}

NTSTATUS SectionReleaseIner(
							 PVOID pkoCB,
							 HANDLE hSection,
							 PVOID pSecKMapping)
{
	NTSTATUS status = STATUS_SUCCESS;

	if (pSecKMapping)
	{
		status = MmUnmapViewInSystemSpace(pSecKMapping);
		ASSERT(NT_SUCCESS(status));
	}

	if (pkoCB)
		ObDereferenceObject(pkoCB);

	if (hSection)
	{
		ZwClose(hSection);
	}

	return status;
}

NTSTATUS SectionCtxAttach(
						   PVOID pkoCB,
						   SIZE_T nSecSize,
						   ULONG dwOffset,
						   PVOID* pSecUMMapping)
{
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hSectionDup = NULL;
	SIZE_T viewSize;
	LARGE_INTEGER offset;
	ASSERT(pkoCB);
	ASSERT(pSecUMMapping);
	*pSecUMMapping = NULL;
	offset.LowPart = dwOffset;
	offset.HighPart = 0;

	POBJECT_TYPE SectonType = MmSectionObjectType;
	NTSTATUS callstatus = ObOpenObjectByPointer(
		pkoCB,
		OBJ_KERNEL_HANDLE,
		NULL,
		READ_CONTROL | SECTION_QUERY | SECTION_MAP_READ,
		SectonType,
		KernelMode,
		&hSectionDup);

	if (!NT_SUCCESS(callstatus))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ObOpenObjectByPointer failed, error %!STATUS!", callstatus);
		return ERROR_INVALID_FUNCTION;
	}
	ASSERT(hSectionDup);

	viewSize = nSecSize;
	callstatus = ZwMapViewOfSection(
		hSectionDup,
		NtCurrentProcess(),
		pSecUMMapping,
		0,
#if _WIN32
		(ULONG)
#endif
		nSecSize,
		&offset,
		&viewSize,
		ViewUnmap,
		0,
		PAGE_READWRITE);
	if (!NT_SUCCESS(callstatus))
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ZwMapViewOfSection failed, error %!STATUS!", callstatus);
		ZwClose(hSectionDup);   //添加申请出错时，句柄的关闭处理
		return ERROR_INVALID_FUNCTION;
	}
	ZwClose(hSectionDup);

	return status;
}

NTSTATUS SectionCtxDetach(
						   HANDLE pUserProcess_KH,
						   PVOID pSecUMMapping)
{
	ASSERT(pUserProcess_KH);

	if (pSecUMMapping)
	{
		NTSTATUS status = ZwUnmapViewOfSection(pUserProcess_KH, pSecUMMapping);
		if (!NT_SUCCESS(status))
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ZwUnmapViewOfSection failed, error %!STATUS!", status);
			return  ERROR_INVALID_FUNCTION;
		}
	}
	else
	{
		return ERROR_INVALID_FUNCTION;
	}

	return STATUS_SUCCESS;
}
