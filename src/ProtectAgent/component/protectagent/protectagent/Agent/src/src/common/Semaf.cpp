/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Semaf.cpp
 * @brief  The implemention about semaf
 * @version 1.0.0.0
 * @date 2015-02-06
 * @author wangguitao 00510599
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

