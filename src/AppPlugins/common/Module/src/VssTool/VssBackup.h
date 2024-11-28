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
#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include "VssBase.h"

/**
 * An Util Class, Used to automatically release a CoTaskMemAlloc allocated pointer
 * when the instance of this class goes out of scopeгиRAII)
 * (even if an exception is thrown)
 **/

const int WILDCARD_POS = 5;
const int POSTFIX_POS = 4;
const int VOLUME_COLON_SLASH_LENGTH = 3;
const int POS_EXCLUDE_VOLUME_COLON = 2;

struct DatabaseInfo {
    std::string logPath;
    std::string databseName;
    std::string mountPath;
};

struct VolumeInfo {
    std::string deviceLetter;
    std::string volumeId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceLetter, deviceLetter);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeId, volumeId);
    END_SERIAL_MEMEBER
};

struct SnapshotInfo {
    std::vector<VolumeInfo> snapInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapInfo, snapInfo);
    END_SERIAL_MEMEBER
};

class SnapshotSetResult {
public:
    std::vector<std::wstring> m_wSnapshotIDList;
    std::vector<VSS_ID> m_vssIdSnapshotIdList;
    std::wstring m_wSnapshotSetID;

public:
    std::vector<std::string> SnapshotIDList() const;
    std::string SnapshotSetID() const;
    std::vector<std::wstring> SnapshotIDListW() const;
    std::wstring SnapshotSetIDW() const;
};

struct BackupParam {
    std::string targetBackupPath;
    CComPtr<IVssExamineWriterMetadata> writerMetadata;
    UINT componentCount;
    std::string guid;
    std::vector<std::string> exposeList;
    std::vector<std::wstring> wVolumePathList;
};

struct ActualBackupParam {
    ActualBackupParam() {}
    ActualBackupParam(const std::string& pBackupType, const std::string& pTargetWriterName,
        const std::string& pTargetMetaPath, const std::string& pJobId)
        : backupType(pBackupType), targetWriterName(pTargetWriterName),
        targetMetaPath(pTargetMetaPath), jobId(pJobId) {}
    std::string backupType;
    std::string targetWriterName;
    std::string targetMetaPath;
    std::string jobId;
};

class VssBackupWrapper : public VssBase {
public:
    VssBackupWrapper();
    ~VssBackupWrapper();

    bool ActualBackup(ActualBackupParam param, const std::vector<std::string>& volumeList,
        const std::vector<std::string>& guidList);
    bool DoDeleteSnapshot(const VSS_ID snapshotId);
    // CheckBackupInterity : 检查数据库快照完整性， 在备份完成后由插件调用
    // 1. log base name , like E01
    // 2. logPath inside snapshot mountpath, absolute path
    // 3. database name, without .edb, just name
    bool CheckBackupInterity(const std::string& baseName,
        const std::string& logPath,
        const std::string& databaseName);

protected:
    bool InitializeBackup();
    void UninitializeBackup();
    virtual bool InitializeBackupContect(const VSS_SNAPSHOT_CONTEXT &context);
    bool InitializeVssComponent();
    bool GatherWriterMetadataSync();
    bool DoGetWriterMetadataCount(UINT &count);
    bool DoGetWriterMetadata(const UINT totalNum, const std::string &targetWriterName, VSS_ID &resultidInstance,
        VSS_ID &resultwriterId, CComPtr<IVssExamineWriterMetadata> &resultMetadata);
    bool DoGetFileCounts(
        CComPtr<IVssExamineWriterMetadata> writerMetadata, UINT &excludeFileCount, UINT &componentCount);
    bool DoAddComponent(VSS_ID writerInstanceId,
        VSS_ID writerClassId,
        CComPtr<IVssExamineWriterMetadata> writerMetadata,
        const UINT componentCount,
        const std::vector<std::string>& guidList);
    bool DoStartSnapShot();
    bool DoAddToSnapshotSet();
    bool WaitAndCheckForAsyncOperation(IVssAsync *pAsync);
    bool PrepareForBackupSync();
    bool DoSnapshotSetSync();
    bool GetVolumeList(CComPtr<IVssExamineWriterMetadata> writerMetadata, const UINT componentCount);
    bool GenerateExcludeFileSet(CComPtr<IVssExamineWriterMetadata> writerMetadata, const UINT excludeCount,
        std::vector<CComPtr<IVssWMFiledesc>> &excludeFileSet);
    bool BackupCompleteSync();
    bool GatherWriterStatusSync();
    bool DoSaveAsXML(CComBSTR &bstrXML);

    bool DoExposeSnapshot(VSS_ID vssSnapshotId, std::string snapshotId, std::string &shadowDir);
    void GetFileList(std::string dataDir, std::vector<std::wstring> &fileList);
    bool GetLogicalPath(const CComPtr<IVssExamineWriterMetadata> writerMetadata, const UINT componentCount,
        const std::string guid, std::string &finalLogicalPath);
    void AddLogComponentToPathList(const std::string tempBstrPath, const std::string strFileName,
        const std::vector<std::string> exposeList, const std::vector<std::wstring> wVolumePathList,
        std::map<std::string, std::string> &pathList);
    void AddNonLogComponentToPathList(const std::string tempBstrPath, const BSTR bstrFileName,
        const std::vector<std::string> exposeList, const std::vector<std::wstring> wVolumePathList,
        std::map<std::string, std::string> &pathList);

    bool BackupInitializePhase();
    bool BackupDiscoveryPhase(const std::string &targetWriterName,
        UINT &componentCount,
        CComPtr<IVssExamineWriterMetadata> &writerMetadata,
        const std::vector<std::string>& guidList);
    bool PreBackupPhase(std::vector<std::string> &exposeList);
    bool BackupActualPhase(BackupParam backupParam, std::string path);
    DWORD SaveXmlFile(const std::string &strPath, CComBSTR bstrBcd);
    bool DoBackupComplete();

private:
    bool IsComponentToBackup(const std::string& componentName,
        const std::string& componentPath,
        const std::vector<std::string>& guidList);

private:
    IVssBackupComponents *m_pVssObject = nullptr;
    std::string m_backupType {"Backup"};
    std::vector<std::string> m_volumeList;
    std::vector<VolumeInfo> m_volumeInfos;
    std::map<std::string, DatabaseInfo> m_databaseInfos;
    std::wstring m_wSnapshotSetID;
    std::string m_metaPath;
    bool m_initialized = false;
    VSS_ID m_writerInstanceId;
    VSS_ID m_writerClassId;
    std::vector<PVSSCOMPONENTINFO> m_selectedComponent;
    std::string m_jobId;
};