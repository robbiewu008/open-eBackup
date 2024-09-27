/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: FILE common function implementation
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "wsecv2_file.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"

#define WSEC_ZERO_COVER_TIMES   1
#define WSEC_ONE_COVER_TIMES    1
#define WSEC_RANDOM_COVER_TIMES 5   /* Defines the number of file overwriting times. */
#define WSEC_KSFSW_RKHEADLEN    256
#define SAFE_EARSER_VALUE 0XFF

typedef enum {
    WSEC_COVER_MODE_ZERO,       /* use zero to cover file */
    WSEC_COVER_MODE_ONE,        /* use one to cover file */
    WSEC_COVER_MODE_RANDOM      /* use random data to cover file */
} WsecCoverMode;

/* Check whether the file exists. */
WsecBool WsecCheckStatus(const char *name)
{
    WsecBool exist = WSEC_FALSE;
    unsigned long ret = WsecFileCheck(name, &exist);
    if (ret != WSEC_SUCCESS || exist == WSEC_FALSE) {
        return WSEC_FALSE;
    }
    return WSEC_TRUE;
}

/* Copying files */
WsecBool WsecCopyFile(const char *srcFile, const char *destFile)
{
    WsecHandle readStream = NULL;
    WsecHandle writeStream = NULL;
    unsigned char *readBuff = NULL;
    long curLen = 0;
    long fileLen = 0;
    long readLen;
    WsecBool returnValue = WSEC_FALSE;
    unsigned long ret;

    if ((srcFile == NULL) || (destFile == NULL)) {
        return WSEC_FALSE;
    }
    ret = WsecReadWriteFilePrepare(srcFile, destFile, &readStream, &writeStream, &fileLen);
    if (ret != WSEC_SUCCESS || fileLen < WSEC_KSFSW_RKHEADLEN) {
        WSEC_LOG_E2("WsecCopyFile fail %lu %ld", ret, fileLen);
        return WSEC_FALSE;
    }
    do {
        if ((readBuff = (unsigned char *)WSEC_MALLOC(WSEC_FILE_IO_SIZE_MAX)) == NULL) {
            WSEC_LOG_E("get memory failed in WsecCopyFile");
            break;
        }
        while (curLen < fileLen) {
            readLen = (curLen + WSEC_FILE_IO_SIZE_MAX <= fileLen) ? WSEC_FILE_IO_SIZE_MAX : (fileLen - curLen);
            returnValue = WSEC_FREAD(readBuff, readLen, readStream);
            if (returnValue == WSEC_FALSE) {
                WSEC_LOG_E1("copy read fail, %d.", WSEC_FERRNO(writeStream));
                break;
            }

            returnValue = WSEC_FWRITE(readBuff, readLen, writeStream);
            if (returnValue == WSEC_FALSE) {
                WSEC_LOG_E1("copy write fail, %d.", WSEC_FERRNO(writeStream));
                break;
            }
            curLen += readLen;
        }
        /* if all read and write success then flush to write stream */
        if ((returnValue == WSEC_TRUE) && WSEC_FFLUSH(writeStream) != 0) {
            WSEC_LOG_E2("WsecCopyFile WSEC_FFLUSH failed, errno %d err %d", WSEC_FERRNO(writeStream), returnValue);
            returnValue = WSEC_FALSE;
        }
    } while (0);
    WSEC_FCLOSE(readStream);
    WSEC_FCLOSE(writeStream);
    WSEC_CLEAR_FREE(readBuff, WSEC_FILE_IO_SIZE_MAX);
    return returnValue;
}

/* Obtains the file length. */
WsecBool WsecGetFileLen(const char *filePathName, long *fileLen)
{
    WsecHandle readStream = NULL;
    long len = -1;

    if ((filePathName == NULL) || (fileLen == NULL)) {
        return WSEC_FALSE;
    }
    readStream = WSEC_FOPEN(filePathName, KMC_FILE_READ_BINARY);
    if (readStream == NULL) {
        return WSEC_FALSE;
    }

    if (WSEC_FSEEK(readStream, 0, KMC_FILE_SEEK_END) != -1) {
        len = WSEC_FTELL(readStream);
    }
    WSEC_FCLOSE(readStream);

    if (len >= 0) {
        *fileLen = len;
        return WSEC_TRUE;
    }
    return WSEC_FALSE;
}

