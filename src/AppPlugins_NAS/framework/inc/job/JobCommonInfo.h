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
#ifndef NEWFRAMEWORKTEST_JOBCOMMONINFO_H
#define NEWFRAMEWORKTEST_JOBCOMMONINFO_H

#include <memory>
#include <thrift/TBase.h>
#include "PluginTypes.h"
#include "ApplicationProtectPlugin_types.h"

using ThriftDataBase = ::apache::thrift::TBase;
class JobCommonInfo {
public:
    JobCommonInfo() {}
    explicit JobCommonInfo(std::shared_ptr<ThriftDataBase> data) : m_infoPtr(data) {}
    virtual ~JobCommonInfo() {}

    void SetJobInfo(std::shared_ptr<ThriftDataBase> data)
    {
        m_infoPtr = data;
    }

    std::shared_ptr<ThriftDataBase> GetJobInfo()
    {
        return m_infoPtr;
    }

private:
    std::shared_ptr<ThriftDataBase> m_infoPtr = nullptr;
};

#endif // NEWFRAMEWORKTEST_JOBCOMMONINFO_H
