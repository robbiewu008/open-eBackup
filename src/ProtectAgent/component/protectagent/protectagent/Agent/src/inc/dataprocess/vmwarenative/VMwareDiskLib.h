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
#ifndef __AGENT_VMWARE_DISK_LIB_H__
#define __AGENT_VMWARE_DISK_LIB_H__

#include <memory>
#include <mutex>
#include <unordered_map>
#include "common/Types.h"
#include "Define.h"
#include "VMwareDiskApiDefine.h"
#include "dataprocess/vmwarenative/MessageLoopThread.h"
#include "dataprocess/vmwarenative/VddkDeadlockCheck.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "message/curlclient/CurlHttpClient.h"

/*
 * This class provides VDDK lib loading, initialzation, ...
 */

class VMwareDiskApi;

class VMwareDiskLib {
public:
    static VMwareDiskLib *GetInstance();
    static void DestroyInstance();

    // Build vCenter or ESXI conection params
    virtual uint64_t BuildConnectParams(
        const std::string &vmRef, const vmware_pm_info &pmInfo, VddkConnectParams &connectParams);
    // Create a VMwareDiskLib instance
    virtual std::shared_ptr<VMwareDiskApi> GetVMwareDiskApiInstance(const VddkConnectParams &params);

    virtual uint64_t Cleanup(const VddkConnectParams &connectParams, std::string &errDesc);
    virtual uint64_t PrepareForAccess(const VddkConnectParams &connectParams, std::string &errDesc);
    virtual uint64_t EndAccess(const VddkConnectParams &connectParams, std::string &errDesc);
    virtual std::string ListTransportModes();
    bool Init();
    bool InitExtractFuction();
    VMWARE_DISK_RET_CODE Uninit(const VddkConnectParams &connectParams, std::string &errDesc);
    std::string GetVddkLibPathInner();
    void SetVddkLibPathAndVersion(const mp_string &path, uint32_t majorVersion, uint32_t minorVersion,
        const mp_string &version);

private:
    VMwareDiskLib(const VMwareDiskLib &VMwareDiskLib);
    VMwareDiskLib &operator=(const VMwareDiskLib &VMwareDiskLib);
    VMwareDiskLib();
    virtual ~VMwareDiskLib();

    bool StartVddkThread();
    bool LoadVddkLibFile();
    bool InitVddkLibEnv();
    void GetVddkLibPath(std::string &libPath, std::string &configFilePath);
    std::string GetErrString(const uint64_t code);
    static void LogFunc(const char *fmt, va_list args);
    static void WarnFunc(const char *fmt, va_list args);
    static void PanicFunc(const char *fmt, va_list args);
    static std::string TrimChars(const std::string &str);
    VMWARE_DISK_RET_CODE Connect(const VddkConnectParams &connectParams, VixDiskLibConnection &connection);
    VMWARE_DISK_RET_CODE DisConnect(const VixDiskLibConnection &connection);

    void GetVixConnectParams(const VddkConnectParams &connectParams, VixDiskLibConnectParams &params);

private:
    static std::mutex m_mutex;
    static mp_string VDDK_CONFI_FILE_NAME;
    static mp_string VIX_DISK_LIBRARY_NAME;
    static mp_string APPLICATION_NAME;
    bool m_isInitialized;
    void *m_vddkLibHandle;
    VMwareDiskOperations m_vddkOperations;
    MessageLoopThread m_vddkThread;
    std::string m_strLibPath;
    mp_double m_version;
    uint32_t m_majorVersion;
    uint32_t m_minorVersion;
    // map of vm and its vCenter/ESXI
    std::unordered_map<std::string, VixDiskLibConnection> m_connMap;
};
#endif
