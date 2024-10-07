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
#ifndef CERT_MGR_H
#define CERT_MGR_H

#include <string>
#include <cstdio>
#include "common/Structs.h"
#include "common/CleanMemPwd.h"
#include "repository_handlers/filesystem/FileSystemHandler.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

class CertInfo {
public:
    ~CertInfo()
    {
        Module::CleanMemoryPwd(m_vdcInfoStr);
        Module::CleanMemoryPwd(m_storages);
        Module::FreeContainer(m_certification);
        Module::FreeContainer(m_revocationList);
    }

    std::string m_vdcInfoStr;
    std::string m_certification;
    std::string m_revocationList;
    std::string m_storages;
    std::string m_enableCert = "0";

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vdcInfoStr, vdcInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_certification, certification)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storages, storages)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_revocationList, revocationList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enableCert, enableCert)
    END_SERIAL_MEMEBER
};

class CpsCertInfo {
public:
    ~CpsCertInfo()
    {
        Module::FreeContainer(m_certification);
        Module::FreeContainer(m_revocationList);
    }

    std::string m_certification;
    std::string m_revocationList;
    std::string m_enableCert = "0";

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_certification, cpsCertification)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_revocationList, cpsRevocationList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enableCert, enableCert)
    END_SERIAL_MEMEBER
};

class CertManger {
public:
    CertManger() : m_fHandler(std::make_shared<VirtPlugin::FileSystemHandler>()) {};
    ~CertManger(void);

    void RemoveCrtFile(const std::string& markId);
    void RemoveCrlFile(const std::string& markId);
    std::string GetSpecifyCerPath(const std::string &markId);
    std::string GetSpecifyRclPath(const std::string &markId);

    std::string GetCPSCertPath();
    std::string GetRevocationList(void) const;
    bool IsVerifyCert(void) const;

    template<typename T>
    bool ParseCertInfo(const std::string& certInfo, T &cert);

    std::string GetCertPath() const;
    std::string GetRevocationListPath() const;
    bool SaveCertToFile(const std::string& fileName = "CA");
    void ClearResource();
    template<typename T>
    bool ParseCert(const std::string &markId, const std::string &certInfo, T &cert);
protected:
    bool SaveDataToTempFile(const std::string& filePath, const std::string& data);
    bool RemoveTempFile(const std::string& filePath);

protected:
    std::string m_certification;
    std::string m_revocationList;
    bool m_enableCert {false};

    std::string m_certPath;
    std::string m_crlPath;
    bool m_needSaveToFile {false};
    std::shared_ptr<VirtPlugin::RepositoryHandler> m_fHandler;
};

VIRT_PLUGIN_NAMESPACE_END

#endif // CERT_MGR_H