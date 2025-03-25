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
#ifndef DME_NAS_SCANNER_CHECK_POINT_H
#define DME_NAS_SCANNER_CHECK_POINT_H

#ifndef WIN32
#include <bits/stdc++.h>
#include <dirent.h>
#endif
#include <sys/types.h>
#include "ParserStructs.h"
#include "ScanStructs.h"
#include "ScanInfo.h"
#include "ScanConfig.h"
#include "ScanQueue.h"
#include "BufferQueue.h"
#include "ScannerTime.h"
#include "ControlFileUtils.h"
#include "ScannerUtils.h"
#include "CheckPointParser.h"
#include "HardlinkCtrlParser.h"
#include "OutputStats.h"
#include "ScanConsts.h"

struct CheckPointData {
public:
    std::string m_path {};
    uint32_t m_filterFlag = 0;
};

class FSScannerCheckPoint {
public:
    std::vector<struct CheckPointData> m_scanInProgressList {};
    bool m_isProcessingQueuesDone = false;
    bool m_ischkPntFlushDone = false;
    std::mutex m_checkpointMtx {};
    std::string m_prefix {};

    // Constructor
    FSScannerCheckPoint(ScanConfig &config, ScanInfo &info);

    // Destructor
    ~FSScannerCheckPoint() = default;

    // Get the status of Check Point
    CHECKPOINT_STATUS GetCheckPointStatus();

    // Checks whether Check Point Directory Exists or not
    bool IsCheckPointDirExists() const;

    // Checks the Scan Restarted Flag
    bool IsScanRestarted() const;

    // Init FSScannerCheckPoint by creating checkPoint Directory and File.This called inside ScannerCheckPoint() API
    CHECKPOINT_STATUS InitCheckPoint();

    // Remove FSScannerCheckPoint Directory after Scan completion
    CHECKPOINT_STATUS RemoveCheckPointDir();

    // Update FSScannerCheckPoint Time
    void UpdateCheckPointTime();

    /**** Write APIs ****/

    // Iterate ScanQueue and call WriteCheckPointEntryToFile
    void ProcessScanQueue(std::shared_ptr<BufferQueue<DirStat>> scanQueue);

    // Iterate Output Buffer Queue and call WriteCheckPointEntryToFile
    void ProcessOutputQueue(std::shared_ptr<BufferQueue<DirectoryScan>> bfrQueue);

    // Iterate ScanInProgressList and call WriteCheckPointEntryToFile
    void ProcessScanInProgressList();

    // Iterate FilterEnqueueList and call WriteCheckPointEntryToFile
    void ProcessFilterEnqList(std::vector<std::pair<std::string, uint8_t>> &enqueueEntryList);

    // Read Entries Present In Scan Queue Buffer and call WriteCheckPointEntryToFile
    void ProcessScanQueueBuffer(const std::stringstream &scanQueueBuffer);

    // Iterate Scan Queue FileList and Read Entries Present in those files and call WriteCheckPointEntryToFile
    void ProcessScanQueueFiles(IterableQueue<std::string> &scanQueuefileList);

    // Close FSScannerCheckPoint file by calling Close API of ScannerCheckPoint
    // class and rename directory from /latest/checkpoint_tmp/ to /latest/checkpoint/
    CHECKPOINT_STATUS EndCheckPoint();

    CHECKPOINT_STATUS InitHardLinkControlFile();
    CHECKPOINT_STATUS WriteHardLinkMap(const std::map<uint64_t, std::vector<Module::HardlinkFileCache>> &hardLinkMap,
        std::map<std::string, uint32_t> &hardlinkFilesCntOfDirPathMap);

    /***** Read APIs *****/

    // Read FSScannerCheckPoint Directory and collect checkpoint_*.txt files in vector
    CHECKPOINT_STATUS ReadCheckPointDirectory(std::vector<std::string> &checkPointFiles);

    // Read FSScannerCheckPoint File and collect CheckPointData
    // entries to temporary List and In Caller, iterate list and Enqueue
    CHECKPOINT_STATUS ReadCheckPointFile(std::string chkPntFileName, std::vector<CheckPointData> &chkPntDataList);

    CHECKPOINT_STATUS GetHardLinkCtrlFiles(std::vector<std::string> &hardLinkCtrlFiles);

    CHECKPOINT_STATUS ReadHardLinkCtrlFile(std::string hardLinkCtrlFile,
        std::queue<Module::HardlinkFileCache> &hardLinkFCacheQue,
        std::vector<std::pair<std::string, uint32_t>> &hardlinkFilesCntList);

private:
    ScanInfo &m_info;
    ScanConfig &m_config;
    int m_chkPntEntriesCnt = 0;
    bool m_isScanRestarted = false;
    std::unique_ptr<Module::CheckPointParser> m_pCheckPointParser = nullptr;
    std::unique_ptr<Module::HardlinkCtrlParser> m_pHardlinkCtrlParser = nullptr;
    Module::CheckPointParser::Params m_params {}; /* FSScannerCheckPoint Parser params */
    Module::HardlinkCtrlParser::Params m_hardLinkParams {}; /* HardLink Control params */
    std::atomic_uint64_t m_lastCheckpointTime {};
    bool m_isCheckPointRunning = false;
    uint16_t m_chkPntFileCnt = 0;
    std::map<std::string, bool> m_superDirsMap {};

    // This API will get the check point file name
    std::string GetCheckPointFileName();

    // create Temperory Checkpoint Directory - "m_metaPath + /latest/checkpoint_tmp/"
    CHECKPOINT_STATUS CreateTempCheckPointDirectory();

    // Create FSScannerCheckPoint File by calling ScannerCheckPoint Class in Parser(MODULE) folder
    CHECKPOINT_STATUS CreateCheckPointFile();

    // Write FSScannerCheckPoint Entry To FSScannerCheckPoint File by calling
    // m_scannerChkPoint->WriteChkPntEntry(entry), If scanType is NFS, it'll call FSScannerCheckPoint Poll
    CHECKPOINT_STATUS WriteCheckPointEntryToFile(std::string chkPntEntry);

    // Read Latest Directory and update the file Indices of meta files
    CHECKPOINT_STATUS ReadLatestDirectory();

    // Parse the given dirPath and search for existence of super dirs
    bool IsSuperDirExists(const std::string &dirPath);

    void WriteScanQueueEntries(std::stringstream &scanQueueBuffer);

    CHECKPOINT_STATUS CreateAndOpenHardLinkFile();
    CHECKPOINT_STATUS CloseAndRenameHardLinkFile();
};
#endif //DME_NAS_SCANNER_DIRSTAT_QUEUE_H