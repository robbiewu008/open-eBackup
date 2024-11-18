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
#ifndef LVMREADER_H
#define LVMREADER_H

#include <unistd.h>
#include <stdint.h>
#include "afs/ImgReader.h"
#include "afs/AfsObject.h"
#include "afs/Afslibrary.h"
#include "afs/LogicalVolume.h"

/**
 * @brief image文件的reader类
 */
class lvmReader : public imgReader {
public:
    lvmReader() : imgReader()
    {
        setMagic("lvm2");
        setObjType(OBJ_TYPE_IMGREADER);

        this->m_lowreader = NULL;
        memset_s(&m_part_info, sizeof(m_part_info), 0, sizeof(m_part_info));
        m_lvol = NULL;
        m_stripe_size = 0; // 表示非跨磁盘stripe LV
    }

    lvmReader(afsObject *lowerReader, struct partition *ppart, void *lv_info) : imgReader()
    {
        memcpy_s(&m_part_info, sizeof(m_part_info), ppart, sizeof(m_part_info));
        AFS_TRACE_OUT_DBG("lvmReader m_part_info.part_id = %d, .offset = %llu", m_part_info.part_id,
            m_part_info.offset);
        m_lvol = (logicalVolume *)(lv_info);
        this->m_lowreader = lowerReader;
        m_stripe_size = m_lvol->m_stripe_size; // 表示非跨磁盘stripe LV
    }

    virtual ~lvmReader()
    {
        m_lvol = NULL;
    }

    virtual void setInfo(void *info)
    {
        m_lvol = (logicalVolume *)(info);
    }

    virtual int64_t read(void *buf, int64_t offset, int64_t count, int32_t is_annotated);
    virtual int64_t readDisk(void *buf, int64_t offset, int64_t count);

    virtual int64_t getVaddrToPaddr(int64_t vaddr, int32_t &disk_id);

    int32_t initImageInfo();
    virtual uint32_t getStripeSize()
    {
        return m_stripe_size;
    }

private:
    logicalVolume *m_lvol;
    struct partition m_part_info;
    uint32_t m_stripe_size;
};

#endif
