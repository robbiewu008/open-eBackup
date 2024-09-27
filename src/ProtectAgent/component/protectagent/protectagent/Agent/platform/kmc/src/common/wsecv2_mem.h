/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interfaces are not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 */

#ifndef KMC_SRC_COMMON_WSECV2_MEM_H
#define KMC_SRC_COMMON_WSECV2_MEM_H

#include "wsecv2_type.h"
#include "wsecv2_callbacks.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 1. Check the validity of the association between compilation switches. */
/* 1.1 WSEC_TRACE_MEMORY can be defined only with WSEC_DEBUG. */
#ifndef WSEC_DEBUG
#ifdef WSEC_TRACE_MEMORY
#undef WSEC_TRACE_MEMORY
#endif
#endif

/* 5. CBB internal common functions */
/* Memory allocation (The file and line number can be traced. WSEC_TRACE_MEMORY needs to be enabled.) */
WsecVoid *WsecMemAlloc(size_t size, const char *file, int lineNum);
/* Releases the allocated memory. (You can trace the file and line number. You need to enable WSEC_TRACE_MEMORY.) */
WsecVoid WsecMemFree(WsecVoid *ptr, const char *file, int lineNum);
/* Release and clear (The file and line number can be traced. You need to enable WSEC_TRACE_MEMORY.) */
WsecVoid WsecMemClearFree(WsecVoid *buff, size_t len, const char *file, int lineNum);

/* Cloning a character string */
char *WsecStringClone(const char *srcString, const char *filePathName, int lineNum);

/* clone buffer */
WsecVoid *WsecBuffClone(const WsecVoid *cloneFrom, size_t size, const char *filePathName, int lineNum);

#define WSEC_STRLEN strlen

#define WSEC_MALLOC(size) WsecMemAlloc((size_t)(size), WSEC_KMC_FILE, __LINE__)
#define WSEC_FREE(buff) do {                    \
    WsecMemFree(buff, WSEC_KMC_FILE, __LINE__); \
    (buff) = NULL;                              \
} while (0)

#define WSEC_CLEAR_FREE(buff, len) do {                             \
    WsecMemClearFree(buff, (size_t)(len), WSEC_KMC_FILE, __LINE__); \
    (buff) = NULL;                                                  \
} while (0);

#define WSEC_CLONE_BUFF(src, size) WsecBuffClone(src, (size_t)(size), WSEC_KMC_FILE, __LINE__)

#define WSEC_CLONE_STR(src) WsecStringClone(src, WSEC_KMC_FILE, __LINE__)

#define WSEC_BUFF_ALLOC(buffer, size) do { \
    (buffer).buff = WSEC_MALLOC(size);     \
    (buffer).len = (WsecUint32)(size);     \
} while (0)

#define WSEC_BUFF_ASSIGN(buffer, ptr, size) do { \
    (buffer).buff = (ptr);                       \
    (buffer).len = (size);                       \
} while (0)

#define WSEC_BUFF_FREE(buffer) WSEC_FREE((buffer).buff)

#define WSEC_MEMCMP(buffA, buffB, n) WsecMemCmp(buffA, buffB, (size_t)(n))

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_MEM_H */
