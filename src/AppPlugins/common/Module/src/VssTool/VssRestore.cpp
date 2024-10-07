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
#include "VssRestore.h"

#include <map>
#include <memory.h>
#include <cstdio>
#include <memory>
#include <comutil.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <atlstr.h>
#include "log/Log.h"

using namespace std;

namespace {
const int POSTFIX_POS = 4;
const int WILDCARD_POS = 5;
const int NUMBER3 = 3;
const std::string BCD_FILENAME = "BCD.xml";
const std::string WMD_FILENAME = "WMD.xml";
std::string g_targetDataPath = "";
std::string g_targetLogPath = "";
}

VssRestore::VssRestore() {}

VssRestore::~VssRestore() {}

bool VssRestore::Restore(const RestoreParam& param)
{
    std::string abusoluteBcdPath = param.metaPath + "\\" + BCD_FILENAME;
    std::string abusoluteWmdPath = param.metaPath + "\\" + WMD_FILENAME;
    std::size_t lastSlash = param.targetEdbPath.rfind("\\");
    std::string restorePath = param.targetEdbPath.substr(0, lastSlash);
    std::string targetEdbFileName = param.targetEdbPath.substr(lastSlash + 1);
    g_targetDataPath = restorePath;
    g_targetLogPath = param.targetLogPath;
    INFOLOG("parse bcd xml file: %s", abusoluteBcdPath.c_str());
    ParseXML(abusoluteBcdPath);
    // Backup Metadata Document
    std::wstring xmlContent;
    DWORD retd = LoadFromFile(abusoluteBcdPath, xmlContent);
    if (retd != 0) {
        ERRLOG("LoadFromFile failed");
        return false;
    }
    CComBSTR Bstr(xmlContent.size(), xmlContent.data());
    m_backupMetadataDocument = Bstr;

    std::wstring xmlContentWriter;
    // Exchange Write Metadata Document
    retd = LoadFromFile(abusoluteWmdPath, xmlContentWriter);
    if (retd != 0) {
        ERRLOG("LoadFromFile failed");
        return false;
    }
    CComBSTR BstrWriter(xmlContentWriter.size(), xmlContentWriter.data());
    m_exchangeWriterMetadataDocument = BstrWriter;

    // VSS流程
    BSTR targetDatabaseGuid = _com_util::ConvertStringToBSTR(param.targetGuid.c_str());
    BSTR orginGuid = _com_util::ConvertStringToBSTR(param.originGuid.c_str());
    VSS_ID exchangeWriterID{};
    bool ret = RestoreInitalization(m_backupMetadataDocument);
    if (!ret) {
        ERRLOG("RestoreInitalization");
        return false;
    }
    ret = RestorePreparetionV2(exchangeWriterID, orginGuid, targetDatabaseGuid);
    if (!ret) {
        ERRLOG("RestorePreparetionV2");
        return false;
    }
    ret = ActualRestoration(param.dataPath, restorePath, param.targetLogPath, param.logPrefix, targetEdbFileName);
    if (!ret) {
        ERRLOG("ActualRestoration");
        return false;
    }
    ret = RestoreCleanUpAndTermination();
    if (!ret) {
        return false;
    }

    return true;
}

