#pragma once


#ifdef _MSC_VER
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;

#pragma warning(disable : 4200)
#endif

#ifndef VOL_ID_LEN
#define VOL_ID_LEN							(16)
#endif



#define PER_DISK_NAME_LEN							64
#define PER_DESC_OFFSET									512
#define PER_DATA_ALIGN									(4096)
#define PER_HEADER_SECTION_SIZE				(4096)
#define PER_DESCRIPTOR_SECTION_SIZE		(4096)

#define PER_DATA_MAGIC			"HWBTMV2"


#pragma pack (push, 1)
typedef struct tagPER_VOL_BITMAP_INFO
{
	struct
	{
		char disk_name[PER_DISK_NAME_LEN];
		char vol_id[VOL_ID_LEN];
	} vol_info;

	uint32_t logical_offset;
	uint32_t bitmap_size;
	uint64_t bitmap_count;
}PER_VOL_BITMAP_INFO, *PPER_VOL_BITMAP_INFO;

typedef struct tagPERS_DATA_HEADER
{
	char magic[7];
	char start_safe_flag;
	uint32_t reg_select;
	uint32_t desc_offset;
	uint32_t bitmap_offset;
	char reserve[496];
}PERS_DATA_HEADER, *PPERS_DATA_HEADER;

typedef struct tagPERS_VOL_BITMAP_DESC
{
	uint32_t granularity;
	uint32_t disk_count;
	PER_VOL_BITMAP_INFO bitmap_info[0];
}PERS_VOL_BITMAP_DESC, *PPERS_VOL_BITMAP_DESC;
#pragma pack (pop)
