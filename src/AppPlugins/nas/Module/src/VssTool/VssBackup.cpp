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
#include "VssBackup.h"
#include <comutil.h>
#include "log/Log.h"
#include "JsonFileTool.h"
#include "chksgfiles.hxx"

namespace {
    const std::string MOUNTPOINT = R"(C:\BackupTempFile\)";
    const std::string SNAP_INFO_PATH = ".json";
    const int NUMBER3 = 3;
}

std::string Utf16ToUtf8(const std::wstring &wstr)
{
    using ConvertTypeX = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<ConvertTypeX> converterX;
    return converterX.to_bytes(wstr);
}

std::vector<std::string> SnapshotSetResult::SnapshotIDList() const
{
    std::vector<std::string> snapshotIDList;
    for (const std::wstring &wSnapshotID : m_wSnapshotIDList) {
        snapshotIDList.emplace_back(Utf16ToUtf8(wSnapshotID));
    }
    return snapshotIDList;
}

std::string SnapshotSetResult::SnapshotSetID() const
{
    return Utf16ToUtf8(m_wSnapshotSetID);
}

std::vector<std::wstring> SnapshotSetResult::SnapshotIDListW() const
{
    return m_wSnapshotIDList;
}

std::wstring SnapshotSetResult::SnapshotSetIDW() const
{
    return m_wSnapshotSetID;
}

VssBackupWrapper::VssBackupWrapper()
{
    InitializeBackup();
}

VssBackupWrapper::~VssBackupWrapper()
{
    if (m_pVssObject != nullptr) {
        m_pVssObject->Release();
        m_pVssObject = nullptr;
    }
    UninitializeBackup();
}

bool VssBackupWrapper::InitializeBackup()
{
    if (m_initialized) {
        return true;
    }
    HRESULT hr = ::CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED);
    CHECK_HR_RETURN(hr, "CoInitializeEx", false);
    m_initialized = true;
    hr = CoInitializeSecurity(nullptr,  //  Allow *all* VSS writers to communicate back!
        -1,                             //  Default COM authentication service
        nullptr,                        //  Default COM authorization service
        nullptr,                        //  reserved parameter
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  //  Strongest COM authentication level
        RPC_C_IMP_LEVEL_IMPERSONATE,    //  Minimal impersonation abilities
        nullptr,                        //  Default COM authentication settings
        EOAC_DYNAMIC_CLOAKING,          //  Cloaking
        nullptr                         //  Reserved parameter
    );
    CHECK_HR_RETURN(hr, "CoInitializeSecurity", false);
    return true;
}

void VssBackupWrapper::UninitializeBackup()
{
    if (!m_initialized) {
        return;
    }
    ::CoUninitialize();
    m_initialized = false;
}

bool VssBackupWrapper::ActualBackup(
    ActualBackupParam param,
    const std::vector<std::string>& volumeList,
    const std::vector<std::string>& guidList)
{
    m_backupType = param.backupType;
    m_metaPath = param.targetMetaPath;
    m_jobId = param.jobId;
    m_volumeList.assign(volumeList.begin(), volumeList.end());
    // 备份阶段一：初始化阶段
    CHECK_BOOL_RETURN(BackupInitializePhase(), "BackupInitializePhase", false);

    // 备份阶段二：发现阶段
    UINT componentCount;
    CComPtr<IVssExamineWriterMetadata> writerMetadata;
    CHECK_BOOL_RETURN(BackupDiscoveryPhase(param.targetWriterName, componentCount, writerMetadata, guidList),
        "BackupDiscoveryPhase",
        false);

    // 备份阶段三：准备阶段
    std::vector<std::string> exposeList;
    bool ret = PreBackupPhase(exposeList);
    if (!ret) {
        ERRLOG("create snapshot failed!");
        return ret;
    }

    // 备份阶段四：实际备份阶段
    BackupParam backupParam;
    backupParam.writerMetadata = writerMetadata;
    backupParam.componentCount = componentCount;
    backupParam.exposeList = exposeList;
    CHECK_BOOL_RETURN(BackupActualPhase(backupParam, param.targetMetaPath), "BackupActualPhase", false);
    return true;
}

bool VssBackupWrapper::BackupInitializePhase()
{
    InitializeBackupContect(VSS_CTX_APP_ROLLBACK);

    // GatherWriterMetadata
    CHECK_BOOL_RETURN(GatherWriterMetadataSync(), "GatherWriterMetadataSync", false);
    return true;
}

