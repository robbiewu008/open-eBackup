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
#ifndef DME_NAS_SCANNER_COMMONSERVICE_H
#define DME_NAS_SCANNER_COMMONSERVICE_H

#include <utility>
#ifndef WIN32
#include <bits/stdc++.h>
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include "securec.h"
#include "log/Log.h"
#include "Crc32.h"
#include "MetaParser.h"
#include "XMetaParser.h"
#include "FileCacheParser.h"
#include "DirCacheParser.h"
#include "CopyCtrlParser.h"
#include "DeleteCtrlParser.h"
#include "HardlinkCtrlParser.h"
#include "MtimeCtrlParser.h"
#include "ControlFileUtils.h"
#include "OutputStats.h"
#include "HardlinkManager.h"
#include "ScanStructs.h"
#include "StatisticsMgr.h"
#include "CtrlFileFilter.h"
#include "AdsParser.h"
#include "ScannerUtils.h"
#include "ParserStructs.h"

const int DIRCACHE_MAX_SIZE = 100000;  /* Max directory cache entries stored in memory */
const int FILECACHE_MAX_SIZE = 100000;  /* Max file cache entries store in memory */
const int FOUR_KB = 4096;
const int32_t ONE_MBYTES = 1024 * 1024;
const int DIRCRC_MAX_SIZE = ONE_MBYTES - 32;

struct TmpFMWrapperFiles {
    TmpFMWrapperFiles(const std::string& tmpfcacheFileName,
        const std::string& tmpMetaFileName,
        const std::string& tmpXMetaFileName)
        :   m_tmpfcacheFileName(tmpfcacheFileName),
            m_tmpMetaFileName(tmpMetaFileName),
            m_tmpXMetaFileName(tmpXMetaFileName) {}
    ~TmpFMWrapperFiles() {}

    std::string m_tmpfcacheFileName = "";
    std::string m_tmpMetaFileName = "";
    std::string m_tmpXMetaFileName = "";
};

class NewMergeFMWrapperObj {
public:
    NewMergeFMWrapperObj() = default;
    ~NewMergeFMWrapperObj()
    {}

