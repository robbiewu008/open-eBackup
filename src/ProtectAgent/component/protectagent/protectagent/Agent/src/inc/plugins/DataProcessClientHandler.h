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
#ifndef AGENT_VMWARENATIVE_DATAPROCESS_HANDLER_H
#define AGENT_VMWARENATIVE_DATAPROCESS_HANDLER_H

#include <map>
#include <memory>
#include "common/Types.h"
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "jsoncpp/include/json/value.h"
#include "jsoncpp/include/json/json.h"
#include "plugins/DataPathProcessClient.h"
#include "message/tcp/CConnection.h"

// Provide a global class for the initialization of data process service client instance
class DataProcessClientHandler {
public:
    static DataProcessClientHandler& GetInstance()
    {
        return singleInst;
    }
    virtual ~DataProcessClientHandler();

    mp_void RemoveDpServiceClient(const mp_string& version);
    DataPathProcessClient *FindDpClient(const mp_string& version);

private:
    DataProcessClientHandler();
    DataProcessClientHandler(const DataProcessClientHandler& dpClientHandler)
    {}
    DataProcessClientHandler& operator=(const DataProcessClientHandler& dpClientHandler)
    {
        return *this;
    }
    DataPathProcessClient* GenerateDpServiceClientMap(mp_int32 serviceType, const mp_string& dpParam);

private:
    static DataProcessClientHandler singleInst;  // single instance
    thread_lock_t m_mapLock;
    std::map<mp_string, DataPathProcessClient*> m_mapDpProcessClientInstance;
};
#endif
