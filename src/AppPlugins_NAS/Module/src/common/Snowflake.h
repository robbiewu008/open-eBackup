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
#ifndef MODULE_SNOWFLAKE_H
#define MODULE_SNOWFLAKE_H

#include <mutex>
#include "define/Defines.h"

namespace Module {
	
class AGENT_API Snowflake {
public:
    Snowflake(void);
    ~Snowflake(void);

    void SetEpoch(uint64_t epoch);
    void SetMachine(size_t machine);
    uint64_t GenerateId();

private:
    std::mutex m_mtx;
    uint64_t m_epoch;
    size_t m_machine;
    uint32_t m_sequence;

    uint64_t GetCPUTime();
};

} // namespace Module
#endif
