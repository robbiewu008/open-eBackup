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
#ifndef DP_MESSAGE_H
#define DP_MESSAGE_H
// std
#include <vector>

namespace Module {
namespace Protocol {

class Message {
public:
    void AddData(const char* data, std::size_t dataLength);
    void AddData(char* data, std::size_t dataLength);
    void Reserve(std::size_t newSize);
    void Resize(std::size_t newSize);
    const std::vector<char>& Data() const;
    std::vector<char>& Data();
    /*!
     * \brief This function is used only for testing purposes. Removes header data that present in m_data.
     *
     * \note This function just removes sizeof(MessageHeader) bytes from the start of the m_data. If m_data.size() <
     *       sizeof(MessageHeader), then nothing will be removed.
     */
    void RemoveHeader();

private:
    std::vector<char> m_data{};
};

}
}
#endif