bool VssBackupWrapper::BackupDiscoveryPhase(
    const std::string &targetWriterName,
    UINT &componentCount,
    CComPtr<IVssExamineWriterMetadata> &writerMetadata,
    const std::vector<std::string>& guidList)
{
    // GetWriterMetadataCount用于遍历所有writer
    UINT writerCount;
    CHECK_BOOL_RETURN(DoGetWriterMetadataCount(writerCount), "DoGetWriterMetadataCount", false);

    // GetWriterMetadata，将name = targetWriterName的筛选出来
    VSS_ID writerInstanceId;
    VSS_ID writerClassId;

    CHECK_BOOL_RETURN(
        DoGetWriterMetadata(writerCount, targetWriterName, writerInstanceId, writerClassId, writerMetadata),
        "DoGetWriterMetadata",
        false);
    m_writerClassId = writerClassId;
    m_writerInstanceId = writerInstanceId;

    // GetFileCount，获取component数量，用于遍历
    UINT excludeFileCount;
    CHECK_BOOL_RETURN(DoGetFileCounts(writerMetadata, excludeFileCount, componentCount), "DoGetFileCounts", false);

    // AddComponent
    CHECK_BOOL_RETURN(
        DoAddComponent(writerInstanceId, writerClassId, writerMetadata, componentCount, guidList),
        "DoAddComponent", false);

    // StartSnapShot
    CHECK_BOOL_RETURN(DoStartSnapShot(), "DoStartSnapShot", false);

    // AddToSnapshotSet
    CHECK_BOOL_RETURN(DoAddToSnapshotSet(), "DoAddToSnapshotSet", false);
    return true;
}

bool VssBackupWrapper::PreBackupPhase(std::vector<std::string> &exposeList)
{
    CHECK_BOOL_RETURN(PrepareForBackupSync(), "PrepareForBackupSync", false);
    CHECK_BOOL_RETURN(DoSnapshotSetSync(), "DoSnapshotSetSync", false);

    std::string volumePath = m_metaPath + "\\" + m_jobId + SNAP_INFO_PATH;
    SnapshotInfo snapshotInfo;
    snapshotInfo.snapInfo = m_volumeInfos;

    if (!JsonFileTool::WriteToFile(snapshotInfo, volumePath)) {
        ERRLOG("Write snap info to file failed for volumePath : %s", volumePath.c_str());
        return false;
    }

    // ExposeSnapshot
    for (auto volume : m_volumeList) {
        std::string tempExposePath;
        std::string volumeId {""};
        for (auto volumeInfo : m_volumeInfos) {
            if (volumeInfo.deviceLetter == volume) {
                volumeId = volumeInfo.volumeId;
            }
        }
        if (volumeId == "") {
            ERRLOG("cannot found volume: %s 's volumeId", volume.c_str());
            return false;
        }
        CHECK_BOOL_RETURN(
            DoExposeSnapshot(VssIDfromWStr(Utf8ToUtf16(volumeId)), volumeId, tempExposePath),
            "DoExposeSnapshot",
            false);
        exposeList.push_back(tempExposePath);
    }
    return true;
}

bool VssBackupWrapper::BackupActualPhase(BackupParam backupParam, std::string path)
{
    INFOLOG("Enter BackupActualPhase");
    CComPtr<IVssExamineWriterMetadata> writerMetadata = backupParam.writerMetadata;
    UINT componentCount = backupParam.componentCount;
    std::string guid = backupParam.guid;
    std::vector<std::string> exposeList = backupParam.exposeList;
    std::vector<std::wstring> wVolumePathList = backupParam.wVolumePathList;
    std::string targetBackupPath = backupParam.targetBackupPath;

    CHECK_BOOL_RETURN(BackupCompleteSync(), "BackupCompleteSync", false);
    CHECK_BOOL_RETURN(GatherWriterStatusSync(), "GatherWriterStatusSync", false);

    CComBSTR bstrXML;
    CHECK_BOOL_RETURN(DoSaveAsXML(bstrXML), "DoSaveAsXML", false);
    std::string bcdPath = path + "\\BCD.xml";
    SaveXmlFile(bcdPath, bstrXML);

    CComBSTR wmdXML;
    HRESULT hr = writerMetadata->SaveAsXML(&wmdXML);
    CHECK_HR_RETURN(hr, "writer SaveAsXML", false);
    std::string wmdPath = path + "\\WMD.xml";
    SaveXmlFile(wmdPath, wmdXML);
    INFOLOG("BackupActualPhase complete!");
    return true;
}

