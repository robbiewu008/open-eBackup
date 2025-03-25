/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file Afslibrary.h
 * @brief AFS - Common function.
 *
 */

#ifndef COMMON_AFSLIBRARY_H_
#define COMMON_AFSLIBRARY_H_

#include <stdarg.h>
#include "securec.h"
#include "afs/LogMsg.h"
#include "afs/AfsError.h"

#define VMDK_SECTOR_SIZE 512

#define CHECK_MEMSET_S_OK(dest, destMax, c, count)                                  \
    {                                                                               \
        errno_t iCheckNotOkRet = memset_s(dest, destMax, c, count);                 \
        if (EOK != iCheckNotOkRet) {                                                \
            AFS_TRACE_OUT_ERROR("Call memset_s() failed, ret %d.", iCheckNotOkRet); \
            return AFS_ERR_API;                                                     \
        }                                                                           \
    }

#define CHECK_MEMSET_S_OK_RETURN(dest, destMax, c, count, returnValue)              \
    {                                                                               \
        errno_t iCheckNotOkRet = memset_s(dest, destMax, c, count);                 \
        if (EOK != iCheckNotOkRet) {                                                \
            AFS_TRACE_OUT_ERROR("Call memset_s() failed, ret %d.", iCheckNotOkRet); \
            return returnValue;                                                     \
        }                                                                           \
    }

#define CHECK_MEMCPY_S_OK(dest, destMax, src, count)                              \
    {                                                                             \
        errno_t iCheckNotOkRet = memcpy_s(dest, destMax, src, count);             \
        if (EOK != iCheckNotOkRet) {                                              \
            AFS_TRACE_OUT_ERROR("Call memcpy_s failed, ret %d.", iCheckNotOkRet); \
            return AFS_ERR_API;                                                   \
        }                                                                         \
    }

#define CHECK_MEMCPY_S_OK_RETURN(dest, destMax, src, count, returnValue)          \
    {                                                                             \
        errno_t iCheckNotOkRet = memcpy_s(dest, destMax, src, count);             \
        if (EOK != iCheckNotOkRet) {                                              \
            AFS_TRACE_OUT_ERROR("Call memcpy_s failed, ret %d.", iCheckNotOkRet); \
            return returnValue;                                                   \
        }                                                                         \
    }

inline int32_t afs_vsnprintf_s(char *strDest, size_t destMax, size_t count, const char *format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    int iCheckNotOkRet = vsnprintf_s(strDest, destMax, count, format, arglist);
    va_end(arglist);
    return iCheckNotOkRet;
}

/**
 * @Description: The vsnprintf_s function is equivalent to the vsnprintf function except for the parameter destMax/count
 * and the explicit runtime-constraints violation
 * @param strDest -  produce output according to a format ,write to the character string strDest
 * @param destMax - The maximum length of destination buffer(including the terminating null  byte ('\0'))
 * @param count - do not write more than count bytes to strDest(not including the terminating null  byte ('\0'))
 * @param format - fromat string
 * @param argptr - instead of  a variable  number of arguments
 * @return:return the number of characters printed(not including the terminating null byte ('\0')), If an error occurred
 * return -1.
 */
#define CHECK_VSNPRINTF_S_OK(strDest, destMax, count, format, arglist)                      \
    {                                                                                       \
        int32_t iCheckNotOkRet = afs_vsnprintf_s(strDest, destMax, count, format, arglist); \
        if (-1 == iCheckNotOkRet) {                                                         \
            AFS_TRACE_OUT_ERROR("call vsnprintf_s() failed, ret %d.", iCheckNotOkRet);      \
            return AFS_ERR_API;                                                             \
        }                                                                                   \
    }

/**
 * @Description:The strncpy_s function copies not more than n successive characters (not including the terminating null
 * character) from the array pointed to by strSrc to the array pointed to by strDest
 * @param strDest - destination  address
 * @param destMax -The maximum length of destination buffer(including the terminating null character)
 * @param strSrc -source  address
 * @param count -copies count  characters from the  src
 * @return  EOK if there was no runtime-constraint violation
 */
#define CHECK_STRNCPY_S_OK(strDest, destMax, strSrc, count)                          \
    {                                                                                \
        errno_t iCheckNotOkRet = strncpy_s(strDest, destMax, strSrc, count);         \
        if (EOK != iCheckNotOkRet) {                                                 \
            AFS_TRACE_OUT_ERROR("call strncpy_s() failed, ret %d.", iCheckNotOkRet); \
            return AFS_ERR_API;                                                      \
        }                                                                            \
    }

#endif /* COMMON_AFSLIBRARY_H_ */
