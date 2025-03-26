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
#ifndef DP_OSAD_ADMIN_MSG_H
#define DP_OSAD_ADMIN_MSG_H

// std
#include <cstdint>
#include <memory>

// Vendors
#include <boost/utility/string_view.hpp>

// DataMove
#include <protocol/Message.h>
#include <protocol/Utilities.h>

using Module::Protocol::Message;

namespace DataMove {
namespace Protocol {

class OsadAdminRequest final {
public:
    enum class RequestType : std::uint8_t {
        QUERY_AVG_SPEED,
        QUERY_CURRENT_SPEED
    };

    OsadAdminRequest() = delete;

    explicit OsadAdminRequest(boost::string_view data) : m_data(data) { }

    RequestType Type() const
    {
        return *reinterpret_cast<const RequestType*>(m_data.data());
    }

    AuxDataSize SourceIdSize() const
    {
        return *reinterpret_cast<const AuxDataSize*>(m_data.data() + sizeof(RequestType));
    }

    std::string SourceId(AuxDataSize sourceIdSize) const
    {
        return { m_data.data() + sizeof(RequestType) + sizeof(AuxDataSize), sourceIdSize };
    }

    static std::shared_ptr<Message> NewQuerySpeedRequest(const std::string& sourceId, bool isAvgSpeed,
        uint16_t sequenceNum);

private:
    boost::string_view m_data;
};

class OsadAdminResponse final {
public:
    enum class ResponseType : std::uint8_t {
        SUCCESS,
        FAIL
    };
    enum class OsadAdminErrType : std::uint8_t {
        SUCCESS = 0,
        ERR_NOENT  = 1,
        PARSE_TIMEPOINT_FAIL = 2,
        WRONG_QUERY_TYPE = 3
    };

    OsadAdminResponse() = delete;

    explicit OsadAdminResponse(boost::string_view data) : m_data(data) { }

    ResponseType Type() const
    {
        return *reinterpret_cast<const ResponseType*>(m_data.data());
    }
    std::string Message() const
    {
        return { m_data.data() + sizeof(ResponseType), m_data.size() - sizeof(ResponseType) };
    }

    static std::shared_ptr<Module::Protocol::Message> NewResponse(ResponseType type, const std::string& message,
        uint16_t sequenceNum);

private:
    boost::string_view m_data;
};

}
}

#endif
