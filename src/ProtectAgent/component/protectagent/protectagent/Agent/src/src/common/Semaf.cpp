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
#include "common/Semaf.h"
#include "common/Utils.h"
#include "common/Log.h"

mp_int32 CSemaf::Init(mp_semaf& semaf)
{
#ifdef WIN32
    semaf = CreateSemaphore(NULL, 0, INT_MAX, NULL);
    if (semaf == NULL) {
        return MP_FAILED;
    }
#elif defined LINUX
    mp_int32 iRet = MP_SUCCESS;
    iRet = sem_init(&semaf, 0, 0); // used in process
    if (MP_SUCCESS != iRet) {
        return MP_FAILED;
    }
#else
    COMMLOG(OS_LOG_ERROR, "Unimplemented function.");
    return MP_FAILED;
#endif

    return MP_SUCCESS;
}

mp_int32 CSemaf::Release(mp_semaf* pSemaf)
{
    mp_int32 iRet = MP_SUCCESS;
#ifdef WIN32
    iRet = ReleaseSemaphore(*pSemaf, 1, NULL); // If the function fails, the resturn value is zero.
    if (!iRet) {
        return MP_FAILED;
    }
#elif defined LINUX
    iRet = sem_post(pSemaf);
    if (iRet != MP_SUCCESS) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, "sem_wait failed, errno[%d]:%s.",
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

#else
    COMMLOG(OS_LOG_ERROR, "Unimplemented function.");
    return MP_FAILED;
#endif

    return MP_SUCCESS;
}

mp_int32 CSemaf::Wait(mp_semaf* pSemaf)
{
    mp_int32 iRet = MP_SUCCESS;
#ifdef WIN32
    iRet = WaitForSingleObject(*pSemaf, INFINITE);
    if (WAIT_OBJECT_0 != iRet) {
        return MP_FAILED;
    }
#elif defined LINUX
    iRet = sem_wait(pSemaf);
    if (iRet != MP_SUCCESS) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, "sem_wait failed, errno[%d]:%s.",
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
#else
    COMMLOG(OS_LOG_ERROR, "Unimplemented function.");
    return MP_FAILED;
#endif

    return MP_SUCCESS;
}

