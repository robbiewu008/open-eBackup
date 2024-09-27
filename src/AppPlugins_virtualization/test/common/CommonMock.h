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
#ifndef __COMMON_MOCK_H__
#define __COMMON_MOCK_H__

#include "common/model/ModelBaseMock.h"
#include "common/IHttpResponseMock.h"
#include "common/IHttpClientMock.h"
#include "common/ResponseModelMock.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;;

namespace HDT_TEST {
static int g_httpStatusCode = 200;
static int g_httpErrCode = 0;
static std::string g_httpResponsebody = "";
static std::string g_httpResponseHeader = "";


static std::function<uint32_t()> stubGetHttpStatusPtr = nullptr;

static uint32_t Stub_GetStatusCode()
{
    return g_httpStatusCode;
}

static uint32_t Customize_GetStatusCode()
{
    static int count = 0;
    if (count == 0) {
        count += 1;
        return 201; // get token success
    } else if (count == 1) {
        count += 1;
        return 401; // get projects unauthorized
    } else {
        return 200; // reget token success
    }
}


static unsigned int Stub_TimeSleep(unsigned int seconds)
{
    return SUCCESS;
}

static Module::IHttpClient* Stub_CreateClient_SendRequest()
{
    if (stubGetHttpStatusPtr == nullptr) {
        stubGetHttpStatusPtr = Stub_GetStatusCode;
    }
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Invoke(stubGetHttpStatusPtr));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(g_httpErrCode));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_httpResponsebody));
    std::map<std::string, std::set<std::string> > getHeadersReturn = {};
    std::set<std::string> headerValue;
    headerValue.insert(g_httpResponseHeader);
    getHeadersReturn["X-Subject-Token"] = headerValue;
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendMemCertRequest(_,_)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static std::shared_ptr<JobHandle> CommonFormJobHandler(AppProtect::BackupJob job, StorageRepository &repo)
{
    repo.__set_repositoryType(RepositoryDataType::META_REPOSITORY);
    repo.path.push_back("/tmp/");
    job.repositories.push_back(repo);
    std::shared_ptr<JobCommonInfo> jobCommonInfoPtr = std::make_shared<JobCommonInfo>(std::make_shared<AppProtect::BackupJob>(job));
    std::shared_ptr<JobHandle> jobHandler = std::make_shared<JobHandle>(JobType::BACKUP, jobCommonInfoPtr);
    return jobHandler;
}

static unsigned int Stub_Sleep(unsigned int seconds)
{
    return SUCCESS;
}

static bool Stub_GetToken_Success(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = "test_token";
    endPoint = "https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b";;
    return true;
}

static bool StubGetToken(void *obj, ModelBase &model, std::string &tokenValue, std::string &endPoint) {
	tokenValue = "stubtokenstring";
	endPoint = "https://identity.az236.dc236.huawei.com/identity/v3";
	return true;
}
}
#endif