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
#ifndef __IHTTP_CLIENT_MOCK_H__
#define __IHTTP_CLIENT_MOCK_H__
#include <set>
#include <curl_http/HttpClientInterface.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace HDT_TEST {
class IHttpClientMock : public Module::IHttpClient {
public:
    IHttpClientMock() : Module::IHttpClient() {}
    ~IHttpClientMock() {}
    MOCK_METHOD(std::shared_ptr<Module::IHttpResponse>, SendRequest, (const Module::HttpRequest &req,  const uint32_t time_out), (override));
    MOCK_METHOD(std::shared_ptr<Module::IHttpResponse>, SendMemCertRequest, (const Module::HttpRequest &req,  const uint32_t time_out), (override));
    MOCK_METHOD(std::shared_ptr<Module::IHttpResponse>, DownloadAttchment, (const Module::HttpRequest &req, const uint32_t time_out), (override));
    MOCK_METHOD(bool, UploadAttachment, (const Module::HttpRequest &req, std::shared_ptr<Module::IHttpResponse> &rsp, const uint32_t time_out), (override));
    MOCK_METHOD(bool, TestConnect, (const std::string &url, const uint32_t time_out), (override));
    MOCK_METHOD(uint32_t, GetThunmPrint, (const std::string &url, std::string &thunmPrint, const uint32_t time_out), (override));
    MOCK_METHOD(std::string, GetCertificate, (const std::string &url, const uint32_t time_out), (override));
};
}

#endif