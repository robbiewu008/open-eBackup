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
#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "llt_stub/stub.h"
#include "security/cmd/CmdParam.h"
#include "security/cmd/CmdExecutor.h"
#include "security/cmd/CmdWhitelist.h"

using namespace Module;

class CmdAntiInjectTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CmdAntiInjectTest::SetUp()
{}

void CmdAntiInjectTest::TearDown()
{}

void CmdAntiInjectTest::SetUpTestCase()
{
    std::unordered_set<std::string> cmdWhitelist{"ps", "grep", "awk", "print", "ping", "ldd", "ls", "wc"};
    std::unordered_set<std::string> pathWhitelist{"/usr/bin"};
    RegisterWhitelist(cmdWhitelist, pathWhitelist);
}

void CmdAntiInjectTest::TearDownTestCase()
{}

/**
 * 测试用例： 测试已存在的硬编码命令执行是否成功
 * 前置条件： 硬编码命令编译期已存在
 * CHECK点： 命令执行成功
 */
TEST_F(CmdAntiInjectTest, ExsitHardCodedCmdExecSuccess)
{
    std::vector<CmdParam> cmd;
    std::vector<std::string> result;
    int retCode = Module::RunCommand(CMD_NAME(CMD_AGENT_GET_MEMORY), cmd, result);
    EXPECT_EQ(retCode, 0);
}

/**
 * 测试用例： 测试不存在的硬编码命令执行是否成功
 * 前置条件： 硬编码命令不存在
 * CHECK点： 命令执行失败
 */
TEST_F(CmdAntiInjectTest, NotExsitHardCodedCmdExecFailed)
{
    std::vector<CmdParam> cmd;
    std::vector<std::string> result;
    int retCode = Module::RunCommand("CMD_AGENT_GET_MEMORY_NOT_EXSIT", cmd, result);
    EXPECT_NE(retCode, 0);
}

/**
 * 测试用例： 测试在白名单中的命令执行是否成功
 * 前置条件： 命令在白名单中
 * CHECK点： 命令执行成功
 */
TEST_F(CmdAntiInjectTest, InWhitelistCmdExecSuccess)
{
    // ping 127.0.0.1 -c 1
    std::vector<CmdParam> cmd{
        CmdParam(COMMON_CMD_NAME, "ping"),
        "127.0.0.1",
        CmdParam(CMD_OPTION_PARAM, "-c"),
        "1"
    };

    std::vector<std::string> result;
    int retCode = Module::RunCommand("ping", cmd, result);
    EXPECT_EQ(retCode, 0);
}

/**
 * 测试用例： 测试不在白名单中的命令执行是否成功
 * 前置条件： 命令不在白名单中
 * CHECK点： 命令执行失败
 */
TEST_F(CmdAntiInjectTest, NotInWhitelistCmdExecFailed)
{
    // iptables -nL
    std::vector<CmdParam> cmd{"iptables", "-nL"};
    std::vector<std::string> result;
    int retCode = Module::RunCommand("iptables", cmd, result);
    EXPECT_NE(retCode, 0);
}

/**
 * 测试用例： 测试命令操作的路径在路径白名单时命令执行是否成功
 * 前置条件： 命令操作的路径在路径白名单
 * CHECK点： 命令执行成功
 */
TEST_F(CmdAntiInjectTest, PathInWhitelistCmdExecSuccess)
{
    // ldd /usr/bin/ping | grep libc.so
    std::vector<CmdParam> cmd{
        CmdParam(COMMON_CMD_NAME, "ldd"),
        CmdParam(PATH_PARAM, "/usr/bin/ping"),
        CmdParam(PIPELINE_PARAM, "|"),
        CmdParam(COMMON_CMD_NAME, "grep"),
        "libc.so"
    };

    std::vector<std::string> result;
    int retCode = Module::RunCommand("ldd", cmd, result);
    EXPECT_EQ(retCode, 0);
}

/**
 * 测试用例： 测试命令操作的路径不在路径白名单时命令执行是否成功
 * 前置条件： 命令操作的路径不在路径白名单
 * CHECK点： 命令执行失败
 */
TEST_F(CmdAntiInjectTest, PathNotInWhitelistCmdExecFailed)
{
    // ldd /usr/bin/notexsit/ping | grep libc.so
    std::vector<CmdParam> cmd{
        CmdParam(COMMON_CMD_NAME, "ldd"),
        CmdParam(PATH_PARAM, "/usr/bin/notexsit/ping"),
        CmdParam(PIPELINE_PARAM, "|"),
        CmdParam(COMMON_CMD_NAME, "grep"),
        "libc.so"
    };

    std::vector<std::string> result;
    int retCode = Module::RunCommand("ldd", cmd, result);
    EXPECT_NE(retCode, 0);
}

/**
 * 测试用例： 测试注册单次生效的路径白名单在第二次命令执行时是否有效
 * 前置条件： 注册单次生效的路径白名单
 * CHECK点： 第一次命令执行成功，第二次命令执行失败
 */
