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
#ifndef HYPERV_GET_VM_VOLUMES_MODEL_H
#define HYPERV_GET_VM_VOLUMES_MODEL_H
#include "common/MpString.h"
#include "common/JsonHelper.h"

namespace HyperVPlugin {
class VolumeInfo {
public:
    std::string m_diskIdentifier;
    std::string m_fileSize;
    std::string m_size;
    std::string m_vhdFormat;
    std::string m_vhdType;
    std::string m_path;
    std::string m_parentPath;
    std::string m_controllerUUID;
    uint64_t m_controllerIndex;
    uint64_t m_controllerPos = 0;
    uint64_t m_controllerType = 0;
    std::string m_vmId;
    std::string m_needAttension;
    bool m_shared;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskIdentifier, DiskIdentifier)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fileSize, FileSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, Size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vhdFormat, VhdFormat)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vhdType, VhdType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_path, Path)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_parentPath, ParentPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_controllerUUID, ControllerUUID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_controllerIndex, ControllerIndex)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_controllerPos, ControllerPos)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_controllerType, ControllerType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmId, VMId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_needAttension, NeedAttension)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shared, SupportPersistentReservations)
    END_SERIAL_MEMEBER
};

// PM will convert all value in json to string type, so all member var of VolumeExtendInfo are string type
class VolumeExtendInfo {
public:
    std::string m_diskIdentifier;
    std::string m_size;
    std::string m_vhdType;
    std::string m_vhdFormat;
    std::string m_path;
    std::string m_parentPath;
    std::string m_controllerUUID;
    std::string m_controllerIndex;
    std::string m_controllerPos;
    std::string m_controllerType;
    std::string m_vmId;
    std::string m_needAttension;
    bool m_shared;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskIdentifier, DiskIdentifier)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, Capacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vhdType, Type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vhdFormat, Format)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_path, Path)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_parentPath, ParentPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_controllerUUID, ControllerUUID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_controllerIndex, ControllerIndex)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_controllerPos, ControllerPos)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_controllerType, ControllerType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmId, VMId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_needAttension, NeedAttension)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shared, IsShared)
    END_SERIAL_MEMEBER
    VolumeExtendInfo() {}
    VolumeExtendInfo(const VolumeInfo &hyperVDiskInfo)
    {
        m_diskIdentifier = hyperVDiskInfo.m_diskIdentifier;
        m_size = hyperVDiskInfo.m_size;
        m_vhdType = hyperVDiskInfo.m_vhdType;
        m_vhdFormat = hyperVDiskInfo.m_vhdFormat;
        m_path = Module::CMpString::StrReplace(hyperVDiskInfo.m_path, "\\", "/");
        m_parentPath = hyperVDiskInfo.m_parentPath;
        m_controllerUUID = hyperVDiskInfo.m_controllerUUID;
        m_controllerIndex = std::to_string(hyperVDiskInfo.m_controllerIndex);
        m_controllerPos = std::to_string(hyperVDiskInfo.m_controllerPos);
        m_controllerType = std::to_string(hyperVDiskInfo.m_controllerType);
        m_vmId = hyperVDiskInfo.m_vmId;
        m_needAttension = hyperVDiskInfo.m_needAttension;
        m_shared = hyperVDiskInfo.m_shared;
    }
};

class GetVMVolumesResponse {
public:
    std::list<VolumeInfo> m_diskList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskList, result)
    END_SERIAL_MEMEBER
};

class GetVMVolumesRequest {
public:
    std::string m_vmName;
    std::string m_vmId;
    std::string m_endPoint;
    std::string m_authKey;
    std::string m_authPwd;
};
}
#endif