bool VssBackupWrapper::InitializeBackupContect(const VSS_SNAPSHOT_CONTEXT &context)
{
    CHECK_BOOL_RETURN(InitializeVssComponent(), "InitializeVssComponent", false);
    VSS_BACKUP_TYPE vssType;
    if (m_backupType == "Backup") {
        vssType = VSS_BACKUP_TYPE::VSS_BT_FULL;
    } else if (m_backupType == "Log") {
        vssType = VSS_BACKUP_TYPE::VSS_BT_LOG;
    }

    HRESULT hr = m_pVssObject->InitializeForBackup();
    CHECK_HR_RETURN(hr, "InitializeForBackup", false);

    hr = m_pVssObject->SetContext(context);
    CHECK_HR_RETURN(hr, "SetContext", false);

    hr = m_pVssObject->SetBackupState(true, false, VSS_BT_FULL, false);
    CHECK_HR_RETURN(hr, "SetBackupState", false);

    return true;
}

bool VssBackupWrapper::InitializeVssComponent()
{
    if (m_pVssObject != nullptr) {
        m_pVssObject->Release();
        m_pVssObject = nullptr;
    }
    HRESULT hr = ::CreateVssBackupComponents(&m_pVssObject);
    CHECK_HR_RETURN(hr, "CreateVssBackupComponents", false);
    return true;
}

bool VssBackupWrapper::GatherWriterMetadataSync()
{
    CComPtr<IVssAsync> pAsync;
    HRESULT hr = m_pVssObject->GatherWriterMetadata(&pAsync);
    CHECK_HR_RETURN(hr, "GatherWriterMetadata", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "GatherWriterMetadata Wait", false);
    return true;
}

bool VssBackupWrapper::DoGetWriterMetadataCount(UINT &count)
{
    HRESULT hr = m_pVssObject->GetWriterMetadataCount(&count);
    CHECK_HR_RETURN(hr, "GetWriterMetadata", false);
    return true;
}

bool VssBackupWrapper::DoGetWriterMetadata(const UINT totalNum, const std::string &targetWriterName,
    VSS_ID &resultidInstance, VSS_ID &resultwriterId, CComPtr<IVssExamineWriterMetadata> &resultMetadata)
{
    HRESULT hr;
    for (UINT i = 0; i < totalNum; i++) {
        VSS_ID pidInstance;
        CComPtr<IVssExamineWriterMetadata> ppMetadata;
        hr = m_pVssObject->GetWriterMetadata(i, &pidInstance, &ppMetadata);
        CHECK_HR_RETURN(hr, "GetWriterMetadata", false);

        VSS_ID pidTempInstance;
        VSS_ID pidTempWriter;
        BSTR pTempBstrWriterName;
        VSS_USAGE_TYPE pTempUsage;
        VSS_SOURCE_TYPE pTempSource;
        hr = ppMetadata->GetIdentity(&pidTempInstance, &pidTempWriter, &pTempBstrWriterName, &pTempUsage, &pTempSource);
        CHECK_HR_RETURN(hr, "GetIdentity", false);

        std::string writerName = (std::string)_com_util::ConvertBSTRToString(pTempBstrWriterName);
        if (targetWriterName == writerName) {
            resultidInstance = pidTempInstance;
            resultwriterId = pidTempWriter;
            resultMetadata = ppMetadata;
            return true;
        }
    }
    return false;
}

bool VssBackupWrapper::DoGetFileCounts(
    CComPtr<IVssExamineWriterMetadata> writerMetadata, UINT &excludeFileCount, UINT &componentCount)
{
    UINT includeFiles;
    HRESULT hr = writerMetadata->GetFileCounts(&includeFiles, &excludeFileCount, &componentCount);
    CHECK_HR_RETURN(hr, "GetFileCounts", false);
    return true;
}

