/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 8/3/2022
 */

#ifndef FS_SCANNER_OPENDIR_RES_DATA_H
#define FS_SCANNER_OPENDIR_RES_DATA_H
#include "NFSSyncCbData.h"

class OpendirResData {
public:
    bool m_isEmpty = true;
    struct nfsdir *m_dir = nullptr;
    struct NFSSyncCbData *m_cbData = nullptr;
    struct opendir_cb_data *m_opendirData = nullptr;
};
#endif