bool VssRestore::RestoreInitalization(BSTR backupMetadataDocument)
{
    // 初始化
    HRESULT hr = ::CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED);
    CHECK_HR_RETURN(hr, "CoInitializeEx", false);

    hr = CoInitializeSecurity(nullptr, //  Allow *all* VSS writers to communicate back!
        -1,                            //  Default COM authentication service
        nullptr,                       //  Default COM authorization service
        nullptr,                       //  reserved parameter
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY, //  Strongest COM authentication level
        RPC_C_IMP_LEVEL_IMPERSONATE,   //  Minimal impersonation abilities
        nullptr,                       //  Default COM authentication settings
        EOAC_DYNAMIC_CLOAKING,         //  Cloaking
        nullptr                        //  Reserved parameter
    );
    CHECK_HR_RETURN(hr, "CoInitializeSecurity", false);

    // 创建实例
    hr = S_OK;
    hr = CreateVssBackupComponents(&m_ppBackupComponent);
    CHECK_HR_RETURN(hr, "CreateVssBackupComponents", false);

    // 装载备份阶段生成的XML, m_ppBackupComponent 是用 bcd 初始化的 backup component
    hr = m_ppBackupComponent->InitializeForRestore(m_backupMetadataDocument);
    CHECK_HR_RETURN(hr, "InitializeForRestore", false);

    // 创建writer实例
    hr = S_OK;
    // 用 备份保存的wmd 初始化 plus examine writer
    hr = CreateVssExamineWriterMetadata(m_exchangeWriterMetadataDocument, &m_ppExchangeWriterComponentPlus);
    CHECK_HR_RETURN(hr, "CreateVssExamineWriterMetadata", false);

    // 通知各writers将各自的metadata发送过来
    CComPtr<IVssAsync> pAsync;
    hr = m_ppBackupComponent->GatherWriterMetadata(&pAsync);
    CHECK_HR_RETURN(hr, "GatherWriterMetadata", false);

    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "Wait GatherWriterMetadata", false);

    // 获取备份阶段使用的writer
    hr = m_ppBackupComponent->GetWriterComponentsCount(&m_pcWriterComponents);
    CHECK_HR_RETURN(hr, "GetWriterComponentsCount", false);

    for (UINT i = 0; i < m_pcWriterComponents; ++i) {
        // 保存所有使用的writer
        VSS_ID pidInstance;
        CComPtr<IVssExamineWriterMetadata> ppExamineWriterMetadata;
        hr = m_ppBackupComponent->GetWriterMetadata(i, &pidInstance, &ppExamineWriterMetadata);
        CHECK_HR_RETURN(hr, "GetWriterMetadata", false);
        m_pidWriterInstances.push_back(pidInstance);
        m_ppExmaineWriterMetadata.push_back(ppExamineWriterMetadata);
    }

    return true;
}

// dataDir: 包含通配符的绝对路径
// fileList: 所有匹配文件的文件名(而不是绝对路径)
// 扫描dataDir所在文件夹, 将所有匹配文件的文件名放入fileList中返回
void VssRestore::GetFileList(std::string dataDir, std::vector<std::string> &fileList)
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    dataDir = dataDir + "\\" + "*";
    INFOLOG("Get File list for dataDir : %s", dataDir.c_str());
    hFind = FindFirstFileA(dataDir.c_str(), &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string tempName = data.cFileName;
            DBGLOG("get file name : %s", tempName.c_str());
            if (tempName == "." || tempName == ".." || (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            } else {
                INFOLOG("Get file : %s", tempName.c_str());
                fileList.emplace_back(tempName);
            }
        } while (FindNextFileA(hFind, &data));
        FindClose(hFind);
    } else {
        auto errCode = GetLastError();
    }
}

void VssRestore::FillDatabaseInfo(DataBaseInfo& databaseInfo, const std::string& key, const std::string& value)
{
    if (key == "DATABASE_NAME") {
        databaseInfo.databaseName = value;
    } else if (key == "DATABASE_GUID") {
        databaseInfo.databaseGuid = value;
    } else if (key == "DATABASE_GUID_ORIGINAL") {
        databaseInfo.databaseGuidOriginal = value;
    } else if (key == "LOG_PATH_ORIGINAL") {
        databaseInfo.logPathOriginal = value;
    } else if (key == "EDB_LOCATION_ORIGINAL") {
        databaseInfo.edbLocationPath = value;
    } else if (key == "EDB_FILENAME_ORIGINAL") {
        databaseInfo.edbFileName = value;
    } else if (key == "LOG_BASE_NAME") {
        databaseInfo.logBaseName = value;
    }
}

tinyxml2::XMLElement* VssRestore::GetChildElement(tinyxml2::XMLElement* pParentElement, const string& strSection)
{
    if (pParentElement == nullptr) {
        return nullptr;
    }
    tinyxml2::XMLElement* pCfgSec = pParentElement->FirstChildElement();
    if (pCfgSec == nullptr) {
        return nullptr;
    }
    while (pCfgSec) {
        const char* sectionName = pCfgSec->Value();
        if (sectionName == nullptr || *sectionName == 0) {
            pCfgSec = pCfgSec->NextSiblingElement();
            continue;
        }

        if (strcmp(sectionName, strSection.c_str()) == 0) {
            return pCfgSec;
        } else {
            pCfgSec = pCfgSec->NextSiblingElement();
        }
    }
    return nullptr;
}

