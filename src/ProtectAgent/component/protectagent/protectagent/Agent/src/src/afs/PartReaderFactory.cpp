/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file partReaderFactory.cpp
 * @brief AFS - Partition reader information.
 *
 */

#include "afs/PartReaderFactory.h"
#include "afs/AfsError.h"
#include "afs/LogMsg.h"
#include "afs/LVMReader.h"

/**
 * @brief partReaderFactory类析构函数
 */
partReaderFactory::~partReaderFactory()
{
    list<imgReader *>::iterator iter_reader;
    iter_reader = partReaderFactory::m_real_part_reader_list.begin();

    for (; iter_reader != partReaderFactory::m_real_part_reader_list.end(); ++iter_reader) {
        if (NULL != (*iter_reader)) {
            delete (*iter_reader);
            (*iter_reader) = NULL;
        }
    }

    partReaderFactory::m_real_part_reader_list.clear();
}

/**
 * @brief 创建分区Reader
 * @param *imgobj  镜像读取的Reader
 * @param *ppart   分区信息
 * @param *part_other_mode  分区mode
 * @return 无
 */
imgReader *partReaderFactory::createPartReaderOBJ(imgReader *imgobj, struct partition *ppart, void *part_other_mode)
{
    imgReader *pobj = imgobj;

    if (ppart->is_lvm) {
        if (NULL == part_other_mode || NULL == pobj) {
            AFS_TRACE_OUT_ERROR("Failed to create image reader by partition.");
            return NULL;
        }

        lvmReader *reader = new lvmReader(pobj, ppart, part_other_mode);
        if (NULL == reader) {
            AFS_TRACE_OUT_ERROR("Failed to create space.");
            return NULL;
        }

        int ret = reader->initImageInfo();
        if (ret != AFS_SUCCESS) {
            delete reader;
            AFS_TRACE_OUT_ERROR("Failed to init image info.");
            return NULL;
        }

        addPartReader(static_cast<imgReader *>(reader));
        // 如果是lvm
        return reader;
    }

    return pobj;
}

/**
 * @brief 设置分区Reader
 * @param *imgobj  镜像读取的Reader
 * @return 无
 */
void partReaderFactory::addPartReader(imgReader *imgobj)
{
    imgReader *tmp_imgobj = imgobj;

    if (NULL != tmp_imgobj) {
        m_real_part_reader_list.push_back(tmp_imgobj);
    }
}
