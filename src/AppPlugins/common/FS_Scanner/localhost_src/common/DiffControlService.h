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
#ifndef FS_SCANNER_DIFF_CTRL_SERVICE_H
#define FS_SCANNER_DIFF_CTRL_SERVICE_H

#include <utility>
#ifndef WIN32
#include <bits/stdc++.h>
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
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
#include "RfiCtrlParser.h"
#include "CtrlFileFilter.h"
#include "CtrlFilterManager.h"

const int16_t CUR_SCAN_DIR = 1;  /* Current scan directory */
const int16_t PREV_SCAN_DIR = 2;  /* Previous scan directory */

enum class SkipType {
    SKIP_DIR,
    SKIP_FILE,
};

class MetaFileManager {
public:
    ~MetaFileManager()
    {
        for (std::shared_ptr<Module::FileCacheParser> fcacheFile : m_fcacheCurFiles) {
            if (fcacheFile) {
                fcacheFile->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
        for (std::shared_ptr<Module::FileCacheParser> fcacheFile : m_fcachePrevFiles) {
            if (fcacheFile) {
                fcacheFile->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
        for (std::shared_ptr<Module::MetaParser> metaFile : m_metaCurFiles) {
            if (metaFile) {
                metaFile->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
        for (std::shared_ptr<Module::MetaParser> metaFile : m_metaPrevFiles) {
            if (metaFile) {
                metaFile->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
        for (std::shared_ptr<Module::XMetaParser> xmetafile : m_xMetaCurFiles) {
            if (xmetafile) {
                xmetafile->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
        for (std::shared_ptr<Module::XMetaParser> xmetafile : m_xMetaPrevFiles) {
            if (xmetafile) {
                xmetafile->Close(Module::CTRL_FILE_OPEN_MODE::READ);
            }
        }
    };

    uint32_t m_prevMetaFileCount = 0;
    uint32_t m_curMetaFileCount = 0;
    uint32_t m_prevXMetaFileCount = 0;
    uint32_t m_curXMetaFileCount = 0;
    uint32_t m_prevFcacheFileCount = 0;
    uint32_t m_curFcacheFileCount = 0;
    std::string m_curMetaVersion;
    std::string m_prevMetaVersion;
    std::string m_curMetaDir;
    std::string m_prevMetaDir;
    std::vector<std::shared_ptr<Module::FileCacheParser>> m_fcacheCurFiles {};
    std::vector<std::shared_ptr<Module::FileCacheParser>> m_fcachePrevFiles {};
    std::vector<std::shared_ptr<Module::MetaParser>> m_metaCurFiles {};
    std::vector<std::shared_ptr<Module::MetaParser>> m_metaPrevFiles {};
    std::vector<std::shared_ptr<Module::XMetaParser>> m_xMetaCurFiles {};
    std::vector<std::shared_ptr<Module::XMetaParser>> m_xMetaPrevFiles {};
};

// TrieNode 类用于表示路径树的节点
class TrieNode {
public:
    std::unordered_map<std::string, std::shared_ptr<TrieNode>> children; // 存储子目录节点
    std::string path;   // 当前路径
    uint32_t tire;      // 当前层级
    uint32_t prevTire;  // 上一个层级

    explicit TrieNode(const std::string& path = "") : path(path) {}

    // 清除当前节点的所有子节点
    void ClearChildren()
    {
        children.clear();
    }
};

class Trie {
public:
    Trie() : m_root(std::make_shared<TrieNode>())
    {
        m_root->tire = 0;
        m_root->prevTire = 0;
    }

    // 插入路径
    void Insert(const std::string& fullPath)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        std::shared_ptr<TrieNode> node = m_root;
        if (fullPath == "/") {
            node->ClearChildren();
            node->prevTire = node->tire + 1;
            return;
        }
        std::vector<std::string> parts = splitPath(fullPath); // 分割路径
        std::string currentPath = "";
        for (size_t i = 0; i < parts.size(); ++i) {
            currentPath += "/" + parts[i];
            // 如果当前部分还不存在于 Trie 中
            if (node->children.find(parts[i]) == node->children.end() && node->prevTire <= node->tire) {
                node->children[parts[i]] = std::make_shared<TrieNode>(currentPath); // 创建新节点
                node->children[parts[i]]->tire = node->tire + 1;
                node->children[parts[i]]->prevTire = node->tire;
                // 将每次fullPath的最后一个层级设为已裁剪，防止最短的路径最先插入导致错误
                if (i == parts.size() - 1) {
                    node->prevTire = node->tire + 1;
                }
            }
            // 说明当前node已经是裁剪过的， 已经包含父目录， 直接返回
            if (node->prevTire > node->tire) {
                return;
            }

            node = node->children[parts[i]];

            // 如果当前节点已有子节点，且我们已到达该父路径的最后一级，则清除之前的子节点
            if (i == parts.size() - 1 && !node->children.empty()) {
                node->ClearChildren();
                node->prevTire = node->tire + 1;
            }
        }
    }

    // 遍历前缀树并输出所有父路径
    std::vector<std::string> GetAllParentPaths()
    {
        std::vector<std::string> paths;
        traverse(m_root, paths);
        return paths;
    }

private:
    // 分割路径为目录部分
    std::vector<std::string> splitPath(const std::string& path)
    {
        std::vector<std::string> parts;
        std::stringstream ss(path);
        std::string part;

        while (getline(ss, part, '/')) {
            if (!part.empty()) {
                parts.push_back(part);
            }
        }
        return parts;
    }

    // 递归遍历 Trie，并输出父路径
    void traverse(std::shared_ptr<TrieNode> node, std::vector<std::string>& paths)
    {
        if (!node->path.empty() && !node->children.empty()) {
            paths.push_back(node->path);
        }

        // 遍历子节点
        for (const auto& child : node->children) {
            traverse(child.second, paths);
        }
    }
private:
    std::shared_ptr<TrieNode> m_root;  // 根节点
    std::mutex m_lock;
};

class DiffControlInfo {
public:
    DiffControlInfo(std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<Trie> trie,
        std::shared_ptr<MetaFileManager> mgr,
        std::shared_ptr<CtrlFilterManager> filterMgr)
        : m_statsMgr(statsMgr), m_trie(trie), m_metaFileManager(mgr), m_ctrlFilterManager(filterMgr) {}
    std::shared_ptr<StatisticsMgr> m_statsMgr;
    std::shared_ptr<Trie> m_trie;
    std::shared_ptr<MetaFileManager> m_metaFileManager;
    std::shared_ptr<CtrlFilterManager> m_ctrlFilterManager;
};

class DcacheHash {
public:
    Module::DirCache m_dcache {};       /* Dircache object */
    std::string m_path;                         /* Directory path */

    DcacheHash() {};
    explicit DcacheHash(const DcacheHash *dcacheHash)
    {
        m_dcache = dcacheHash->m_dcache;
        m_path = dcacheHash->m_path;
    };
    DcacheHash(Module::DirCache dcache, std::string path)
    {
        m_dcache = dcache;
        m_path = path;
    };
    ~DcacheHash() {};
};

class DcacheComparator {
public:
    int operator() (const DcacheHash p1, const DcacheHash p2)
    {
        return p1.m_path > p2.m_path;
    }
};

class FcacheHash {
public:
    Module::FileCache m_fcache {};       /* Fcache object */
    std::string m_name {};                    /* File inode */

    FcacheHash() {};
    explicit FcacheHash(const FcacheHash *fcacheHash)
    {
        m_fcache = fcacheHash->m_fcache;
        m_name = fcacheHash->m_name;
    };
    FcacheHash(Module::FileCache fcache, std::string name)
    {
        m_fcache = fcache;
        m_name = name;
    };
    ~FcacheHash() {};
};

class FcacheComparator {
public:
    int operator() (const FcacheHash p1, const FcacheHash p2)
    {
        return p1.m_name > p2.m_name;
    }
};

class CompareDirectory {
public:
    CompareDirectory(Module::DirCache curDcache, Module::DirCache prevDcache, std::string diffFlag)
    {
        m_curDcache = curDcache;
        m_prevDcache = prevDcache;
        m_diffFlag = diffFlag;
    }
    CompareDirectory() {};
    Module::DirCache m_curDcache {};
    Module::DirCache m_prevDcache {};
    std::string m_diffFlag {};
};

class IncrementalScanner {
public:
    Module::Hash m_prevHash {0};
    Module::Hash m_fcachePrevHash {0};
    std::queue<Module::DirCache> m_dcQueue1 {};
    std::queue<Module::DirCache> m_dcQueue2 {};
    Module::DirCache m_dcache1 {};
    Module::DirCache m_dcache2 {};
    std::priority_queue<DcacheHash, std::vector<DcacheHash>,
        DcacheComparator> m_dirCacheHashCurQueue {};
    std::priority_queue<DcacheHash, std::vector<DcacheHash>,
        DcacheComparator> m_dirCacheHashPrevQueue {};
    std::priority_queue<FcacheHash, std::vector<FcacheHash>,
        FcacheComparator> m_fileCacheHashCurQueue {};
    std::priority_queue<FcacheHash, std::vector<FcacheHash>,
        FcacheComparator> m_fileCacheHashPrevQueue {};
};

class MetadataStat {
public:
    uint32_t m_metaFileCount = 0;
    uint32_t m_xMetaFileCount = 0;
    uint32_t m_fcacheFileCount = 0;

    std::string m_metaVersion;

    std::queue<Module::DirCache> m_dcQueue {};
    Module::DirCache m_dcache {};

    std::vector<std::shared_ptr<Module::FileCacheParser>> m_fcacheFiles {};
    std::vector<std::shared_ptr<Module::MetaParser>> m_metaFiles {};
    std::vector<std::shared_ptr<Module::XMetaParser>> m_xMetaFiles {};

    std::priority_queue<DcacheHash, std::vector<DcacheHash>, DcacheComparator> m_dirCacheHashQueue {};
    std::priority_queue<FcacheHash, std::vector<FcacheHash>, FcacheComparator> m_fileCacheHashQueue {};
};

struct DirModifiedEntry {
    std::string m_dirOutStr {};
    std::string m_dirFlag {};
    uint32_t m_dirAclFlag = 0;
    std::string m_dirModifiedString {};
    int m_dirMetaLength = 0;
    std::string m_dirpath {};
    std::string m_metafileName {};
    uint64_t m_mdataOffset = 0;  /* offset in meta data file */
    uint16_t m_metaFileIndex = 0; /* Meta file index for the directory */
    bool m_writeToCtrlFile = false;
};

class DiffControlService : public ControlFileUtils {
public:
    DiffControlService();
    DiffControlService(int threadId, ScanConfig &config, HardlinkManager &hardLinkManager, const DiffControlInfo& info);
    ~DiffControlService() override;

    int Init();
    void CleanData();
    void WriteToControlFile();

    int DiffScan(std::string finalDcacheFile, bool isFull);
    bool ValidateCurrFileCount();
    bool ValidatePrevFileCount();
    bool ValidateIncFiles(bool isFull);
    bool ValidateIncFiles(std::string currDcacheFile, std::string prevDcacheFile);
    bool OpenCurrentFcacheMetaFiles();
    bool OpenPreviousFcacheMetaFiles();
    bool OpenAllFcacheMetaFiles();
    bool InitDiffService(bool isFull);
    void DiffCompleted();
    void ProcessDirectory(const CompareDirectory &compareObj);

    void PreviousDirNotExists(std::shared_ptr<Module::DirCacheParser> file1);
    void DiffDirMetaAddDir(Module::DirCache &dcache);
    void WriteFcacheEntries(Module::DirCache &dcache1, Module::DirMetaWrapper &dmWrapper,
        Module::CopyCtrlDirEntry &dirEntry, std::string opt);
    bool FillFileMetaWrapper(Module::FileMetaWrapper &fmWrapper,
        const std::tuple<uint16_t&, uint64_t&, uint16_t&> &metaInfo, bool isCurrent);
    bool ReadFileMetaV20(Module::FileMetaWrapper &fmWrapper,
        const std::tuple<uint16_t&, uint64_t&, uint16_t&> &metaInfo, bool isCurrent);
    bool ReadFileMetaV10(Module::FileMetaWrapper &fmWrapper,
        const std::tuple<uint16_t&, uint64_t&, uint16_t&> &metaInfo, bool isCurrent);
    uint32_t ValidateFcacheEntries(Module::CopyCtrlDirEntry &dirEntry,
        Module::DirCache &dcache, Module::DirMetaWrapper &dmWrapper, std::string opt);
    void ValidateAndWriteToCtrlFile(Module::CopyCtrlDirEntry &dirEntry, uint32_t &count,
        Module::FileMetaWrapper &fmWrapper, Module::FileCache &fc1, std::string opt);
    uint32_t CheckValidateFileEntryCount(int count, Module::CopyCtrlDirEntry &dirEntry, Module::DirCache &dcache,
        Module::DirMetaWrapper &dmWrapper, std::string opt);
    void WriteDirectoryToControFile(Module::CopyCtrlDirEntry &dirEntry);

    void FindAllModifiedDirs(std::shared_ptr<Module::DirCacheParser> file1,
        std::shared_ptr<Module::DirCacheParser> file2);
    void WriteNewDirectories(std::queue<Module::DirCache> &currDcacheQueue);
    void WriteDelDirectories(std::queue<Module::DirCache> &prevDcacheQueue);
    void DiffDirMetaDelDir(Module::DirCache &dcache);
    bool IsValidFileCache(Module::FileCache& fc);
    bool NeedSkip(uint16_t metaType, const std::string& prefixPath, const std::vector<Module::XMetaField> &xMeta,
        SkipType type);
    bool NeedSkipDir(const std::string& path);
    bool NeedSkipFile(const std::string& path);
    SCANNER_STATUS VerifyDircacheObj();
    SCANNER_STATUS VerifyDircacheObj(const std::string &flag);
    void CompareDirPathHash(std::queue<Module::DirCache> &dcQueue1, std::queue<Module::DirCache> &dcQueue2);
    void CompareDirPathCRCHash(std::queue<Module::DirCache> &dcQueue1, std::queue<Module::DirCache> &dcQueue2);
    void CompareDirPathSHA1Hash(std::queue<Module::DirCache> &dcQueue1, std::queue<Module::DirCache> &dcQueue2);
    void DiffDirMetaModDir();
    void DiffFcache(DirModifiedEntry &dirModEntry);
    bool DiffFcacheForNewFiles(DirModifiedEntry &dirModEntry, Module::DirCache &dcache);
    bool ReadMetaAndWriteToBuffer(bool isCurrent,
        DirModifiedEntry &dirModEntry, Module::FileCache &fc1, std::string modType);
    int WriteToBuffer(DirModifiedEntry &dirModEntry, Module::FileMetaWrapper &fmWrapper,
        std::string modType, Module::FileCache &fc1);
    void FillDirEntry(Module::CopyCtrlDirEntry &dirEntry, const DirModifiedEntry &dirModEntry, std::string flag);
    int DiffFcacheForDeletedFile(DirModifiedEntry &dirModEntry, Module::DirCache &dcache);
    int DiffFcachMetaChangeforDir(std::string dirFlag, const DirModifiedEntry &dirModEntry);
    void CheckDirWriteStatus(Module::CTRL_FILE_RETCODE result);
    int ReadAndCompareFcacheEntries(std::queue<Module::FileCache> &fcQueue1, std::queue<Module::FileCache> &fcQueue2,
        DirModifiedEntry &dirModEntry, uint32_t prevTotal, uint32_t curTotal, uint64_t nextOffset1, uint64_t nextOffset2);
    SCANNER_STATUS VerifyFcacheObj(Module::FileCache  &fc1, Module::FileCache &fc2);
    int CompareAndWriteAllFiles(DirModifiedEntry &dirModEntry, std::queue<Module::FileCache> &fcQueue1,
        std::queue<Module::FileCache> &fcQueue2);
    int CompareAndWriteAllFilesIfCRC(DirModifiedEntry &dirModEntry, std::queue<Module::FileCache> &fcQueue1,
        std::queue<Module::FileCache> &fcQueue2, Module::FileCache &fc1, Module::FileCache &fc2);
    int CompareAndWriteAllFilesIfSHA1(DirModifiedEntry &dirModEntry, std::queue<Module::FileCache> &fcQueue1,
        std::queue<Module::FileCache> &fcQueue2, Module::FileCache &fc1, Module::FileCache &fc2);
    void WriteDirEntryForDiffFcache(Module::DirCache dcache1, DirModifiedEntry &dirModEntry);

    void CloseIncrementalScanFiles();
    int CompareFileCrc(Module::FileCache &fc1, Module::FileCache &fc2, DirModifiedEntry &dirModEntry);
    void CrcComparisonBtwSameInode(Module::FileMetaWrapper &fmWrapper, Module::FileMetaWrapper &fmWrapper2,
        Module::FileCache &fc, Module::FileCache &fc2, DirModifiedEntry &dirModifiedEntry);
    int VerifyAndWriteDirEntryToCtrlFile(int &result, DirModifiedEntry &dirModifiedEntry);
    int ValidatePreviousDirectory();
    Module::CTRL_FILE_RETCODE HandleHardlinkModifiedFile(Module::FileMetaWrapper &fmWrapper, Module::FileCache &fc,
        std::string dirPath, uint32_t aclFlag, std::string metafileName);
    Module::CTRL_FILE_RETCODE WriteFileEntryToControlBuffer(Module::FileMetaWrapper &fmWrapper, std::string modType,
        uint32_t aclFlag, Module::FileCache &fc);
    int WriteHardlinkEntry(Module::FileMetaWrapper &fmWrapper, Module::FileCache &fc,
        uint32_t aclFlag, DirModifiedEntry &dirModifiedEntry);
    void WriteNewDelFcacheEntries(std::queue<Module::FileCache> &fcQueue, DirModifiedEntry &dirModEntry,
        std::string modType);
    int FlushHardlinkMap();
    int InitHardLinkControlFile();
    void WriteToHardlinkCtlFile();
    bool IsDiffCompleted() const;
    bool NeedSkipForControlGen(uint64_t errNum);

private:
    IncrementalScanner m_incScnr {};
    std::shared_ptr<Module::CopyCtrlParser> m_pCopyCtrlParser = nullptr;
    std::shared_ptr<Module::MtimeCtrlParser> m_pMtimeCtrlParser = nullptr;
    std::shared_ptr<Module::DeleteCtrlParser> m_pDeleteCtrlParser = nullptr;
    std::shared_ptr<Module::HardlinkCtrlParser> m_pHardlinkCtrlParser = nullptr;
    std::shared_ptr<Module::RfiCtrlParser> m_rfiCtrlParser = nullptr;
    Module::CopyCtrlParser::Params m_params {}; /* Backup Control params */
    Module::HardlinkCtrlParser::Params m_hardLinkParams {}; /* HardLink Control params */
    bool isDelDirEntryAdded = false; /* Flag to check dir entry already written or not */
    Module::DirMetaWrapper m_dmWrapper {};  /* Directory meta object */
    int m_threadId = 0;   /* Thread id */
    bool m_isGeneratorFull = false;
    ScanConfig &m_config;
    HardlinkManager &m_hardlinkManager;
    OutputStats m_opStats {};
    std::string m_directory = "";   /* base directory path */
    std::string m_prevDirectory = ""; /* previos base directory */
    std::string m_controlFileDirectory = ""; /* control file base directory */
    bool m_hardLinkFilePreent = false;
    uint64_t m_metaReadTime = 0;
    uint64_t m_ctrlWriteTime = 0;
    uint64_t m_dirMetaReadTime = 0;
    uint64_t m_fcacheReadTime = 0;
    uint64_t m_dirCacheTime = 0;
    SCANNER_STATUS m_status = SCANNER_STATUS::CTRL_DIFF_IN_PROGRESS;
    std::shared_ptr<StatisticsMgr> m_statsMgr;
    std::shared_ptr<CtrlFileFilter> m_ctrlFileFilter;
    bool m_diffCompleted {true};
    std::shared_ptr<Trie> m_trie;     /* 用于记录根目录 */
    std::shared_ptr<MetaFileManager> m_metaFileManager;
    std::shared_ptr<CtrlFilterManager> m_ctrlFilterManager;

    // for rfi files(indexation)
    Module::RfiCtrlParserParams m_rfiParams;
    std::string m_prePath;
    bool m_rfiGenerationComplete {false};

    int InitControlFile();
    int InitMtimeFile();
    int InitDeletCtrlFile();
    int InitRfiFile();
    int WriteDirectoryMtimeCtrlFile(Module::DirMetaWrapper &dmWrapper);
    int CloseMtimeFile();
    int WriteDeleteDirectoryEntry(std::string &path, int delFlag);
    int WriteDeleteFileEntry(std::string &fileName, std::string &path);
    int CloseDeleteCtrlFile() const;
    void HandleSameHashTagEntries(std::queue<Module::DirCache> &dcQueue1, std::queue<Module::DirCache> &dcQueue2,
        bool checkHashTag);
    void HandleSameHashTagEntries(bool checkHashTag);
    void ProcessSameHashTagEntries();
    void HandleNewAndDelDirs(std::queue<Module::DirCache> &dcQueue1, std::queue<Module::DirCache> &dcQueue2,
        bool checkHashTag);
    void PushSameCrcDirs(const std::string& curPath, const std::string& prevPath);
    void SetDirModifiedFlag(DirModifiedEntry &dirModEntry, Module::DirMetaWrapper &dmWrapper,
        Module::DirMetaWrapper &dmWrapperPrev);

    void PushSameCrcFiles(Module::FileCache &curFcache, Module::FileCache &prevFcache,
        Module::FileMetaWrapper fmWrapper, Module::FileMetaWrapper fmWrapper2);
    bool CheckAndAddToQueue(Module::FileCache &fc, const std::string &modType, DirModifiedEntry &dirModEntry,
        Module::FileMetaWrapper &fmWrapper);
    void HandleNewAndDelFiles(DirModifiedEntry &dirModEntry);
    void HandleSameFileHashTagEntries(DirModifiedEntry &dirModEntry,
        Module::FileCache &curFc, Module::FileCache &prevFc, bool checkHashTag);
    bool checkSha1Match(Module::FileCache &curFc, Module::FileCache &prevFc, bool checkHashTag);
    bool CheckFileHashTagSame(Module::FileCache &fc, int16_t scanDirOpt);
    void PrintFcache(Module::FileCache& fileCache);
    bool FillDirMetaWrapper(Module::DirMetaWrapper &dmWrapper,
        std::tuple<uint16_t&, uint64_t&, uint16_t&> &metaInfo, bool isCurrent);
    bool ReadDirMetaV20(Module::DirMetaWrapper &dmWrapper,
        const std::tuple<uint16_t&, uint64_t&, uint16_t&> &metaInfo, bool isCurrent);
    bool ReadDirMetaV10(Module::DirMetaWrapper &dmWrapper,
        const std::tuple<uint16_t&, uint64_t&, uint16_t&> &metaInfo, bool isCurrent);

    // rfi gen methods
    Module::CTRL_FILE_RETCODE WriteDirectoryToRfiFile(const Module::DirCache& dcache,
        const Module::DirMetaWrapper& dmWrapper, Module::RfiEntryStatus status);
    Module::CTRL_FILE_RETCODE WriteFileToRfiFile(const Module::FileCache& fcache,
        const Module::FileMetaWrapper& fmWrapper, Module::RfiEntryStatus status);
    Module::CTRL_FILE_RETCODE WriteUpdateFileToRfiFile(const Module::FileCache& fcache1,
        const Module::FileMetaWrapper& fmWrapper1, const Module::FileCache& fcache2,
        const Module::FileMetaWrapper& fmWrapper2);
    Module::CTRL_FILE_RETCODE WriteUpdateDirectoryToRfiFile(const Module::DirCache& dcache1,
        const Module::DirMetaWrapper& dmWrapper1, const Module::DirCache& dcache2,
        const Module::DirMetaWrapper& dmWrapper2);
    void WriteToRfiFile(bool isComplete);
    void ExecRfiCb(const std::string& rfiPath);
    bool IsDirModified(Module::DirCache curDcache, Module::DirCache prevDcache);
    bool IsFileModified(Module::FileCache curFcache, Module::FileCache prevFcache);
    bool IsHashNotEmpty(Module::Hash &hash);
};

#endif // DME_SCANNER_INCREMENTAL_H