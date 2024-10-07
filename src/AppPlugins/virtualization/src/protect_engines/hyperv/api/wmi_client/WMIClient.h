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
#ifdef WIN32
#ifndef WMI_CLIENT_H
#define WMI_CLIENT_H

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atlcomcli.h>
#include <comdef.h>
#include <string>
#include <vector>
#include <iostream>
#include <Wbemidl.h>
#include "MpString.h"
#include <common/JsonHelper.h>
#include "protect_engines/hyperv/api/powershell/model/GetVMVolumesModel.h"
#include "volume_handlers/hyperv_volume/HyperVVolumeHandler.h"
#pragma comment(lib, "wbemuuid.lib")

using WmiMethodParamType = long;
enum class HyperVControllerType {
    IDE = 5,
    SCSI = 6
};

enum class HyperVResourceType {
    DISK_DRIVE = 17,
    LOGICAL_DISK = 31
};

class WmiMethodParam {
public:
    LPCWSTR name;
    WmiMethodParamType type;
    VARIANT value;
    WmiMethodParam(LPCWSTR name, WmiMethodParamType type, VARIANT value) : name(name), type(type), value(value) {}
};

class WmiMethodExecuteParam {
public:
    std::string methodOwnerClass;
    std::string methodName;
    std::vector<WmiMethodParam> inputeParams;
    std::string errDetails;
    WmiMethodExecuteParam(std::string methodOwnerClass, std::string methodName)
        : methodOwnerClass(methodOwnerClass), methodName(methodName)
    {}
};

class VmModifyParam {
public:
    std::string configFileSrcPath;
    std::string configFileTgtPath;
    bool disableNetwork;
    std::string newVMName;
    std::string snapshotDataRoot;
    std::string swapFileDataRoot;
    VmModifyParam() {}
    VmModifyParam(std::string configFileSrcPath, std::string configFileTgtPath, bool disableNetwork,
        std::string newVMName)
        : configFileSrcPath(configFileSrcPath),
          configFileTgtPath(configFileTgtPath),
          disableNetwork(disableNetwork),
          newVMName(newVMName),
          snapshotDataRoot(configFileTgtPath),
          swapFileDataRoot(configFileTgtPath)
    {}
};

constexpr int32_t DISABLE_CONNECT = 3;

class WMIClient {
public:
    ~WMIClient();
    static std::shared_ptr<WMIClient> GetInstance();
    /* *
     * @brief 初始化wmi
     *
     * @return 错误码：0 成功, -1  失败
     */
    int32_t Init();

    /* *
     * @brief 挂载磁盘
     *
     * @param diskInfo   [IN]磁盘信息
     * @param vmId [IN]虚拟机Id
     * @return 错误码：0 成功, -1  失败
     */
    int32_t AttachDiskToVM(const HyperVPlugin::VolumeExtendInfo &diskInfo, const std::string &vmId);

    /* *
     * @brief 创建虚拟机
     *
     * @param modifyParam   [IN]虚拟机配置信息
     * @param createdVmId [OUT]创建的虚拟机Id
     * @param errorDetails [OUT]错误详情
     * @return 错误码：0 成功, -1  失败
     */
    int32_t CreateVM(const VmModifyParam &modifyParam, std::string &createdVmId, std::string &errorDetails);

    /* *
     * @brief 删除虚拟机
     *
     * @param vmId   [IN]虚拟机Id
     * @return 错误码：0 成功，非0 失败
     */
    int32_t DeleteVM(const std::string &vmId);

    /* *
     * @brief 删除快照
     *
     * @param vmId   [IN]虚拟机Id
     * @return 错误码：0 成功，非0 失败
     */
    int32_t DeleteSnapshot(const std::string &vmId);

    /* *
     * @brief 卸载磁盘
     *
     * @param vmId   [IN]虚拟机Id
     * @param diskInfos [IN]磁盘信息
     * @return 错误码：0 成功，非0 失败
     */
    int32_t DetachDiskFromVM(const std::string &vmId, const std::vector<HyperVPlugin::VolumeExtendInfo> &diskInfos);

    /* *
     * @brief 禁用网络
     *
     * @param vmId   [IN]虚拟机Id
     * @return 错误码：0 成功，非0 失败
     */
    int32_t DisableVMNetwork(const std::string &vmId, const bool disableNetwork);

    /* *
     * @brief 查询磁盘信息
     *
     * @param vmId   [IN]虚拟机Id
     * @param diskInfos [OUT]磁盘信息
     * @return 错误码：0 成功，非0 失败
     */
    int32_t QueryVMDiskList(const std::string &vmId, std::vector<HyperVPlugin::VolumeExtendInfo> &diskInfos);

    /* *
     * @brief 实现或验证计划虚拟机
     *
     * @param vmId   [IN]计划虚拟机Id（创建出的虚拟机都是计划虚拟机，还需要调用此方法进行实现）
     * @param isRealize   [IN]是否实现 true：实现， false：验证能否实现
     * @param errorDetails [OUT]错误信息
     * @return 错误码：0 成功，非0 失败
     */
    int32_t RealizeOrValidatePlannedVM(const std::string &vmId, bool isRealize, std::string &errorDetails);

