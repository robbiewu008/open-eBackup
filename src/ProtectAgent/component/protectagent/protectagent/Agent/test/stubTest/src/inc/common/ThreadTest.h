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
#ifndef _THREADTEST_H_
#define _THREADTEST_H_

#define private public

#include "common/CMpThread.h"
#include "common/CMpTime.h"
#include "common/Utils.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCMpThreadVoid(mp_void* pthis);

class CMpThreadTest: public testing::Test{
public:
    Stub stub;
};

mp_void StubCMpThreadVoid(mp_void* pthis){
    return;
}

mp_int32 Stubpthread_join(pthread_t thread, void **value_ptr){
    return 1;
}

mp_int32 Stubpthread_join0(pthread_t thread, void **value_ptr){
    return 0;
}

mp_int32 Stubpthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize){
    return 0;
}

mp_int32 Stubpthread_create(pthread_t *restrictthread,const pthread_attr_t *restrictattr,void *(*start_routine)(void*), void *restrictarg){
    return 1;
}

mp_int32 Stubpthread_create0(pthread_t *restrictthread,const pthread_attr_t *restrictattr,void *(*start_routine)(void*), void *restrictarg){
    return 0;
}

mp_int32 Stubpthread_cond_destroy(pthread_cond_t *cond){
    int i = 0;
}

#endif
