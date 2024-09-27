/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: common function implementation
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "wsecv2_mem.h"
#include "securec.h"
#include "wsecv2_util.h"

/* The \0 and unicode codes must be considered for storing character strings. */
/* Two bytes are allocated to the end character. */
#define LINE_BREAK_LEN 2

/* Clones src and returns the clone result character string. */
char *WsecStringClone(const char *srcString, const char *filePathName, int lineNum)
{
    char *newStringBuff = NULL;
    size_t len; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WSEC_UNREFER(filePathName);
    WSEC_UNREFER(lineNum);

    if (srcString == NULL) {
        return NULL;
    }

    len = WSEC_STRLEN(srcString);
    if (len < 1) {
        return NULL;
    }
    /* The \0 and unicode codes must be considered for storing character strings. */
    /* Two bytes are allocated to the end character. */
    len += LINE_BREAK_LEN;

    newStringBuff = (char *)WSEC_MALLOC(len);
    if (newStringBuff != NULL) {
        /* If the replication fails, the memory is released. */
        if (strcpy_s(newStringBuff, len, srcString) != EOK) {
            WSEC_FREE(newStringBuff);
        }
    }
#ifdef WSEC_TRACE_MEMORY
    if (newStringBuff != NULL) {
        WSEC_LOG_E2("WsecStringClone() at %s, Line-%d", filePathName, lineNum);
    }
#endif

    return newStringBuff;
}

/* buffer clone */
WsecVoid *WsecBuffClone(const WsecVoid *cloneFrom, size_t size, const char *filePathName, int lineNum)
{
    WsecVoid *newMem = NULL;
    WSEC_UNREFER(filePathName);
    WSEC_UNREFER(lineNum);

    if (cloneFrom == NULL || !(size > 0)) {
        return NULL;
    }

    newMem = WSEC_MALLOC(size);
    if (newMem == NULL) {
        return NULL;
    }

    /* If the replication fails, the memory is released. */
    (void)memcpy_s(newMem, size, cloneFrom, size);

#ifdef WSEC_TRACE_MEMORY
    WSEC_LOG_E2("WsecBuffClone() at %s, Line-%d", filePathName, lineNum);
#endif

    return newMem;
}

/* Memory allocation (The file and line number can be traced. WSEC_TRACE_MEMORY needs to be enabled.) */
WsecVoid *WsecMemAlloc(size_t size, const char *file, int lineNum)
{
    WsecVoid *ptr = NULL;
    WSEC_UNREFER(file);
    WSEC_UNREFER(lineNum);

    if (size == 0) {
        return ptr;
    }

    ptr = WsecMalloc(size);
    if (ptr != NULL) {
        (void)memset_s(ptr, (size_t)size, 0, (size_t)size);
    }

    /* Memory application recorded in the debug version */
#ifdef WSEC_TRACE_MEMORY
    WSEC_LOG_E3("WsecMemAlloc(%zu) at: %s, Line-%d", size, file, lineNum);
#endif

    return ptr;
}

/* Releases the allocated memory. (You can trace the file and line number. You need to enable WSEC_TRACE_MEMORY.) */
WsecVoid WsecMemFree(WsecVoid *ptr, const char *file, int lineNum)
{
    if (ptr == NULL) {
        return; /* Avoid repeated release. */
    }
    WSEC_UNREFER(file);
    WSEC_UNREFER(lineNum);

    /* Memory release is recorded in the debug version. */
#ifdef WSEC_TRACE_MEMORY
    WSEC_LOG_E2("WsecMemFree() at: %s, Line-%d", file, lineNum);
#endif

    WsecFree(ptr);
}

/* Release and clear (The file and line number can be traced. You need to enable WSEC_TRACE_MEMORY.) */
WsecVoid WsecMemClearFree(WsecVoid *buff, size_t len, const char *file, int lineNum)
{
    if (buff == NULL) {
        return;
    }
    if (len > 0) {
        (void)memset_s(buff, len, 0, len);
    }
    WsecMemFree(buff, file, lineNum);
}
