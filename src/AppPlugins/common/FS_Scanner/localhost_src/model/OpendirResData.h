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