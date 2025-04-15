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
#ifndef PACKET_STATS_H
#define PACKET_STATS_H

#include <string>
#include <atomic>
#include "log/Log.h"

enum class PKT_TYPE {
    OPEN        = 1,
    READ        = 2,
    CLOSE       = 3,
    READLINK    = 4,
    MKDIR       = 5,
    CREATE      = 6,
    WRITE       = 7,
    SETMETA     = 8,
    LSTAT       = 9,
    SYMLINK     = 10,
    HARDLINK    = 11,
    LINKDELETE  = 12,
    TOTAL       = 13,
};

enum class PKT_COUNTER {
    SENT            = 1,
    RECVD           = 2,
    PENDING         = 3,
    FAILED          = 4,
    RETRIABLE_ERR   = 5,
    NO_SPACE_ERR    = 6,
    NO_ACCESS_ERR   = 7,
    RETRIED         = 8,
};

enum class PKT_ERROR {
    NO_ERR        = 0,
    RETRIABLE_ERR = 1,
    NO_SPACE_ERR  = 2,
    NO_ACCESS_ERR = 3,
};

class PacketStatsCounter {
public:
    void IncrementError(PKT_ERROR errorType, uint32_t size)
    {
        switch (errorType) {
            case PKT_ERROR::RETRIABLE_ERR: {
                m_retriableErr += size;
                break;
            }
            case PKT_ERROR::NO_SPACE_ERR: {
                m_noSpaceErr += size;
                break;
            }
            case PKT_ERROR::NO_ACCESS_ERR: {
                m_noAccessErr += size;
                break;
            }
            default:
                break;
        }
    }

    void DecrementError(PKT_ERROR errorType, uint32_t size)
    {
        switch (errorType) {
            case PKT_ERROR::RETRIABLE_ERR: {
                m_retriableErr -= size;
                break;
            }
            case PKT_ERROR::NO_SPACE_ERR: {
                m_noSpaceErr -= size;
                break;
            }
            case PKT_ERROR::NO_ACCESS_ERR: {
                m_noAccessErr -= size;
                break;
            }
            default:
                break;
        }
    }

    void Increment(PKT_COUNTER counterType, PKT_ERROR errorType = PKT_ERROR::NO_ERR, uint32_t size = 1)
    {
        switch (counterType) {
            case PKT_COUNTER::SENT: {
                m_sent++;
                m_pending++;
                break;
            }
            case PKT_COUNTER::RECVD: {
                m_recvd++;
                m_pending--;
                break;
            }
            case PKT_COUNTER::FAILED: {
                m_failed++; // For few errors, the size may be incremented by 10
                IncrementError(errorType, size);
                break;
            }
            case PKT_COUNTER::RETRIED: {
                m_retried++;
                break;
            }
            default:
                break;
        }
    }

    uint64_t GetValue(PKT_COUNTER counterType)
    {
        switch (counterType) {
            case PKT_COUNTER::SENT: {
                return m_sent;
            }
            case PKT_COUNTER::RECVD: {
                return m_recvd;
            }
            case PKT_COUNTER::PENDING: {
                return m_pending;
            }
            case PKT_COUNTER::FAILED: {
                return m_failed;
            }
            case PKT_COUNTER::RETRIABLE_ERR: {
                return m_retriableErr;
            }
            case PKT_COUNTER::NO_SPACE_ERR: {
                return m_noSpaceErr;
            }
            case PKT_COUNTER::NO_ACCESS_ERR: {
                return m_noAccessErr;
            }
            case PKT_COUNTER::RETRIED: {
                return m_retried;
            }
            default:
                return 0;
        }
    }

    std::string GetStatsInString()
    {
        std::string statsString = "";
        statsString += "Sent: " + std::to_string(m_sent.load());
        statsString += ", Received: " + std::to_string(m_recvd.load());
        statsString += ", Pending: " + std::to_string(m_pending.load());
        statsString += ", Failed: " + std::to_string(m_failed.load());
        statsString += ", Retriable error: " + std::to_string(m_retriableErr.load());
        statsString += ", No space error: " + std::to_string(m_noSpaceErr.load());
        statsString += ", No access error: " + std::to_string(m_noAccessErr.load());
        statsString += ", Retries: " + std::to_string(m_retried.load());
        return statsString;
    }

    void ResetErrorCounter()
    {
        m_retriableErr = 0;
    }

private:
    std::atomic<uint64_t> m_sent                {0};    /* Total requests send */
    std::atomic<uint64_t> m_recvd               {0};    /* Total responces received */
    std::atomic<uint64_t> m_pending             {0};    /* Total requests pending */
    std::atomic<uint64_t> m_failed              {0};    /* Total requests failed */
    std::atomic<uint64_t> m_retriableErr        {0};    /* Total requests timed out */
    std::atomic<uint64_t> m_noSpaceErr          {0};    /* Total requests failed with noSpace error */
    std::atomic<uint64_t> m_noAccessErr         {0};    /* Total requests failed with noAccess error  */
    std::atomic<uint64_t> m_retried             {0};    /* Total requests retried */
};

