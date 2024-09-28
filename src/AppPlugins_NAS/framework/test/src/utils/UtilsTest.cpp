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
#include "log/Log.h"
#include "Utils.h"
#include "param_checker/ParamChecker.h"
#include "ApplicationProtectFramework_types.h"


using namespace AppProtect;
using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

class UtilsTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase();
    static void TearDownTestCase() {};
};

void UtilsTest::SetUpTestCase()
{
    std::string xmlPath = "../src/utils/param_check_test.xml";
    Module::StructChecker::Instance().Init(xmlPath);
}
/*
 * 用例名称: 测试COMMON_VALUE_UINT64类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, COMMON_VALUE_UINT64_TEST)
{
    Json::Value jsonValue;
    jsonValue["unint64Test"] = 0;
    EXPECT_NO_THROW(ParamCheck("COMMON_VALUE_UINT64_TEST", jsonValue));

    jsonValue["unint64Test"] = 999;
    EXPECT_NO_THROW(ParamCheck("COMMON_VALUE_UINT64_TEST", jsonValue));

    jsonValue["unint64Test"] = -1;
    EXPECT_THROW(ParamCheck("COMMON_VALUE_UINT64_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
/*
 * 用例名称: 测试COMMON_VALUE_10000类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, COMMON_VALUE_10000_TEST)
{
    Json::Value jsonValue;
    jsonValue["uintLg10000Test"] = 999;
    EXPECT_NO_THROW(ParamCheck("COMMON_VALUE_10000_TEST", jsonValue));

    jsonValue["uintLg10000Test"] = 10001;
    EXPECT_THROW(ParamCheck("COMMON_VALUE_10000_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
/*
 * 用例名称: 测试COMMON_BOOL_VALUE类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, COMMON_BOOL_VALUE_TEST)
{
    Json::Value jsonValue;
    jsonValue["bollTest"] = "false";
    EXPECT_NO_THROW(ParamCheck("COMMON_BOOL_VALUE_TEST", jsonValue));

    jsonValue["bollTest"] = "true";
    EXPECT_NO_THROW(ParamCheck("COMMON_BOOL_VALUE_TEST", jsonValue));

    jsonValue["bollTest"] = "1";
    EXPECT_NO_THROW(ParamCheck("COMMON_BOOL_VALUE_TEST", jsonValue));

    jsonValue["bollTest"] = "0";
    EXPECT_NO_THROW(ParamCheck("COMMON_BOOL_VALUE_TEST", jsonValue));

    jsonValue["bollTest"] = "else";
    EXPECT_THROW(ParamCheck("COMMON_BOOL_VALUE_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
/*
 * 用例名称: 测试JOB_NAME类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, JOB_NAME_TEST)
{
    Json::Value jsonValue;
    jsonValue["jobNameTest"] = "backup123";
    EXPECT_NO_THROW(ParamCheck("JOB_NAME_TEST", jsonValue));

    jsonValue["jobNameTest"] = "10001913c47e3-c5b0-43bd-b70b-b0ca3c48029410001913c47e3-c5b0-43bd-b70b-b0ca3c480294";
    EXPECT_THROW(ParamCheck("JOB_NAME_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
/*
 * 用例名称: 测试COMMON_PATH类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, COMMON_PATH_TEST)
{
    Json::Value jsonValue;
    jsonValue["commonPathTest"] = "/opt/DataBackup/ProtectClient";
    EXPECT_NO_THROW(ParamCheck("COMMON_PATH_TEST", jsonValue));

    jsonValue["commonPathTest"] = "10001913c47e3-c5b0-43bd-b70b-b0ca3c480294";
    EXPECT_THROW(ParamCheck("COMMON_PATH_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
/*
 * 用例名称: 测试PAGE_SIZE类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, PAGE_SIZE_TEST)
{
    Json::Value jsonValue;
    jsonValue["pageSizeTest"] = 10;
    EXPECT_NO_THROW(ParamCheck("PAGE_SIZE_TEST", jsonValue));

    jsonValue["pageSizeTest"] = 10000;
    EXPECT_THROW(ParamCheck("PAGE_SIZE_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
/*
 * 用例名称: 测试SCRIPT_PATH类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, SCRIPT_PATH_TEST)
{
    Json::Value jsonValue;
    jsonValue["scritPathTest"] = "test.sh";
    EXPECT_NO_THROW(ParamCheck("SCRIPT_PATH_TEST", jsonValue));

    jsonValue["scritPathTest"] = "test.hs";
    EXPECT_THROW(ParamCheck("SCRIPT_PATH_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
/*
 * 用例名称: 测试RESOURCE_FILTER_MODE类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, RESOURCE_FILTER_MODE_TEST)
{
    Json::Value jsonValue;
    jsonValue["resouceFilterModeTest"] = "INCLUDE";
    EXPECT_NO_THROW(ParamCheck("RESOURCE_FILTER_MODE_TEST", jsonValue));

    jsonValue["resouceFilterModeTest"] = "INCLUED";
    EXPECT_THROW(ParamCheck("RESOURCE_FILTER_MODE_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
/*
 * 用例名称: 测试RESTORE_MODE类型参数校验
 * 前置条件: 加载测试xml配置文件
 * check点：测试是否抛出异常
 */
TEST_F(UtilsTest, RESTORE_MODE_TEST)
{
    Json::Value jsonValue;
    jsonValue["restoreModeTest"] = "RemoteRestore";
    EXPECT_NO_THROW(ParamCheck("RESTORE_MODE_TEST", jsonValue));

    jsonValue["restoreModeTest"] = "RemoteRestore2";
    EXPECT_THROW(ParamCheck("RESTORE_MODE_TEST", jsonValue), AppProtect::AppProtectPluginException);
}
