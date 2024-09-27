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
#ifndef __MODEL_BASE_MOCK_H__
#define __MODEL_BASE_MOCK_H__

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <map>
#include <string>
#include <memory>
#include <list>
#include <unordered_map>
#include <common/Structs.h>
#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

namespace HDT_TEST {
class ModelBaseMock : public ModelBase {
public:
    ModelBaseMock() : ModelBase() {}
    virtual ~ModelBaseMock() {}

    MOCK_METHOD(Scope, GetScopeType, (), (const override));
    MOCK_METHOD(ApiType, GetApiType, (), (override));
    MOCK_METHOD(std::string, GetEnvAddress, ());
};
}

#endif