class PacketStats {
public:
    void IncrementRead(PKT_COUNTER counterType, uint32_t blockSize)
    {
        switch (counterType) {
            case PKT_COUNTER::SENT: {
                m_read.Increment(counterType);
                m_total.Increment(counterType);
                break;
            }
            case PKT_COUNTER::RECVD: {
                m_read.Increment(counterType);
                m_total.Increment(counterType);
                break;
            }
            default:
                break;
        }
    }

    void Increment(
        PKT_TYPE        reqType,
        PKT_COUNTER     counterType,
        uint32_t        size = 1,
        PKT_ERROR       errorType = PKT_ERROR::NO_ERR)
    {
        PacketStatsCounter* counterPtr = GetPacketStatsCounterByEnum(reqType);
        if (counterPtr == nullptr) {
            ERRLOG("unknown request type: %u", reqType);
            return;
        }
        counterPtr->Increment(counterType, errorType, size);
        m_total.Increment(counterType, errorType, size);
    }

    void DecrementError(PKT_ERROR errorType = PKT_ERROR::NO_ERR, uint32_t size = 1)
    {
        m_total.DecrementError(errorType, size);
    }

    uint64_t GetValue(PKT_TYPE reqType, PKT_COUNTER counterType)
    {
        PacketStatsCounter* counterPtr = GetPacketStatsCounterByEnum(reqType);
        if (counterPtr == nullptr) {
            ERRLOG("unknown request type: %u", reqType);
            return 0;
        }
        return counterPtr->GetValue(counterType);
    }

    void Print()
    {
        INFOLOG("Stats: Open - %s,\n Read - %s,\n Close - %s,\n ReadLink - %s,\n Mkdir - %s,\n Lstat - %s,\n"
            "Create - %s,\n Write - %s,\n SetMeta - %s,\n SymLink - %s,\n HardLink - %s,\n"
            "LinkDelete - %s,\n Total - %s,",
            m_open.GetStatsInString().c_str(), m_read.GetStatsInString().c_str(), m_close.GetStatsInString().c_str(),
            m_readlink.GetStatsInString().c_str(), m_mkdir.GetStatsInString().c_str(), m_lstat.GetStatsInString().c_str(),
            m_create.GetStatsInString().c_str(), m_write.GetStatsInString().c_str(), m_metaSet.GetStatsInString().c_str(),
            m_symlink.GetStatsInString().c_str(), m_hardlink.GetStatsInString().c_str(), m_linkDelete.GetStatsInString().c_str(),
            m_total.GetStatsInString().c_str());
    }

    void ResetErrorCounter(PKT_TYPE reqType)
    {
        switch (reqType) {
            case PKT_TYPE::TOTAL: {
                m_total.ResetErrorCounter();
                break;
            }
            default:
                return;
        }
    }

private:
    PacketStatsCounter* GetPacketStatsCounterByEnum(PKT_TYPE packetType)
    {
        switch (packetType) {
            case PKT_TYPE::OPEN:        return &m_open;
            case PKT_TYPE::READ:        return &m_read;
            case PKT_TYPE::CLOSE:       return &m_close;
            case PKT_TYPE::READLINK:    return &m_readlink;
            case PKT_TYPE::MKDIR:       return &m_mkdir;
            case PKT_TYPE::CREATE:      return &m_create;
            case PKT_TYPE::WRITE:       return &m_write;
            case PKT_TYPE::SETMETA:     return &m_metaSet;
            case PKT_TYPE::LSTAT:       return &m_lstat;
            case PKT_TYPE::SYMLINK:     return &m_symlink;
            case PKT_TYPE::HARDLINK:    return &m_hardlink;
            case PKT_TYPE::LINKDELETE:  return &m_linkDelete;
            case PKT_TYPE::TOTAL:       return &m_total;
            default:                    return nullptr;
        }
        return nullptr;
    }

private:
    PacketStatsCounter m_open       {};
    PacketStatsCounter m_read       {};
    PacketStatsCounter m_close      {};
    PacketStatsCounter m_readlink   {};
    PacketStatsCounter m_mkdir      {};
    PacketStatsCounter m_lstat      {};
    PacketStatsCounter m_create     {};
    PacketStatsCounter m_write      {};
    PacketStatsCounter m_metaSet    {};
    PacketStatsCounter m_symlink    {};
    PacketStatsCounter m_hardlink   {};
    PacketStatsCounter m_linkDelete {};
    PacketStatsCounter m_total      {};
};

#endif // PACKET_STATS_H