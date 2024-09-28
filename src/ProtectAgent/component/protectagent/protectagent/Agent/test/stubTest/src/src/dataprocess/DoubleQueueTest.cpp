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
#include "dataprocess/DoubleQueueTest.h"
#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubFileDoubleQueueGetValueInt32Return);                                                                   \
    } while (0)

using namespace std;
using namespace AGENT_VMWARENATIVE_DOUBLEQUEUE;

static const mp_int32 QUEUE_SIZE = 100;

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(DoubleQueueTest, DoubleQueue)
{
    DoubleQueue om;
}

TEST_F(DoubleQueueTest, EnQueue)
{
    st_disk_datablock datablock;
    datablock.index = 1;
    DoubleQueue om;
    om.EnQueue(datablock);
}

TEST_F(DoubleQueueTest, DeQueueToDataLun)
{
    FILE *file;
    int done = 0;
    int condition = 0;
    DoubleQueue om;
    bool iRet = false;

    // StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //NULL == file
    {
        iRet = om.DeQueueToDataLun(file, done, condition);
        EXPECT_EQ(true, iRet);
    }
    //return false
    { 
        done = 1;
        file = fopen("test", "w");
        iRet = om.DeQueueToDataLun(file, done, condition);
        EXPECT_EQ(false, iRet);
    }
    //read queue is empty
    {
        st_disk_datablock tmp;
        tmp.size = 1;
        tmp.index = 1;
        tmp.dataBuff = "tttt";
        om.m_readQueue.push_back(tmp);
        file = fopen("test", "w");
        iRet = om.DeQueueToDataLun(file, done, condition);
        EXPECT_EQ(false, iRet);
    }
    // stub.reset(ADDR(CLogger, Log));
}

TEST_F(DoubleQueueTest, DeQueueToVMwareDisk)
{
    int done = 0;
    int condition = 0;
    DoubleQueue om;
    bool iRet = false;
    std::shared_ptr<VMwareDiskApi> sPtr;

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //NULL == sPtr
    {
        iRet = om.DeQueueToVMwareDisk(sPtr, done, condition);
        EXPECT_EQ(false, iRet);
    }
    // return false
    { 
        done = 1;
        VMwareDiskOperations operations;
        VixDiskLibConnection connection;
        MessageLoop *const messageLoop = nullptr;
        VMwareDiskApi* om_api = new VMwareDiskApi(operations, connection, messageLoop);
        std::shared_ptr<VMwareDiskApi> sPtr(om_api);
        iRet = om.DeQueueToVMwareDisk(sPtr, done, condition);
        EXPECT_EQ(false, iRet);
    }
    // read queue is empty
    {
        st_disk_datablock tmp;
        tmp.size = 1;
        tmp.index = 1;
        tmp.dataBuff = "tttt";
        om.m_readQueue.push_back(tmp);
        VMwareDiskOperations operations;
        VixDiskLibConnection connection;
        MessageLoop *const messageLoop = nullptr;
        VMwareDiskApi* om_api = new VMwareDiskApi(operations, connection, messageLoop);
        typedef VMWARE_DISK_RET_CODE (*fptr)(VMwareDiskApi*,VMWARE_DISK_RET_CODE);
        fptr VMwareDiskApi_Write = (fptr)(&VMwareDiskApi::Write);   //获取虚函数地址
        stub.set(VMwareDiskApi_Write, StubWrite);
        std::shared_ptr<VMwareDiskApi> sPtr(om_api);
        iRet = om.DeQueueToVMwareDisk(sPtr, done, condition);
        EXPECT_EQ(true, iRet);
    }
    // stub.reset(ADDR(CLogger, Log));
}

TEST_F(DoubleQueueTest, DeQueue)
{
    StubClogToVoidLogNullPointReference();
    bool iRet = false;
    DoubleQueue om;
    iRet = om.DeQueue();
    EXPECT_EQ(true, iRet);
}

TEST_F(DoubleQueueTest, GetReadQueue)
{
    StubClogToVoidLogNullPointReference();
    DoubleQueue om;
    om.GetReadQueue();
}

TEST_F(DoubleQueueTest, GetWriteQueue)
{
    StubClogToVoidLogNullPointReference();
    DoubleQueue om;
    om.GetWriteQueue();
}

TEST_F(DoubleQueueTest, GetNumberOfDataBlockCompleted)
{
    StubClogToVoidLogNullPointReference();
    DoubleQueue om;
    om.GetNumberOfDataBlockCompleted();
}

TEST_F(DoubleQueueTest, SetNumberOfDataBlockCompleted)
{
    StubClogToVoidLogNullPointReference();
    mp_uint64 datablockNum = 2;
    DoubleQueue om;
    om.SetNumberOfDataBlockCompleted(datablockNum);
}
