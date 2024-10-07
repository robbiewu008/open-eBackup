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
#ifndef VSS_TOOL_RESTORE_H
#define VSS_TOOL_RESTORE_H

#include <string>
#include <vector>
#include "tinyxml2.h"
#include "VssBase.h"

struct DataBaseInfo {
    std::string databaseName;
    std::string databaseGuid;
    std::string databaseGuidOriginal;
    std::string logPathOriginal;
    std::string edbLocationPath;
    std::string edbFileName;
    std::string logBaseName;
    std::string componentName;
    std::string componentType;
    std::string logicalPath;
};

struct RestoreParam {
    std::string metaPath;
    std::string dataPath;
    std::string targetEdbPath;
    std::string targetLogPath;
    std::string originGuid;
    std::string targetGuid;
    std::string logPrefix;
};

struct XmlInfo {
    std::string writerId;
    std::vector<DataBaseInfo> dBInfos;
};

class VssRestore : public VssBase {
public:
    VssRestore();
    ~VssRestore();
    bool Restore(const RestoreParam& param);

private:
    bool RestoreInitalization(BSTR backupMetadataDocument);
    bool ActualRestoration(const std::string &dataRepoPath, const std::string &restorePath, const std::string &logPath,
        const std::string &logPrefix, const std::string &targetEbdFileName);
    bool RestoreCleanUpAndTermination();
    bool FetchWriterMetadata(VSS_ID exchangeWriterID);
    bool FindRestoreFile(int i, BSTR restorePath, BSTR bstrName, BSTR originGuid);
    void FillDatabaseInfo(DataBaseInfo& databaseInfo, const std::string& key, const std::string& value);
    tinyxml2::XMLElement* GetChildElement(tinyxml2::XMLElement* pParentElement, const std::string& strSection);
    void ParseXML(const std::string& metaPath);
    void PrintDbInfo(const DataBaseInfo& databaseInfo);
    bool TriggerPreRestore();
    bool SetRestoreOptions(BSTR originGuid, BSTR targetGuid, BSTR componentLogicPath, BSTR componentName);
    CComPtr<IVssBackupComponents> m_ppBackupComponent;
    std::vector<VSS_ID> m_pidWriterInstances;
    std::vector<CComPtr<IVssExamineWriterMetadata>> m_ppExmaineWriterMetadata;
    UINT m_pcWriterComponents;
    LPCWSTR CreateRestoreOptionsXml(const std::string& originGUID, const std::string& targetGUID);

    // Exchange Writer相关
    CComPtr<IVssExamineWriterMetadata> m_ppExchangeExamineWriterMetadata;
    CComPtr<IVssWriterComponentsExt> m_ppExchangeWriterComponentsExt;
    VSS_ID m_pidExchangeInstance;
    VSS_ID m_pidExchangeWriter;
    CComPtr<IVssWMComponent> ppExchangeWriterSelectedWMComponent;

    std::vector<BSTR> m_bstrPathToRestore;

    // 联调临时加入
    BSTR m_backupMetadataDocumentWriter;
    CComPtr<IVssExamineWriterMetadata> m_ppExchangeWriterComponentPlus;
    bool RestorePreparetionV2(VSS_ID exchangeWriterID, BSTR originGuid, BSTR targetDatabaseGuid);
    void GetFileList(std::string dataDir, std::vector<std::string> &fileList);
    std::vector<BSTR> m_bstrFilenameToRestore;

    // 联调后续应加入
    BSTR m_backupMetadataDocument;
    BSTR m_exchangeWriterMetadataDocument;
    BSTR m_targetDatabaseGuid;
    XmlInfo m_xmlInfo;
};
#endif