bool VssBackupWrapper::DoAddComponent(VSS_ID writerInstanceId,
    VSS_ID writerClassId,
    CComPtr<IVssExamineWriterMetadata> writerMetadata,
    const UINT componentCount,
    const std::vector<std::string>& guidList)
{
    for (UINT i = 0; i < componentCount; i++) {
        CComPtr<IVssWMComponent> tempComponent;
        HRESULT hr = writerMetadata->GetComponent(i, &tempComponent);
        CHECK_HR_RETURN(hr, "GetComponent", false);

        PVSSCOMPONENTINFO componentInfo;
        hr = tempComponent->GetComponentInfo(&componentInfo);
        CHECK_HR_RETURN(hr, "GetComponentInfo", false);

        BSTR componentName = componentInfo->bstrComponentName;
        LPCWSTR compName = componentName ? componentName : L"";
        BSTR componentPath = componentInfo->bstrLogicalPath;
        LPCWSTR compPath = componentPath ? componentPath : L"";
        std::string componentNameStr = _com_util::ConvertBSTRToString(componentName);
        std::string componentPathStr = _com_util::ConvertBSTRToString(componentPath);
        // 去掉不需要备份的guid
        if (!IsComponentToBackup(componentNameStr, componentPathStr, guidList)) {
            continue;
        }
        m_selectedComponent.push_back(componentInfo);
        INFOLOG("AddComponent: %s, %s", componentNameStr.c_str(), componentPathStr.c_str());
        hr = m_pVssObject->AddComponent(writerInstanceId, writerClassId, componentInfo->type, compPath, compName);
        CHECK_HR_RETURN(hr, "AddComponent", false);
    }
    return true;
}

bool VssBackupWrapper::IsComponentToBackup(const std::string& componentName,
    const std::string& componentPath,
    const std::vector<std::string>& guidList)
{
    if (guidList.size() == 0) {
        return true;
    }
    for (UINT i = 0; i < guidList.size(); i++) {
        if (componentName == guidList[i]) {
            INFOLOG("Accept componentName: %s, componentPath: %s", componentName.c_str(), componentPath.c_str());
            return true;
        }
    }
    INFOLOG("Discard componentName: %s, componentPath :%s", componentName.c_str(), componentPath.c_str());
    return false;
}

bool VssBackupWrapper::DoStartSnapShot()
{
    VSS_ID snapshotSetId;
    HRESULT hr = m_pVssObject->StartSnapshotSet(&snapshotSetId);
    CHECK_HR_RETURN(hr, "StartSnapshotSet", false);
    m_wSnapshotSetID = VssID2WStr(snapshotSetId);
    return true;
}

bool VssBackupWrapper::DoAddToSnapshotSet()
{
    for (const std::string &volumePath : m_volumeList) {
        INFOLOG("create snapshot for : %s", volumePath.c_str());
        VSS_ID snapshotId;
        WCHAR volume[MAX_PATH] = {L'\0'};
        std::wstring mVolumePath = Utf8ToUtf16(volumePath);
        errno_t err = wcscpy_s(volume, MAX_PATH, mVolumePath.c_str());
        HRESULT hr = m_pVssObject->AddToSnapshotSet(volume, GUID_NULL, &snapshotId);
        CHECK_HR_RETURN(hr, "AddToSnapshotSet", false);
        m_volumeInfos.push_back({volumePath, Utf16ToUtf8(VssID2WStr(snapshotId))});
    }
    return true;
}

bool VssBackupWrapper::WaitAndCheckForAsyncOperation(IVssAsync *pAsync)
{
    HRESULT hr = pAsync->Wait();
    CHECK_HR_RETURN(hr, "WaitAndCheckForAsyncOperation pAsync->Wait", false);

    /* Check the result of the asynchronous operation */
    HRESULT hrReturned = S_OK;
    hr = pAsync->QueryStatus(&hrReturned, nullptr);
    CHECK_HR_RETURN(hr, "WaitAndCheckForAsyncOperation pAsync->QueryStatus", false);

    /* Check if the async operation succeeded... */
    if (hrReturned != VSS_S_ASYNC_FINISHED) {
        return false;
    }
    return true;
}

bool VssBackupWrapper::PrepareForBackupSync()
{
    CComPtr<IVssAsync> pAsync;
    HRESULT hr = m_pVssObject->PrepareForBackup(&pAsync);
    CHECK_HR_RETURN(hr, "PrepareForBackup", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "PrepareForBackup Wait", false);
    return true;
}

bool VssBackupWrapper::DoSnapshotSetSync()
{
    CComPtr<IVssAsync> pAsync;
    HRESULT hr = m_pVssObject->DoSnapshotSet(&pAsync);
    CHECK_HR_RETURN(hr, "DoSnapshotSet", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "DoSnapshotSet Wait", false);
    return true;
}