void VssRestore::ParseXML(const std::string& metaPath)
{
    tinyxml2::XMLDocument doc;
    std::wstring wValue;
    DWORD retd = LoadFromFile(metaPath, wValue);
    if (retd != 0) {
        ERRLOG("LoadFromFile failed");
        return;
    }
    std::string value = Utf16ToUtf8(wValue);
    doc.Parse(value.c_str());
    tinyxml2::XMLElement* root = doc.RootElement();
    tinyxml2::XMLElement* node = GetChildElement(root, "WRITER_COMPONENTS");
    if (node == nullptr) {
        ERRLOG("get writer component failed for %s",  metaPath.c_str());
        return;
    }
    m_xmlInfo.writerId = node->Attribute("writerId");
    for (tinyxml2::XMLElement* child = node->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
        const char* name = child->Name();
        tinyxml2::XMLElement* sonNode = GetChildElement(child, "BACKUP_METADATA");
        if (sonNode != nullptr) {
            DataBaseInfo databaseInfo;
            const char* metadata = sonNode->Attribute("metadata");
            tinyxml2::XMLDocument sonDoc;
            sonDoc.Parse(metadata);
            tinyxml2::XMLElement *component = sonDoc.RootElement();
            if (component == nullptr) {
                ERRLOG("root Doc is nullptr");
                return;
            }
            for (tinyxml2::XMLElement* componentChild = component->FirstChildElement(); componentChild != nullptr;
                componentChild = componentChild->NextSiblingElement()) {
                const char* name = componentChild->Name();
                const char* value = componentChild->GetText();
                FillDatabaseInfo(databaseInfo, name, value);
            }
            databaseInfo.logicalPath = child->Attribute("logicalPath");
            databaseInfo.componentName = child->Attribute("componentName");
            databaseInfo.componentType = child->Attribute("componentType");
            PrintDbInfo(databaseInfo);
            m_xmlInfo.dBInfos.push_back(databaseInfo);
        }
    }
}

void VssRestore::PrintDbInfo(const DataBaseInfo& databaseInfo)
{
    DBGLOG("databaseName: %s, databaseGuid: %s, databaseGuidOriginal: %s, logPathOriginal: %s,"
        "edbLocationPath: %s, edbFileName: %s, logBaseName: %s,componentName: %s,"
        "componentType: %s, logicalPath: %s",
        databaseInfo.databaseName.c_str(),
        databaseInfo.databaseGuid.c_str(),
        databaseInfo.databaseGuidOriginal.c_str(),
        databaseInfo.logPathOriginal.c_str(),
        databaseInfo.edbLocationPath.c_str(),
        databaseInfo.edbFileName.c_str(),
        databaseInfo.logBaseName.c_str(),
        databaseInfo.componentName.c_str(),
        databaseInfo.componentType.c_str(),
        databaseInfo.logicalPath.c_str());
}