    /* *
     * @brief 创建逻辑磁盘或磁盘驱动
     *
     * @param diskInfo   [IN]磁盘信息
     * @param vmId [IN]虚拟机Id
     * @param diskDrive   [IN]true: 创建驱动   false: 创建逻辑磁盘
     * @param createdResourcePath [OUT]创建的资源路径
     * @return 错误码：0 成功，非0 失败
     */
    int32_t CreateLogicDiskOrDiskDrive(const HyperVPlugin::VolumeExtendInfo &diskInfo, const std::string &vmId,
                                       const bool diskDrive, BSTR &createdResourcePath);

    /* *
     * @brief 变更虚拟机电源状态
     *
     * @param vmId   [IN]虚拟机Id
     * @param powerOff [IN]true:关机   false:开机
     * @return 错误码：0 成功，非0 失败
     */
    int32_t ChangeVMPowerState(const std::string &vmId, bool powerOff);
private:
    int32_t InitLocate();
    std::string ConvertSlash(const std::string& input);
    int32_t PrepareModifyParam(const VmModifyParam &param, IWbemClassObject *pVirtualSystemSettingDataInstance);
    int32_t ModifyVM(const VmModifyParam &param, const std::string &vmId, std::string &errorDetails);
    int32_t SetNetwork(IWbemClassObject *&netSetting, VARIANT &varArray, bool &isDisableNet);
    int32_t PrepareDiskFromVM(const std::string &vmId, const std::vector<HyperVPlugin::VolumeExtendInfo> &diskInfos,
        std::vector<IWbemClassObject *> &pLogicDisks, std::vector<IWbemClassObject *> &pDrives);
    int32_t SpiltVMDisk(IWbemClassObject *classObj, HyperVPlugin::VolumeExtendInfo &diskInfo);
    int32_t PrepareDiskDriver(const HyperVPlugin::VolumeExtendInfo &diskInfo, IWbemClassObject *controller,
                              const bool diskDrive, IWbemClassObject *pDiskDriver);
    int32_t PrepareVMSettings(const std::string &vmId, VARIANT &vmSettingPath);
    int32_t AddLogicDiskOrDiskDrive(const VARIANT &vmSettingPath, const VARIANT &resourceSettingsArray,
        BSTR &createdResourcePath);
    int32_t ExecMethodPrepare(WmiMethodExecuteParam &param, IWbemClassObject *&pMethodInstance,
        VARIANT &classInstancePath);
    int32_t ExecMethod(WmiMethodExecuteParam &param, CComPtr<IWbemClassObject> &pOutParam);
    int32_t ExecMethodOnSpecifiedObject(WmiMethodExecuteParam &param, CComPtr<IWbemClassObject> &pOutParam,
        const VARIANT& obj);
    int32_t CheckAsynchJobResult(VARIANT &jobPath, std::string &errorDetail);
    int32_t CheckExecuteMethodResult(CComPtr<IWbemClassObject> pOutParam, CComPtr<IWbemCallResult> pCallResult,
        std::string &errorDetail);
    int32_t ExecQuery(std::vector<IWbemClassObject *> &result, const std::string queryCmd, bool returnFindingOne);
    int32_t GetWMIProxyClassObj(IWbemClassObject *&instPtr, const std::string &className);
    int32_t GetMethodParamCollector(IWbemClassObject *&instPtr, const std::string &className,
        const std::string &methodName);
    int32_t AddParamsToParamCollector(std::vector<WmiMethodParam> &params, IWbemClassObject *paramCollector);
    int32_t GetObjectTextInSpecifiedFormat(int32_t format, IWbemClassObject *obj, std::string &resultString);
    int32_t GetObjectAttributeInBSTR(IWbemClassObject *obj, const std::string &key, VARIANT &attributeValue);
    int32_t GetObjectAttributeInBSTRARRAY(IWbemClassObject *obj, const std::string &key, std::vector<BSTR> &result);
    int32_t GetBSTRArray(const std::vector<std::string> &input, VARIANT &result);
    int32_t RetryToQueryControler(const HyperVPlugin::VolumeExtendInfo& diskInfo, const std::string& vmId,
        std::vector<IWbemClassObject *> &controllers);
    VARIANT String2BSTRVariant(const std::string& strValue);
    VARIANT Bool2BoolVariant(const bool b);
    void PrepareDriver(std::vector<WmiMethodParam> &params, const VARIANT &controller_path,
        const HyperVPlugin::VolumeExtendInfo &diskInfo);
    IWbemServices *m_pSvc = nullptr;
    IWbemClassObject *m_pVirutalSysMgrService = nullptr;
    IWbemClassObject *m_pStorageAllocationSettingData = nullptr;
    int64_t m_maxTimeOut = WBEM_INFINITE;
    BSTR m_DiskDrivePath;
    BSTR m_LogicDiskPath;
    IWbemLocator *m_pLoc = nullptr;
};
#endif
#endif