/* CoverFile via mode */
static WsecBool CoverFile(WsecHandle fp, long fileLen, WsecCoverMode mode)
{
    WSEC_ASSERT(fp != NULL);
    WSEC_ASSERT(fileLen >= 0);
    unsigned char *writeBuff = NULL;
    WsecBool ret = WSEC_TRUE;
    long writeLen;
    long current = fileLen;
    if ((writeBuff = (unsigned char *)WSEC_MALLOC(WSEC_FILE_IO_SIZE_MAX)) == NULL) {
        WSEC_LOG_E("get memory failed when delete file");
        return WSEC_FALSE;
    }
    switch (mode) {
        case WSEC_COVER_MODE_ZERO:
            (void)memset_s(writeBuff, (size_t)WSEC_FILE_IO_SIZE_MAX, 0, (size_t)WSEC_FILE_IO_SIZE_MAX);
            break;
        case WSEC_COVER_MODE_ONE:
            (void)memset_s(writeBuff, (size_t)WSEC_FILE_IO_SIZE_MAX, SAFE_EARSER_VALUE, (size_t)WSEC_FILE_IO_SIZE_MAX);
            break;
        case WSEC_COVER_MODE_RANDOM:
            if (CacRandom(writeBuff, WSEC_FILE_IO_SIZE_MAX) != WSEC_SUCCESS) {
                ret = WSEC_FALSE;
            }
            break;
        default:
            ret = WSEC_FALSE;
            break;
    }
    if (ret == WSEC_FALSE) {
        WSEC_CLEAR_FREE(writeBuff, WSEC_FILE_IO_SIZE_MAX);
        return ret;
    }
    while (current > 0) {
        writeLen = (current > WSEC_FILE_IO_SIZE_MAX) ? WSEC_FILE_IO_SIZE_MAX : current;
        if (WSEC_FWRITE(writeBuff, writeLen, fp) == WSEC_FALSE) {
            ret = WSEC_FALSE;
            break;
        }
        current -= writeLen;
    }
    WSEC_CLEAR_FREE(writeBuff, WSEC_FILE_IO_SIZE_MAX);
    return ret;
}

/* Get Cover mode by times */
static WsecCoverMode GetCoverMode(int times)
{
    if (times < WSEC_ZERO_COVER_TIMES) {
        return WSEC_COVER_MODE_ZERO;
    } else if (times < WSEC_ZERO_COVER_TIMES + WSEC_ONE_COVER_TIMES) {
        return WSEC_COVER_MODE_ONE;
    }
    return WSEC_COVER_MODE_RANDOM;
}

/* Securely delete files. For security consider, if the file length exceed the fileMaxLen,
 * only earse the first $fileMaxLen bytes of the file. The fileMaxLen is set according to actual needs,
 * and it will not check the file length if set fileMaxLen = 0.
 */
WsecBool WsecDeleteFileSafe(const char *filePathName, long fileMaxLen)
{
    WSEC_ASSERT(filePathName != NULL);
    WsecHandle writeStream = NULL;
    long fileLen = 0;
    WsecBool ret = WSEC_TRUE;
    int i; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    int total = (WSEC_ZERO_COVER_TIMES + WSEC_ONE_COVER_TIMES + WSEC_RANDOM_COVER_TIMES);

    if (WsecGetFileLen(filePathName, &fileLen) == WSEC_FALSE) {
        WSEC_LOG_E("WsecGetFileLen fail.");
        return WSEC_FALSE;
    }

    /* If the file Len is great than the fileMaxLen, only erase the first $fileMaxLen of the file
     * if the fileMaxLen == 0, needn't check */
    if (fileMaxLen != 0 && fileLen > fileMaxLen) {
        WSEC_LOG_W2("File is too big,  fileLen:%ld, max limit fileLen:%ld", fileLen, fileMaxLen);
        fileLen = fileMaxLen;
    }

    /* DTS2017042406832 is overwritten only once. */
    for (i = 0; (i < total) && (ret == WSEC_TRUE); i++) {
        writeStream = WSEC_FOPEN(filePathName, KMC_FILE_WRITE_BINARY);
        if (writeStream == NULL) {
            WSEC_LOG_E("Open need delete file fail.");
            ret = WSEC_FALSE;
            break;
        }
        if (CoverFile(writeStream, fileLen, GetCoverMode(i)) == WSEC_FALSE) {
            WSEC_FCLOSE(writeStream);
            WSEC_LOG_E("ConverFile failed.");
            ret = WSEC_FALSE;
            break;
        }
        WSEC_FCLOSE(writeStream);
    }
    /* remove file after erase all data */
    if (ret == WSEC_TRUE) {
        ret = (WSEC_FREMOVE(filePathName) == 0);
    }
    return ret;
}

/* Open the file to be read and written and obtain the length of the file to be read. */
unsigned long WsecReadWriteFilePrepare(const char *readFile, const char *writeFile,
    WsecHandle *readStream, WsecHandle *writeStream,
    long *remainLen)
{
    unsigned long ret = WSEC_SUCCESS;
    WsecHandle readTemp = NULL;
    WsecHandle writeTemp = NULL;
    if (WSEC_IS_EMPTY_STRING(readFile) || WSEC_IS_EMPTY_STRING(writeFile)) {
        return WSEC_ERR_INVALID_ARG;
    }
    if (WsecGetFileLen(readFile, remainLen) == WSEC_FALSE) {
        WSEC_LOG_E("Cannot acess file");
        return WSEC_ERR_OPEN_FILE_FAIL;
    }
    do {
        readTemp = WSEC_FOPEN(readFile, KMC_FILE_READ_BINARY);
        writeTemp = WSEC_FOPEN(writeFile, KMC_FILE_WRITE_BINARY);
        if (readTemp == NULL) {
            WSEC_LOG_E("Cannot read file");
            ret = WSEC_ERR_OPEN_FILE_FAIL;
            break;
        }
        if (writeTemp == NULL) {
            WSEC_LOG_E("Cannot write file");
            ret = WSEC_ERR_WRI_FILE_FAIL;
            break;
        }
    } while (0);

    if (ret != WSEC_SUCCESS) {
        WSEC_FCLOSE(readTemp);
        WSEC_FCLOSE(writeTemp);
        readTemp = NULL;
        writeTemp = NULL;
    }
    *readStream = readTemp;
    *writeStream = writeTemp;
    return ret;
}
