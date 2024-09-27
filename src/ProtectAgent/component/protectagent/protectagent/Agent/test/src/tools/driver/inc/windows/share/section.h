#ifndef _SECTION_H
#define _SECTION_H

#include "driver.h"
#include <dderror.h>

#define SECTION_SIZE (50*1024*1024)
#define MAX_USE_POS (40*1024*1024)
#define SEND_MAX_NUM (100)
#define SEND_MAX_SIZE (5*1024*1024)

typedef struct SectionConfig
{
	uint32_t in_size;
	uint32_t out_size;
	uint32_t in_num;
	uint32_t out_num;
	uint32_t frame_in_num;
	uint32_t frame_in_size;
	uint32_t write_pos;
	uint32_t read_pos;
	uint32_t write_end_pos;
	uint32_t read_end_pos;
	uint32_t max_pos;
}SectionConfig;

VOID SectionInit(PVOID pSecKMapping);

NTSTATUS SectionCreateIner(
						   PWCH name,
						   SIZE_T nSecSize,
						   HANDLE* hSection,
						   PVOID* ppkoCB,
						   PVOID* pSecKMapping);


NTSTATUS SectionReleaseIner(
							PVOID pkoCB,
							HANDLE hSection,
							PVOID pSecKMapping);


NTSTATUS SectionCtxAttach(
						  PVOID pkoCB,
						  SIZE_T nSecSize,
						  ULONG dwOffset,
						  PVOID* pSecUMMapping);


NTSTATUS SectionCtxDetach(
						  HANDLE pUserProcess_KH,
						  PVOID pSecUMMapping);

NTSTATUS SectionOpen(
	PWCH name,
	SIZE_T nSecSize,
	HANDLE* hSection,
	PVOID* ppkoCB,
	PVOID* pSecKMapping,
	BOOL *open_flag);

VOID SectionClose(HANDLE hSection, PVOID pkoCB, PVOID pSecKMapping);

#endif