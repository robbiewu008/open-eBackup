#ifndef DISK_DIRTY_RANGES_H_
#define DISK_DIRTY_RANGES_H_
#include <list>
#include <stdint.h>
#include <memory>
#include <sstream>
#include "common/Log.h"

const uint32_t VMWARE_BLOCK_SIZE = 4194304;

struct DirtyRange {
    DirtyRange() : start(0), size(0) {}
    DirtyRange(uint64_t _start, uint64_t _size) : start(_start), size(_size) {}
    uint64_t start;
    uint64_t size;
    inline uint64_t End()
    {
        return start + size;
    }
};

struct DirtyBlock {
    DirtyBlock() : number(0), size(0)
    {}
    uint64_t number;
    uint64_t size;
    inline uint64_t Offset() const
    {
        return number * size;
    }
    inline uint64_t EndOffset() const
    {
        return (number + 1) * size;
    }
    inline uint64_t Num(void) const
    {
        return number;
    }
    inline uint64_t Length(void) const
    {
        return size;
    }
};

class DirtyRanges {
public:
    class iterator {
    public:
        // The constructor also finds the first dirty block if possible.
        iterator(uint64_t blockSize, std::shared_ptr<std::list<DirtyRange>> ranges, bool isEnd)
        {
            m_block.size = blockSize;
            m_ranges = ranges;
            m_it = isEnd ? ranges->end() : ranges->begin();
            if (m_it != ranges->end()) {
                m_block.number = m_it->start / blockSize;
            } else {
                m_block.number = 0; // This line is useless
            }
        }

        iterator(const iterator &it)
        {
            m_block = it.m_block;
            m_ranges = it.m_ranges;
            m_it = it.m_it;
        }

        ~iterator() {}

        iterator &operator = (const iterator &it)
        {
            if (this == &it) {
                return *this;
            }
            m_block = it.m_block;
            m_ranges = it.m_ranges;
            m_it = it.m_it;
            return *this;
        }

        bool operator == (const iterator &it) const
        {
            return (m_it == it.m_it);
        }

        bool operator != (const iterator &it) const
        {
            return (m_it != it.m_it);
        }

        const DirtyBlock &operator*() const
        {
            return m_block;
        }

        const DirtyBlock *operator->() const
        {
            return &m_block;
        }

        iterator &operator ++ ()
        {
            FindNextBlock();
            return *this;
        }

        const iterator operator ++ (int) const
        {
            DirtyRanges::iterator it(*this);
            return (++it);
        }

    private:
        // Actully looks up the next block
        // Note: when it find it, the inner dirty ranges iterator will be positioned
        void FindNextBlock()
        {
            uint64_t endOffset = m_block.EndOffset();
            for (; (m_it != m_ranges->end()) && (m_it->End() <= endOffset); ++m_it)
                ;
            if (m_it != m_ranges->end()) {
                ++(m_block.number);
                uint64_t startNumber = m_it->start / m_block.size;
                if (m_block.number < startNumber) {
                    m_block.number = startNumber;
                }
            }
        }

    private:
        DirtyBlock m_block;
        std::list<DirtyRange>::iterator m_it;
        std::shared_ptr<std::list<DirtyRange>> m_ranges;
    };

public:
    DirtyRanges()
    {
        m_ranges.reset(new (std::nothrow) std::list<DirtyRange>());
    }

    ~DirtyRanges() {}

    bool AddRange(DirtyRange range)
    {
        return AddRangeForNoDedupe(range);
    }

    iterator begin(uint64_t blockSize) const
    {
        return DirtyRanges::iterator(blockSize, m_ranges, false);
    }

    // Returns the end of an iterator for dirty blocks of size: blockSize.
    iterator end(uint64_t blockSize) const
    {
        return DirtyRanges::iterator(blockSize, m_ranges, true);
    }

    // Returns the size of the dirty ranges list.
    std::size_t size() const
    {
        return m_ranges->size();
    }

    // Clears the dirty ranges list.
    void clear() const
    {
        m_ranges->clear();
    }

    uint64_t GetBlockNum(uint64_t blockSize)
    {
        uint64_t blockNum = 0;
        DirtyRanges::iterator end = DirtyRanges::iterator(blockSize, m_ranges, true);
        DirtyRanges::iterator it = DirtyRanges::iterator(blockSize, m_ranges, false);
        for (; it != end; ++it) {
            ++blockNum;
        }
        return blockNum;
    }

    void Display() const
    {
        DBGLOG("DirtyRange:{");
        std::size_t index = 0;
        const int outSize = 20;
        std::ostringstream oss;
        std::list<DirtyRange>::const_iterator it = m_ranges->begin();
        for (; it != m_ranges->end(); it++) {
            oss << it->start << ":" << it->size << ",";
            index++;
            if (index % outSize == 0) {
                DBGLOG("%s}.", oss.str().c_str());
                oss.str("");
                index = 0;
            }
        }

        DBGLOG("%s}.", oss.str().c_str());
    }

private:
    /* 将range转换为以blockSize为单位的区间 */
    DirtyRange Transform(DirtyRange &range, const uint64_t blockSize)
    {
        uint64_t blockNum = range.start / blockSize;
        uint64_t blockCount = ((range.End() + blockSize - 1) / blockSize) - blockNum;
        DirtyRange dirtyRange(blockNum * blockSize, blockCount * blockSize);
        return dirtyRange;
    }

    bool AddRangeForNoDedupe(DirtyRange &range)
    {
        std::list<DirtyRange>::reverse_iterator rit = m_ranges->rbegin();
        if (m_ranges->rend() == rit) {
            m_ranges->push_back(Transform(range, VMWARE_BLOCK_SIZE));
            return true;
        }

        if (range.start <= rit->start) {
            ERRLOG("ranges must be inserted in order.");
            m_ranges->clear();
            return false;
        }

        DirtyRange tmp = Transform(range, VMWARE_BLOCK_SIZE);
        if (tmp.End() > rit->End()) {
            /* 不存在交叉 */
            if (tmp.start > rit->End()) {
                m_ranges->push_back(tmp);
            } else { /* 存在交叉 */
                rit->size += (tmp.End() - rit->End());
            }
        }
        return true;
    }

private:
    std::shared_ptr<std::list<DirtyRange>> m_ranges;
};

#endif /* DIRTY_RANGES_H */
