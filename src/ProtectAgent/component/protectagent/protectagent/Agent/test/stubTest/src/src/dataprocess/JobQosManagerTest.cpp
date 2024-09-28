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
#include "dataprocess/JobQosManagerTest.h"
#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubFileJobQosManagerGetValueInt32Return);                                                                      \
    } while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}
TEST_F(JobQosManagerTest, JobQosManager)
{
    JobQosManager om;    
}

TEST_F(JobQosManagerTest, SetJobQos)
{
    JobQosManager om;
    double qos = 2.2;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SetJobQos(qos); 
}

TEST_F(JobQosManagerTest, Init)
{
    JobQosManager om;
    om.Init(); 
}

TEST_F(JobQosManagerTest, GetJobQosManager)
{
    JobQosManager om;
    std::shared_ptr<JobQosManager> iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string identity = "test";
    iRet = om.GetJobQosManager(identity);
}

TEST_F(JobQosManagerTest, UnRegisterQos)
{
    JobQosManager om;
    string identity = "test";
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.UnRegisterQos(identity);
}

TEST_F(JobQosManagerTest, ProduceQos)
{
    JobQosManager om;
    om.m_Stoped = false;
    {
        om.m_ConsumeQos = 2000004;
        om.ProduceQos();
    }
}

TEST_F(JobQosManagerTest, TryGetEnoughQos)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    JobQosManager om;
    mp_double require = 2.2;
    mp_double obtain = 0.0;

    {
        om.m_ConsumeQos = 12.9;
        om.m_ProduceQos =2.2;
        om.m_Qos = 1.5;
        om.TryGetEnoughQos(require,obtain);
    }

    {
        om.m_ConsumeQos = 5.9;
        om.m_ProduceQos =2.2;
        om.m_Qos = 1.5;
        om.TryGetEnoughQos(require,obtain);
    }
}

TEST_F(JobQosManagerTest, ConsumeQos)
{
    JobQosManager om;
    bool iRet = false;
    mp_double require = 2.2;
    mp_int32 timeout = -2;

    stub.set(&CLogger::Log, StubCLoggerLog);
    //m_Stoped = true
    {
        iRet = om.ConsumeQos(require,timeout);
        EXPECT_EQ(true, iRet);
    }
    //m_Stoped = false
    {
        om.m_Stoped = false;
        iRet = om.ConsumeQos(require,timeout);
        EXPECT_EQ(false, iRet);
    }
}

TEST_F(JobQosManagerTest, SetProduceFrequency)
{
    JobQosManager om;
    unsigned int frequency = 22;
    om.SetProduceFrequency(frequency);

    frequency = 0;
    om.SetProduceFrequency(frequency); 
}

TEST_F(JobQosManagerTest, GetJobSpeed)
{
    JobQosManager om;
    om.GetJobSpeed();
}

TEST_F(JobQosManagerTest, StopQosLimit)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    JobQosManager om;
    om.StopQosLimit();
}

TEST_F(JobQosManagerTest, StartQosLimit)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    JobQosManager om;
    om.StartQosLimit();
}
