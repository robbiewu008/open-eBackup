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
#include "log/Log.h"
#include "common/Constants.h"
#include "common/utils/Utils.h"
#include "repository_handlers/filesystem/FileSystemHandler.h"

#include "CertMgr.h"

using namespace VirtPlugin;

namespace {
    const std::string CINDER_CERT_DIR = VIRT_PLUGIN_PATH + "/cert/cinder/";
    const std::string TEMP_CERT_DIR = VIRT_PLUGIN_PATH + "/tmp/cert";
    const std::string CINDER_CERT_FILE = "cinder.pem";
    static std::mutex certMutex;
}

VIRT_PLUGIN_NAMESPACE_BEGIN

CertManger::~CertManger(void) {}

std::string CertManger::GetSpecifyCerPath(const std::string &markId)
{
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string path = agentHomedir + TEMP_CERT_DIR + "/" + markId + ".pem";
    if (!m_fHandler->Exists(path)) {
        ERRLOG("File(%s) does not exist.", path.c_str());
        return "";
    }
    return path;
}

std::string CertManger::GetSpecifyRclPath(const std::string &markId)
{
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string path = agentHomedir + TEMP_CERT_DIR + "/" + markId + ".crl";
    if (!m_fHandler->Exists(path)) {
        DBGLOG("File(%s) does not exist.", path.c_str());
        return "";
    }
    return path;
}

/* ------------------------------------------------------------
Description  : 获取证书
Return       : 证书
Create By    : shengkun 30034564
------------------------------------------------------------- */
std::string CertManger::GetCPSCertPath()
{
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string cpsCertPath = agentHomedir + CINDER_CERT_DIR + CINDER_CERT_FILE;
    if (!m_enableCert) {
        DBGLOG("Cert is disabled.");
        return "";
    }
    if (m_enableCert && !m_fHandler->Exists(cpsCertPath)) {
        ERRLOG("Cps cert(%s) does not exists, please import first.", cpsCertPath.c_str());
        return "";
    }
    return agentHomedir + CINDER_CERT_DIR + CINDER_CERT_FILE;
}

/* ------------------------------------------------------------
Description  : 获取证书吊销列表
Return       : 证书吊销列表
Create By    : shengkun 30034564
------------------------------------------------------------- */
std::string CertManger::GetRevocationList(void) const
{
    return m_revocationList;
}

/* ------------------------------------------------------------
Description  : 是否证书认证
Return       : 是否证书认证
Create By    : shengkun 30034564
------------------------------------------------------------- */
bool CertManger::IsVerifyCert(void) const
{
    return m_enableCert;
}

std::string CertManger::GetCertPath() const
{
    return m_certPath;
}

std::string CertManger::GetRevocationListPath() const
{
    return m_crlPath;
}

bool CertManger::SaveCertToFile(const std::string& fileName)
{
    m_needSaveToFile = true;
    
    if (m_fHandler == nullptr) {
        ERRLOG("Get file handler fail.");
        return false;
    }
    std::string tmpCertDir = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + TEMP_CERT_DIR;
    if (m_fHandler->IsDirectory(tmpCertDir)) {
        DBGLOG("dir(%s) already exist.", tmpCertDir.c_str());
    } else {
        if (!m_fHandler->CreateDirectory(tmpCertDir)) {
            ERRLOG("Create dir(%s) fail.", tmpCertDir.c_str());
            return false;
        }
        INFOLOG("Create dir(%s) success.", tmpCertDir.c_str());
    }

    if (fileName.find("../") != std::string::npos) {
        ERRLOG("Invalid file name.");
        return false;
    }

    std::string content;
    int ret;
    if (!m_certification.empty()) {
        m_certPath = tmpCertDir + "/" + fileName + ".pem";
        ret = Utils::ReadFile(m_fHandler, m_certPath, content);
        if (ret == SUCCESS && content != m_certification) {
            if (!SaveDataToTempFile(m_certPath, m_certification)) {
                ERRLOG("Save cert data to file(%s) fail.", m_certPath.c_str());
                return false;
            }
        }
    }
    if (!m_revocationList.empty()) {
        m_crlPath = tmpCertDir + "/" + fileName + ".crl";
        content = "";
        ret = Utils::ReadFile(m_fHandler, m_crlPath, content);
        if (ret == SUCCESS && content != m_revocationList) {
            if (!SaveDataToTempFile(m_crlPath, m_revocationList)) {
                ERRLOG("Save crl data to file(%s) fail.", m_crlPath.c_str());
                return false;
            }
        } else {
            WARNLOG("No need to update file(%s).", m_crlPath.c_str());
        }
    } else {
        RemoveCrlFile(fileName);
    }
    return true;
}

