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
#include "apps/vmwarenative/StorageLayerNas.h"
#include "common/Defines.h"
#include "common/Log.h"
#include "common/MpString.h"
#include "common/Uuid.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "rootexec/SystemCall.h"

mp_int32 StorageLayerNas::NasMount(const std::ostringstream &cmdParam)
{
    INFOLOG("Enter NasMount.");
#ifndef WIN32
    CRootCaller rootCaller;
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PREPARE_NASMEDIA, cmdParam.str(), NULL);
    if (ret != MP_SUCCESS) {
        WARNLOG("mount nas failed!");
        return MP_ERROR;
    }
    INFOLOG("mount nas success!");
#endif
    return MP_SUCCESS;
}

mp_int32 StorageLayerNas::NasUnMount(const std::ostringstream &cmdParam)
{
    INFOLOG("Enter NasUnMount.");
#ifndef WIN32
    CRootCaller rootCaller;
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_UMOUNT_NASMEDIA, cmdParam.str(), NULL);
    if (ret != MP_SUCCESS) {
        ERRLOG("umount failed!");
        return MP_ERROR;
    }
#endif
    return MP_SUCCESS;
}