    void Close()
    {
        for (uint32_t i = 0; i < m_metaFiles.size(); i++) {
            if (m_metaFiles[i]) {
                m_metaFiles[i]->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
        for (uint32_t i = 0; i < m_xMetaFiles.size(); i++) {
            if (m_xMetaFiles[i]) {
                m_xMetaFiles[i]->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
        for (uint32_t i = 0; i < m_fcacheFiles.size(); i++) {
            if (m_fcacheFiles[i]) {
                m_fcacheFiles[i]->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
    }

    void Clear()
    {
        Close();
        for (uint32_t i = 0; i < m_metaFiles.size(); i++) {
            if (m_metaFiles[i]) {
                FS_SCANNER::RemoveFile(m_metaFiles[i]->GetFileName());
            }
        }
        for (uint32_t i = 0; i < m_xMetaFiles.size(); i++) {
            if (m_xMetaFiles[i]) {
                FS_SCANNER::RemoveFile(m_xMetaFiles[i]->GetFileName());
            }
        }
        for (uint32_t i = 0; i < m_fcacheFiles.size(); i++) {
            if (m_fcacheFiles[i]) {
                FS_SCANNER::RemoveFile(m_fcacheFiles[i]->GetFileName());
            }
        }
    }
    std::vector<std::shared_ptr<Module::FileCacheParser>> m_fcacheFiles{};
    std::vector<std::shared_ptr<Module::MetaParser>> m_metaFiles{};
    std::vector<std::shared_ptr<Module::XMetaParser>> m_xMetaFiles{};
};

class CommonService : public ControlFileUtils {
public:
    using FCacheQueueType
        = std::priority_queue<Module::FileCache, std::vector<Module::FileCache>, Module::FileCacheParser::Comparator>;
    using DCacheQueueType
        = std::priority_queue<Module::DirCache, std::vector<Module::DirCache>, Module::DirCacheParser::Comparator>;
    using FMWrapperQueueType
        = std::priority_queue<Module::FileMetaWrapper, std::vector<Module::FileMetaWrapper>,
            Module::CompareFileMetaWrapper>;

    using FCacheQueueFileCtrType = std::priority_queue<Module::FileCacheRecord, std::vector<Module::FileCacheRecord>,
        Module::FileCacheParser::FileCacheComparator>;

    CommonService() = default;
    CommonService(int threadId, ScanConfig &config, ScanInfo &info, std::shared_ptr<StatisticsMgr> statsMgr);
    ~CommonService() override;
    int Init();
    bool m_isTaskRunning = false;
    void Exit();
    void CleanData();
    void CleanData2();
    void WriteToControlFile();
    int WriteToControlBuffer();
    int WriteDirToControlBuffer(std::string dirType);
    void WriteToHardlinkCtlFile();
    void GetHardlinkMaps(std::map<uint64_t, std::vector<Module::HardlinkFileCache>> &hardLinkMap,
        std::map<std::string, uint32_t> &hardlinkFilesCntOfDirPathMap);
    void RestoreHardLinkFCacheMap(std::queue<Module::HardlinkFileCache> &hardLinkFCacheQue);
    void RestoreHardLinkFileCntOfDirPathMap(std::vector<std::pair<std::string, uint32_t>> &hardlinkFilesCntList);

    /*
    * Add entry to file meta
    */
    int PushToFilemeta(const DirectoryScan& dscan, Module::FileMetaWrapper &fmWrapper, const std::string& dirName);
    void FileCrcCalculation(const std::string &fileName, Module::FileCache &fc, char* &str);
    int CheckFileModified(Module::FileMetaWrapper &fmWrapper, Module::FileCache &fc);
    void VerifyAndWriteToControlFile(int &ret, int64_t &stime);

    /*
    * Is write completed
    */
    bool IsWriteCompleted() const;
    /*
    * Write file meta to file
    */
    void WriteToFileMetaFile();

    bool WriteToTmpFMWrapperFile(FMWrapperQueueType& queue, uint64_t dirInode);

    /*
    * Once the scan completes for a directory then
    * write all the file cache entries to file
    */
    int WriteToFileCacheFileBuffer(int32_t &dataLen);
    /*
    * Check Buffer reached DIRCRC_MAX_SIZE and Reset the Buffer
    */
    int CheckAndResetDirCrcBuffer(int32_t &dataLen);

    /*
    * Add new directory cache entry to queue
    * if the queue is full then write queue to file.
    */
    void PushToDirCache(Module::DirCache dircache);

    /*
    * Set the directory meta object
    */
    void SetDirectoryMeta(Module::DirMetaWrapper &dmWrapper);

    /*
    * Write directory cache queue to file if teh queue is full. then add that
    * filename to list.
    */
    void WriteToTmpDirCacheFile();

    /*
    * Write all the inmemory data to file once the dirctory scan complets
    */
    int DirScanCompleted(DirectoryScan &dscan);
    void FlushDataToFile();

    int WriteFileEntryToControlBuffer(
        Module::FileMetaWrapper &fmWrapper, std::string modType, uint32_t aclFlag, Module::FileCache &fc);
    int HandleHardlinkModifiedFile(Module::FileMetaWrapper &fmWrapper, Module::FileCache &fcache, std::string dirPath,
        uint32_t aclFlag, std::string metafileName);
    int ConstructDirCacheStruct();
    int ComputeDirCrcStr(const Module::DirMetaWrapper& dmWrapper);
    char* ComputeFileCrcStr(const Module::FileMetaWrapper& fmWrapper);
    int ConstructFileCacheStruct(Module::FileCache &fCache, Module::FileMetaWrapper &fmWrapper,
        uint16_t metaLength);
    void MergeAllDirCacheFile();
    void SetDirScanObj(DirectoryScan dirObj);
    void AddDirScanObjList(std::vector<DirectoryScan> &dirObjList);
    void Exec();
    void PushChkPntObjToInBfrList(DirectoryScan &dscan);
    void FlushAllData(bool checkTime);
    bool UpdateMetaFileCountInFile();
    bool CreateNewMetaFile();
    bool CreateNewXMetaFile();
    bool CreateNewMetaFileCheck();
    bool CreateNewFcacheFile();
    std::size_t GetDirectoryQueueSize() const;
    void GetMetaFileIndex();
    void GetXMetaFileIndex();
    void GetFcacheFileIndex();
    bool IsLastBackupTimeCheckEnabled() const;
    int  FlushHardlinkMap();
    int CloseDeleteCtrlFile() const;
    void SetTaskStatus(bool writeJob);
    int FlushMtimeFile();
    bool NeedSkipFile(const std::string& path);
    int MergeTmpFMWrapperEntries(int32_t& dataLen);
    int WriteTmpFMWrapperToBufferIfSHA1(int32_t& dataLen);
    int WriteTmpFMWrapperToBufferIfCRC(int32_t& dataLen);
    void FlushToFile();
    std::size_t GetRequestId() const
    {
        return m_config.reqID;
    }

#ifdef WIN32
    void HandleStreamData(Module::FileMetaWrapper& fmWrapper);
    void HandleReadStreamData(const std::string& fileName, int streamIndex, int totalStreams);
    void WriteAdsEntry(char* buffer, DWORD bytesRead, int streamIndex, int totalStreams, int& currentBlock);
#endif

    void IterTmpFMWrapperInner(Module::FileMetaWrapper &fmWrapper, Module::FileCache &fcache, int32_t &dataLen);
    bool WriteFMWrapperFile(FMWrapperQueueType& queue, int64_t fileIndex, std::shared_ptr<Module::XMetaParser> tmpXMetaObj,
        std::shared_ptr<Module::MetaParser> tmpMetaObj, std::shared_ptr<Module::FileCacheParser> tmpFcacheObj);
    bool ReadBatchFileCacheEntries(
        const NewMergeFMWrapperObj &objs, std::vector<std::queue<Module::FileCache>> &vecQueueFiles);
    bool PopMinFcacheAndWrite(const NewMergeFMWrapperObj &objs, Module::FileCache &prevFcache,
        std::vector<std::queue<Module::FileCache>> &vecQueueFiles, int32_t &dataLen);
    bool WriteFcacheToFile(Module::FileCache &fcache, Module::FileCache &prevFcache, int32_t &dataLen,
        std::shared_ptr<Module::MetaParser> metaObj, std::shared_ptr<Module::XMetaParser> xMetaObj);
    bool MergeTmpFMWrapperFileEx(const NewMergeFMWrapperObj &objs, int32_t &dataLen);
    bool PreMergeFMWrapperFileEx(NewMergeFMWrapperObj &mergeObj);
    bool MergeTmpFMWrapperFilesEx(int32_t &dataLen);

private:
    std::shared_ptr<Module::CopyCtrlParser> m_pCopyCtrlParser = nullptr;
    std::shared_ptr<Module::HardlinkCtrlParser> m_pHardlinkCtrlParser = nullptr;
    std::shared_ptr<Module::MtimeCtrlParser> m_pMtimeCtrlParser = nullptr;
    std::shared_ptr<Module::DeleteCtrlParser> m_pDeleteCtrlParser = nullptr;
    Module::CopyCtrlParser::Params m_params {}; /* Backup Control params */
    Module::HardlinkCtrlParser::Params m_hardLinkParams {}; /* HardLink Control params */
    bool m_writeJobTask = true;
    bool m_writeCompletedFlag = true;
    int m_threadId = 0;   /* Thread id */
    ScanConfig &m_config;
    ScanInfo &m_scanInfo;
    uint32_t m_totalFiles = 0; /* Total files */
    /* Sorted dircache queue */
    DCacheQueueType m_dirCacheQueue {};
    
    /* Sorted file cache queue */
    FCacheQueueType m_fileCacheQueue {};

    /* Sorted file meta wrapper queue */
    FMWrapperQueueType m_fileMetaQueue {}; /* File meta wrapper queue */
    
    std::priority_queue<Module::FileCache, std::vector<Module::FileCache>,
        Module::FileCacheParser::ComparatorV20> m_fileCacheQueueV20 {};
    std::deque<std::string> m_fileCacheFileList {}; /* File cache file list */

    std::deque<TmpFMWrapperFiles> m_tmpFMWrapperFileList{};
    // <dir_inode, std::deque<TmpFMWrapperFiles>>
    std::unordered_map<uint64_t, std::deque<TmpFMWrapperFiles>> m_tmpDirFMWrapperFileList;
    std::string m_directory = "";   /* base directory path */
    std::string m_prevDirectory = ""; /* previos base directory */
    std::string m_controlFileDirectory = ""; /* control file base directory */
    Module::DirMetaWrapper m_dmWrapper {};  /* Directory meta wrapper object */
    Module::DirCache m_dirCache {};    /* Directory cache object */
    uint32_t m_metaFileCount = 0;
    uint32_t m_xMetaFileCount = 0;
    uint32_t m_fcacheFileCount = 0;
    uint32_t m_adsMetaFileCount = 0;
    bool isDelDirEntryAdded = false; /* Flag to check dir entry already written or not */
    int64_t m_fileCacheIndex = 0;
    int m_adsMetaFileIndex = 0;
    std::condition_variable m_writeComplete {};
    std::mutex m_writeCompleteMtx {};
    std::queue<DirectoryScan> m_directoryScanQueue {};
    Crc32 m_crc32 {};
    SHA_CTX m_sha1Ctx;
    OutputStats m_opStats {};
    HardlinkManager m_hardlinkManager {};
    bool m_exitThreadFlag = false;
    int64_t m_metaWriteTime = 0;
    int64_t m_xmetaWriteTime = 0;
    int64_t m_fcacheWriteTime = 0;
    int64_t m_dcacheWriteTime = 0;
    int64_t m_ctrlBufferTime = 0;
    std::mutex mtxDirectoryScanQueue {};
    bool m_hardLinkFilePreent = false;
    char *m_dirCrcStr = nullptr;
    std::shared_ptr<Module::MetaParser> m_pMetaParser = nullptr;
    std::shared_ptr<Module::XMetaParser> m_pXMetaParser = nullptr;
    std::shared_ptr<Module::AdsParser> m_pAdsMetaParser = nullptr;
    std::shared_ptr<Module::DirCacheParser> m_pDirCacheParser = nullptr;
    std::shared_ptr<Module::FileCacheParser> m_pFileCacheParser = nullptr;
    std::queue<Module::DirCache> GetDirCacheEntries(std::ifstream &file1);
    int InitControlFile();
    int InitHardLinkControlFile();
    int InitMetaFile();
    int InitXMetaFile();
    int InitFcacheFile();
    int InitDcacheFile();
    int InitAdsMetaFile();
    int InitMtimeFile();
    int InitDeletCtrlFile();
    int WriteDirectoryMtimeCtrlFile(Module::DirMetaWrapper &dmWrapper);
    int CloseMtimeFile();
    int WriteDeleteDirectoryEntry(std::string &path, int delFlag);
    int WriteDeleteFileEntry(std::string &fileName, std::string &path);
    std::shared_ptr<StatisticsMgr> m_statsMgr;
    std::shared_ptr<CtrlFileFilter> m_ctrlFileFilter;
    std::unordered_map<uint64_t, FMWrapperQueueType> m_fileMetaWrapperMap;
};

#endif // DME_NAS_SCANNER_COMMONSERVICE_H
