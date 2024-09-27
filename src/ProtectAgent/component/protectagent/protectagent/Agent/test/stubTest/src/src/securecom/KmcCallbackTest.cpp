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
#include "securecom/KmcCallbackTest.h"

namespace {
static mp_void StubCLoggerLog(mp_void){
    return;

}

#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(mp_string,mp_string,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_bool StubTrue()
{
    return MP_TRUE;
}

mp_bool StubFalse()
{
    return MP_FALSE;
}

pthread_mutex_t* StubPthreadMutexNull()
{
    return nullptr;
}
}

TEST_F(KmcCallbackTest, Fopen){
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    char *filePathName;
    unsigned int mode;
    Fopen(filePathName, mode);
}

TEST_F(KmcCallbackTest, Fread){
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    void *buffer;
    size_t count;
    void *stream;
    Fread(buffer, count, stream);
}

TEST_F(KmcCallbackTest, Fflush){
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    void *stream;
    Fflush(stream);
}

TEST_F(KmcCallbackTest, Fremove){
    mp_int32 iRet;
    char *path;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(remove, StubSuccess);
    iRet = Fremove(path);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(KmcCallbackTest, CreateThreadLock){
    mp_int32 iRet;
    void *stream;
    void **Pstream = nullptr;
    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = CreateThreadLock(Pstream);
    EXPECT_EQ(WSEC_FALSE, iRet);

    Pstream = &stream;
    iRet = CreateThreadLock(Pstream);
    EXPECT_EQ(WSEC_FALSE, iRet);

    *Pstream = nullptr;
    iRet = CreateThreadLock(Pstream);
    EXPECT_EQ(WSEC_TRUE, iRet);
}

TEST_F(KmcCallbackTest, DestroyThreadLock){
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    pthread_mutex_t *stream = nullptr;
    DestroyThreadLock(stream);
}

TEST_F(KmcCallbackTest, ThreadLock){
    mp_int32 iRet;
    pthread_mutex_t *stream = nullptr;
    pthread_mutex_t *Pstream;
    stub.set(&CLogger::Log, StubCLoggerLog);

    ThreadLock(stream);
    stub.set(pthread_mutex_lock, StubFailed);
    ThreadLock(Pstream);
}

TEST_F(KmcCallbackTest, ThreadUnlock){
    mp_int32 iRet;
    pthread_mutex_t *stream = nullptr;
    pthread_mutex_t *Pstream;
    stub.set(&CLogger::Log, StubCLoggerLog);

    ThreadUnlock(stream);
    stub.set(pthread_mutex_unlock, StubFailed);
    ThreadUnlock(Pstream);
}

TEST_F(KmcCallbackTest, GetEntropy){
    mp_int32 iRet;
    unsigned char **ppEnt;
    unsigned char *pEnt;
    size_t buffLen = 1;
    stub.set(&CLogger::Log, StubCLoggerLog);

    ppEnt = nullptr;
    iRet = GetEntropy(ppEnt, buffLen);
    EXPECT_EQ(WSEC_FALSE, iRet);
}