bool VssBackupWrapper::BackupCompleteSync()
{
    INFOLOG("Enter BackupCompleteSync");
    CComPtr<IVssAsync> pAsync;
    HRESULT hr;
    for (UINT i = 0; i < m_selectedComponent.size(); i++) {
        PVSSCOMPONENTINFO componentInfo = m_selectedComponent[i];
        BSTR componentName = componentInfo->bstrComponentName;
        LPCWSTR compName = componentName ? componentName : L"";
        BSTR componentPath = componentInfo->bstrLogicalPath;
        LPCWSTR compPath = componentPath ? componentPath : L"";
        std::string componentNameStr = _com_util::ConvertBSTRToString(componentName);
        std::string componentPathStr = _com_util::ConvertBSTRToString(componentPath);
        INFOLOG("set component backup succeeded : %s, %s", componentNameStr.c_str(),
            componentPathStr.c_str());
        hr = m_pVssObject->SetBackupSucceeded(m_writerInstanceId, m_writerClassId, componentInfo->type,
            compPath, compName, true);
        CHECK_HR_RETURN(hr, "SetBackupSucceeded", false);
    }
    INFOLOG("Set component backup succeeded, start to call backup complete");
    hr = m_pVssObject->BackupComplete(&pAsync);
    CHECK_HR_RETURN(hr, "BackupComplete", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "BackupComplete Wait", false);
    INFOLOG("Exit BackupCompleteSync");
    return true;
}

bool VssBackupWrapper::GatherWriterStatusSync()
{
    CComPtr<IVssAsync> pAsync;
    HRESULT hr = m_pVssObject->GatherWriterStatus(&pAsync);
    CHECK_HR_RETURN(hr, "GatherWriterStatus", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "GatherWriterStatus Wait", false);
    return true;
}

bool VssBackupWrapper::DoSaveAsXML(CComBSTR &bstrXML)
{
    HRESULT hr = m_pVssObject->SaveAsXML(&bstrXML);
    CHECK_HR_RETURN(hr, "SaveAsXML", false);
    return true;
}

bool VssBackupWrapper::DoDeleteSnapshot(const VSS_ID snapshotId)
{
    InitializeBackupContect(VSS_CTX_APP_ROLLBACK);
    LONG lSnapshots = 0;
    VSS_ID idNonDeletedSnapshotId = GUID_NULL;
    VSS_ID vssSnapshotId;

    HRESULT hr =
        m_pVssObject->DeleteSnapshots(snapshotId, VSS_OBJECT_SNAPSHOT, FALSE, &lSnapshots, &idNonDeletedSnapshotId);
    CHECK_HR_RETURN(hr, "DeleteSnapshots", false);
    return true;
}

bool VssBackupWrapper::GenerateExcludeFileSet(CComPtr<IVssExamineWriterMetadata> writerMetadata,
    const UINT excludeCount, std::vector<CComPtr<IVssWMFiledesc>> &excludeFileSet)
{
    for (UINT i = 0; i < excludeCount; i++) {
        CComPtr<IVssWMFiledesc> pFiledesc;
        HRESULT hr = writerMetadata->GetExcludeFile(i, &pFiledesc);
        CHECK_HR_RETURN(hr, "GetExcludeFile", false);
        excludeFileSet.push_back(pFiledesc);
    }
    return true;
}

bool VssBackupWrapper::GetVolumeList(
    CComPtr<IVssExamineWriterMetadata> writerMetadata, const UINT componentCount)
{
    std::map<std::string, std::string> pathList;
    std::string finalLogicalPath;

    for (UINT i = 0; i < componentCount; i++) {
        CComPtr<IVssWMComponent> pSonComponent;
        HRESULT sonHr = writerMetadata->GetComponent(i, &pSonComponent);
        CHECK_HR_RETURN(sonHr, "GetComponent", {});

        PVSSCOMPONENTINFO sonComponentInfo;
        sonHr = pSonComponent->GetComponentInfo(&sonComponentInfo);
        CHECK_HR_RETURN(sonHr, "GetComponentInfo", {});
        std::string sonGuid = (std::string)_com_util::ConvertBSTRToString(sonComponentInfo->bstrComponentName);
        INFOLOG("sonGuid : %s", sonGuid.c_str());
        CComPtr<IVssWMFiledesc> pFiledesc;
        if (sonComponentInfo->cFileCount == 0) {
            WARNLOG("sonGuid : %s size is equal to 0", sonGuid.c_str());
            continue;
        }
        sonHr = pSonComponent->GetFile(0, &pFiledesc);
        CHECK_HR_RETURN(sonHr, "GetFile", {});
        BSTR bstrPath;
        sonHr = pFiledesc->GetPath(&bstrPath);
        CHECK_HR_RETURN(sonHr, "GetPath", {});
        std::string logPath = (std::string)_com_util::ConvertBSTRToString(bstrPath);
        INFOLOG("sonGuid: %s, logPath : %s", sonGuid.c_str(), logPath.c_str());
        m_volumeList.push_back(logPath.substr(0, NUMBER3));
        m_databaseInfos[sonGuid] = {logPath, "", ""};
    }
    return true;
}

