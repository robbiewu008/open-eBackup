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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "IHttpResponseMock.h"
#include "IHttpClientMock.h"
#include "protect_engines/hcs/utils/StorageMgr.h"

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

using namespace VirtPlugin;
using namespace HcsPlugin;
using FunGetInfo = int32_t (*)(ApiOperator*, const std::string&, UserRoleLevel&, std::string&, const std::string&);


namespace HDT_TEST {
class StorageMgrTest : public testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

int32_t GetRestApiOperator_Failed(void *obj, const ControlDeviceInfo &info, std::shared_ptr<ApiOperator> &spRestApi)
{
    return FAILED;
}
int32_t GetSystemInfo_Failed(void *obj, StorageSysInfo &storageSysInfo, std::string &errorDes)
{
    return FAILED;
}

int32_t GetRestApiOperator_Success(void *obj, const ControlDeviceInfo &info, std::shared_ptr<ApiOperator> &spRestApi)
{
    spRestApi = std::make_shared<ApiOperator>(info);
    return SUCCESS;
}
int32_t GetSystemInfo_Success(void *obj, StorageSysInfo &storageSysInfo, std::string &errorDes)
{
    return SUCCESS;
}
int32_t GetRestApiOperator_GetApiSuccess(void *obj, const ControlDeviceInfo &info, std::shared_ptr<ApiOperator> &spRestApi)
{
    spRestApi = nullptr;
    return SUCCESS;
}

static int32_t GetUserRoleAndLevelSuccess(ApiOperator* obj, const std::string& userName,
    UserRoleLevel &userInfo, std::string &errorDes)
{
    userInfo.m_level = "1";
    userInfo.m_roleID = "2";
    return SUCCESS;
}

static int32_t GetUserRoleAndLevelReadOnly(const std::string& userName, UserRoleLevel &userInfo, std::string &errorDes)
{
    userInfo.m_level = "3";
    userInfo.m_roleID = "2";
    return SUCCESS;
}

static int32_t GetUserRoleAndLevelFailed(const std::string& userName, UserRoleLevel &userInfo, std::string &errorDes)
{
    userInfo.m_level = "3";
    userInfo.m_roleID = "2";
    return FAILED;
}

/*
 * 测试用例： 检查存储联通性
 * 前置条件： 消息发送成功
 * CHECK点： 
 */
TEST_F(StorageMgrTest, GetStorageSystemInfoSuccess)
{
    ControlDeviceInfo deviceInfo;
    StorageSysInfo storageSysInfo;
    std::string errorStr;
    deviceInfo.m_ip = "1.1.1.1";
    deviceInfo.m_port = "8888";
    deviceInfo.m_userName = "name";
    deviceInfo.m_password = "123456";

    typedef int32_t (*fptr)(ApiOperatorManager*,const ControlDeviceInfo &, std::shared_ptr<ApiOperator> &);
    fptr ApiOperatorManager_GetRestApiOperator = (fptr)(&ApiOperatorManager::GetRestApiOperator);
    typedef int32_t (*fpt)(ApiOperator*,StorageSysInfo &, std::string &);
    fpt ApiOperatorr_GetSystemInfo = (fpt)(&ApiOperator::GetSystemInfo);
    FunGetInfo funGet = (FunGetInfo)(&ApiOperator::GetUserRoleAndLevel);

    Stub stub;
    stub.set(ApiOperatorManager_GetRestApiOperator, GetRestApiOperator_Success);
    stub.set(funGet, GetUserRoleAndLevelSuccess);
    stub.set(ApiOperatorr_GetSystemInfo, GetSystemInfo_Success);
    bool ret = StorageMgr::GetStorageSystemInfo(deviceInfo, storageSysInfo, errorStr);
    EXPECT_TRUE(ret);
    stub.reset(ADDR(ApiOperatorManager, GetRestApiOperator));
    stub.reset(funGet);
    stub.reset(ADDR(ApiOperator, GetSystemInfo));
}

/*
 * 测试用例： 检查存储联通性
 * 前置条件： 消息发送成功
 * CHECK点： getapi失败
 */
TEST_F(StorageMgrTest, GetStorageSystemInfoFalied)
{
    ControlDeviceInfo deviceInfo;
    StorageSysInfo storageSysInfo;
    std::string errorStr;
    deviceInfo.m_ip = "1.1.1.1";
    deviceInfo.m_port = "8888";
    deviceInfo.m_userName = "name";
    deviceInfo.m_password = "123456";

    typedef int32_t (*fptr)(ApiOperatorManager*,const ControlDeviceInfo &, std::shared_ptr<ApiOperator> &);
    fptr ApiOperatorManager_GetRestApiOperator = (fptr)(&ApiOperatorManager::GetRestApiOperator);
    typedef int32_t (*fpt)(ApiOperator*,StorageSysInfo &, std::string &);
    fpt ApiOperatorr_GetSystemInfo = (fpt)(&ApiOperator::GetSystemInfo);

    Stub stub;
    stub.set(ApiOperatorManager_GetRestApiOperator, GetRestApiOperator_Failed);
    stub.set(ApiOperatorr_GetSystemInfo, GetSystemInfo_Success);
    bool ret = StorageMgr::GetStorageSystemInfo(deviceInfo, storageSysInfo, errorStr);
    EXPECT_TRUE(ret);
    stub.reset(ADDR(ApiOperatorManager, GetRestApiOperator));
    stub.reset(ADDR(ApiOperator, GetSystemInfo));
}

/*
 * 测试用例： 检查存储联通性
 * 前置条件： 消息发送成功
 * CHECK点： 获取系统信息失败
 */
TEST_F(StorageMgrTest, GetStorageSystemInfoFalied2)
{
    ControlDeviceInfo deviceInfo;
    StorageSysInfo storageSysInfo;
    std::string errorStr;
    deviceInfo.m_ip = "1.1.1.1";
    deviceInfo.m_port = "8888";
    deviceInfo.m_userName = "name";
    deviceInfo.m_password = "123456";

    typedef int32_t (*fptr)(ApiOperatorManager*,const ControlDeviceInfo &, std::shared_ptr<ApiOperator> &);
    fptr ApiOperatorManager_GetRestApiOperator = (fptr)(&ApiOperatorManager::GetRestApiOperator);
    typedef int32_t (*fpt)(ApiOperator*,StorageSysInfo &, std::string &);
    fpt ApiOperatorr_GetSystemInfo = (fpt)(&ApiOperator::GetSystemInfo);
    FunGetInfo funGet = (FunGetInfo)(&ApiOperator::GetUserRoleAndLevel);

    Stub stub;
    stub.set(ApiOperatorManager_GetRestApiOperator, GetRestApiOperator_Success);
    stub.set(funGet, GetUserRoleAndLevelSuccess);
    stub.set(ApiOperatorr_GetSystemInfo, GetSystemInfo_Failed);
    bool ret = StorageMgr::GetStorageSystemInfo(deviceInfo, storageSysInfo, errorStr);
    EXPECT_TRUE(!ret);
    stub.reset(ADDR(ApiOperatorManager, GetRestApiOperator));
    stub.reset(funGet);
    stub.reset(ADDR(ApiOperator, GetSystemInfo));
}

/*
 * 测试用例： 检查存储联通性
 * 前置条件： 消息发送成功
 * CHECK点： getapi成功但返回为空
 */
TEST_F(StorageMgrTest, GetStorageSystemInfoFalied3)
{
    ControlDeviceInfo deviceInfo;
    StorageSysInfo storageSysInfo;
    std::string errorStr;
    deviceInfo.m_ip = "1.1.1.1";
    deviceInfo.m_port = "8888";
    deviceInfo.m_userName = "name";
    deviceInfo.m_password = "123456";

    typedef int32_t (*fptr)(ApiOperatorManager*,const ControlDeviceInfo &, std::shared_ptr<ApiOperator> &);
    fptr ApiOperatorManager_GetRestApiOperator = (fptr)(&ApiOperatorManager::GetRestApiOperator);
    typedef int32_t (*fpt)(ApiOperator*,StorageSysInfo &, std::string &);
    fpt ApiOperatorr_GetSystemInfo = (fpt)(&ApiOperator::GetSystemInfo);

    Stub stub;
    stub.set(ApiOperatorManager_GetRestApiOperator, GetRestApiOperator_GetApiSuccess);
    stub.set(ApiOperatorr_GetSystemInfo, GetSystemInfo_Success);
    bool ret = StorageMgr::GetStorageSystemInfo(deviceInfo, storageSysInfo, errorStr);
    EXPECT_TRUE(!ret);
    stub.reset(ADDR(ApiOperatorManager, GetRestApiOperator));
    stub.reset(ADDR(ApiOperator, GetSystemInfo));
}

/*
 * 测试用例： 检查存储联通性
 * 前置条件： 获取用户等级和角色ID失败
 * CHECK点： getapi成功但返回为空
 */
TEST_F(StorageMgrTest, GetStorageSystemInfoGetLevelRoleIdFailed)
{
    ControlDeviceInfo deviceInfo;
    StorageSysInfo storageSysInfo;
    std::string errorStr;
    deviceInfo.m_ip = "1.1.1.1";
    deviceInfo.m_port = "8888";
    deviceInfo.m_userName = "name";
    deviceInfo.m_password = "123456";

    typedef int32_t (*fptr)(ApiOperatorManager*,const ControlDeviceInfo &, std::shared_ptr<ApiOperator> &);
    fptr ApiOperatorManager_GetRestApiOperator = (fptr)(&ApiOperatorManager::GetRestApiOperator);
    typedef int32_t (*fpt)(ApiOperator*,StorageSysInfo &, std::string &);
    fpt ApiOperatorr_GetSystemInfo = (fpt)(&ApiOperator::GetSystemInfo);
    FunGetInfo funGet = (FunGetInfo)(&ApiOperator::GetUserRoleAndLevel);

    Stub stub;
    stub.set(ApiOperatorManager_GetRestApiOperator, GetRestApiOperator_Success);
    stub.set(ApiOperatorr_GetSystemInfo, GetSystemInfo_Success);
    stub.set(funGet, GetUserRoleAndLevelFailed);
    bool ret = StorageMgr::GetStorageSystemInfo(deviceInfo, storageSysInfo, errorStr);
    EXPECT_TRUE(!ret);
    stub.reset(ADDR(ApiOperatorManager, GetRestApiOperator));
    stub.reset(ADDR(ApiOperator, GetSystemInfo));
}

/*
 * 测试用例： 检查存储联通性
 * 前置条件： 获取用户等级和角色ID成功，但未只读用户
 * CHECK点： getapi成功但返回为空
 */
TEST_F(StorageMgrTest, GetStorageSystemInfoReadOnlyUserFailed)
{
    ControlDeviceInfo deviceInfo;
    StorageSysInfo storageSysInfo;
    std::string errorStr;
    deviceInfo.m_ip = "1.1.1.1";
    deviceInfo.m_port = "8888";
    deviceInfo.m_userName = "name";
    deviceInfo.m_password = "123456";

    typedef int32_t (*fptr)(ApiOperatorManager*,const ControlDeviceInfo &, std::shared_ptr<ApiOperator> &);
    fptr ApiOperatorManager_GetRestApiOperator = (fptr)(&ApiOperatorManager::GetRestApiOperator);
    typedef int32_t (*fpt)(ApiOperator*,StorageSysInfo &, std::string &);
    fpt ApiOperatorr_GetSystemInfo = (fpt)(&ApiOperator::GetSystemInfo);
    FunGetInfo funGet = (FunGetInfo)(&ApiOperator::GetUserRoleAndLevel);

    Stub stub;
    stub.set(ApiOperatorManager_GetRestApiOperator, GetRestApiOperator_Success);
    stub.set(ApiOperatorr_GetSystemInfo, GetSystemInfo_Success);
    stub.set(funGet, GetUserRoleAndLevelReadOnly);
    bool ret = StorageMgr::GetStorageSystemInfo(deviceInfo, storageSysInfo, errorStr);
    EXPECT_TRUE(!ret);
    stub.reset(ADDR(ApiOperatorManager, GetRestApiOperator));
    stub.reset(ADDR(ApiOperator, GetSystemInfo));
}
}
