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
#ifndef DIRTY_RANGES_H
#define DIRTY_RANGES_H

#include <list>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>
#include "json/json.h"
#include "log/Log.h"
#include "system/System.hpp"
#ifndef WIN32
#include "lz4.h"
#endif
#include "repository_handlers/RepositoryHandler.h"

namespace VirtPlugin {
struct DirtyRange {
    DirtyRange() noexcept : start(0), size(0)
    {}

    DirtyRange(uint64_t start, uint64_t size) : start(start), size(size)
    {}

    uint64_t start{0};
    uint64_t size{0};

    inline uint64_t End() const
    {
        return start + size;
    }

    inline std::string toString() const
    {
        std::stringstream ss;
        ss << DBG(start) << DBG(size) << DBG(End());
        return ss.str();
    }
};

struct DirtyBlock {
    uint64_t number{0};
    uint64_t size{0};

    inline uint64_t Offset() const
    {
        return number * size;
    }

    inline uint64_t EndOffset() const
    {
        return (number + 1) * size;
    }

    inline uint64_t Num() const
    {
        return number;
    }
};

using CompressedBufs=std::vector<std::shared_ptr<char[]>>;
class DirtyRanges {
public:
    class iterator {
    public:
        // The constructor also finds the first dirty block if possible.
        iterator(uint64_t blockSize, std::vector<uint32_t> &offsets, CompressedBufs compBuffer)
            : m_partOffsets(offsets), m_compressedParts(std::move(compBuffer))
        {
            m_block.size = blockSize;
            m_block.number = 0;
            m_ranges = std::make_shared<std::list<DirtyRange>>();
            if (!m_partOffsets.empty()) {
                (void) DecompressPart();
                m_it = m_ranges->begin();
                if (m_it != m_ranges->end()) {
                    m_block.number = m_it->start / blockSize;
                }
            } else {
                m_it = m_ranges->begin();
            }
        }

        iterator(const iterator &it)
        {
            m_block = it.m_block;
            m_ranges = it.m_ranges;
            m_it = it.m_it;
        }

        ~iterator() = default;

        bool End();

        iterator &operator=(const iterator &it)
        {
            if (this == &it) {
                return *this;
            }
            m_block = it.m_block;
            m_ranges = it.m_ranges;
            m_it = it.m_it;
            return *this;
        }

        const DirtyBlock &operator*() const
        {
            return m_block;
        }

        const DirtyBlock *operator->() const
        {
            return &m_block;
        }

        iterator &operator++()
        {
            FindNextBlock();
            return *this;
        }

        const iterator operator++(int) const
        {
            DirtyRanges::iterator it(*this);
            return (++it);
        }

    private:
        void FindNextBlock();
        bool DecompressPart();

    private:
        DirtyBlock m_block{};
        size_t m_partIndex{0};
        std::vector<uint32_t> m_partOffsets{};
        CompressedBufs m_compressedParts{};
        std::list<DirtyRange>::iterator m_it;
        std::shared_ptr<std::list<DirtyRange>> m_ranges;
    };

    explicit DirtyRanges(bool isForDedupe = false) : m_isFull(false), m_isRestore(false), m_isForDedupe(isForDedupe)
    {
        {
            m_ranges.reset(new(std::nothrow) std::list<DirtyRange>());
            if (nullptr == m_ranges) {
                ERRLOG("Memory is used out.");
            }
        }
    }

    ~DirtyRanges() = default;

    iterator begin(uint64_t blockSize)
    {
        return DirtyRanges::iterator(blockSize, m_partOffsets, m_compressedParts);
    }

    std::size_t size() const
    {
        return m_partOffsets.size();
    }

    void clear()
    {
        m_ranges->clear();
        m_partOffsets.clear();
        m_compressedParts.clear();
    }

    bool IsFull() const
    {
        return m_isFull;
    }

    void UseFull()
    {
        m_isFull = true;
    }

    void NotFull()
    {
        m_isFull = false;
    }

    bool IsRestore() const
    {
        return m_isRestore;
    }

    void UseRestore()
    {
        m_isRestore = true;
    }

    bool IsForDedupe() const
    {
        return m_isForDedupe;
    }

    void UseForDedupe()
    {
        m_isForDedupe = true;
    }

    std::shared_ptr<std::list<DirtyRange>> GetRanges() const
    {
        return m_ranges;
    }

    bool Initialize(const std::string &path, const std::string &taskID, std::shared_ptr<RepositoryHandler> &rangeFile);
    bool AddRange(const DirtyRange &range);
    uint64_t GetBlockNum(uint64_t blockSize);
    void Display();
    std::string Serialize() const;
    bool Deserialize(const Json::Value &val);
    bool NeedCompressBuff();
    bool CompressRanges();
    bool FlushToStorage();
    bool LoadFromStorage();
    bool CleanDirtyRangesFile()
    {
        return CleanDirtyRanges();
    }

private:
    DirtyRange Transform(const DirtyRange &range, const uint64_t blockSize);
    bool String2DirtyRanges(const std::string &ranges);
    bool ParserRange(const std::string &strRange);
    std::string GetDirtyRangeFileName();
    bool SaveDirtyRanges();
    bool WriteToFile(std::shared_ptr<uint8_t[]> rangeBuffer, size_t bufferSize);
    bool ReadFromFile(size_t bufferSize);
    bool CleanDirtyRanges();
    bool GetSha256(const void *inBuf, std::size_t size, std::shared_ptr<uint8_t[]> outBuf);

    bool m_isFull{false};
    bool m_isRestore{false};
    bool m_isForDedupe{false}; // just for backup(volume and vmware)
    std::shared_ptr<std::list<DirtyRange>> m_ranges;
    uint32_t m_totalSize{0};
    std::vector<uint32_t> m_partOffsets{};
    CompressedBufs m_compressedParts{};
    std::string m_taskID;
    std::string m_storagePath;
    std::shared_ptr<RepositoryHandler> m_rangeFile {nullptr};
};
}

#endif // DIRTY_RANGES_H