bool VssBackupWrapper::GetLogicalPath(const CComPtr<IVssExamineWriterMetadata> writerMetadata,
    const UINT componentCount, const std::string guid, std::string &finalLogicalPath)
{
    for (UINT i = 0; i < componentCount; i++) {
        CComPtr<IVssWMComponent> pComponent;
        HRESULT hr = writerMetadata->GetComponent(i, &pComponent);
        CHECK_HR_RETURN(hr, "GetComponent", false);

        PVSSCOMPONENTINFO componentInfo;
        hr = pComponent->GetComponentInfo(&componentInfo);
        CHECK_HR_RETURN(hr, "GetComponentInfo", false);

        std::string tempComponentName = (std::string)_com_util::ConvertBSTRToString(componentInfo->bstrComponentName);
        if (tempComponentName == guid) {
            std::string tempLogicalPath = (std::string)_com_util::ConvertBSTRToString(componentInfo->bstrLogicalPath);
            finalLogicalPath = tempLogicalPath + '\\' + tempComponentName;
            return true;
        }
    }
    return false;
}

void VssBackupWrapper::AddLogComponentToPathList(const std::string tempBstrPath, const std::string strFileName,
    const std::vector<std::string> exposeList, const std::vector<std::wstring> wVolumePathList,
    std::map<std::string, std::string> &pathList)
{
    std::vector<std::wstring> logList;
    std::string waitToPush = tempBstrPath;
    if (waitToPush.substr(0, VOLUME_COLON_SLASH_LENGTH) == Utf16ToUtf8(wVolumePathList[0])) {
        waitToPush = waitToPush.substr(POS_EXCLUDE_VOLUME_COLON);
        waitToPush = exposeList[0] + waitToPush;
    } else {
        waitToPush = waitToPush.substr(POS_EXCLUDE_VOLUME_COLON);
        waitToPush = exposeList[1] + waitToPush;
    }

    GetFileList(waitToPush + strFileName, logList);
    int countPath = 1;
    for (auto log : logList) {
        std::string finalSourcePath = waitToPush + Utf16ToUtf8(log);

        std::string wstrFileName = Utf16ToUtf8(log);
        pathList.insert(make_pair(wstrFileName, finalSourcePath));
        countPath++;
    }
}

void VssBackupWrapper::AddNonLogComponentToPathList(const std::string tempBstrPath, const BSTR bstrFileName,
    const std::vector<std::string> exposeList, const std::vector<std::wstring> wVolumePathList,
    std::map<std::string, std::string> &pathList)
{
    std::string tempBstrFileName = (std::string)_com_util::ConvertBSTRToString(bstrFileName);
    std::string tempFinalPath = tempBstrPath + tempBstrFileName;

    if (tempFinalPath.substr(0, VOLUME_COLON_SLASH_LENGTH) == Utf16ToUtf8(wVolumePathList[0])) {
        tempFinalPath = tempFinalPath.substr(POS_EXCLUDE_VOLUME_COLON);
        tempFinalPath = exposeList[0] + tempFinalPath;
    } else {
        tempFinalPath = tempFinalPath.substr(POS_EXCLUDE_VOLUME_COLON);
        tempFinalPath = exposeList[1] + tempFinalPath;
    }
    pathList.insert(make_pair(tempBstrFileName, tempFinalPath));
}

