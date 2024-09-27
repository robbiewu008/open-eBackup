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
#include "common/ThreadTest.h"

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

TEST_F(CMpThreadTest,Create){
    DoGetJsonStringTest();
    thread_id_t* id;
    thread_proc_t proc;
    mp_void* arg;
    mp_uint32 uiStackSize;
    
    CMpThread::Create(id,proc,arg,uiStackSize);
    
    stub.set(&pthread_attr_setstacksize, Stubpthread_attr_setstacksize);
    stub.set(&pthread_create, Stubpthread_create);
    CMpThread::Create(id,proc,arg,uiStackSize);
    
    stub.set(&pthread_create, Stubpthread_create0);
    CMpThread::Create(id,proc,arg,uiStackSize);
}

TEST_F(CMpThreadTest,WaitForEnd){
    DoGetJsonStringTest();
    thread_id_t* id;
    mp_void** retValue;
    
    stub.set(&pthread_join, Stubpthread_join);
    CMpThread::WaitForEnd(id,retValue);
    
    stub.set(&pthread_join, Stubpthread_join0);
    CMpThread::WaitForEnd(id,retValue);
}

TEST_F(CMpThreadTest,InitCond){
    thread_cond_t pcond;
    
    CMpThread::InitCond(pcond);
    
    // thread_cond_t* pcond1 = NULL;
    // CMpThread::InitCond(pcond1);
}

TEST_F(CMpThreadTest,DestroyCond){
    thread_cond_t pcond;
    
    stub.set(&pthread_cond_destroy, Stubpthread_cond_destroy);
    CMpThread::DestroyCond(pcond);
    
    // thread_cond_t* pcond1 = NULL;
    // CMpThread::DestroyCond(pcond1);
}

TEST_F(CMpThreadTest,TryLock){
    thread_lock_t plock;
    
    CMpThread::TryLock(&plock);
}

TEST_F(CMpThreadTest,CalcRetryInterval){
    mp_uint32 uiTimeoutInMillisecs;
    
    CMpThread::CalcRetryInterval(uiTimeoutInMillisecs);
}

TEST_F(CMpThreadTest,IsTimeout){
    mp_uint64 ulStartTime;
    mp_uint32 uiTimeoutInMillisecs;
    
    CMpThread::IsTimeout(ulStartTime,uiTimeoutInMillisecs);
}

TEST_F(CMpThreadTest,TimedLock){
    thread_lock_t plock;
    mp_uint32 uiTimeoutInMillisecs;
    
    CMpThread::TimedLock(&plock,uiTimeoutInMillisecs);
}

TEST_F(CMpThreadTest,GetThreadId){
    CMpThread::GetThreadId();
}
