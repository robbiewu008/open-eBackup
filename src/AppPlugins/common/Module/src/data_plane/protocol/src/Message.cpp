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
#include <protocol/Message.h>

#include <protocol/MessageHeader.h>
namespace Module {
namespace Protocol {

void Message::AddData(const char* data, std::size_t dataLength)
{
    m_data.insert(m_data.end(), data, data + dataLength);
}

void Message::AddData(char* data, std::size_t dataLength)
{
    m_data.insert(m_data.end(), data, data + dataLength);
}

void Message::Reserve(std::size_t newSize)
{
    m_data.reserve(newSize);
}

void Message::Resize(std::size_t newSize)
{
    m_data.resize(newSize);
}

const std::vector<char>& Message::Data() const
{
    return m_data;
}

std::vector<char>& Message::Data()
{
    return m_data;
}

void Message::RemoveHeader()
{
    if (m_data.size() < sizeof(MessageHeader)) {
        return;
    }
    m_data.erase(m_data.begin(), m_data.begin() + sizeof(MessageHeader));
}
}
}