bool VssBackupWrapper::DoExposeSnapshot(VSS_ID vssSnapshotId, std::string snapshotId, std::string &shadowDir)
{
    shadowDir = MOUNTPOINT + snapshotId;
    std::string command = "\"mkdir " + shadowDir + "\"";
    system(command.c_str());
    INFOLOG("shadowDir: %s, snapshotId: %s", shadowDir.c_str(), snapshotId.c_str());
    std::wstring wShadowDir = Utf8ToUtf16(shadowDir);
    LPWSTR pwszExposed;
    HRESULT hr = m_pVssObject->ExposeSnapshot(
        vssSnapshotId, nullptr, VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY, (VSS_PWSZ)wShadowDir.c_str(), &pwszExposed);
    CHECK_HR_RETURN(hr, "ExposeSnapshot", false);

    return true;
}

void VssBackupWrapper::GetFileList(std::string dataDir, std::vector<std::wstring> &fileList)
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    hFind = FindFirstFileA(dataDir.c_str(), &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string tempName = data.cFileName;
            if (tempName == "." || tempName == ".." || (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            } else {
                std::wstring wTempName = Utf8ToUtf16(tempName);
                fileList.emplace_back(wTempName);
            }
        } while (FindNextFileA(hFind, &data));
        FindClose(hFind);
    } else {
        auto errCode = GetLastError();
    }
}

DWORD VssBackupWrapper::SaveXmlFile(const std::string &strPath, CComBSTR bstrBcd)
{
    DWORD dwRet = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    hFile = CreateFile(strPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        dwRet = GetLastError();
        return dwRet;
    }
    DWORD dwSize = (bstrBcd.Length() + 1) * sizeof(WCHAR);
    DWORD dwWritten = 0;
    if (!WriteFile(hFile, (LPVOID)(BSTR)bstrBcd, dwSize, &dwWritten, NULL)) {
        dwRet = GetLastError();
        return dwRet;
    }
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
    return dwRet;
}

bool VssBackupWrapper::DoBackupComplete()
{}

bool VssBackupWrapper::CheckBackupInterity(const std::string& baseName,
    const std::string& logPath,
    const std::string& databaseName)
{
    CCheckSGFiles::ERR err = CCheckSGFiles::errSuccess;
    ULONG iDbError = (ULONG)CCheckSGFiles::iDbInvalid;
    CCheckSGFiles * const pcchecksgfiles = CCheckSGFiles::New();
    if (nullptr == pcchecksgfiles) {
        ERRLOG("ERROR: Could not allocate CCheckSGFiles object.");
        CCheckSGFiles::Delete(pcchecksgfiles);
        return false;
    }
    ULONG size = 1;
    ULONG pages = 1;
    ULONG * const psize = &size;
    ULONG * const ppages = &pages;
    ULONG * const piDbError = &iDbError;
    WCHAR *databaseNameListW[1];
    std::wstring baseNameW = Utf8ToUtf16(baseName);
    std::wstring wlogPath = Utf8ToUtf16(logPath);
    WCHAR* const pbaseName = (WCHAR*)baseNameW.data();
    WCHAR* const plogPath = (WCHAR*)wlogPath.data();
    databaseNameListW[0] = (WCHAR*)databaseName.data();
    INFOLOG("check param: %S, %S, %S", databaseNameListW[0], plogPath, pbaseName);
    err = pcchecksgfiles->ErrInit(databaseNameListW, 0, plogPath, pbaseName, 0);
    INFOLOG("error : %d", err);
    err = pcchecksgfiles->ErrCheckDbHeaders(psize, ppages, piDbError);
    INFOLOG("error : %d, size: %d, pages: %d", err, *psize, *ppages);
    if (CCheckSGFiles::errSuccess != err) {
        if (CCheckSGFiles::iDbInvalid != iDbError) {
            ERRLOG("Database header validation for %s failed with error %d, 0x%x ", databaseName.c_str(),
                err, err);
            CCheckSGFiles::Delete(pcchecksgfiles);
            return false;
        }
    }
    BOOL onlyUnnecessaryLogsCorrup = false;
    BOOL * const pfOnlyUnnecessaryLogsCorrup = &onlyUnnecessaryLogsCorrup;
    err = pcchecksgfiles->ErrCheckLogs(pfOnlyUnnecessaryLogsCorrup);
    if (err != 0) {
        ERRLOG("err : %d", err);
        CCheckSGFiles::Delete(pcchecksgfiles);
        return false;
    }
    err = pcchecksgfiles->ErrTerm();
    if (err != 0) {
        ERRLOG("err : %d", err);
        CCheckSGFiles::Delete(pcchecksgfiles);
        return false;
    }
    CCheckSGFiles::Delete(pcchecksgfiles);
    return true;
}