TEST_F(CmdAntiInjectTest, OnlyOncePathCmdExecFirstOKSecondNOK)
{
    // ls -l /etc/sysconfig/ | wc -l
    std::vector<CmdParam> cmd{
        CmdParam(COMMON_CMD_NAME, "ls"),
        CmdParam(CMD_OPTION_PARAM, "-l"),
        CmdParam(PATH_PARAM, "/etc/sysconfig/"),
        CmdParam(PIPELINE_PARAM, "|"),
        CmdParam(COMMON_CMD_NAME, "wc"),
        CmdParam(CMD_OPTION_PARAM, "-l"),
    };

    std::unordered_set<std::string> pathWhite{"/etc/sysconfig"};
    std::vector<std::string> result;
    int retCode = Module::RunCommand("ls", cmd, result, pathWhite); // 带临时路径白名单
    EXPECT_EQ(retCode, 0); // 第一次执行成功

    retCode = Module::RunCommand("ls", cmd, result); // 不带路径白名单
    EXPECT_NE(retCode, 0); // 第二次执行失败
}

/**
 * 测试用例： 测试包含特殊字符的白名单命令执行是否成功
 * 前置条件： 命令在白名单中，且包含特殊字符
 * CHECK点： 命令执行成功
 */
TEST_F(CmdAntiInjectTest, InWhitelistWithSpecailCharCmdExecSuccess)
{
    // ps -ef | grep 'test_module' | grep -v grep | awk -F' ' '{print $8}'
    std::vector<CmdParam> cmd{
        CmdParam(COMMON_CMD_NAME, "ps"),
        CmdParam(CMD_OPTION_PARAM, "-ef"),
        CmdParam(PIPELINE_PARAM, "|"),
        CmdParam(COMMON_CMD_NAME, "grep"),
        CmdParam(HARD_QUOTE_PARAM, "'"),
        CmdParam("test_module", false),
        CmdParam(HARD_QUOTE_PARAM, "'", false),
        CmdParam(PIPELINE_PARAM, "|"),
        CmdParam(COMMON_CMD_NAME, "grep"),
        CmdParam(CMD_OPTION_PARAM, "-v"),
        "grep",
        CmdParam(PIPELINE_PARAM, "|"),
        CmdParam(COMMON_CMD_NAME, "awk"),
        CmdParam(CMD_OPTION_PARAM, "-F"),
        CmdParam(HARD_QUOTE_PARAM, "'", false),
        CmdParam(HARD_QUOTE_PARAM, "'"),
        CmdParam(HARD_QUOTE_PARAM, "'"),
        CmdParam("{print", false),
        CmdParam(VAR_SUBS_PARAM, "$"),
        CmdParam("8}", false),
        CmdParam(HARD_QUOTE_PARAM, "'", false),
    };

    std::vector<std::string> result;
    int retCode = Module::RunCommand("ps", cmd, result);
    EXPECT_EQ(retCode, 0);
}

/**
 * 测试用例： 测试普通参数包含特殊字符命令是否执行成功，模拟通过外部参数进行命令注入的场景
 * 前置条件： 字符串参数包含特殊字符
 * CHECK点： 命令执行失败
 */
TEST_F(CmdAntiInjectTest, ParamWithSpecailCharCmdExecFailed)
{
    // ping -c 1 127.0.0.1 && echo 'cmd injection'
    std::vector<CmdParam> cmd{
        CmdParam(COMMON_CMD_NAME, "ping"),
        CmdParam(CMD_OPTION_PARAM, "-c"),
        "1",
        R"(127.0.0.1 && echo 'cmd injection')"
    };

    std::vector<std::string> result;
    int retCode = Module::RunCommand("ping", cmd, result);
    EXPECT_NE(retCode, 0);

    // ping -c 1 127.0.0.1;echo 'cmd injection'
    cmd = {
        CmdParam(COMMON_CMD_NAME, "ping"),
        CmdParam(CMD_OPTION_PARAM, "-c"),
        "1",
        R"(127.0.0.1;echo 'cmd injection')"
    };
    retCode = Module::RunCommand("ping", cmd, result);
    EXPECT_NE(retCode, 0);
}

/**
 * 测试用例： 测试命令路径中含相对路径，检查命令能否执行成功
 * 前置条件： 命令路径中含相对路径
 * CHECK点： 命令执行成功
 */
TEST_F(CmdAntiInjectTest, RelativePathCmdExecSucess)
{
    // ls -l /etc/sysconfig/ | wc -l
    std::vector<CmdParam> cmd{
        CmdParam(COMMON_CMD_NAME, "ls"),
        CmdParam(CMD_OPTION_PARAM, "-l"),
        CmdParam(PATH_PARAM, "/etc/sysconfig/modules/../.././sysconfig"),
        CmdParam(PIPELINE_PARAM, "|"),
        CmdParam(COMMON_CMD_NAME, "wc"),
        CmdParam(CMD_OPTION_PARAM, "-l"),
    };

    std::unordered_set<std::string> pathWhite{"/etc/sysconfig"};
    std::vector<std::string> result;
    int retCode = Module::RunCommand("ls", cmd, result, pathWhite);
    EXPECT_EQ(retCode, 0);
}