void CertManger::ClearResource()
{
    if (m_needSaveToFile) {
        if (!m_certPath.empty()) {
            RemoveTempFile(m_certPath);
        }
        if (!m_crlPath.empty()) {
            RemoveTempFile(m_crlPath);
        }
    }
}

bool CertManger::SaveDataToTempFile(const std::string& filePath, const std::string& data)
{
    std::lock_guard<std::mutex> lock(certMutex);
    if (m_fHandler == nullptr) {
        ERRLOG("Get file handler failed.");
        return false;
    }

    if (m_fHandler->Open(filePath, "w") != VirtPlugin::SUCCESS) {
        ERRLOG("Open file(%s) fail.", filePath.c_str());
        return false;
    }

    VirtPlugin::Utils::Defer _(nullptr, [&](...) {
        if (m_fHandler != nullptr) {
            m_fHandler->Close();
        }
    });

    size_t writeCount = m_fHandler->Write(data);
    if (writeCount != data.size()) {
        ERRLOG("Write file(%s) failed. Size return: %zu, read expect: %zu", filePath.c_str(), writeCount, data.size());
        return false;
    }

    INFOLOG("Save data to file(%s) success.", filePath.c_str());
    return true;
}

bool CertManger::RemoveTempFile(const std::string& filePath)
{
    if (m_fHandler == nullptr) {
        ERRLOG("Get file(%s) handler failed.", filePath.c_str());
        return false;
    }
    if (!m_fHandler->Remove(filePath)) {
        ERRLOG("Remove file(%s) fail.", filePath.c_str());
        return false;
    }
    INFOLOG("Remove file(%s) success.", filePath.c_str());
    return true;
}

void CertManger::RemoveCrtFile(const std::string& markId)
{
    if (markId.find("../") != std::string::npos) {
        ERRLOG("Invalid file name.");
        return;
    }
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string path = agentHomedir + TEMP_CERT_DIR + "/" + markId + ".pem";
    if (m_fHandler->Exists(path)) {
        m_fHandler->Remove(path);
    }
}

void CertManger::RemoveCrlFile(const std::string& markId)
{
    if (markId.find("../") != std::string::npos) {
        ERRLOG("Invalid file name.");
        return;
    }
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string path = agentHomedir + TEMP_CERT_DIR + "/" + markId + ".crl";
    if (m_fHandler->Exists(path)) {
        m_fHandler->Remove(path);
    }
}

/* ------------------------------------------------------------
Description  : 解析证书内容
Input        : certInfo -- 证书内容
Return       : true -- 成功
               false -- 失败
Create By    : shengkun 30034564
------------------------------------------------------------- */

template<typename T>
bool CertManger::ParseCertInfo(const std::string &certInfo, T &cert)
{
    m_enableCert = false;
    if (certInfo.empty()) {
        ERRLOG("CertInfo is empty.");
        return false;
    }

    if (!Module::JsonHelper::JsonStringToStruct(certInfo, cert)) {
        ERRLOG("Failed to parse certInfo.");
        return false;
    }
    
    m_enableCert = (cert.m_enableCert == "0") ? false : true;

    m_certification = cert.m_certification;
    m_revocationList = cert.m_revocationList;
    return true;
}
template bool CertManger::ParseCertInfo<CertInfo>(const std::string &certInfo, CertInfo &cert);
template bool CertManger::ParseCertInfo<CpsCertInfo>(const std::string &certInfo, CpsCertInfo &cert);

template<typename T>
bool CertManger::ParseCert(const std::string &markId, const std::string &certInfo, T &cert)
{
    if (!ParseCertInfo(certInfo, cert)) {
        ERRLOG("Failed to parse storage certinfo.");
        return false;
    }
    if (IsVerifyCert()) {
        if (!SaveCertToFile(markId)) {
            ERRLOG("Save storage cert mgr cert info to file failed.");
            return false;
        }
    } else {
        RemoveCrtFile(markId);
        RemoveCrlFile(markId);
    }
    return true;
}
template bool CertManger::ParseCert<CertInfo>(const std::string &markId, const std::string &certInfo,
    CertInfo &cert);
template bool CertManger::ParseCert<CpsCertInfo>(const std::string &markId, const std::string &certInfo,
    CpsCertInfo &cert);

VIRT_PLUGIN_NAMESPACE_END
