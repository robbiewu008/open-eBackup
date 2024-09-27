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
#ifndef RFICTRLPARSER_H
#define RFICTRLPARSER_H

#include <string>
#include <mutex>
#include "ParserStructs.h"
#include "FileParser.h"
#include "common/JsonHelper.h"
#include "common/Crc32.h"
#include "define/Defines.h"
#include "define/Types.h"

namespace Module {

const std::string RFICTRL_HEADER_VERSION_V20 = "2.0";
const std::string RFICTRL_HEADER_VERSION_V30 = "3.0";

enum class RfiEntryStatus {
    RFI_ENTRY_STATUS_OLD,
    RFI_ENTRY_STATUS_NEW
};

enum class StatisticType {
    STATISTIC_TYPE_CREATE_FOLDER,
    STATISTIC_TYPE_CREATE_FILE,
    STATISTIC_TYPE_UPDATE_FOLDER,
    STATISTIC_TYPE_UPDATE_FILE,
    STATISTIC_TYPE_DELETE_FOLDER,
    STATISTIC_TYPE_DELETE_FILE
};

struct RfiCtrlParserParams {
    std::string rfiFileName;
    uint32_t maxEntriesPerFile {0};
    bool filterFlag = false;            // to filter
    std::string key;                    // to filter
    std::string version {};
};

struct IndexStatisticData {
    uint64_t createFileCount;
    uint64_t createFolderCount;
    uint64_t updateFileCount;
    uint64_t updateFolderCount;
    uint64_t deleteFileCount;
    uint64_t deleteFolderCount;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(createFileCount, createFileCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(createFolderCount, createFolderCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(updateFileCount, updateFileCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(updateFolderCount, updateFolderCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deleteFileCount, deleteFileCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deleteFolderCount, deleteFolderCount)
    END_SERIAL_MEMEBER
};

struct RfiDocument {
    std::string     path;
    std::string     mtime;
    std::string     size;
    std::string     inode;
    std::string     id;
    std::string     type;
    std::string     status;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(path, path);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mtime, mtime);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(size, size);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(inode, inode);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, id);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(type, type);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(status, status);
    END_SERIAL_MEMEBER
};

class AGENT_API RfiCtrlParser : public FileParser {

public:
    // Constructor to be used for write rfi file
    explicit RfiCtrlParser(RfiCtrlParserParams& params);

    ~RfiCtrlParser();

    CTRL_FILE_RETCODE OpenWrite() override;

    CTRL_FILE_RETCODE CloseWrite() override;

    CTRL_FILE_RETCODE FlushToFile() override;
    
    CTRL_FILE_RETCODE ReadHeader() override;

    CTRL_FILE_RETCODE ValidateHeader() override;

    CTRL_FILE_RETCODE WriteHeader() override;

    std::string GetRfiZipFileName();

    CTRL_FILE_RETCODE WriteUpdateDirMeta(const Module::DirCache& dcache1, const Module::DirMetaWrapper& dmeta1,
        const Module::DirCache& dcache2, const Module::DirMetaWrapper& dmeta2);
    CTRL_FILE_RETCODE WriteUpdateFileMeta(const Module::FileCache& fcache1, const Module::FileMetaWrapper& fmeta1,
        const Module::FileCache& fcache2,const Module::FileMetaWrapper& fmeta2, const std::string& prePath);
    CTRL_FILE_RETCODE WriteSingleDirMeta(const Module::DirCache& dcache,const Module::DirMetaWrapper& dmeta,
        RfiEntryStatus status);
    CTRL_FILE_RETCODE WriteSingleFileMeta(const Module::FileCache& fcache,const Module::FileMetaWrapper& fmeta,
        const std::string& prePath, RfiEntryStatus status);
    CTRL_FILE_RETCODE GenerateZip();
    /**
        * Check the number of entries in the control file
        */
    uint32_t GetEntries();

private:
    CTRL_FILE_RETCODE WriteRfiHeader();
    CTRL_FILE_RETCODE WriteDirMeta(const Module::DirCache& dirCache, const Module::DirMetaWrapper& dirMeta,
        RfiEntryStatus status);
    CTRL_FILE_RETCODE WriteFileMeta(const Module::FileCache& fcache, const Module::FileMetaWrapper& fileMeta,
        const std::string& prePath, RfiEntryStatus status);
    void FillRfiFileDoc(const FileCache& fcache, const FileMetaWrapper& fileMeta,
        const std::string& filePath, RfiEntryStatus status, RfiDocument& document);
    void WriteStatisticInfo();
    std::string GetRfiFileName();
    bool CheckMultiSystemRootDirExist(std::string& fileName);
    bool NeedCheckMultiSystemRootDirExist();
    void PrepareStatisticType(StatisticType statisticType, bool takeCharge);
    void ExecStatisticType();
    std::string GetFileOrDirNameFromXMeta(const std::vector<XMetaField> &xMeta);
    void CheckZipInstalled();


private:
    std::mutex m_lock;
    uint32_t m_maxEntriesPerFile {0};
    uint32_t m_entries {0};
    bool m_filterFlag {false};              /* filter dtree directory */
    std::string m_filterKey;                  /* filter dtree directory */
    std::string m_version {};
    StatisticType m_statisticType {};
    bool m_takeCharge {true};                   /* 为了解决 update 会算两次 */
    IndexStatisticData m_statistic {};
    bool m_zipAvailable {false};
    CRC32 m_crc32 {};
};
};

#endif