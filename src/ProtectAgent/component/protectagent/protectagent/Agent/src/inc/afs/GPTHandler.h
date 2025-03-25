/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file GPTHandler.h
 * @brief AFS - Analyze GPT partition.
 *
 */
#ifndef __AFS_PARTITION_H__
#define __AFS_PARTITION_H__

#include "afs/PartitionHandler.h"

typedef uint16_t efi_char16_t;

typedef struct {
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_hi_and_version;
    uint8_t clock_seq_hi;
    uint8_t clock_seq_low;
    uint8_t node[6];
} efi_guid_t;

struct gpt_header {
    uint64_t signature; /* "EFI PART" */
    uint32_t revision;
    uint32_t header_size;  /* usually 92 bytes */
    uint32_t header_crc32; /* checksum of header with this
                            * field zeroed during calculation */
    uint32_t reserved1;

    uint64_t my_lba;           /* location of this header copy */
    uint64_t alternate_lba;    /* location of the other header copy */
    uint64_t first_usable_lba; /* first usable LBA for partitions */
    uint64_t last_usable_lba;  /* last usable LBA for partitions */

    efi_guid_t disk_guid; /* disk UUID */

    uint64_t partition_entries_lba; /* always 2 in primary header copy */
    uint32_t num_partition_entries;
    uint32_t sizeof_partition_entry;
    uint32_t partition_entry_array_crc32;

    /*
     * The rest of the block is reserved by UEFI and must be zero. EFI
     * standard handles this by:
     *
     * uint8_t        reserved2[ BLKSSZGET - 92 ];
     *
     * This definition is useless in practice. It is necessary to read
     * whole block from the device rather than sizeof(struct gpt_header)
     * only.
     */
} __attribute__((packed));

struct gpt_entry {
    efi_guid_t partition_type_guid;   /* type UUID */
    efi_guid_t unique_partition_guid; /* partition UUID */
    uint64_t starting_lba;
    uint64_t ending_lba;

    /* struct gpt_entry_attributes    attributes; */

    uint64_t attributes;

    efi_char16_t partition_name[72 / sizeof(efi_char16_t)]; /* UTF-16LE string */
} __attribute__((packed));

/**
 * @brief 处理GPT分区的类
 */
class GPTHandler : public partitionHandler {
public:
    GPTHandler()
    {
        setObjType(OBJ_TYPE_PARTITION);
        settype(PARTITION_GPT);
        setMagic("gpt");
    }

    ~GPTHandler() {}

    virtual int32_t parseAllOfPart();
    int32_t getBitmap(BitMap &bitmap);

private:
    int32_t parseSingleEntry(uint8_t *part_table_data, uint32_t part_table_num);
    int32_t parseSingleEntryDoLVM(int32_t real_part_num, int32_t part_num);
};
#endif /* __AFS_PARTITION_H__ */