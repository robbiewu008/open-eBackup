/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

#include "apps/vmwarenative/VmfsHandler.h"
#include "common/Defines.h"
#include "common/Log.h"
#include "common/MpString.h"
#include "common/Uuid.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "rootexec/SystemCall.h"

using namespace std;

constexpr mp_int32 MIN_WWN_LEN = 16;

VmfsHandler::VmfsHandler()
{}

VmfsHandler::~VmfsHandler()
{}

mp_int32 VmfsHandler::CheckTool()
{
    CRootCaller rootCaller;
    std::ostringstream scriptParam;
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_VMFS_CHECK_TOOL, scriptParam.str(), NULL);
    if (ret != MP_SUCCESS) {
            ERRLOG("vmfs_tool does not exist!, ret = %d!", ret);
            return MP_FAILED;
        }
    return MP_SUCCESS;
}

mp_int32 VmfsHandler::Mount(const std::vector<mp_string> &wwn, mp_string &mountpoint)
{
    if (wwn.empty()) {
        ERRLOG("wwn size == 0!");
        return MP_FAILED;
    }

    mp_string uuidStr;
    mp_int32 iRet = CUuidNum::GetUuidStandardStr(uuidStr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get taskstep uuid failed, iRet %d.", iRet);
        return iRet;
    }

    mountpoint = VMFS_MOUNT_PATH + uuidStr;
    std::ostringstream scriptParam;
    scriptParam << "mnt_path=" << mountpoint << NODE_COLON;
    for (const auto &iter : wwn) {
        if (iter.length() < MIN_WWN_LEN) {
            COMMLOG(OS_LOG_ERROR, "too short for a WWN number, wwn = %s.", iter.c_str());
            return MP_FAILED;
        }
        scriptParam << "wwn=" << iter << NODE_COLON;
    }
    CRootCaller rootCaller;
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_VMFS_MOUNT, scriptParam.str(), NULL);
    if (ret != MP_SUCCESS) {
        ERRLOG("Mount Filesystem Failed, ret = %d!", ret);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 VmfsHandler::Umount(const mp_string& mountpoint)
{
    if (mountpoint.length() == 0) {
        ERRLOG("mountpoint length invalid!");
        return MP_FAILED;
    }
    CRootCaller rootCaller;
    std::ostringstream scriptParam;
    scriptParam << "mnt_path=" << mountpoint << NODE_COLON;
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_VMFS_UMOUNT, scriptParam.str(), NULL);
    if (ret != MP_SUCCESS) {
        ERRLOG("Umount Filesystem Failed, ret = %d!", ret);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}