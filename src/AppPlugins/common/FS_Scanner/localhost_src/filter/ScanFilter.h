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
#ifndef SCAN_FILTER_H
#define SCAN_FILTER_H

#include <vector>
#include <regex>
#include <mutex>
#include "ScanStructs.h"
#include "ScanConfig.h"

using EnqueueEntry = std::pair<std::string, uint8_t>;
using EnqueueEntryList =  std::vector<std::pair<std::string, uint8_t>>;

const uint8_t FLAG_EXCLUDE = 0x0; // scan in exclude mode
const uint8_t FLAG_MAYBE = 0x0; // appears when include /dir/*/*, /dir/dir1 may be accepted, or may be skiped
const uint8_t FLAG_NON_RECURSIVE = 0x1; // don't scan this dir, only save meta of dir
const uint8_t FLAG_RECURSIVE = 0x2; // scan this dir recursively
const uint8_t FLAG_DIR = 0x4; // accept sub dir of this dir
const uint8_t FLAG_FILE = 0x8; // accept file of this dir
const uint8_t FLAG_FILE_FLTR = 0x10; // need to check filters to determine whether to accept this file or not
const uint8_t FLAG_ACCEPT_ALL = FLAG_RECURSIVE | FLAG_DIR | FLAG_FILE; // accept all items recursively, no need to check

struct FilterItem {
    std::string m_path {};
    uint8_t m_flag {};
    std::regex m_pattern {};
    bool m_hasWildcard { false };
    bool m_caseSensitive { true };
    std::string m_pathSeparatorStr = "/";

    FilterItem(const std::string &path, uint8_t filterFlag, bool caseSensitive, const std::string& separator);
    bool MatchFile(std::string filePath) const;
    bool MatchDir(std::string dirPath) const;
    std::string ToString() const;
};

class ScanFilter {
public:
    ScanFilter();
    ScanFilter(const ScanDirectoryFilter &dirFilter, const ScanFileFilter &fileFilter, bool win32Path = false);
    void EnableNasPathSyntax(NAS_PROTOCOL pathSyntax);
    void SetCaseSensitive(bool caseSensitive);
    void Enqueue(std::string dirPath, const std::string &prefix = "");

    void InitEnqueueEntryList();
    EnqueueEntryList GetEnqueueEntryList();
    bool AllFiltersDisabled() const;
    bool ShouldStopTraverse(uint8_t filterFlag) const;
    bool AcceptDir(DirStat &dirStat, uint8_t baseFilterFlag);
    bool AcceptFile(std::string filePath, uint8_t baseFilterFlag, const std::string &prefix = "") const;
    bool AcceptDir(std::string dirPath, const std::string &prefix = "") const;
    bool AcceptFile(std::string filePath, const std::string &prefix = "") const;
    bool DiscardDirectory(int fileCount, std::string dirPath, uint8_t baseFilterFlag, const std::string &prefix = "");
    bool DiscardDirectory(int fileCount, std::string dirPath, const std::string &prefix = "");
    std::vector<std::string> CheckMissingParentDirectory(const std::string& dirPath);
private:
    // fields need to be init from constructor
    std::vector<std::string> m_baseEnqueueList {};
    ScanDirectoryFilter m_inputDirFilter {};
    ScanFileFilter m_inputFileFilter {};

    bool m_nasPathSyntaxEnabled { false }; // if need to convert path to nas path syntax
    NAS_PROTOCOL m_nasPathSyntax {NAS_PROTOCOL::NFS};
    bool m_caseSensitive { true };

    bool m_win32PathFormat { false }; // only used on windows host scan, default false
    std::string m_pathSeparatorStr; // using unix path separator by default

    // generated
    std::vector<FilterItem> m_fileFilters {};
    std::vector<FilterItem> m_dirFilters {};
    EnqueueEntryList m_enqueueEntryList {};
    FILTER_TYPE m_mode {FILTER_TYPE::DISABLED};

    // concurrent mutable variables
    std::mutex m_discardDirSetMutex {};
    std::set<std::string> m_discardDirsSet {};

private:
    void ParseToFilterItem(const std::string& filterPath, bool isDir);

    bool FileFilterMatched(const std::string &filePath) const;
    bool DirFilterMatched(const std::string &dirPath) const;
    bool DirFilterMatched(const std::string &dirPath, uint8_t &filterFlag) const;

    void InitFilterItems();
    void ResolveRawInput();
    void ResolveInputFilters();
    // Dir include, File include
    void ResolveInputFilterDIncFInc(const std::vector<FilterItem> &dirFilters);
    // Dir include, File exclude
    void ResolveInputFilterDIncFExc(const std::vector<FilterItem> &dirFilters);
    // Dir exclude, File include
    void ResolveInputFilterDExcFInc(const std::vector<FilterItem> &dirFilters);

    bool ShouldSetExcludeMode() const;
    void ResolveDisableModeEnqueueEntryList();
    void ResolveExcludeModeEnqueueEntryList();
    void ResolveIncludeModeEnqueueEntryList();

    void ParseToEnqueueList(std::string filterPath, bool isDir);
    void ParseIncludeDirToEnqueueEntry(std::string filterPath);
    void ParseIncludeFileToEnqueueEntry(std::string filterPath);
    void ParseExcludeFileToEnqueueEntry(std::string filterPath);

    bool IncludeDirToEnqueueEntry(const std::string &currentFilterPath, const std::string &dirName,
        int currentLevel, int totalLevel);
    bool IncludeFileToEnqueueEntry(const std::string &currentFilterPath, const std::string &dirName,
        int currentLevel, int totalLevel);
    bool ExcludeFileToEnqueueEntry(const std::string &currentFilterPath, const std::string &dirName);
    bool AddToEnqueueList(std::string enqueuePath, uint8_t filterFlag);
    void ResolveConflictFlagWhenAdd(uint8_t &filterFlag) const;
    void ResolveEnqueueEntryListForHost();

    void LogRawInput() const;
    void LogPreInitState() const;
    void LogPostInitState(const std::string& mode) const;
    void ResolvePathPrefix(const std::string &prefix, std::string &path) const;
    void ResolveNasPathSyntax(std::string& path) const;

    void RecordDiscardDirectory(const std::string& dirPath);
};

#endif
