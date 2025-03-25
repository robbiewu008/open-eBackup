/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file partReaderFactory.h
 * @brief AFS - Partition reader information.
 *
 */

#ifndef IMAGE_PARTREADERFACTORY_H_
#define IMAGE_PARTREADERFACTORY_H_
#include <list>
#include "afs/ImgReader.h"

class partReaderFactory {
public:
    partReaderFactory() {}
    ~partReaderFactory();

    imgReader *createPartReaderOBJ(imgReader *imgobj, struct partition *ppart, void *part_other_mode);

private:
    // 空间管理
    list<imgReader *> m_real_part_reader_list;

    void addPartReader(imgReader *imgobj);
};

#endif /* IMAGE_PARTREADERFACTORY_H_ */