bool VssRestore::ActualRestoration(const std::string &dataRepoPath, const std::string &restorePath,
    const std::string &logPath, const std::string &logPrefix, const std::string &targetEdbFileName)
{
    // 生成Restore Set
    INFOLOG("Enter ActualRestoration");
    std::map<std::string, std::string> restorePathList; // (src, des)
    std::string logExtension = ".log";
    std::string chkExtension = ".chk";
    std::string jrsExtension = ".jrs";
    std::string edbExtension = ".edb";

    std::vector<std::string> fileList;
    GetFileList(dataRepoPath, fileList);
    for (int i = 0; i < fileList.size(); ++i) {
        std::string fileNameStr = fileList[i];
        std::string targetFileNameStr = fileNameStr;
        std::string desPath;
        if (fileNameStr.find(logExtension) != std::string::npos ||
            fileNameStr.find(chkExtension) != std::string::npos ||
            fileNameStr.find(jrsExtension) != std::string::npos) {
            targetFileNameStr = logPrefix + fileNameStr.substr(NUMBER3);
            desPath = logPath + "\\" + targetFileNameStr;
        } else if (fileNameStr.find(edbExtension) != std::string::npos &&
            fileNameStr.find("tmp") == std::string::npos) {
            targetFileNameStr = targetEdbFileName;
            desPath = restorePath + "\\" + targetFileNameStr;
        } else if (fileNameStr == "tmp.edb") {
            targetFileNameStr = fileNameStr;
            desPath = restorePath + "\\" + targetFileNameStr;
        } else {
            WARNLOG("discard file : %s", fileNameStr.c_str());
            continue;
        }
        INFOLOG("get fileNameStr: %s", fileNameStr.c_str());
        std::string srcPath = dataRepoPath + "\\" + fileNameStr;
        // debug add restored log path
        restorePathList.insert(make_pair(srcPath, desPath));
        INFOLOG("restorePathList add : %s, %s", srcPath.c_str(), desPath.c_str());
    }
    // CopyFile
    int count = 0;
    for (auto &item : restorePathList) {
        DBGLOG("Copy file src : %s dst: %s", item.first.c_str(), item.second.c_str());
        bool CopyFlag = CopyFile(item.first.c_str(), item.second.c_str(), FALSE);
        if (!CopyFlag) {
            ERRLOG("Copy file failed src : %s dst: %s, errmsg: %s", item.first.c_str(),
                item.second.c_str(), GetLastError());
            return false;
        }
        count++;
    }
    INFOLOG("Copy file success");
    return true;
}

bool VssRestore::RestoreCleanUpAndTermination()
{
    // 触发PostRestore事件
    HRESULT hr;
    CComPtr<IVssAsync> pAsync;
    hr = m_ppBackupComponent->PostRestore(&pAsync);
    CHECK_HR_RETURN(hr, "PostRestore", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "Wait GatherWriterMetadata", false);
    return true;
}

bool VssRestore::FetchWriterMetadata(VSS_ID exchangeWriterID)
{
    HRESULT hr;
    for (UINT i = 0; i < m_pcWriterComponents; ++i) {
        CComPtr<IVssWriterComponentsExt> ppWriter;
        hr = m_ppBackupComponent->GetWriterComponents(i, &ppWriter);
        CHECK_HR_RETURN(hr, "GetWriterComponents", false);
        // 获取Exchange writer
        VSS_ID pidInstance;
        VSS_ID pidWriter;
        hr = ppWriter->GetWriterInfo(&pidInstance, &pidWriter);
        // 正常情况下只有一个writer即exchange writer
        // 但后续应加入判断机制, 防止异常情况
        m_ppExchangeExamineWriterMetadata = m_ppExmaineWriterMetadata[i];
        m_ppExchangeWriterComponentsExt = ppWriter;
        m_pidExchangeInstance = pidInstance;
        m_pidExchangeWriter = pidWriter;
    }
    return true;
}

