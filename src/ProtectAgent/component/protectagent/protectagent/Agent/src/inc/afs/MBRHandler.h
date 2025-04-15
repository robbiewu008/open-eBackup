/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef __AFS_MBRHANDLER_H__
#define __AFS_MBRHANDLER_H__

#include <queue>
#include "afs/PartitionHandler.h"

/**
 * @brief MBR分区表格式
 *
 */
struct dos_partition {
    unsigned char boot_ind;   /* 0x80 - active */
    unsigned char bh, bs, bc; /* begin CHS */
    unsigned char sys_ind;
    unsigned char eh, es, ec; /* end CHS */
    unsigned char start_sect[4];
    unsigned char nr_sects[4];
} __attribute__((packed));

#define MBR_PT_OFFSET 0x1be
#define MBR_PT_BOOTBITS_SIZE 440
#define MBR_SWAP_PARTITION 0x82
#define MBR_EXTEND_PARTITION_1 0x05
#define MBR_EXTEND_PARTITION_2 0x0F
#define MBR_UNUSE_PARTITION_ENTRY 0x0


/**
 * @brief 处理MBR分区的类
 */
class MBRHandler : public partitionHandler {
public:
    MBRHandler(int32_t type) : partitionHandler()
    {
        setObjType(OBJ_TYPE_PARTITION);
        setMagic("mbr");
        settype(PARTITION_MBR);
        m_extent_lba = 0;
        m_partitionflags = type;
    }

    MBRHandler() : partitionHandler()
    {
        setObjType(OBJ_TYPE_PARTITION);
        setMagic("mbr");
        settype(PARTITION_MBR);
        m_extent_lba = 0;
        m_partitionflags = 0;
    }

    ~MBRHandler() {}

    int32_t analyzeExtent();
    int32_t analyzePrimary();
    int32_t getBitmap(BitMap &bitmap);
    int32_t partitionEntryHandle(dos_partition *pMbr_partition, const char entry_index);
    uint64_t getExtentLba();

    void setExtentLba(uint64_t extent_lba);
    virtual int32_t parseAllOfPart();

    static inline struct dos_partition *mbr_get_partition(unsigned char *mbr, int32_t i)
    {
        return (struct dos_partition *)(mbr + MBR_PT_OFFSET + (i * sizeof(struct dos_partition)));
    }

    static inline uint32_t __dos_assemble_4le(const unsigned char *p)
    {
        return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    }

    static inline uint32_t dos_partition_get_start(struct dos_partition *p)
    {
        return __dos_assemble_4le(&(p->start_sect[0]));
    }

    static inline uint32_t dos_partition_get_size(struct dos_partition *p)
    {
        return __dos_assemble_4le(&(p->nr_sects[0]));
    }

    static inline int32_t mbr_is_valid_magic(const unsigned char *mbr)
    {
        return mbr[510] == 0x55 && mbr[511] == 0xaa ? 1 : 0;
    }

    static inline int32_t mbr_no_part_magic(const unsigned char *mbr)
    {
        return mbr[510] == 0 && mbr[511] == 0 ? 1 : 0;
    }

    vector<uint64_t> m_extentSect;

private:
    int32_t nonePartTableHandle();
    int32_t primaryPartHandle(unsigned char *data_buf);
    int32_t analyzeExtentParsePart(unsigned char *buf, queue<uint64_t> &que_extent, uint64_t extentBase,
        uint64_t extentL1, int32_t real_part_index);
    int32_t analyzeExtentSetSinglePart(struct partition &part, int32_t real_partNum, int32_t partNum);

    int32_t m_partitionflags;
    uint64_t m_extent_lba;
};
#endif