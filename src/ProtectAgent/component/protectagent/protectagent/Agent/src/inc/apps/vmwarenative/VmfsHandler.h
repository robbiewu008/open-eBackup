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
#ifndef __VMFSHANDLER_H__
#define __VMFSHANDLER_H__

#include <vector>
#include "common/Types.h"

class VmfsHandler {
public:
    VmfsHandler();
    ~VmfsHandler();

    mp_int32 CheckTool();
    mp_int32 Mount(const std::vector<mp_string> &wwn, mp_string &mountpoint);
    mp_int32 Umount(const mp_string& mountpoint);
};
#endif