bool VssRestore::RestorePreparetionV2(VSS_ID exchangeWriterID, BSTR originGuid, BSTR targetDatabaseGuid)
{
    // 取回从Backup文档中显式包含的writer components信息
    CHECK_BOOL_RETURN(FetchWriterMetadata(exchangeWriterID), "FetchWriterMetadata", false);

    // 遍历已加载的writer, 取得要恢复的数据库
    HRESULT hr = S_OK;
    UINT pcExchangeWriterIncludeFiles;
    UINT pcExchangeExcludeFiles;
    UINT pcExchangeComponents;
    BSTR bstrTargetComponentLogicPath{};
    hr = m_ppExchangeWriterComponentPlus->GetFileCounts(&pcExchangeWriterIncludeFiles, &pcExchangeExcludeFiles,
        &pcExchangeComponents);
    CHECK_HR_RETURN(hr, "GetFileCounts", false);

    BSTR bstrExchangeRestoreComponentLogicPath{};
    BSTR bstrExchangeRestoreComponentName{};
    std::string tempOriginDatabaseGuid;
    // 遍历所有component, 找到所要恢复的component
    for (UINT i = 0; i < pcExchangeComponents; ++i) {
        CComPtr<IVssWMComponent> ppComponent;
        hr = m_ppExchangeWriterComponentPlus->GetComponent(i, &ppComponent);
        CHECK_HR_RETURN(hr, "GetComponent", false);
        PVSSCOMPONENTINFO ppComponentInfo;
        hr = ppComponent->GetComponentInfo(&ppComponentInfo);
        CHECK_HR_RETURN(hr, "GetComponentInfo", false);
        // 找到所要恢复的component, 拼接其logicPath和componentName, 所得字符串即真正要恢复的component的logicPath
        std::string tempBstrComponentName = _com_util::ConvertBSTRToString(ppComponentInfo->bstrComponentName);
        tempOriginDatabaseGuid = _com_util::ConvertBSTRToString(originGuid);
        INFOLOG("get database guid : %s, %s", tempBstrComponentName.c_str(), tempOriginDatabaseGuid.c_str());
        if (tempBstrComponentName == tempOriginDatabaseGuid) {
            // 选择恢复该component
            hr = m_ppBackupComponent->SetSelectedForRestore(m_pidExchangeWriter, VSS_CT_FILEGROUP,
                ppComponentInfo->bstrLogicalPath, ppComponentInfo->bstrComponentName, true);
            CHECK_HR_RETURN(hr, "SetSelectedForRestore", false);
            bstrExchangeRestoreComponentLogicPath = ppComponentInfo->bstrLogicalPath;
            bstrExchangeRestoreComponentName = ppComponentInfo->bstrComponentName;

            std::string tempComponentName =
                static_cast<std::string>(_com_util::ConvertBSTRToString(ppComponentInfo->bstrComponentName));
            std::string tempLogicalPath =
                static_cast<std::string>(_com_util::ConvertBSTRToString(ppComponentInfo->bstrLogicalPath));
            std::string finalLogicalPath = tempLogicalPath + "\\" + tempComponentName;
            bstrTargetComponentLogicPath = _com_util::ConvertStringToBSTR(finalLogicalPath.c_str());
            break; // 若有多个writer, 不应break, 而是都放到某处记录
        }
    }
    SetRestoreOptions(originGuid, targetDatabaseGuid, bstrExchangeRestoreComponentLogicPath,
        bstrExchangeRestoreComponentName);

    // 再次遍历component, 找到所要恢复的数据库文件/日志文件/检查点文件
    for (UINT i = 0; i < pcExchangeComponents; ++i) {
        FindRestoreFile(i, bstrExchangeRestoreComponentLogicPath, bstrExchangeRestoreComponentName, originGuid);
    }

    // 触发PreRestore事件
    return TriggerPreRestore();
}

bool VssRestore::SetRestoreOptions(BSTR originGuid, BSTR targetGuid, BSTR componentLogicPath, BSTR componentName)
{
    HRESULT hr = S_OK;
    std::string originGuidStr = _com_util::ConvertBSTRToString(originGuid);
    std::string targetGuidStr = _com_util::ConvertBSTRToString(targetGuid);
    if (originGuidStr != targetGuidStr) {
        LPCWSTR restoreOption = CreateRestoreOptionsXml(originGuidStr, targetGuidStr);
        INFOLOG("===debug call SetRestoreOptions: %S, %S, %S", componentLogicPath,
            componentName, restoreOption);
        hr = m_ppBackupComponent->SetRestoreOptions(m_pidExchangeWriter, VSS_CT_FILEGROUP,
            componentLogicPath, componentName, restoreOption);
        CHECK_HR_RETURN(hr, "SetRestoreOptions", false);
    }
    return true;
}

bool VssRestore::TriggerPreRestore()
{
    HRESULT hr = S_OK;
    CComPtr<IVssAsync> pAsync;
    hr = m_ppBackupComponent->PreRestore(&pAsync);
    CHECK_HR_RETURN(hr, "PreRestore", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "Wait GatherWriterMetadata", false);
    return true;
}

