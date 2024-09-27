/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interfaces are not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 */

#ifndef KMC_SRC_COMMON_WSECV2_FILE_H
#define KMC_SRC_COMMON_WSECV2_FILE_H

#include "wsecv2_type.h"
#include "wsecv2_callbacks.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WSEC_FILE_IO_SIZE_MAX 4096 /* Maximum length of a file I/O. */

/* Check whether the file exists. */
WsecBool WsecCheckStatus(const char *file);

/* Copying files */
WsecBool WsecCopyFile(const char *srcFile, const char *destFile);

/* Obtains the file length. */
WsecBool WsecGetFileLen(const char *filePathName, long *fileLen);

/* Securely delete files. For security consider, if the file length exceed the fileMaxLen,
 * the delete operation will not continue. The fileMaxLen is set according to actual needs,
 * and it will not check the file length if set fileMaxLen = 0.
 */
WsecBool WsecDeleteFileSafe(const char *filePathName, long fileMaxLen);

/* Open the file to be read and written and obtain the length of the file to be read. */
unsigned long WsecReadWriteFilePrepare(const char *readFile, const char *writeFile,
    WsecHandle *readStream, WsecHandle *writeStream,
    long *remainLen);

#define WSEC_FOPEN(filePathName, mode)     (WsecFopen(filePathName, mode))
#define WSEC_FCLOSE(stream)                (((stream) != NULL) ? (void)WsecFclose(stream) : (void)0)
#define WSEC_FREAD(buffer, count, stream)  (WsecFread(buffer, (size_t)(count), stream))
#define WSEC_FWRITE(buffer, count, stream) (WsecFwrite(buffer, (size_t)(count), stream))
#define WSEC_FREMOVE(filePathName)         (WsecFremove(filePathName))
#define WSEC_FFLUSH(stream)                (WsecFflush(stream))
#define WSEC_FTELL(stream)                 (WsecFtell(stream))
#define WSEC_FSEEK(stream, offset, origin) (WsecFseek(stream, (long)(offset), origin))
#define WSEC_FERRNO(stream)                (WsecFerrno(stream))
#define WSEC_FSTATUS(name)                 (WsecCheckStatus(name))

#define WSEC_FREAD_MUST(buff, buffLen, stream)  (WSEC_FREAD(buff, buffLen, stream) == WSEC_TRUE)
#define WSEC_FWRITE_MUST(buff, buffLen, stream) (WSEC_FWRITE(buff, buffLen, stream) == WSEC_TRUE)

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_FILE_H */
