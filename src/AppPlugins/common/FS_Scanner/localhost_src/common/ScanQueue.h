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
#ifndef FS_SCANNER_SCAN_QUEUE_H
#define FS_SCANNER_SCAN_QUEUE_H

#include <iostream>
#include <thread>
#include <ctime>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <list>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include "ScanStructs.h"
#include "ScanQueueUtils.h"
#include "BufferQueue.h"
#include "IterableQueue.h"
#include "ScannerUtils.h"
#include "FileParser.h"

const uint32_t GLOBAL_QUEUE_WRITE_BUFFER_MAX_SIZE = (1024 * 1024);   // 1 MB
const std::string DIRSTAT_FILE_NAME = "directoryqueue";

class ScanQueue : public Module::FileParser {
public:
    IterableQueue<std::string> m_fileList {};
    std::stringstream m_queueBuffer {};
    std::shared_ptr<BufferQueue<DirStat>> m_input;

    explicit ScanQueue(std::shared_ptr<BufferQueue<DirStat>> input,
        std::string scanOutputDirectory,
        uint32_t upperLimit,
        uint32_t lowerLimit);
    ~ScanQueue();

    bool Push(DirStat &dirStat);
    bool Pop(DirStat &dirStat);
    bool BlockingPop(DirStat &dirStat, uint32_t timeout);
    bool BlockingPush(DirStat &dirStat, uint32_t timeout);
    bool PopBatch(std::vector<DirStat> &dirStatList, int readCount);

private:
    std::mutex m_dirStatqMtx {};
    uint32_t m_maxSize {0};
    uint32_t m_upperLimit {10000};
    uint32_t m_lowerLimit {8000};
    uint32_t m_numOfFilesCreated {0};
    uint32_t m_bufferSize {0};
    uint32_t m_totalLengthWritten {0};
    std::string m_scanOutputDirectory;

    bool WriteDirStat(const DirStat &dirStat);
    bool WriteBufferToFile();
    bool ReadFromDirStatFile(const std::string &fileName);
    void ReadDirStatFiles();
    void GetDirStatFileName();
    Module::CTRL_FILE_RETCODE OpenWrite() override;
    Module::CTRL_FILE_RETCODE CloseWrite() override;
    Module::CTRL_FILE_RETCODE ReadHeader() override;
    Module::CTRL_FILE_RETCODE WriteHeader() override;
    Module::CTRL_FILE_RETCODE FlushToFile() override;
    Module::CTRL_FILE_RETCODE ValidateHeader() override;
};

#endif // FS_SCANNER_SCAN_QUEUE_H