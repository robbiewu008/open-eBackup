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
#include "taskmanager/externaljob/JobStateDBTest.h"
#include "common/DB.h"

using namespace AppProtect;
static mp_int32 StubExecSqlSUC(const mp_string& strSql, DbParamStream& dpl)
{
    return MP_SUCCESS;
}

static mp_int32 StubExecSqlFailed(const mp_string& strSql, DbParamStream& dpl)
{
    return MP_FAILED;
}

mp_int32 QueryTableSUC(mp_void* pThis, const mp_string& strSql, DbParamStream& dpl, DBReader& readBuff, 
    mp_int32& iRowCount, mp_int32& iColCount)
{
    iRowCount = 1;
    mp_string strTmp = "pluginName";
    readBuff << strTmp;
    strTmp = "mainJobID";
    readBuff << strTmp;
    strTmp = "subJobID";
    readBuff << strTmp;
    strTmp = "1";
    readBuff << strTmp;
    strTmp = "1";
    readBuff << strTmp;
    strTmp = "6";
    readBuff << strTmp;
    strTmp = "/root";
    readBuff << strTmp;
    strTmp = "192.168.1.1";
    readBuff << strTmp;
    return MP_SUCCESS;
}

/*
* 用例名称：任务插入成功
* 前置条件：1、无
* check点：1、返回值为 MP_SUCCESS
*/
TEST_F(JobStateDBTest, INSERT_SUCC)
{
    Stub stub;
    stub.set(&CDB::ExecSql, &StubExecSqlSUC);
    PluginJobData jobInfo;
    jobInfo.mainID = m_mainID;
    jobInfo.status = m_status;
    mp_int32 iRet = JobStateDB::GetInstance().InsertRecord(jobInfo);
    ASSERT_EQ(iRet, MP_SUCCESS);
}

/*
* 用例名称：任务插入失败
* 前置条件：1、无
* check点：1、返回值为 MP_FAILED
*/
TEST_F(JobStateDBTest, INSERT_FAILED)
{
    Stub stub;
    stub.set(&CDB::ExecSql, &StubExecSqlFailed);
    PluginJobData jobInfo;
    jobInfo.mainID = m_mainID;
    jobInfo.status = m_status;
    mp_int32 iRet = JobStateDB::GetInstance().InsertRecord(jobInfo);
    ASSERT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：任务查询成功
* 前置条件：1、无
* check点：1、返回值为 MP_SUCCESS
*/
TEST_F(JobStateDBTest, QUERY_SUCC)
{
    Stub stub;
    stub.set(&CDB::QueryTable, &QueryTableSUC);
    std::vector<PluginJobData> jobs;
    mp_int32 iRet = JobStateDB::GetInstance().QueryAllJob(jobs);
    ASSERT_EQ(iRet, MP_SUCCESS);
}

/*
* 用例名称：查询失败
* 前置条件：1、无
* check点：1、返回值为 MP_FAILED
*/
TEST_F(JobStateDBTest, QUERY_FAILED)
{
    Stub stub;
    stub.set(&CDB::ExecSql, &StubExecSqlFailed);
    std::vector<PluginJobData> jobs;
    mp_int32 iRet = JobStateDB::GetInstance().QueryAllJob(jobs);
    ASSERT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：查询数据成功
* 前置条件：1、mock QueryTable
* check点：1、返回值为 MP_SUCCESS   2. 插件名为pluginName
*/
TEST_F(JobStateDBTest, QueryJob_TEST)
{
    Stub stub;
    stub.set(&CDB::QueryTable, &QueryTableSUC);
    PluginJobData jobData;
    EXPECT_EQ(MP_SUCCESS, JobStateDB::GetInstance().QueryJob(m_mainID, m_subID, jobData));
    EXPECT_EQ("pluginName", jobData.appType);
}

/*
* 用例名称：删除成功
* 前置条件：1、无
* check点：1、返回值为 MP_SUCCESS
*/
TEST_F(JobStateDBTest, DEL_SUCC)
{
    Stub stub;
    stub.set(&CDB::ExecSql, &StubExecSqlSUC);
    EXPECT_EQ(MP_SUCCESS, JobStateDB::GetInstance().DeleteRecord(m_mainID, m_subID));
    EXPECT_EQ(MP_SUCCESS, JobStateDB::GetInstance().DeleteRecord(m_mainID));
}

/*
* 用例名称：删除失败
* 前置条件：1、无
* check点：1、返回值为 MP_FAILED
*/
TEST_F(JobStateDBTest, DEL_FAILED)
{
    Stub stub;
    stub.set(&CDB::ExecSql, &StubExecSqlFailed);
    EXPECT_EQ(MP_FAILED, JobStateDB::GetInstance().DeleteRecord(m_mainID, m_subID));
    EXPECT_EQ(MP_FAILED, JobStateDB::GetInstance().DeleteRecord(m_mainID));
}

/*
* 用例名称：更新数据成功
* 前置条件：1、无
* check点：1、返回值为 MP_SUCCESS
*/
TEST_F(JobStateDBTest, UPDATE_JOB_STATE_SUCCESS)
{
    Stub stub;
    stub.set(&CDB::ExecSql, &StubExecSqlSUC);
    mp_int32 iRet = JobStateDB::GetInstance().UpdateStatus(m_mainID, m_subID, m_status);
    ASSERT_EQ(iRet, MP_SUCCESS);
}

/*
* 用例名称：更新数据失败
* 前置条件：1、无
* check点：1、返回值为 MP_FAILED
*/
TEST_F(JobStateDBTest, UPDATE_JOB_STATE_FAILED)
{
    Stub stub;
    stub.set(&CDB::ExecSql, &StubExecSqlFailed);
    mp_int32 iRet = JobStateDB::GetInstance().UpdateStatus(m_mainID, m_subID, m_status);
    ASSERT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：更新可运行状态
* 前置条件：1、mock CDB::ExecSql
* check点：
*/
TEST_F(JobStateDBTest, UpdateRunEnable)
{
    Stub stub;
    stub.set(&CDB::ExecSql, &StubExecSqlSUC);
    EXPECT_EQ(MP_SUCCESS, JobStateDB::GetInstance().UpdateRunEnable(m_mainID, 0));
    stub.set(&CDB::ExecSql, &StubExecSqlFailed);
    EXPECT_EQ(MP_FAILED, JobStateDB::GetInstance().UpdateRunEnable(m_mainID, 0));
}