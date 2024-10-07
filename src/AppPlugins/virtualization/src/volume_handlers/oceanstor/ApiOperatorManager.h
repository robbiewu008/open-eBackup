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
#ifndef API_OPERATOR_MANAGER_H
#define API_OPERATOR_MANAGER_H
#include <map>
#include <set>
#include <memory>
#include <mutex>
#include "common/Macros.h"
#include "ApiOperator.h"
#include "volume_handlers/common/ControlDevice.h"

namespace VirtPlugin {
class ApiOperatorManager {
public:
    static ApiOperatorManager *GetInstance();

    virtual int32_t GetRestApiOperator(const ControlDeviceInfo &info, std::shared_ptr <ApiOperator> &spRestApi,
                                       bool isGetFromCache = true);

    void GetLevelAndRoleID(const std::string& mode, std::string& level, std::string& roleId) const;
private:
    ApiOperatorManager();
    virtual ~ApiOperatorManager();
    void InitStorageModel();
private:
    std::set<std::string> m_oceanStorageModel;
    std::set<std::string> m_doradoStorageModel;
    std::map<std::string, std::shared_ptr<ApiOperator>> m_mapApiOperator {};
    std::mutex m_Mutext;
};
}
#endif  // STORAGE_DISK_LIB_H
