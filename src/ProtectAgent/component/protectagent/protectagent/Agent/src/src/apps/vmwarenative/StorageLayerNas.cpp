/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
