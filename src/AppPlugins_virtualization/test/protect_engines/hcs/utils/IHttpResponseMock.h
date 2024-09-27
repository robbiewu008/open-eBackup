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
#ifndef __IHTTP_RESPONSE_MOCK_MOCK_H__
#define __IHTTP_RESPONSE_MOCK_MOCK_H__
#include <set>
#include <curl_http/HttpClientInterface.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace HDT_TEST {
class IHttpResponseMock : public Module::IHttpResponse {
public:
    IHttpResponseMock() : Module::IHttpResponse() {}
    ~IHttpResponseMock() {}

    using SetType = std::set<std::string>;
    using MapType = std::map<std::string, std::set<std::string> >;
    
    MOCK_METHOD(bool, Success, (), (override));
    MOCK_METHOD(bool, Busy, (), (override));
    MOCK_METHOD(uint32_t, GetHttpStatusCode, (), (override));
    MOCK_METHOD(std::string, GetHttpStatusDescribe, (), (override));
    MOCK_METHOD(int32_t, GetErrCode, (), (override));
    MOCK_METHOD(std::string, GetErrString, (), (override));
    MOCK_METHOD(SetType, GetHeadByName, (const std::string& header_name), (override));
    MOCK_METHOD(std::string, GetBody, (), (override));
    MOCK_METHOD(MapType, GetHeaders, (), (override));
    MOCK_METHOD(SetType, GetCookies, (), (override));
    MOCK_METHOD(uint32_t, GetStatusCode, (), (override));
};
}

#endif