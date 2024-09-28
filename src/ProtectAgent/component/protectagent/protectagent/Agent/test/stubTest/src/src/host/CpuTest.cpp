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
#include "host/CpuTest.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_bool StubGetCpuArchSuccess(const mp_string& archMapKey)
{
    return MP_TRUE;
}

mp_bool StubGetCpuArchFail(const mp_string& archMapKey)
{
    return MP_FALSE;
}

mp_int32 StubGetLinuxCpuInfoSuccess(void *obj, vector<cpu_info_t>& cpus)
{
    cpu_info_t temp = { "arch", "modelName", "mhz", "cache", 8 };
    cpus.push_back(temp);
    return MP_SUCCESS;
}

mp_int32 StubGetLinuxCpuInfoSuccessEmpty(vector<cpu_info_t>& cpus)
{
    return MP_SUCCESS;
}

mp_int32 StubGetLinuxCpuInfoFail(vector<cpu_info_t>& cpus)
{
    return MP_FAILED;
}

/* 初始化用例，无返回值 */
TEST_F(CpuTest, init)
{
    CCpu work;
    cpu_info_t cpu;
    work.InitCpuInfo(cpu);
    work.InitWinCpuArchMap();
}

TEST_F(CpuTest, GetCpuInfo)
{
    CCpu work;
    mp_int32 iRet;
    vector<cpu_info_t> cpuPool;
    stub.set(&CLogger::Log, StubCLoggerLog);

    iRet = work.GetCpuInfo(cpuPool);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CpuTest, UTTest)
{
    CCpu work;
    mp_int32 iRet;
    vector<cpu_info_t> cpuPool;
    stub.set(&CLogger::Log, StubCLoggerLog);

    iRet = work.GetARMLinuxCpuInfo(cpuPool);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：获取CPU信息
 * 前置条件：
 * check点：读取成功，检查返回值
            读取失败，检查返回值
 */
TEST_F(CpuTest, GetCpuInfoValue)
{
    CCpu work;
    mp_bool iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    
    mp_string infoStr = "cpu MHz         : 2400.000";
    mp_string Value;
    work.cpuInfoKey = "cpu MHz";
    iRet = work.GetCpuInfoValue(infoStr, Value);
    EXPECT_EQ(iRet, MP_TRUE);
    EXPECT_EQ(Value, "2400.000");

    infoStr = "";
    iRet = work.GetCpuInfoValue(infoStr, Value);
    EXPECT_EQ(iRet, MP_FALSE);

    infoStr = "111";
    iRet = work.GetCpuInfoValue(infoStr, Value);
    EXPECT_EQ(iRet, MP_FALSE);

}

/*
 * 用例名称：字符串转换为uint32
 * 前置条件：
 * check点：转换后数值
 */
TEST_F(CpuTest, StringToUnsignedNumber32)
{
    CCpu work;
    mp_bool iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    
    mp_string originStr = "111";
    iRet = work.StringToUnsignedNumber32(originStr);
    EXPECT_EQ(iRet, 111);
}

/*
 * 用例名称：获取CPU信息
 * 前置条件：
 * check点：读取成功，检查返回值
            读取失败，检查返回值
 */
TEST_F(CpuTest, StubGetCpuInfo)
{
    CCpu work;
    mp_bool iRet;
    
    stub.set(&CCpu::GetCpuArch, StubGetCpuArchSuccess);
    stub.set(&CCpu::GetARMLinuxCpuInfo, StubGetLinuxCpuInfoSuccess);
    stub.set(&CCpu::GetX86LinuxCpuInfo, StubGetLinuxCpuInfoSuccess);
    work.m_cpuArch = "aarch64";
    vector<cpu_info_t> cpus;
    iRet = work.GetCpuInfo(cpus);
    EXPECT_EQ(iRet, MP_SUCCESS);
    cpus.clear();

    work.m_cpuArch = "x86";
    iRet = work.GetCpuInfo(cpus);
    EXPECT_EQ(iRet, MP_SUCCESS);
    cpus.clear();

    stub.set(&CCpu::GetCpuArch, StubGetCpuArchFail);
    iRet = work.GetCpuInfo(cpus);
    EXPECT_EQ(iRet, MP_FAILED);
    cpus.clear();

    stub.set(&CCpu::GetCpuArch, StubGetCpuArchSuccess);
    stub.set(&CCpu::GetX86LinuxCpuInfo, StubGetLinuxCpuInfoSuccessEmpty);
    iRet = work.GetCpuInfo(cpus);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：获取X86CPU信息
 * 前置条件：
 * check点：读取成功，检查返回值
            读取失败，检查返回值
 */
TEST_F(CpuTest, GetX86LinuxCpuInfo)
{
    CCpu work;
    mp_bool iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    
    stub.set(&CCpu::GetCpuArch, StubGetCpuArchSuccess);
    stub.set(&CCpu::GetARMLinuxCpuInfo, StubGetLinuxCpuInfoSuccess);
    stub.set(&CCpu::GetX86LinuxCpuInfo, StubGetLinuxCpuInfoSuccess);
    vector<cpu_info_t> cpus;
    iRet = work.GetX86LinuxCpuInfo(cpus);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * 用例名称：获取ARMCPU信息
 * 前置条件：
 * check点：读取成功，检查返回值
            读取失败，检查返回值
 */
TEST_F(CpuTest, GetARMLinuxCpuInfo)
{
    CCpu work;
    mp_bool iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    
    vector<cpu_info_t> cpus;
    iRet = work.GetARMLinuxCpuInfo(cpus);
    EXPECT_EQ(iRet, MP_FAILED);
}
