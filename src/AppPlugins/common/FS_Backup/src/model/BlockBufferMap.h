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
#ifndef BLOCK_BUFFER_MAP_H
#define BLOCK_BUFFER_MAP_H

#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include <set>
#include "BackupStructs.h"
#include "log/Log.h"

class BlockBufferMapQueue {
public:

    FileHandle Get(FileHandle &fileHandleKey)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        FileHandle fileHandle;
        std::set<FileHandle>::iterator it = m_set.find(fileHandleKey);
        if (it != m_set.end()) {
            fileHandle = *it;
        }
        return fileHandle;
    }

    FileHandle GetAndPop(FileHandle &fileHandleKey)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        FileHandle fileHandle;
        std::set<FileHandle>::iterator it = m_set.find(fileHandleKey);
        if (it != m_set.end()) {
            fileHandle = *it;
            m_set.erase(it);
        }
        return fileHandle;
    }

    FileHandle GetAndPop()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        FileHandle fileHandle;
        std::set<FileHandle>::iterator it = m_set.begin();
        if (it != m_set.end()) {
            fileHandle = *it;
            m_set.erase(it);
        }
        return fileHandle;
    }

    void Push(FileHandle &fileHandle)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_set.insert(fileHandle);
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_set.empty();
    }

    size_t Size()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_set.size();
    }

public:
    std::set<FileHandle> m_set;
    std::mutex m_mtx;
};

class BlockBufferMap {
public:
    bool Empty()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_blockBufferMap.empty();
    }

    bool Add(std::string &fileName, FileHandle &fileHandle)
    {
        DBGLOG("Add %s", fileName.c_str());
        std::lock_guard<std::mutex> lk(m_mtx);
        std::shared_ptr<BlockBufferMapQueue> bufferMapQueue = GetBufferMapQueue(fileName);
        if (bufferMapQueue == nullptr) {
            bufferMapQueue = std::make_shared<BlockBufferMapQueue>();
            m_blockBufferMap.emplace(make_pair(fileName, bufferMapQueue));
        }
        FileHandle existFileHandle = bufferMapQueue->Get(fileHandle);
        if (IS_VALID_FILEHANDLE(existFileHandle)) {
            return false;
        }
        bufferMapQueue->Push(fileHandle);
        m_blockBufferCount++;
        m_blockBufferSize += fileHandle.m_block.m_size;
        return true;
    }

    bool Delete(std::string &fileName, FileHandle &fileHandle)
    {
        DBGLOG("Delete %s", fileName.c_str());
        std::lock_guard<std::mutex> lk(m_mtx);
        std::shared_ptr<BlockBufferMapQueue> bufferMapQueue = GetBufferMapQueue(fileName);
        if (bufferMapQueue == nullptr) {
            return false;
        }
        FileHandle existFileHandle = bufferMapQueue->GetAndPop(fileHandle);
        if (!IS_VALID_FILEHANDLE(existFileHandle)) {
            return false;
        }
        m_blockBufferCount--;
        m_blockBufferSize -= fileHandle.m_block.m_size;
        delete[] fileHandle.m_block.m_buffer;
        fileHandle.m_block.m_buffer = nullptr;

        if (bufferMapQueue->Empty()) {
            m_blockBufferMap.erase(fileName);
        }
        return true;
    }

    bool Delete(std::string &fileName)
    {
        DBGLOG("Delete %s", fileName.c_str());
        std::lock_guard<std::mutex> lk(m_mtx);
        std::shared_ptr<BlockBufferMapQueue> bufferMapQueue = GetBufferMapQueue(fileName);
        if (bufferMapQueue == nullptr) {
            return false;
        }
        while (!bufferMapQueue->Empty()) {
            FileHandle fileHandle = bufferMapQueue->GetAndPop();
            if (IS_VALID_FILEHANDLE(fileHandle)) {
                m_blockBufferCount--;
                m_blockBufferSize -= fileHandle.m_block.m_size;
                delete[] fileHandle.m_block.m_buffer;
                fileHandle.m_block.m_buffer = nullptr;
            }
        }
        m_blockBufferMap.erase(fileName);
        return true;
    }

    bool Delete(std::vector<FileHandle>& fileHandleList)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        for (auto &fh : fileHandleList) {
            std::shared_ptr<BlockBufferMapQueue> bufferMapQueue = GetBufferMapQueue(fh.m_file->m_fileName);
            if (bufferMapQueue == nullptr) {
                continue;
            }
            while (!bufferMapQueue->Empty()) {
                FileHandle fileHandle = bufferMapQueue->GetAndPop();
                if (IS_VALID_FILEHANDLE(fileHandle)) {
                    m_blockBufferCount--;
                    m_blockBufferSize -= fileHandle.m_block.m_size;
                    delete[] fileHandle.m_block.m_buffer;
                    fileHandle.m_block.m_buffer = nullptr;
                }
            }
            m_blockBufferMap.erase(fh.m_file->m_fileName);
        }
        return true;
    }

    std::shared_ptr<BlockBufferMapQueue> Get(const std::string &fileName)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return GetBufferMapQueue(fileName);
    }

    void Print()
    {
        DBGLOG("Stats: BlockBufferCount: %lu, BlockBufferSize: %lu",
            m_blockBufferCount.load(), m_blockBufferSize.load());
    }

    uint64_t GetTotalBufferSize()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        uint64_t totalBufferSize = m_blockBufferSize;
        return totalBufferSize;
    }

    ~BlockBufferMap()
    {
        for (auto it = m_blockBufferMap.begin(); it != m_blockBufferMap.end();) {
            while (!it->second->Empty()) {
                FileHandle fileHandle = it->second->GetAndPop();
                if (fileHandle.m_block.m_buffer != nullptr) {
                    delete[] fileHandle.m_block.m_buffer;
                    fileHandle.m_block.m_buffer = nullptr;
                }
            }
            it = m_blockBufferMap.erase(it);
        }
    }


private:
    std::shared_ptr<BlockBufferMapQueue> GetBufferMapQueue(const std::string& fileName)
    {
        auto it = m_blockBufferMap.find(fileName);
        if (it != m_blockBufferMap.end()) {
            return it->second;
        }
        return nullptr;
    }

public:
    std::atomic<uint64_t> m_blockBufferCount {0};    /* Total block buffer count */
    std::atomic<uint64_t> m_blockBufferSize  {0};    /* Total block buffer size */

private:
    std::mutex m_mtx {};
    std::unordered_map<std::string, std::shared_ptr<BlockBufferMapQueue>> m_blockBufferMap;
};

#endif