bool VssRestore::FindRestoreFile(int i, BSTR restorePath, BSTR bstrName, BSTR originGuid)
{
    CComPtr<IVssWMComponent> ppComponent;
    HRESULT hr = m_ppExchangeWriterComponentPlus->GetComponent(i, &ppComponent);
    PVSSCOMPONENTINFO ppComponentInfo;
    hr = ppComponent->GetComponentInfo(&ppComponentInfo);
    std::string tempBstrLogicPath = _com_util::ConvertBSTRToString(ppComponentInfo->bstrLogicalPath);
    std::string tempBstrTargetComponentLogicPath = _com_util::ConvertBSTRToString(restorePath);
    std::string componentName = _com_util::ConvertBSTRToString(bstrName);
    // 找到隐式包含的component
    INFOLOG("===debug: Find RestoreFile:, %s, %s, %s", tempBstrLogicPath.c_str(),
        tempBstrTargetComponentLogicPath.c_str(), componentName.c_str());
    std::string originGuidStr = _com_util::ConvertBSTRToString(originGuid);
    if (tempBstrLogicPath.find(originGuidStr) != std::string::npos) {
        // 选择恢复所有隐式包含的component
        INFOLOG("===debug: AddRestoreSubcomponent params: %S, %S, %S, %S", restorePath, bstrName,
            ppComponentInfo->bstrLogicalPath, ppComponentInfo->bstrComponentName);
        m_ppBackupComponent->AddRestoreSubcomponent(m_pidExchangeWriter, VSS_CT_FILEGROUP, restorePath, bstrName,
            ppComponentInfo->bstrLogicalPath, ppComponentInfo->bstrComponentName, false);
        // 获取待恢复文件的实际位置
        CComPtr<IVssWMFiledesc> ppFiledesc;
        BSTR pbstrPath;
        BSTR pbstrFilespec;
        for (UINT j = 0; j < ppComponentInfo->cFileCount; ++j) {
            hr = ppComponent->GetFile(j, &ppFiledesc);
            CHECK_HR_RETURN(hr, "GetFile", false);
            hr = ppFiledesc->GetPath(&pbstrPath);
            CHECK_HR_RETURN(hr, "GetPath", false);
            m_bstrPathToRestore.push_back(pbstrPath);
            INFOLOG("===debug: get file : %S", pbstrPath);
            hr = ppFiledesc->GetFilespec(&pbstrFilespec);
            m_bstrFilenameToRestore.push_back(pbstrFilespec);
            INFOLOG("===debug: get file desc: %S", pbstrFilespec);
            
            BSTR alternatePath = _com_util::ConvertStringToBSTR(g_targetLogPath.c_str());
            std::string fileDescStr = _com_util::ConvertBSTRToString(pbstrFilespec);
            if (fileDescStr.find(".log") != std::string::npos ||
                fileDescStr.find(".chk") != std::string::npos) {
                alternatePath = _com_util::ConvertStringToBSTR(g_targetLogPath.c_str());
            } else if (fileDescStr.find(".edb") != std::string::npos) {
                alternatePath = _com_util::ConvertStringToBSTR(g_targetDataPath.c_str());
            }
            // call add new target
            INFOLOG("===debug: Call AddNewTarget: %S, %S, %S, %S, %S", ppComponentInfo->bstrLogicalPath,
                ppComponentInfo->bstrComponentName, pbstrPath, pbstrFilespec, alternatePath);
            hr = m_ppBackupComponent->AddNewTarget(m_pidExchangeWriter, VSS_CT_FILEGROUP,
                ppComponentInfo->bstrLogicalPath, ppComponentInfo->bstrComponentName,
                pbstrPath, pbstrFilespec, false, alternatePath);
            CHECK_HR_RETURN(hr, "SetRestoreOptions", false);
        }
    }
    return true;
}

LPCWSTR VssRestore::CreateRestoreOptionsXml(const std::string& originGUID, const std::string& targetGUID)
{
    tinyxml2::XMLDocument doc;
    // 创建根节点
    tinyxml2::XMLNode *pRoot = doc.NewElement("DATABASE_RESTORE_OPTIONS");
    doc.InsertFirstChild(pRoot);

    // 创建元素
    tinyxml2::XMLElement *pElement = doc.NewElement("DATABASE_GUID_ORIGINAL");
    pElement->SetText(originGUID.c_str());  // origin guid
    pRoot->InsertFirstChild(pElement);

    tinyxml2::XMLElement *pElement1 = doc.NewElement("DATABASE_GUID_TARGET");
    pElement1->SetText(targetGUID.c_str()); // target guid
    pRoot->InsertEndChild(pElement1);

    // 生成XML
    tinyxml2::XMLPrinter printer;
    doc.Print(&printer);

    INFOLOG("print xml doc : %s", printer.CStr());
    std::string str(printer.CStr());
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    std::wstring wstr = converter.from_bytes(str);
    return wstr.c_str();
}
