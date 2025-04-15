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
#include "LibS3IO.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <fcntl.h>
#include <securec.h>
#include <algorithm>
#include <ext/stdio_filebuf.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "system/System.hpp"
#include "securec.h"
#include "config_reader/ConfigIniReader.h"
#include "common/Timer.h"
#include "sqlite3.h"
#include "log/Log.h"

using namespace std;
using namespace Module;

namespace {
    const char *ALGORITHM = "AES256";
    const int RetriesG = 3;
    const int NUM_2 = 2;
    const int NUM_3 = 3;
    const int NUM_10 = 10;
    const int NUM_16 = 16;
    constexpr int NUM_1000 = 1000;
    constexpr int MAX_MARKER_LEN = 1024;
    const int MAX_S3_KEYS_SIZE = 10000;
    const uint64_t S3_BRICK_DEFAULT_SIZE = ((uint64_t)1024) * 1024 * 1024 * 1024 * 1024 * 1024;
};

// globals -----------------------------------------
#define SLEEP_UNITS_PER_SECOND 60
#define FIRST_HALF 1
#define SECOND_HALF 2
#define LOCK_ROOT_DIR "/tmp/.eBackupLock/"
#define UPLOAD_PART_SEGMENT_SIZE (5ULL * 1024 * 1024 * 1024)
#define CHECK_INIT_RETURN(name) { }

static char *Itoa(int num, char *str, int radix)
{
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;
    int i = 0, j, k;

    if (str == nullptr) {
        HCP_Log(ERR, LIBS3) << "str is nullptr " << HCPENDLOG;
        return nullptr;
    }
    if (radix == NUM_10 && num < 0) {
        unum = static_cast<unsigned>(-num);
        str[i++] = '-';
    } else {
        unum = static_cast<unsigned>(num);
    }
    do {
        str[i++] = index[unum % static_cast<unsigned>(radix)];
        unum /= radix;
    } while (unum);

    str[i] = '\0';

    if (str[0] == '-') {
        k = 1;
    } else {
        k = 0;
    }
    char temp;
    for (j = k; j <= (i - 1) / NUM_2; j++) {
        temp = str[j];
        str[j] = str[i - 1 + k - j];
        str[i - 1 + k - j] = temp;
    }
    return str;
}

// static callback functions
static int UploadDataCallback(int bufferSize, char *buffer, void *callbackData)
{
    S3Base *s3Obj = static_cast<S3Base *>(callbackData);
    return s3Obj->CallbackUploadData(bufferSize, buffer);
}

static obs_status listPartsCallback(
    obs_uploaded_parts_total_info *uploaded_parts, obs_list_parts *parts, void *callbackData)
{
    S3Base *s3Obj = static_cast<S3Base *>(callbackData);
    return s3Obj->CallbackListParts(
        uploaded_parts->is_truncated, uploaded_parts->nextpart_number_marker, uploaded_parts->parts_count, parts);
}

obs_status CompleteMultipartUploadCallback(
    const char *location, const char *bucket, const char *key, const char *eTag, void *callbackData)
{
    (void)callbackData;
    HCP_Log(DEBUG, LIBS3) << "location = " << location << HCPENDLOG;
    HCP_Log(DEBUG, LIBS3) << "bucket = " << bucket << HCPENDLOG;
    HCP_Log(DEBUG, LIBS3) << "key = " << key << HCPENDLOG;
    HCP_Log(DEBUG, LIBS3) << "eTag = " << eTag << HCPENDLOG;
    return OBS_STATUS_OK;
}

static obs_status ListBucketCallback(int isTruncated, const char *nextMarker, int contentsCount,
    const obs_list_objects_content *contents, int commonPrefixesCount, const char **commonPrefixes, void *callbackData)
{
    HCP_Log(DEBUG, LIBS3) << "Enter ListBucketCallback ." << HCPENDLOG;
    S3Base *s3Obj = static_cast<S3Base *>(callbackData);
    return s3Obj->ListBucketCallbackInternal(
        isTruncated, nextMarker, contentsCount, contents, commonPrefixesCount, commonPrefixes);
}

void closeLocalFile(int *fd)
{
    HCP_Log(DEBUG, LIBS3) << "Enter close file." << HCPENDLOG;
    if (fd != nullptr) {
        HCP_Log(DEBUG, LIBS3) << "Close file,fd=" << *fd << HCPENDLOG;
        (void)close(*fd);
        delete fd;
        fd = nullptr;
    }
}

shared_ptr<int> OpenLocalFile(const string &fileName, int openMode)
{
    return shared_ptr<int>(
        new (nothrow) int(open(fileName.c_str(), openMode, S_IRUSR | S_IWUSR)), closeLocalFile);
}

ssize_t ReadLocalFile(int fd, char *buffer, size_t bufSize)
{
    return read(fd, buffer, bufSize);
}

ssize_t WriteLocalFile(int fd, const char *buffer, size_t bufSize)
{
    return write(fd, buffer, bufSize);
}

// functions of S3BucketContextProxy

ssize_t libs3IO::CopyToExternalBuf(const char *buf, size_t bufSize)
{
    if (buf == nullptr) {
        HCP_Log(ERR, LIBS3) << "Param invalid." << HCPENDLOG;
        return 0;
    }

    ssize_t copySize = min((size_t)(m_external_buf.buffer_size - m_external_buf.offset), bufSize);

    int retVal = memcpy_s(m_external_buf.buffer + m_external_buf.offset,
                          m_external_buf.buffer_size - m_external_buf.offset,
                          buf,
                          copySize);
    if (retVal != 0) {
        HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
    }
    m_external_buf.offset += copySize;

    return copySize;
}

int libs3IO::CopyFromExternalBuf(char *buffer, int bufferSize)
{
    if (buffer == nullptr) {
        HCP_Log(ERR, LIBS3) << "Param invalid." << HCPENDLOG;
        return 0;
    }

    int copySize = min((int)(m_external_buf.buffer_size - m_external_buf.offset), bufferSize);

    int retVal = memcpy_s(buffer, bufferSize, m_external_buf.buffer + m_external_buf.offset, copySize);
    if (retVal != 0) {
        HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
    }
    m_external_buf.offset += copySize;

    return copySize;
}

ssize_t libs3IO::CallbackWrite(const char *buf, size_t bufSize)
{
    if (m_objType == OBJECT_FILE_DATA) {
        return WriteLocalFile(*m_localFile, buf, bufSize);
    } else if (m_objType == OBJECT_EXTERNAL_DATA) {
        return CopyToExternalBuf(buf, bufSize);
    }

    return m_cache->WriteSegement(const_cast<char *>(buf), bufSize);
}

size_t libs3IO::CallbackRead(char *buf, size_t bufSize)
{
    if (m_objType == OBJECT_FILE_DATA) {
        return static_cast<size_t>(ReadLocalFile(*m_localFile, buf, bufSize));
    } else if (m_objType == OBJECT_EXTERNAL_DATA) {
        return static_cast<size_t>(CopyFromExternalBuf(buf, bufSize));
    }

    return m_cache->ReadSegement(buf, bufSize);
}

libs3IO::libs3IO(const S3IOParams &params, int objType) : m_dataLoaded(false), m_S3IOInfo(params), m_fileHandle(nullptr)
{
    m_objType = objType;
    m_cache = nullptr;
    m_ListBucketInfo.isTruncated = 0;
    m_ListBucketInfo.keyCount = 0;
    if (memset_s(m_ListBucketInfo.nextMarker, sizeof(m_ListBucketInfo.nextMarker), 0x0, MAX_MARKER_LEN) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
    }

    if (m_objType == OBJECT_NO_CACHE_DATA) {
        m_cache = new (nothrow) NoCacheBuffer(m_S3IOInfo);
    } else {
        m_cache = new (nothrow) Cache();
    }

    m_useVpp = false;
}

libs3IO::~libs3IO()
{
    HCP_Log(DEBUG, LIBS3) << "Enter disconstruct..." << HCPENDLOG;

    if (m_dataLoaded) {
        (void)Close();
    }

    if (m_cache != nullptr) {
        m_cache->Reset();
        delete m_cache;
        m_cache = nullptr;
    }
}

const char *libs3IO::GetObjectKey(const string &fileName)
{
    if (fileName.size() > (m_S3IOInfo.bucketFullName.size() + 1)) {
        if (fileName.substr(0, m_S3IOInfo.bucketFullName.size()) == m_S3IOInfo.bucketFullName) {
            const char *pos = fileName.c_str();

            return (pos + (m_S3IOInfo.bucketFullName.size() + 1));
        }
    }

    return fileName.c_str();
}

string libs3IO::GetErrorInfo()
{
    const char *msg = obs_get_status_name(m_completedStatus);
    string strError = "";
    if (!msg || OBS_STATUS_OK == m_completedStatus) {
        strError = "S3 Unknown Error";
    } else {
        strError = "S3 return ";
        strError += msg;
    }
    return strError;
}

bool libs3IO::GetNotExistError()
{
    return (m_completedStatus == OBS_STATUS_NoSuchKey || m_completedStatus == OBS_STATUS_NoSuchBucket ||
            m_completedStatus == OBS_STATUS_HttpErrorNotFound);
}

#define CHECK_READ_AND_APPEND_MODE                                                   \
    if (m_objType == OBJECT_CACHE_DATA) {                                            \
        RecvData();                                                                  \
    }                                                                                \
    if ((m_mode.compare("a") != 0) && (m_completedStatus != OBS_STATUS_OK)) {        \
        status = false;                                                              \
        m_dataLoaded = false;                                                        \
    } else if ((m_mode.compare("a") == 0) && (m_completedStatus != OBS_STATUS_OK) && \
               (m_completedStatus != OBS_STATUS_NoSuchKey)) {                        \
        status = false;                                                              \
        m_dataLoaded = false;                                                        \
    } else {                                                                         \
        status = true;                                                               \
        m_dataLoaded = true;                                                         \
    }

#define CHECK_READ_AND_APPEND_MODE_NEW               \
    if (m_objType == OBJECT_NO_CACHE_DATA) {         \
        uintmax_t size = 0;                          \
        FileSize(m_objKey, size);                    \
        m_cache->SetSize(static_cast<size_t>(size)); \
        m_completedStatus = OBS_STATUS_OK;           \
    }                                                \
    CHECK_READ_AND_APPEND_MODE

bool libs3IO::OpenSub(EBKFileHandle *handle)
{
    bool status = true;
    if ((m_mode.compare("r") == 0) || (m_mode.compare("r+") == 0) || (m_mode.compare("a") == 0)) {
        CHECK_READ_AND_APPEND_MODE_NEW
    } else if ((m_mode.compare("w") == 0) || (m_mode.compare("w+") == 0) || (m_mode.compare("t") == 0)) {
        status = true;
        m_dataLoaded = true;
    } else {
        HCP_Log(ERR, LIBS3) << "Unsupport mode. mode is:" << m_mode << HCPENDLOG;
        status = false;
        m_dataLoaded = false;
    }

    if (m_mode.compare("a") == 0) {
        m_cache->SetOffset(m_cache->Size());
        if (!m_cache->CreateHandle(handle)) {
            status = false;
        }
        m_fileHandle = handle;
    }
    return status;
}

bool libs3IO::Open(const char *fileName, const char *mode, EBKFileHandle *handle)
{
    if (fileName == nullptr || mode == nullptr) {
        HCP_Log(ERR, LIBS3) << "param is error!" << HCPENDLOG;
        return false;
    }

    m_mode = mode;

    HCP_Log(DEBUG, LIBS3) << "Open fileName = " << fileName << " mode =" << mode << ",fileType:" << m_objType
                          << ",partNum:" << (handle == nullptr ? 0 : handle->partNum) << HCPENDLOG;

    if (m_dataLoaded) {
        HCP_Log(ERR, LIBS3) << fileName << " has been opened before." << HCPENDLOG;
        return false;
    }

    CHECK_INIT_RETURN(fileName);

    SetObjectKey(GetObjectKey(fileName));
    m_cache->SetObjectKey(m_objKey);

    bool status = OpenSub(handle);

    HCP_Log(DEBUG, LIBS3) << "datasize = " << m_cache->Size() << ",status = " << status << HCPENDLOG;
    return status;
}

bool libs3IO::IsChaindbFile()
{
    string key = m_objKey;
    string::size_type npos2 = key.find("ChainDB.db");
    if (npos2 == string::npos) {
        return false;
    }

    string chaindbname = key.substr(npos2);
    if (chaindbname.compare("ChainDB.db") != 0) {
        return false;
    }

    return true;
}

string GetFileName(const string &key)
{
    hash<string> h;
    const int maxLength = 512;
    char guid[maxLength + 1] = {0};
    int len = snprintf_s(guid, sizeof(guid), maxLength, "%016llX", static_cast<unsigned long long>(h(key)));
    if (len <= 0) {
        HCP_Log(ERR, "libs3IO") << "snprintf_s failed, return len = " << len << HCPENDLOG;
        return "";
    }
    string keyHash(guid);
    return keyHash + ".ChainDB";
}

bool libs3IO::CheckLocalChainDBIntegrity()
{
    uint32_t checkIntegrity = ConfigReader::getInt("BackupNode", "CheckLocalChainDBIntegrity");
    if (checkIntegrity == 0) {
        HCP_Log(DEBUG, LIBS3) << " Ignore CheckLocalChainDBIntegrity." << HCPENDLOG;
        return true;
    }
    // write data to a tmp file under /tmp, file name is KEY+request id
    string fileName = "/tmp/" + ConfigReader::getString("General", "MicroServiceName") + GetFileName(m_objKey);
    if (fileName == "/tmp/") {
        HCP_Log(ERR, LIBS3) << " failed to get tmp file name." << HCPENDLOG;
        return false;
    }
    bool writeResult = m_cache->DumpToLocalFile(fileName);
    if (!writeResult) {
        HCP_Log(ERR, LIBS3) << " dump chaindb file to local failed." << HCPENDLOG;
        return false;
    }
    // Open tmp file  and check integrity;
    sqlite3 *tmpDB = nullptr;
    char *errmsg = nullptr;
    int result;

    result = sqlite3_open(fileName.c_str(), &tmpDB);
    if (result != SQLITE_OK) {
        HCP_Log(ERR, LIBS3) << " open database failed." << DBG(fileName) << HCPENDLOG;
        return false;
    }
    const char *sql = "PRAGMA integrity_check;";
    result = sqlite3_exec(tmpDB, sql, nullptr, nullptr, &errmsg);
    if (result != SQLITE_OK) {
        HCP_Log(WARN, LIBS3) << " check database failed." << DBG(fileName) << errmsg << HCPENDLOG;
        return false;
    }
    sqlite3_close(tmpDB);
    // remove file
    remove(fileName.c_str());
    HCP_Log(DEBUG, LIBS3) << "CheckLocalChainDBIntegrity Success." << DBG(fileName) << HCPENDLOG;
    return true;
}

bool libs3IO::Close(FileCloseStatus status)
{
    HCP_Log(DEBUG, LIBS3) << "comming in close" << HCPENDLOG;
    bool rc = true;

    if ((m_mode.compare("r") == 0) || (m_mode.compare("t") == 0)) {
        HCP_Log(DEBUG, LIBS3) << "close mode 'r' :" << DBG(m_objKey) << HCPENDLOG;

        m_dataLoaded = false;
        m_completedStatus = OBS_STATUS_ErrorUnknown;
        m_cache->Reset();
        return true;
    }

    m_cache->SetOffset(0);
    if (m_mode.compare("a") == 0) {
        rc = m_cache->Finish(m_fileHandle, status);
    }
    if (m_dataLoaded && m_objType == OBJECT_CACHE_DATA) {
        SendData();  // load the buffer data to UDS
        if (m_completedStatus != OBS_STATUS_OK) {
            rc = false;
            PrintStatusErr("Close");
        }
    } else {
        HCP_Log(DEBUG, LIBS3) << "file is not loaded, and mode is: " << m_mode << HCPENDLOG;
    }
    HCP_Log(DEBUG, LIBS3) << "close not mode 'r' :" << DBG(m_objKey) << HCPENDLOG;

    m_dataLoaded = false;
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    m_cache->Reset();

    return rc;
}

size_t libs3IO::Read(char *buf, size_t bufSize)
{
    if (m_mode.compare("w") == 0 || buf == nullptr) {
        HCP_Log(ERR, LIBS3) << "mode 'w' can't read." << HCPENDLOG;
        return 0;
    }

    return m_cache->Read(buf, bufSize);
}

size_t libs3IO::ReadFS(char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "FLOW: libs3IO::ReadFS" << HCPENDLOG;
    if (m_mode.compare("w") == 0 || buf == nullptr) {
        HCP_Log(ERR, LIBS3) << "mode 'w' can't read." << HCPENDLOG;
        return 0;
    }

    return m_cache->ReadFS(buf, bufSize);
}

size_t libs3IO::Write(char *buf, size_t bufSize)
{
    if (m_mode.compare("r") == 0 || buf == nullptr) {
        HCP_Log(ERR, LIBS3) << "mode 'r' can't write." << HCPENDLOG;
        return 0;
    }

    return m_cache->Write(buf, bufSize);
}

size_t libs3IO::WriteFS(char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "FLOW1: libs3IO::WriteFS" << HCPENDLOG;
    if (m_mode.compare("r") == 0 || buf == nullptr) {  // only read
        HCP_Log(ERR, LIBS3) << "mode 'r' can't write." << HCPENDLOG;
        return 0;
    }

    HCP_Log(DEBUG, LIBS3) << "FLOW2: libs3IO::WriteFS" << HCPENDLOG;
    return m_cache->WriteFS(buf, bufSize);
}

size_t libs3IO::Append(char *buf, size_t bufSize)
{
    if (m_mode.compare("a") != 0 || buf == nullptr) {
        HCP_Log(ERR, LIBS3) << "mode is not 'a or buffer is empty'." << HCPENDLOG;
        return 0;
    }

    return m_cache->Append(m_fileHandle, buf, bufSize);
}

void libs3IO::ConvertHex2ASCII(const string &in, string &out)
{
    unsigned int hexLen = (unsigned int)in.size();
    char hexTmp[3] = {0};
    char *end = nullptr;
    long int hexData;
    unsigned char tmp;
    string strOut;

    HCP_Log(DEBUG, LIBS3) << "ConvertHex2ASCII start." << HCPENDLOG;
    HCP_Log(DEBUG, LIBS3) << "Input size:" << hexLen << "." << HCPENDLOG;

    if (hexLen % NUM_2 == 0) {
        strOut.assign(hexLen / NUM_2, 0);
        int j = 0;
        for (unsigned int i = 0; i < hexLen; i = i + NUM_2) {
            hexTmp[0] = in[i];
            hexTmp[1] = in[i + 1];
            hexData = strtol(hexTmp, &end, NUM_16);
            if (end[0] != 0) {
                HCP_Log(ERR, LIBS3) << "ConvertHex2ASCII failed." << HCPENDLOG;
                end[0] = static_cast<char>(0xcc);
                break;
            }

            tmp = (unsigned char)hexData;
            strOut[j] = tmp;
            j++;
        }
    } else {
        HCP_Log(ERR, LIBS3) << "dek size is not correct." << HCPENDLOG;
    }

    for (int k = 0; k < NUM_3; k++) {
        hexTmp[k] = static_cast<char>(0xcc);
    }
    hexData = 0;
    tmp = 0;
    out = strOut;
    CleanMemoryPwd(strOut);

    HCP_Log(DEBUG, LIBS3) << "out size:" << out.size() << HCPENDLOG;
    HCP_Log(DEBUG, LIBS3) << "ConvertHex2ASCII end." << HCPENDLOG;
}

void libs3IO::SetDek(const string &dek)
{
    if (dek.size() > 0) {
        ConvertHex2ASCII(dek, m_dek);
    }
    m_cache->SetDEK(m_dek);
}

void libs3IO::RecvData()
{
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    size_t oldDataLen = m_cache->Size();
    (void)ReadDataFromS3(bucketContext, 0, 0);
    size_t recvSize = m_cache->Size() - oldDataLen;

    HCP_Log(DEBUG, LIBS3) << "recvSize: " << recvSize << HCPENDLOG;
}

void libs3IO::RecvDataNoCache(const size_t startBytes, const size_t countBytes)
{
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    (void)ReadDataFromS3(bucketContext, startBytes, countBytes);
    size_t recvSize = 0;
    if (CheckStatus()) {
        recvSize = countBytes;
    }
    HCP_Log(DEBUG, LIBS3) << "recvSize: " << recvSize << HCPENDLOG;
}

bool libs3IO::UseVpp()
{
    // add w90006072 fix  DTS2019022612416
    if (m_objType == OBJECT_NO_CACHE_DATA) {
        NoCacheBuffer *pNoCache = dynamic_cast<NoCacheBuffer *>(m_cache);
        if (pNoCache == nullptr) {
            return m_useVpp;
        }
        return pNoCache->UseVpp();
    }
    return m_useVpp;
}

bool libs3IO::ReadNoCache(const string &filename, const string &dek, size_t offset, char *buffer,
    const size_t bufferLen, size_t &readedLen)
{
    m_objType = OBJECT_NO_CACHE_DATA;
    m_cache->Init(buffer, bufferLen);
    SetDek(dek);
    SetObjectKey(GetObjectKey(filename));
    m_cache->SetObjectKey(m_objKey);

    RecvDataNoCache(offset, bufferLen);

    if (m_completedStatus != OBS_STATUS_OK) {
        readedLen = 0;
        PrintStatusErr("ReadNoCache");
        return false;
    } else {
        readedLen = m_cache->Size();
    }

    HCP_Log(DEBUG, LIBS3) << "readedLen: " << readedLen << HCPENDLOG;
    return true;
}

int libs3IO::Truncate(long int position)
{
    if ((m_cache->Capacity() == 0) || (position < 0)) {
        HCP_Log(ERR, LIBS3) << "param invalid" << HCPENDLOG;
        return -1;
    }

    if (m_mode.compare("r") == 0) {
        HCP_Log(ERR, LIBS3) << "opened in mode 'r'." << HCPENDLOG;
        return -1;
    }

    m_cache->Truncate(position);
    HCP_Log(DEBUG, LIBS3) << "truncate successfully." << HCPENDLOG;
    return 0;
}

#define CHECK_SEEK_SET                                                                                 \
    {                                                                                                  \
        if ((offset < 0) || ((size_t(offset) > m_cache->Size()) && (m_mode.compare("r") == 0))) {      \
            HCP_Log(ERR, LIBS3) << "Seek failed" << HCPENDLOG;                                         \
            return -1;                                                                                 \
        }                                                                                              \
        oldOffset = offset;                                                                            \
    }

#define CHECK_SEEK_CUR                                                                \
    {                                                                                 \
        if (((long int)oldOffset + offset < 0) ||                                     \
            ((oldOffset + offset > m_cache->Size()) && (m_mode.compare("r") == 0))) { \
            HCP_Log(ERR, LIBS3) << "Seek failed" << HCPENDLOG;                        \
            return -1;                                                                \
        }                                                                             \
        oldOffset += offset;                                                          \
    }

#define CHECK_SEEK_END                                                                                    \
    {                                                                                                     \
        if (((long int)(m_cache->Size()) + offset < 0) || ((offset > 0) && (m_mode.compare("r") == 0))) { \
            HCP_Log(ERR, LIBS3) << "Seek failed" << HCPENDLOG;                                            \
            return -1;                                                                                    \
        }                                                                                                 \
        oldOffset = m_cache->Size() + offset;                                                             \
    }

int libs3IO::Seek(long int offset, int origin)
{
    HCP_Log(DEBUG, LIBS3) << "offset: " << offset << HCPENDLOG;
    size_t oldOffset = m_cache->Offset();
    if (origin == SEEK_SET) {
        CHECK_SEEK_SET
    } else if (origin == SEEK_CUR) {
        CHECK_SEEK_CUR
    } else if (origin == SEEK_END) {
        CHECK_SEEK_END
    } else {
        HCP_Log(ERR, LIBS3) << "Seek failed" << HCPENDLOG;
        return -1;
    }

    m_cache->SetOffset(oldOffset);
    HCP_Log(DEBUG, LIBS3) << "Seek successful." << HCPENDLOG;
    return 0;
}

int libs3IO::Flush(bool sync)
{
    sync = sync;
    return 0;
}

boost::tribool libs3IO::FileExists(const char *fileName)
{
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    return S3Base::FileExists(bucketContext, string(GetObjectKey(fileName)));
}

boost::tribool libs3IO::FileExists(S3BucketContextProxy &bucketContext, const string &fileName)
{
    return S3Base::FileExists(bucketContext, fileName);
}

bool libs3IO::Exists(S3BucketContextProxy &bucketContext, const string &fileName)
{
    HCP_Log(DEBUG, LIBS3) << "fileName = " << fileName << HCPENDLOG;

    CHECK_INIT_RETURN(fileName);

    (void)ListBucket(bucketContext, fileName.c_str(), nullptr, nullptr, 1);

    if (m_objectInfoList.size() == 0) {
        HCP_Log(WARN, LIBS3) << "File or directory of " << fileName << " is not exists." << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "File or directory is exists, size is:" << m_objectInfoList.size() << HCPENDLOG;

    return true;
}

// it cann't handle this situation: one file name contains another's
bool libs3IO::Exists(const char *fileName)
{
    if (fileName == nullptr || strlen(fileName) == 0) {
        HCP_Log(ERR, LIBS3) << "file name is empty. " << HCPENDLOG;
        return false;
    }
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    return Exists(bucketContext, GetObjectKey(fileName));
}

// it cann't handle this situation: call this function just after CreateDirectory
bool libs3IO::IsDirectory(const char *pathName)
{
    if (pathName == nullptr) {
        HCP_Log(DEBUG, LIBS3) << "pathName is null" << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "pathName = " << pathName << HCPENDLOG;
    if (strlen(pathName) == 0) {
        HCP_Log(ERR, LIBS3) << "Path name is empty. " << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "pathName = " << pathName << HCPENDLOG;

    string path = pathName;
    string dirString = pathName;

    if (dirString.at(dirString.length() - 1) != '/') {
        dirString += '/';
    }

    string objKey = GetObjectKey(dirString);
    HCP_Log(DEBUG, LIBS3) << "GetObjectKey(path) = " << objKey << HCPENDLOG;

    if (!ListBucket(m_S3IOInfo, const_cast<char *>(objKey.c_str()), nullptr, nullptr, 1)) {
        HCP_Log(ERR, LIBS3) << "list bucket error" << HCPENDLOG;
        return false;
    }
    if (m_objectInfoList.size() > 0) {
        HCP_Log(DEBUG, LIBS3) << "It is a directory. " << HCPENDLOG;
        return true;
    }

    HCP_Log(DEBUG, LIBS3) << "not a directory. " << HCPENDLOG;
    return false;
}

bool libs3IO::Remove(const char *fileName)
{
    if (fileName == nullptr || strlen(fileName) == 0) {
        HCP_Log(ERR, LIBS3) << "file name is empty. " << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "Remove fileName = " << fileName << HCPENDLOG;
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    bool bRet = RemoveFile(bucketContext, GetObjectKey(fileName));
    return bRet;
}

obs_status S3DeleteObjectDataCB(int contentsCount, obs_delete_objects *contents, void *callbackData)
{
    (void)callbackData;
    uint32_t i;
    for (i = 0; i < contentsCount; i++) {
        const obs_delete_objects *content = &(contents[i]);
        HCP_Log(INFO, LIBS3) << "Remove fileName = " << content->key << ", code=" << content->code << HCPENDLOG;
    }

    return OBS_STATUS_OK;
}

obs_put_properties GetS3PutProperties(obs_object_info (&bls)[OBS_MAX_DELETE_OBJECT_NUMBER], int size, char *buff)
{
    stringstream ss;
    ss << "<Delete>";
    for (uint32_t i = 0; i < size; ++i) {
        ss << "<Object><Key>" << bls[i].key << "</Key></Object>";
    }
    ss << "</Delete>";

    const int DECRYPT_SIZE = 16;
    unsigned char decrypt[DECRYPT_SIZE] = {0};
    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);
    MD5_Update(&md5_ctx, ss.str().c_str(), ss.str().length());
    MD5_Final(decrypt, &md5_ctx);

    BIO *bmem = nullptr;
    BIO *b64 = nullptr;
    BUF_MEM *bptr = nullptr;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, (char *)decrypt, DECRYPT_SIZE);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    int retVal = memcpy_s(buff, bptr->length, bptr->data, bptr->length);
    if (retVal != 0) {
        HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
    }
    buff[bptr->length] = 0;
    buff[24] = 0;
    BIO_free_all(b64);

    obs_put_properties stPutProperties;
    init_put_properties(&stPutProperties);
    int fullControl = ConfigReader::getInt("BackupNode", "S3BucketFullControl");
    if (fullControl > 0) {
        stPutProperties.canned_acl = OBS_CANNED_ACL_BUCKET_OWNER_FULL_CONTROL;
    }
    stPutProperties.content_type = const_cast<char *>("application/xml");
    stPutProperties.md5 = buff;
    return stPutProperties;
}

bool IsAllBlockInTheSameBucket(const vector<string> &directoryNames)
{
    string directoryTemp = directoryNames[0];
    for (const auto &directoryName : directoryNames) {
        if (directoryName != directoryTemp) {
            return false;
        }
    }

    return true;
}

bool libs3IO::MultiBlocksDelete4Single(const vector<string> &directoryNames, vector<string> &objs,
    uintmax_t &size, const uint8_t &snapVersion)
{
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};
    obs_delete_object_handler handle;
    handle.delete_object_data_callback = &S3DeleteObjectDataCB;
    handle.response_handler = responseHandler;
    m_completedStatus = OBS_STATUS_ErrorUnknown;

    S3BucketContextProxy bucketContext(m_S3IOInfo);
    vector<string> s3objs;
    auto it1 = objs.begin();
    auto it2 = directoryNames.begin();

    for (; it1 != objs.end(); ++it1, ++it2) {
        s3objs.push_back(string(GetObjectKey(*it2)) + "/" + *it1);
    }

    obs_object_info bls[OBS_MAX_DELETE_OBJECT_NUMBER] = {0};
    uint32_t index = 0;
    for (auto it = s3objs.begin(); it != s3objs.end(); ++it) {
        bls[index].key = const_cast<char *>(it->c_str());
        if (snapVersion < NUM_2) {
            if (false == CalcSize(bucketContext, *it, size)) {
                return false;
            }
        }

        index++;
    }

    // 26 dont modify, this char array to save a check code(size 24), but when perform md5 and base64, it will be
    // larger, so use 26
    uint32_t buffSize = 26;
    char buff[buffSize];
    memset_s(buff, buffSize, 0, buffSize);

    obs_put_properties stPutProperties = GetS3PutProperties(bls, s3objs.size(), buff);
    obs_delete_object_info delobj;
    memset_s(&delobj, sizeof(obs_delete_object_info), 0, sizeof(obs_delete_object_info));
    delobj.keys_number = (unsigned)s3objs.size();
    delobj.quiet = 0;
    Timer timer;
    do {
        batch_delete_objects(&bucketContext, bls, &delobj, &stPutProperties, &handle, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("MultiBlocksDelete4Single");
        if (OBS_STATUS_NoSuchBucket != m_completedStatus && OBS_STATUS_NoSuchKey != m_completedStatus &&
            OBS_STATUS_HttpErrorNotFound != m_completedStatus) {
            HCP_Log(ERR, LIBS3) << "remove objs failed,request ID : " << m_requestID
                                << ",duration = " << timer.Duration() << HCPENDLOG;
            return false;
        }
    }
    HCP_Log(DEBUG, LIBS3) << "remove objs success,request ID : " << m_requestID << ",duration = " << timer.Duration()
                          << HCPENDLOG;
    return true;
}

bool libs3IO::CalcSize(S3BucketContextProxy &bucketContext, string &s3ObjPath, uintmax_t &size)
{
    uintmax_t tempSize = 0;
    if (false == FileSize(bucketContext, s3ObjPath, tempSize)) {
        HCP_Log(ERR, LIBS3) << "remove objs failed, check s3 network connect or your dek set or not" << HCPENDLOG;
        return false;
    }

    size += tempSize;

    return true;
}

inline void ClassifierObjs(vector<string> &objs, const vector<string> &directoryNames,
    vector<string> &uniqueDirectoryNames, vector<vector<string>> &s3objsAll)
{
    auto it1 = objs.begin();
    auto it2 = directoryNames.begin();
    for (; it1 != objs.end(); ++it1, ++it2) {
        for (uint32_t i = 0; i < uniqueDirectoryNames.size(); ++i) {
            if (*it2 == uniqueDirectoryNames[i]) {
                s3objsAll[i].push_back(*it2 + "/" + *it1);
                break;
            }
        }
    }
}

bool libs3IO::MultiBlocksDelete4Multiple(const vector<string> &directoryNames, vector<string> &objs,
    uintmax_t &size, const uint8_t &snapVersion)
{
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};
    obs_delete_object_handler handle;
    handle.delete_object_data_callback = &S3DeleteObjectDataCB;
    handle.response_handler = responseHandler;
    m_completedStatus = OBS_STATUS_ErrorUnknown;

    vector<string> uniqueDirectoryNames(directoryNames);
    sort(uniqueDirectoryNames.begin(), uniqueDirectoryNames.end());
    uniqueDirectoryNames.erase(
        unique(uniqueDirectoryNames.begin(), uniqueDirectoryNames.end()), uniqueDirectoryNames.end());
    vector<vector<string>> s3objsAll(uniqueDirectoryNames.size());

    // Classifier all objects to vectors in s3objsAll by directory.
    ClassifierObjs(objs, directoryNames, uniqueDirectoryNames, s3objsAll);

    for (auto &uniqueDirectoryName : uniqueDirectoryNames) {
        uniqueDirectoryName = uniqueDirectoryName.substr(uniqueDirectoryName.find("/") + 1);
        uniqueDirectoryName = uniqueDirectoryName.substr(0, uniqueDirectoryName.find("/"));
    }

    for (uint32_t i = 0; i < s3objsAll.size(); ++i) {
        m_S3IOInfo.bucket = uniqueDirectoryNames[i];
        S3BucketContextProxy bucketContext(m_S3IOInfo);
        obs_object_info bls[OBS_MAX_DELETE_OBJECT_NUMBER] = {0};
        for (uint32_t j = 0; j < s3objsAll[i].size(); ++j) {
            s3objsAll[i][j] = s3objsAll[i][j].substr(
                s3objsAll[i][j].find(uniqueDirectoryNames[i]) + uniqueDirectoryNames[i].size() + 1);
            bls[j].key = const_cast<char *>(s3objsAll[i][j].c_str());
            if (snapVersion < NUM_2) {
                return false;
            }
        }

        // 26 dont modify, this char array to save a check code(size 24), but when perform md5 and base64, it will be
        // larger, so use 26
        uint32_t bufSize = 26;
        char buff[bufSize];
        memset_s(buff, bufSize, 0, bufSize);

        obs_put_properties stPutProperties = GetS3PutProperties(bls, s3objsAll[i].size(), buff);
        obs_delete_object_info delobj;
        memset_s(&delobj, sizeof(obs_delete_object_info), 0, sizeof(obs_delete_object_info));
        delobj.keys_number = (unsigned)s3objsAll[i].size();
        delobj.quiet = 0;
        Timer timer;

        do {
            batch_delete_objects(&bucketContext, bls, &delobj, &stPutProperties, &handle, this);
        } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

        if (m_completedStatus != OBS_STATUS_OK) {
            PrintStatusErr("MultiBlocksDelete4Multiple");
            if (OBS_STATUS_NoSuchBucket != m_completedStatus && OBS_STATUS_NoSuchKey != m_completedStatus &&
                OBS_STATUS_HttpErrorNotFound != m_completedStatus) {
                HCP_Log(ERR, LIBS3) << "remove objs failed,request ID : " << m_requestID
                                    << ",duration = " << timer.Duration() << HCPENDLOG;
                return false;
            }
        }
    }
    HCP_Log(DEBUG, LIBS3) << "remove objs success" << HCPENDLOG;
    return true;
}

bool libs3IO::RemoveObjs(const vector<string> &directoryNames, vector<string> &objs,
    uintmax_t &size, const uint8_t &snapVersion)
{
    bool ret = true;
    Timer timer;
    if (directoryNames.size() == 0) {
        HCP_Log(ERR, LIBS3) << "directoryNames size is 0" << HCPENDLOG;
        return false;
    }

    bool allTheSameBucket = IsAllBlockInTheSameBucket(directoryNames);
    if (true == allTheSameBucket) {
        ret = MultiBlocksDelete4Single(directoryNames, objs, size, snapVersion);
    } else {
        ret = MultiBlocksDelete4Multiple(directoryNames, objs, size, snapVersion);
    }

    HCP_Log(INFO, LIBS3) << "performance, End, duration=" << timer.Duration() << HCPENDLOG;

    return ret;
}

bool libs3IO::RemoveFilesMatched(S3BucketContextProxy &bucketContext)
{
    for (size_t i = 0; i < m_matchObjList.size(); ++i) {
        string objName = m_matchObjList[i];
        if (!RemoveFile(bucketContext, objName)) {
            HCP_Log(ERR, LIBS3) << "failed to remove the object:" << objName.c_str() << HCPENDLOG;
            return false;
        }
        HCP_Log(DEBUG, LIBS3) << "remove the object: " << objName.c_str() << HCPENDLOG;
        if (m_callbackHandle.callBackFunc) {
            ((DeleteFileCallback)m_callbackHandle.callBackFunc)(
                LayoutRetCode::SUCCESS, 1, m_callbackHandle.callBackData);
        }
    }
    return true;
}

bool libs3IO::RemoveAll(const char *pathName)
{
    if (pathName == nullptr || strlen(pathName) == 0) {
        HCP_Log(ERR, LIBS3) << "path name is empty. " << HCPENDLOG;
        return false;
    }
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    uint64_t finishedSize = 0;
    do {
        if (!ListBucket(bucketContext, GetObjectKey(pathName), nullptr, nullptr, MAX_S3_KEYS_SIZE)) {
            HCP_Log(ERR, LIBS3) << "list bucket error" << HCPENDLOG;
            return false;
        }
        finishedSize += m_matchObjList.size();
        HCP_Log(DEBUG, LIBS3) << "RemoveAll"
                              << " pathName=" << pathName << " size=" << m_matchObjList.size()
                              << " finishedSize=" << finishedSize << HCPENDLOG;
        if (!RemoveFilesMatched(bucketContext)) {
            HCP_Log(ERR, LIBS3) << "Remove matched files failed." << HCPENDLOG;
            return false;
        }
    } while (m_matchObjList.size() > 0);

    return true;
}

int64_t libs3IO::GetAllSize(const char *pathName)
{
    if (pathName == nullptr || strlen(pathName) == 0) {
        HCP_Log(ERR, LIBS3) << "path name is empty. " << HCPENDLOG;
        return -1;
    }
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    int64_t finishedSize = 0, objectCount = 0;
    string tmpNextMarker = "";
    do {
        if (!ListBucket(bucketContext, GetObjectKey(pathName), tmpNextMarker.data(), nullptr, MAX_S3_KEYS_SIZE)) {
            HCP_Log(ERR, LIBS3) << "list bucket error" << HCPENDLOG;
            return -1;
        }
        for (const S3ObjectContent &object : m_objectInfoList) {
            finishedSize += object.size;
            objectCount++;
        }
        tmpNextMarker = m_ListBucketInfo.nextMarker;
    } while (m_objectInfoList.size() > 0);
    HCP_Log(INFO, LIBS3) << " pathName=" << pathName << " size=" << finishedSize << " object count=" << objectCount
                         << HCPENDLOG;
    return finishedSize;
}

bool libs3IO::Rename(const char *oldName, const char *newName)
{
    if (oldName == nullptr || strlen(oldName) == 0 || newName == nullptr || strlen(newName) == 0) {
        HCP_Log(ERR, LIBS3) << "name is empty. " << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "Renaming file: "
                          << "oldName = " << oldName << " newName = " << newName << HCPENDLOG;

    S3BucketContextProxy bucketContext(m_S3IOInfo);
    return Rename(bucketContext, GetObjectKey(oldName), GetObjectKey(newName));
}

bool libs3IO::Rename(S3BucketContextProxy &bucketContext, const string &oldName, const string &newName)
{
    HCP_Log(DEBUG, LIBS3) << "Renaming file: "
                          << "oldName = " << oldName << " newName = " << newName << HCPENDLOG;
    if (!Copy(bucketContext, oldName, newName)) {
        HCP_Log(ERR, LIBS3) << "copy file failed: "
                            << "oldName = " << oldName << " newName = " << newName << HCPENDLOG;
        return false;
    }
    return RemoveFile(bucketContext, oldName);
}

bool libs3IO::Copy(const char *oldName, const char *newName)
{
    if (oldName == nullptr || strlen(oldName) == 0 || newName == nullptr || strlen(newName) == 0) {
        HCP_Log(ERR, LIBS3) << "name is empty. " << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "oldName = " << oldName << " newName = " << newName << HCPENDLOG;

    S3BucketContextProxy bucketContext(m_S3IOInfo);
    bool bRet = Copy(bucketContext, GetObjectKey(oldName), GetObjectKey(newName));

    HCP_Log(DEBUG, LIBS3) << "copy successfully." << HCPENDLOG;
    return bRet;
}

void libs3IO::Reset(const char *fileName, const char *mode)
{
    if (fileName == nullptr || strlen(fileName) == 0 || mode == nullptr || strlen(mode) == 0) {
        HCP_Log(ERR, LIBS3) << "param is empty. " << HCPENDLOG;
        return;
    }

    bool ret = Close();
    if (ret) {
        ret = Open(fileName, mode);
    }

    if (ret) {
        HCP_Log(DEBUG, LIBS3) << "reset successfully." << HCPENDLOG;
    } else {
        HCP_Log(ERR, LIBS3) << "reset failed." << HCPENDLOG;
    }
}

bool libs3IO::FileSize(const char *fileName, uintmax_t &size)
{
    if ((fileName == nullptr) || (strlen(fileName) == 0)) {
        HCP_Log(DEBUG, LIBS3) << "File name is empty." << HCPENDLOG;
        size = (uintmax_t)m_cache->Size();
        return true;
    }

    HCP_Log(DEBUG, LIBS3) << "File name is not null." << fileName << HCPENDLOG;

    S3BucketContextProxy bucketCtx(m_S3IOInfo);

    return FileSize(bucketCtx, GetObjectKey(fileName), size);
}

bool libs3IO::FilePrefixExists(const char *filePrefixName, vector<string> &suffixs)
{
    if ((filePrefixName == nullptr) || (strlen(filePrefixName) == 0)) {
        HCP_Log(ERR, LIBS3) << "File Prefix name is empty." << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "File prefix name is not null." << filePrefixName << HCPENDLOG;

    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    string objKey = GetObjectKey(filePrefixName);
    if (!ListBucket(bucketCtx, objKey.c_str())) {
        HCP_Log(ERR, LIBS3) << "list bucket error, filePrefixName is:" << filePrefixName << HCPENDLOG;
        return false;
    }

    for (string obj : m_matchObjList) {
        HCP_Log(DEBUG, LIBS3) << "list bucket m_matchObjList is:" << obj << " ,prefix is  " << WIPE_SENSITIVE(objKey)
            << HCPENDLOG;
        if (string::npos != obj.find(objKey)) {
            string suffix(obj.begin() + objKey.size(), obj.end());
            suffixs.push_back(suffix);
        }
    }

    return true;
}

long int libs3IO::DirSize(S3BucketContextProxy &bucketContext, const string &fileName)
{
    uint64_t size = 0;
    string objKey = fileName;
    m_matchObjList.clear();
    m_objectInfoList.clear();
    m_completedStatus = OBS_STATUS_ErrorUnknown;

    if (!ListBucket(bucketContext, objKey.c_str())) {
        HCP_Log(ERR, LIBS3) << "list bucket error, fileName is:" << fileName << HCPENDLOG;
        return -1;
    }

    HCP_Log(DEBUG, LIBS3) << "objectInfoList size: " << m_objectInfoList.size() << HCPENDLOG;
    for (size_t i = 0; i < m_objectInfoList.size(); ++i) {
        size += m_objectInfoList[i].size;
    }
    return size;
}

long int libs3IO::DirSize(const char *fileName)
{
    if ((fileName == nullptr) || (strlen(fileName) == 0)) {
        HCP_Log(DEBUG, LIBS3) << "File name is null." << HCPENDLOG;
        return m_cache->Size();
    }

    HCP_Log(DEBUG, LIBS3) << "File name is not null." << fileName << HCPENDLOG;

    S3BucketContextProxy bucketCtx(m_S3IOInfo);

    return DirSize(bucketCtx, GetObjectKey(fileName));
}

bool libs3IO::GetDirectoryList(const char *directoryName, vector<string> &elementList)
{
    if ((directoryName == nullptr) || (strlen(directoryName) == 0)) {
        HCP_Log(DEBUG, LIBS3) << "directory name is empty." << HCPENDLOG;
        return m_cache->Size();
    }

    HCP_Log(DEBUG, LIBS3) << "Dir:" << directoryName << HCPENDLOG;
    string fullpath(directoryName);
    if (fullpath.at(fullpath.length() - 1) != '/') {
        fullpath += '/';
    }

    CHECK_INIT_RETURN(directoryName);

    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    vector<string> dirList;
    if (!GetDirectoryList(bucketCtx, GetObjectKey(fullpath), dirList)) {
        HCP_Log(ERR, LIBS3) << "Get UDS directory failed." << HCPENDLOG;
        return false;
    }

    unsigned int i = 0;
    for (i = 0; i < dirList.size(); ++i) {
        elementList.push_back(m_S3IOInfo.bucketFullName + "/" + dirList[i]);
    }

    HCP_Log(DEBUG, LIBS3) << "GetDirectoryList successfully:" << dirList.size() << HCPENDLOG;

    return true;
}

bool libs3IO::GetDirectoryList(
    S3BucketContextProxy &bucketCtx, const string &directoryName, vector<string> &elementList)
{
    HCP_Log(DEBUG, LIBS3) << "Dir:" << directoryName << HCPENDLOG;

    CHECK_INIT_RETURN(directoryName);
    if (!ListBucket(bucketCtx, directoryName.c_str(), nullptr, "/")) {
        HCP_Log(ERR, LIBS3) << "list bucket error" << HCPENDLOG;
        return false;
    }

    unsigned int i = 0;

    for (i = 0; i < m_matchDirList.size(); ++i) {
        elementList.push_back(m_matchDirList[i].substr(0, m_matchDirList[i].length() - 1));
    }

    HCP_Log(DEBUG, LIBS3) << "GetDirectoryList successfully." << HCPENDLOG;

    return true;
}

bool libs3IO::GetCommonPrefixList(
    const char *directoryName, const string &delimiter, vector<string> &elementList)
{
    if ((directoryName == nullptr) || (strlen(directoryName) == 0)) {
        HCP_Log(DEBUG, LIBS3) << "directory name is empty." << HCPENDLOG;
        return m_cache->Size();
    }

    HCP_Log(DEBUG, LIBS3) << "Dir:" << directoryName << HCPENDLOG;
    string fullpath(directoryName);
    if (fullpath.at(fullpath.length() - 1) != '/') {
        fullpath += '/';
    }

    CHECK_INIT_RETURN(directoryName);

    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    vector<string> dirList;
    if (!GetCommonPrefixList(bucketCtx, GetObjectKey(fullpath), delimiter, dirList)) {
        HCP_Log(ERR, LIBS3) << "Get UDS directory failed." << HCPENDLOG;
        return false;
    }

    unsigned int i = 0;
    for (i = 0; i < dirList.size(); ++i) {
        elementList.push_back(m_S3IOInfo.bucketFullName + "/" + dirList[i]);
    }

    HCP_Log(DEBUG, LIBS3) << "GetDirectoryList successfully:" << dirList.size() << HCPENDLOG;

    return true;
}

bool libs3IO::GetCommonPrefixList(S3BucketContextProxy &bucketContext, const string &directoryName,
    const string &delimiter, vector<string> &elementList)
{
    HCP_Log(DEBUG, LIBS3) << "Dir:" << directoryName << HCPENDLOG;

    CHECK_INIT_RETURN(directoryName);
    if (!ListBucket(bucketContext, directoryName.c_str(), nullptr, delimiter.c_str())) {
        HCP_Log(ERR, LIBS3) << "list bucket error" << HCPENDLOG;
        return false;
    }

    unsigned int i = 0;

    for (i = 0; i < m_matchDirList.size(); ++i) {
        elementList.push_back(m_matchDirList[i].substr(0, m_matchDirList[i].length()));
    }

    HCP_Log(DEBUG, LIBS3) << "GetDirectoryList successfully." << HCPENDLOG;

    return true;
}

bool libs3IO::GetObjectContent(const string &fileName, obs_list_objects_content &objContent)
{
    if (fileName.empty()) {
        HCP_Log(ERR, LIBS3) << "file name is empty. " << HCPENDLOG;
        return false;
    }
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    return GetObjectContent(bucketContext, GetObjectKey(fileName), objContent);
}

bool libs3IO::GetObjectContent(
    S3BucketContextProxy &bucketContext, const string &filename, obs_list_objects_content &objContent)
{
    HCP_Log(DEBUG, LIBS3) << "fileName = " << filename << HCPENDLOG;
    if (!ListBucket(bucketContext, filename.c_str(), nullptr, nullptr, 1)) {
        HCP_Log(ERR, LIBS3) << "list bucket error" << HCPENDLOG;
        return false;
    }

    if (m_objectInfoList.size() == 0) {
        HCP_Log(WARN, LIBS3) << "File or directory of " << filename << " is not exists." << HCPENDLOG;
        return false;
    }

    objContent = m_objectInfoList[0];
    HCP_Log(DEBUG, LIBS3) << "File or directory is exists, size is:" << m_objectInfoList.size() << HCPENDLOG;

    return true;
}

bool libs3IO::GetFileListInDirectory(const char *directoryName, vector<string> &elementList)
{
    if ((directoryName == nullptr) || (strlen(directoryName) == 0)) {
        HCP_Log(DEBUG, LIBS3) << "directory name is empty." << HCPENDLOG;
        return m_cache->Size();
    }

    HCP_Log(DEBUG, LIBS3) << "Dir:" << directoryName << HCPENDLOG;
    string fullpath(directoryName);
    if (fullpath.at(fullpath.length() - 1) != '/') {
        fullpath += '/';
    }

    CHECK_INIT_RETURN(directoryName);

    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    vector<string> dirList;
    if (!GetFileListInDirectory(bucketCtx, GetObjectKey(fullpath), dirList)) {
        HCP_Log(ERR, LIBS3) << "Get UDS directory failed." << HCPENDLOG;
        return false;
    }

    unsigned int i = 0;
    for (i = 0; i < dirList.size(); ++i) {
        elementList.push_back(m_S3IOInfo.bucketFullName + "/" + dirList[i]);
    }

    HCP_Log(DEBUG, LIBS3) << "GetDirectoryList successfully:" << dirList.size() << HCPENDLOG;

    return true;
}

bool libs3IO::GetFileListInDirectory(
    S3BucketContextProxy &bucketCtx, const string &directoryName, vector<string> &elementList)
{
    HCP_Log(DEBUG, LIBS3) << "Dir:" << directoryName << HCPENDLOG;

    CHECK_INIT_RETURN(directoryName);
    if (!ListBucket(bucketCtx, directoryName.c_str(), nullptr, "/")) {
        HCP_Log(ERR, LIBS3) << "ListBucket failed in GetFileListInDirectory." << HCPENDLOG;
        return false;
    }

    unsigned int i = 0;

    for (i = 0; i < m_matchObjList.size(); ++i) {
        if (directoryName == m_matchObjList[i]) {
            // dont return the directory
            continue;
        }
        elementList.push_back(m_matchObjList[i].substr(0, m_matchObjList[i].length()));
    }

    HCP_Log(DEBUG, LIBS3) << "GetDirectoryList successfully." << HCPENDLOG;

    return true;
}

bool libs3IO::GetObjectListWithMarker(
    const string &dir, string &marker, bool &isEnd, int maxKeys, vector<string> &elementList)
{
    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    string prefixPath = bucketCtx.m_Host + ":/" + bucketCtx.m_Bucket;
    string s3Dir = dir.substr(prefixPath.size() + 1, dir.size());
    const char *cMarker = marker.empty() ? nullptr : marker.c_str();
    bool ret = ListBucket(bucketCtx, s3Dir.c_str(), cMarker, nullptr, maxKeys);
    if (ret != true) {
        HCP_Log(ERR, LIBS3) << "ListBucket failed in GetObjectListWithMarker." << HCPENDLOG;
        return false;
    }
    marker = m_ListBucketInfo.nextMarker;
    isEnd = m_ListBucketInfo.isTruncated ? false : true;
    for (const auto &object : m_objectInfoList) {
        elementList.emplace_back(object.m_ObjectName);
    }
    HCP_Log(DEBUG, LIBS3) << "GetObjectListWithMarker successfully." << HCPENDLOG;
    return true;
}

bool libs3IO::UploadFile(const string &objName, const string &destFile)
{
    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    return NormalFileUpload(bucketCtx, objName, destFile);
}

bool libs3IO::DownloadFile(const string &objName, const string &destFile)
{
    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    return FileDownload(bucketCtx, objName, destFile);
}

/**********************************************************************
@ Description  : Check sub bucket is existed or not.
@ param bucketName [IN] sub bucket name
@ param retryTimes [IN] retry times
@ Return       :Success return 0, Failed to return error code
***********************************************************************/
bool libs3IO::TestSubBucketIsNotExisted(const char *bucketName, const int retryTimes)
{
    if (bucketName == nullptr || strlen(bucketName) == 0) {
        HCP_Log(ERR, LIBS3) << "Param invalid.";
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "bucketName = " << bucketName << HCPENDLOG;

    S3BucketContextProxy bucketContext(m_S3IOInfo);

    obs_status iRet = TestBucket(bucketContext, retryTimes);
    // if the bucket is not existed, the error code from S3 is OBS_STATUS_HttpErrorNotFound
    if (iRet == OBS_STATUS_NoSuchBucket || iRet == OBS_STATUS_HttpErrorNotFound) {
        HCP_Log(DEBUG, LIBS3) << "subBucket is not existed, will return true" << HCPENDLOG;
        return true;
    }
    return (iRet == OBS_STATUS_OK);
}

bool libs3IO::TestConnect(const char *bucketName, const int retryTimes)
{
    if (bucketName == nullptr || strlen(bucketName) == 0) {
        HCP_Log(ERR, LIBS3) << "Param invalid." << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "bucketName = " << bucketName << HCPENDLOG;

    S3BucketContextProxy bucketContext(m_S3IOInfo);

    obs_status iRet = TestBucket(bucketContext, retryTimes);
    return (iRet == OBS_STATUS_OK);
}

bool libs3IO::SetUploadPartInfo(const string &objName, uint64_t segmentSize)
{
    string realPath;
    try {
        boost::system::error_code errcode;
        realPath = boost::filesystem::canonical(boost::filesystem::path(objName), errcode).string();
        if (errcode.value() != 0) {
            HCP_Log(ERR, LIBS3) << "Get real path failed. FileName :" << objName << ", errcode:" << errcode.value()
                                << HCPENDLOG;
            return false;
        }
    } catch (const boost::filesystem::filesystem_error &errExp) {
        HCP_Log(ERR, LIBS3) << "Get real path failed. FileName :" << objName << ", exception:" << errExp.what()
                            << HCPENDLOG;
        return false;
    }
    uint64_t contentLength = 0;

    FILE* fp = fopen(realPath.c_str(), "r");
    if (fp == nullptr) {
        HCP_Log(ERR, LIBS3) << "Failed to open file :" << realPath << HCPENDLOG;
        return false;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        HCP_Log(ERR, LIBS3) << "fseek failed." << HCPENDLOG;
        fclose(fp);
        return false;
    }

    int64_t fileSize = ftell(fp);
    if (0 > fileSize) {
        HCP_Log(ERR, LIBS3) << "ftell failed." << HCPENDLOG;
        fclose(fp);
        return false;
    }

    contentLength = (uint64_t)fileSize;
    if (fseek(fp, 0, SEEK_SET) != 0) {
        HCP_Log(ERR, LIBS3) << "fseek failed." << HCPENDLOG;
        fclose(fp);
        return false;
    }

    if (contentLength < segmentSize) {
        HCP_Log(ERR, LIBS3) << objName << ":file = " << contentLength << "little than segmentSize" << segmentSize
                            << HCPENDLOG;
        fclose(fp);
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "contentLength:" << contentLength << HCPENDLOG;
    m_UploadPartInfo.contentLength = contentLength;
    m_UploadPartInfo.originalContentLength = contentLength;

    int uploadpartNum = 0;
    if (segmentSize == 0) {
        fclose(fp);
        return false;
    }
    if (contentLength % segmentSize != 0) {
        uploadpartNum = static_cast<int>((contentLength / segmentSize) + 1);
    } else {
        uploadpartNum = static_cast<int>(contentLength / segmentSize);
    }
    m_UploadPartInfo.uploadpartNum = uploadpartNum;

    HCP_Log(DEBUG, LIBS3) << "uploadpartNum:" << uploadpartNum << HCPENDLOG;
    m_UploadPartInfo.fp = fp;

    return true;
}

size_t libs3IO::GetUploadFileData(char *buffer, int dataLen)
{
    upload_callback_data *data = (upload_callback_data *)&m_UploadPartInfo;

    size_t readSizeReturn = 0;
    size_t toReadSize = 0;
    int ret = 0;

    if (data->contentLength) {
        toReadSize = ((data->contentLength > static_cast<unsigned>(dataLen)) ?
            static_cast<unsigned>(dataLen) : data->contentLength);
        if (data->fp) {
            readSizeReturn = fread(buffer, 1, toReadSize, data->fp);
            ret = static_cast<int>(readSizeReturn);
        }
    }
    data->contentLength -= readSizeReturn;

    return ret;
}

bool libs3IO::RetryUpload(uint64_t segmentNum, uint64_t segmentSize)
{
    size_t offset = (segmentNum - 1) * segmentSize;
    HCP_Log(ERR, LIBS3) << "enter BeginUploadParts,offset" << offset << HCPENDLOG;
    if (fseek(m_UploadPartInfo.fp, offset, SEEK_SET) != 0) {
        fclose(m_UploadPartInfo.fp);
        HCP_Log(ERR, LIBS3) << "fseek failed." << HCPENDLOG;
        return false;
    }

    return true;
}

long int libs3IO::Tell()
{
    return static_cast<int>(m_cache->Offset());
}

bool libs3IO::Loaded()
{
    return m_dataLoaded;
}

bool libs3IO::CreateDirectory(const char *directoryName)
{
    // Begin modify by w90006072 for DTS2018072806806
    return true;
    // End
}

void libs3IO::FillHttpProxyParam(S3BucketContextProxy &bucketCtx, string &host, string &user)
{
    if (bucketCtx.m_Proxy.Enable) {
        host = bucketCtx.m_Proxy.HostName + ":" + bucketCtx.m_Proxy.Port;
        user = (bucketCtx.m_Proxy.UserName == "") ? ""
                                                  : (bucketCtx.m_Proxy.UserName + ":" + bucketCtx.m_Proxy.UserPassword);
    }
}

// When the bucket explicitly does not exist, the function returns false, else returns true.
bool libs3IO::TestBucketExisted(const char *bucketName)
{
    HCP_Log(INFO, LIBS3) << "Begin to TestBucketExisted.bucketName = " << bucketName << HCPENDLOG;
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    string tempBucketName = bucketName;
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    string keyStr(":/");
    string::size_type pos = tempBucketName.find(keyStr);
    if (string::npos == pos) {
        HCP_Log(ERR, LIBS3) << "bucketpath is invalid. path= " << bucketName << HCPENDLOG;
        return false;
    }
    string tempBucket = tempBucketName.substr(pos + keyStr.size());
    HCP_Log(DEBUG, LIBS3) << "bucketName after assembling = " << tempBucket << HCPENDLOG;
    bucketContext.SetBucketPath(tempBucket);
    obs_status iRet = TestBucket(bucketContext, retryTimes);
    if (iRet == OBS_STATUS_NoSuchBucket || iRet == OBS_STATUS_HttpErrorNotFound) {
        HCP_Log(DEBUG, LIBS3) << "Bucket is not existed, will return false" << HCPENDLOG;
        return false;
    }
    return true;
}

void libs3IO::RegisterCallbackHandle(CallBackHandle &handle)
{
    m_callbackHandle = handle;
}

void libs3IO::RegisterReadFileCallbackFun(const ReadFileCallback &handle)
{
    m_readFileFun = handle;
}

// The function returns True when the bucket does not exist or exists and is accessible.
bool libs3IO::TestSubBucketExisted(const char *bucketName)
{
    if (bucketName == nullptr || strlen(bucketName) == 0) {
        HCP_Log(ERR, LIBS3) << "Param invalid.";
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "bucketName = " << bucketName << HCPENDLOG;

    S3BucketContextProxy bucketContext(m_S3IOInfo);
    string tempBucketName = bucketName;

    string keyStr(":/");
    string::size_type pos = tempBucketName.find(keyStr);
    if (string::npos == pos) {
        return false;
    }
    string tempBucket = tempBucketName.substr(pos + keyStr.size());
    HCP_Log(DEBUG, LIBS3) << "bucketName after assembling = " << tempBucket << HCPENDLOG;
    bucketContext.SetBucketPath(tempBucket);

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    obs_status iRet = TestBucket(bucketContext, retryTimes);
    // if the bucket is not existed, the error code from S3 is OBS_STATUS_HttpErrorNotFound
    if (iRet == OBS_STATUS_NoSuchBucket || iRet == OBS_STATUS_HttpErrorNotFound) {
        HCP_Log(DEBUG, LIBS3) << "subBucket is not existed, will return true" << HCPENDLOG;
        return true;
    }

    return (iRet == OBS_STATUS_OK);
}

#ifndef EBKSDK_LIBS3_COMPILE

void libs3IO::SendDataInner(S3BucketContextProxy &bucketContext, size_t &sendSize)
{
    obs_put_object_handler putObjectHandler = {{&ResponseCallback, &CompleteCallback}, &PutObjectDataCallback};
    server_side_encryption_params serverSideEncryptionParams;
    if (memset_s(&serverSideEncryptionParams, sizeof(server_side_encryption_params),
        0, sizeof(server_side_encryption_params)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
    }

    string base = Base64Encode();
    if (m_dek.length() > 0) {
        serverSideEncryptionParams.encryption_type = OBS_ENCRYPTION_SSEC;
        serverSideEncryptionParams.ssec_customer_algorithm = const_cast<char *>(ALGORITHM);
        serverSideEncryptionParams.ssec_customer_key = const_cast<char *>(base.c_str());
    }

    obs_put_properties put_properties;
    int isMd5Upload = ConfigReader::getInt("Md5", "UploadWithMd5");
    GetUploadPutProperties(put_properties, isMd5Upload, m_objKey);

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    size_t oldoffset = m_cache->Offset();
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(m_objKey);

    do {
        if (IsS3Accelerator()) {
            bucketContext.request_options.proxy_host = const_cast<char *>(m_accelerator_proxy.c_str());
            bucketContext.request_options.bbr_switch = OBS_BBR_CLOSE;

            m_cache->SetOffset(oldoffset);
            put_object(&bucketContext, m_objKey, m_cache->Size(),
                &put_properties, &serverSideEncryptionParams, &putObjectHandler, this);
            m_useVpp = true;
            if (bucketContext.GetProxyAddress() != "") {
                bucketContext.request_options.proxy_host = const_cast<char *>(bucketContext.GetProxyAddress().c_str());
            }
            if (bucketContext.GetProxyUser() != "") {
                bucketContext.request_options.proxy_auth = const_cast<char *>(bucketContext.GetProxyUser().c_str());
            }
            bucketContext.request_options.bbr_switch =
                bucketContext.GetBbrSwitch() == 0 ? OBS_BBR_CLOSE : OBS_BBR_OPEN;
            if (S3StatusIsRetryable()) {
                m_useVpp = false;
                bucketContext.request_options.proxy_host = 0;
                bucketContext.request_options.bbr_switch = OBS_BBR_CLOSE;
                bucketContext.request_options.proxy_auth = 0;
                HCP_Log(WARN, LIBS3) << "put_object failed. object name=" << WIPE_SENSITIVE(m_objKey)
                                     << ", m_completedStatus=" << m_completedStatus << HCPENDLOG;
            }
        }

        if (!m_useVpp) {
            m_cache->SetOffset(oldoffset);
            put_object(&bucketContext, m_objKey, m_cache->Size(),
                &put_properties, &serverSideEncryptionParams, &putObjectHandler, this);
        }
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));
}

void libs3IO::SendData()
{
    S3BucketContextProxy bucketContext(m_S3IOInfo);

    size_t sendSize = 0;

    Timer timer;

    SendDataInner(bucketContext, sendSize);

    if (m_completedStatus != OBS_STATUS_OK) {
        sendSize = 0;
        PrintStatusErr("SendData");
        HCP_Log(ERR, LIBS3) << "SendData failed, PutObjectWithServerSideEncryption End, object name=" << m_objectName
                            << ", duration=" << timer.Duration() << HCPENDLOG;
    } else {
        sendSize = m_cache->Size();
    }

    // Begin add by w90006072 20180404 for print the key
    if (sendSize == 0) {
        HCP_Log(WARN, LIBS3) << "sendSize 0 btye ,key= " << WIPE_SENSITIVE(m_objKey) << HCPENDLOG;
    }
    // End add by w90006072
    HCP_Log(DEBUG, LIBS3) << "sendSize: " << sendSize << ". SendData End, object name=" << m_objectName
                          << ", request ID:" << m_requestID << ", duration=" << timer.Duration() << HCPENDLOG;
}

boost::tribool libs3IO::FileExists(const string &fileName, uint64_t *contentLength, int64_t *lastModified, string &etag)
{
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    return S3Base::FileExists(bucketContext, fileName, contentLength, lastModified, etag);
}

bool libs3IO::RemoveBatchFile(S3BucketContextProxy &bucketContext, const string &fileName)
{
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    obs_object_info object_info;
    if (memset_s(&object_info, sizeof(obs_object_info), 0, sizeof(obs_object_info)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
    }
    object_info.key = const_cast<char *>(fileName.c_str());
    object_info.version_id = 0;

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(fileName);
    Timer timer;
    do {
        delete_object(&bucketContext, &object_info, &responseHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    bool ret = true;
    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("RemoveBatchFile");
        if (m_completedStatus != OBS_STATUS_NoSuchBucket && m_completedStatus != OBS_STATUS_NoSuchKey &&
            m_completedStatus != OBS_STATUS_HttpErrorNotFound) {
            HCP_Log(INFO, LIBS3) << "The file of " << fileName << " is not on storage,request ID : " << m_requestID
                                 << ",duration = " << timer.Duration() << HCPENDLOG;
            ret = false;
        }
    }
    HCP_Log(DEBUG, LIBS3) << "remove from UDS successfully. File: " << fileName << HCPENDLOG;

    return ret;
}

bool libs3IO::Copy(S3BucketContextProxy &bucketContext, const string &oldName, const string &newName, int metaDataCount,
    obs_name_value *metaData)
{
    HCP_Log(DEBUG, LIBS3) << "enter Copy " << DBG(oldName) << DBG(newName) << DBG(metaDataCount) << HCPENDLOG;
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    int64_t lastModified;
    char eTag[256] = {0};
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    Timer timer;
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(oldName);

    obs_put_properties put_properties;
    InitPutProperties(&put_properties);
    put_properties.content_type = const_cast<char *>("text/plain");
    put_properties.meta_data_count = metaDataCount;
    put_properties.meta_data = metaData;

    obs_copy_destination_object_info objectinfo = {0};
    objectinfo.destination_bucket = bucketContext.bucket_options.bucket_name;
    objectinfo.destination_key = const_cast<char *>(newName.c_str());
    objectinfo.etag_return = eTag;
    objectinfo.etag_return_size = sizeof(eTag);
    objectinfo.last_modified_return = &lastModified;

    do {
        copy_object(&bucketContext,
            const_cast<char *>(oldName.c_str()),
            0,
            &objectinfo,
            (oldName == newName) ? 0 : 1,
            &put_properties,
            0,
            &responseHandler,
            this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("Rename");
        HCP_Log(ERR, LIBS3) << "CopyObject failed, old object name=" << m_objectName << DBG(newName)
                            << DBG(metaDataCount) << ", duration=" << timer.Duration()
                            << ",request ID : " << m_requestID << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "copy successfully.copy_object End " << DBG(oldName) << DBG(newName) << DBG(metaDataCount)
                          << ", duration=" << timer.Duration() << ",request ID : " << m_requestID << HCPENDLOG;

    return true;
}

bool libs3IO::FileSize(S3BucketContextProxy &bucketContext, const string &fileName, uintmax_t &size)
{
    HCP_Log(DEBUG, LIBS3) << "File name is: " << fileName << HCPENDLOG;
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(fileName);

    char *ssecCustomerAlgorithm = 0;
    char use_ssec = 0;
    string base = Base64Encode();
    string md5 = DekToMd5();
    if (m_dek.length() > 0) {
        use_ssec = 1;
        ssecCustomerAlgorithm = const_cast<char *>(ALGORITHM);
    }

    server_side_encryption_params serverSideEncryptionParams;
    if (memset_s(&serverSideEncryptionParams,
                 sizeof(server_side_encryption_params),
                 0,
                 sizeof(server_side_encryption_params)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
    }
    if (use_ssec == 1) {
        serverSideEncryptionParams.encryption_type = OBS_ENCRYPTION_SSEC;
        serverSideEncryptionParams.ssec_customer_algorithm = ssecCustomerAlgorithm;
        serverSideEncryptionParams.ssec_customer_key = const_cast<char *>(base.c_str());
    }
    obs_object_info object_info;
    if (memset_s(&object_info, sizeof(obs_object_info), 0, sizeof(obs_object_info)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
    }
    object_info.key = const_cast<char *>(fileName.c_str());
    object_info.version_id = 0;

    Timer timer;
    do {
        get_object_metadata(&bucketContext, &object_info, &serverSideEncryptionParams, &responseHandler, this);
    } while (S3HeadStatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        if (OBS_STATUS_NoSuchBucket == m_completedStatus || OBS_STATUS_NoSuchKey == m_completedStatus ||
            OBS_STATUS_HttpErrorNotFound == m_completedStatus) {
            HCP_Log(DEBUG, LIBS3) << fileName << " is not exists no need to get size,request ID : " << m_requestID
                                  << ",duration = " << timer.Duration() << HCPENDLOG;
            return true;
        }
        PrintStatusErr("FileSize");
        return false;
    }
    size = (uintmax_t)m_contentLength;
    HCP_Log(DEBUG, LIBS3) << "FileSize successfully. m_contentLengthis: " << size << HCPENDLOG;

    return true;
}

bool libs3IO::GetBucketObjects(S3BucketContextProxy &bucketCtx, vector<string> &elementList)
{
    bool ret = ListBucket(bucketCtx, "", "");
    if (!ret)
        return false;

    elementList = m_matchObjList;

    HCP_Log(DEBUG, LIBS3) << "GetBucketObjects ended successfully." << HCPENDLOG;

    return true;
}

bool libs3IO::NormalFileUpload(const string &local_objName, const string &remote_objName)
{
    if (local_objName.size() == 0) {
        HCP_Log(ERR, LIBS3) << "local_objName is empty. " << HCPENDLOG;
        return false;
    }
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    m_objType = OBJECT_FILE_DATA;
    return NormalFileUpload(bucketContext, local_objName, remote_objName);
}

bool libs3IO::NormalFileUpload(
    S3BucketContextProxy &bucketCtx, const string &local_objName, const string &remote_objName)
{
    struct stat statInfo;
    if (stat(local_objName.c_str(), &statInfo) != 0) {
        HCP_Log(ERR, LIBS3) << "Invoke stat failed." << HCPENDLOG;
        return false;
    }

    m_localFile = OpenLocalFile(local_objName, O_RDONLY);
    if (!m_localFile) {
        HCP_Log(ERR, LIBS3) << "Open file[" << local_objName << "] failed." << HCPENDLOG;
        return false;
    }

    obs_put_object_handler putObjectHandler = {{&ResponseCallback, &CompleteCallback}, &PutObjectDataCallback};

    m_localFileName = local_objName;
    uint64_t fileSize = statInfo.st_size;
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    obs_put_properties put_properties;
    int isMd5Upload = ConfigReader::getInt("Md5", "UploadWithMd5");
    GetUploadPutProperties(put_properties, isMd5Upload, local_objName);

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(remote_objName);
    Timer timer;
    RWBegin();
    do {
        RWRetry(false);
        put_object(&bucketCtx,
            const_cast<char *>(remote_objName.c_str()),
            fileSize,
            &put_properties,
            0,
            &putObjectHandler,
            this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("SendData");
        HCP_Log(ERR, LIBS3) << "SendData failed,file: " << local_objName
                            << ". PutObject End , duration=" << timer.Duration() << HCPENDLOG;
        return false;
    }

    if (m_callbackHandle.callBackFunc) {
        ((WriteFileCallback)m_callbackHandle.callBackFunc)(
            LayoutRetCode::SUCCESS, 0, fileSize, m_callbackHandle.callBackData);
    }

    HCP_Log(INFO, LIBS3) << "upload file success."
                         << " PutObject End, object name=" << m_objectName << ", duration=" << timer.Duration()
                         << HCPENDLOG;

    return true;
}

bool libs3IO::UploadObject(S3BucketContextProxy &bucketCtx, const string &objName, const char *buffer,
    size_t bufSize, int metaDataCount, obs_name_value *metaData)
{
    HCP_Log(DEBUG, LIBS3) << "enter UploadObject " << DBG(objName) << DBG(bufSize) << HCPENDLOG;

    m_external_buf.buffer = const_cast<char *>(buffer);
    m_external_buf.offset = 0;
    m_external_buf.buffer_size = bufSize;

    m_objType = OBJECT_EXTERNAL_DATA;

    obs_put_object_handler putObjectHandler = {{&ResponseCallback, &CompleteCallback}, &PutObjectDataCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    obs_put_properties put_properties;
    int isMd5Upload = ConfigReader::getInt("Md5", "UploadWithMd5");
    GetUploadPutProperties(put_properties, isMd5Upload, objName);

    if (metaDataCount > 0) {
        put_properties.content_type = const_cast<char *>("text/plain");
        put_properties.meta_data = metaData;
        put_properties.meta_data_count = metaDataCount;
    }

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(objName);
    Timer timer;
    RWBegin();
    do {
        RWRetry(false);
        put_object(
            &bucketCtx, const_cast<char *>(objName.c_str()), bufSize, &put_properties, 0, &putObjectHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("SendData");
        HCP_Log(ERR, LIBS3) << "SendData failed,file: " << objName << ". PutObject End , duration=" << timer.Duration()
                            << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "NormalBufferUpload success, file: " << objName << DBG(bufSize) << HCPENDLOG;

    return true;
}

bool libs3IO::ReadPartialObject(S3BucketContextProxy &bucketCtx, const string &objName, uint64_t startOffset,
    uint64_t rangeInBytes, char *buffer, uint64_t buffer_size)
{
    if (rangeInBytes > buffer_size) {
        HCP_Log(ERR, LIBS3) << "rangeInBytes=" << rangeInBytes << " is larger than buffer_size=" << buffer_size
                            << HCPENDLOG;
        return false;
    }

    m_external_buf.buffer = buffer;
    m_external_buf.offset = 0;
    m_external_buf.buffer_size = buffer_size;

    m_objType = OBJECT_EXTERNAL_DATA;

    return FileDownload(bucketCtx, objName, "", startOffset, rangeInBytes);
}

bool libs3IO::FileDownload(const string &objName, const string &destFile)
{
    if (objName.size() == 0) {
        HCP_Log(ERR, LIBS3) << "objName is empty. " << HCPENDLOG;
        return false;
    }
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    m_objType = OBJECT_FILE_DATA;
    return FileDownload(bucketContext, objName, destFile);
}

bool libs3IO::FileDownload(S3BucketContextProxy &bucketCtx, const string &objName, const string &destFile,
    uint64_t startOffset, uint64_t rangeInBytes)
{
    HCP_Log(DEBUG, LIBS3) << "enter FileDownload " << DBG(objName) << DBG(destFile) << DBG(startOffset)
        << DBG(rangeInBytes) << HCPENDLOG;

    if (!destFile.empty()) {
        m_localFile = OpenLocalFile(destFile, O_WRONLY | O_CREAT);
        if (!m_localFile) {
            HCP_Log(ERR, LIBS3) << "Open file[" << objName << "] failed." << HCPENDLOG;
            return false;
        }
    }

    HCP_Log(DEBUG, LIBS3) << "FileDownload cache mode: " << ((m_objType == OBJECT_FILE_DATA) ? "OBJECT_FILE_DATA" :
        ((m_objType == OBJECT_EXTERNAL_DATA) ? "OBJECT_EXTERNAL_DATA" : "OBJECT_CACHE_DATA")) << HCPENDLOG;

    obs_get_object_handler getObjectHandler = {{&ResponseCallback, &CompleteCallback}, &GetObjectDataCallback};

    obs_object_info object_info;
    if (memset_s(&object_info, sizeof(obs_object_info), 0, sizeof(obs_object_info)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
    }
    object_info.key = const_cast<char *>(objName.c_str());
    object_info.version_id = 0;

    // 定义范围下载参数
    obs_get_conditions getcondition;
    memset_s(&getcondition, sizeof(obs_get_conditions), 0, sizeof(obs_get_conditions));
    init_get_properties(&getcondition);
    getcondition.start_byte = startOffset;
    getcondition.byte_count = rangeInBytes;

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    m_localFileName = objName;
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(objName);
    Timer timer;
    (void)ftruncate(*m_localFile, 0);
    m_downloadedSize = 0;
    do {
        getcondition.start_byte = destFile.empty() ? startOffset : m_downloadedSize;
        // reset the fd
        lseek(*m_localFile, m_downloadedSize, SEEK_SET);
        HCP_Log(DEBUG, LIBS3) << "Start download in offset: " << m_downloadedSize << DBG(objName) << HCPENDLOG;
        get_object(&bucketCtx, &object_info, &getcondition, 0, &getObjectHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("RecvData");
        HCP_Log(ERR, LIBS3) << "GetObject End, object name=" << m_objectName << ", duration=" << timer.Duration()
                            << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "Download file sucess.GetObject End," << DBG(objName) << DBG(startOffset)
        << DBG(rangeInBytes) << ", request ID:" << m_requestID << ",duration = " << timer.Duration()
        << HCPENDLOG;

    return true;
}

obs_status libs3IO::TestBucket(S3BucketContextProxy &bucketCtx, const int retryTimes)
{
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    HCP_Logger_noid(DEBUG, LIBS3) << "domain:" << bucketCtx.bucket_options.host_name << HCPENDLOG;
    HCP_Logger_noid(DEBUG, LIBS3) << "protocol:" << bucketCtx.bucket_options.protocol << HCPENDLOG;
    HCP_Logger_noid(DEBUG, LIBS3) << "bucketname:" << bucketCtx.bucket_options.bucket_name << HCPENDLOG;

    int retry_times = GetS3RetryTimeAndSetConnectTImeOut();
    if (retryTimes >= 0) {
        retry_times = retryTimes;
    }

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(bucketCtx.bucket_options.bucket_name);
    do {
        string host = "";
        string user = "";
        FillHttpProxyParam(bucketCtx, host, user);
        bucketCtx.request_options.proxy_host = const_cast<char *>(host.c_str());
        bucketCtx.request_options.proxy_auth = const_cast<char *>(user.c_str());

        HCP_Log(DEBUG, LIBS3) << "proxy_host: " << bucketCtx.request_options.proxy_host << HCPENDLOG;
        obs_head_bucket(&bucketCtx, &responseHandler, this);
        HCP_Log(DEBUG, LIBS3) << "headbucket test proxy :" << bucketCtx.GetProxyAddress() << HCPENDLOG;
    } while (S3HeadStatusIsRetryable() && ShouldRetry(retry_times));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("TestBucket");
        return m_completedStatus;
    }
    HCP_Log(DEBUG, LIBS3) << "Test bucket successfully." << HCPENDLOG;

    return m_completedStatus;
}

/**********************************************************************
@ Description  : Get object number of S3.
@ param bucketName [IN] sub bucket name
@ param objectNum  [IN] object number
@ Return       :Success return 0, Failed to return error code
***********************************************************************/
bool libs3IO::GetBucketObjNum(const char *bucketName, int64_t &objectNum)
{
    S3BucketContextProxy bucketContext(m_S3IOInfo);
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};
    if (bucketName == nullptr || strlen(bucketName) == 0) {
        HCP_Log(ERR, LIBS3) << "Param invalid.";
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "Begin to get Object Num, bucketName = " << bucketName << HCPENDLOG;

    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    bucketCtx.PrintBucketContext();

    int retry_times = GetS3RetryTimeAndSetConnectTImeOut();
    m_completedStatus = OBS_STATUS_ErrorUnknown;

    char bucketObjectNum[64] = {0};
    char CapacitysizeReturn[64] = {0};

    Timer timer;
    do {
        get_bucket_storage_info(&bucketContext,
            sizeof(CapacitysizeReturn),
            CapacitysizeReturn,
            sizeof(bucketObjectNum),
            bucketObjectNum,
            &responseHandler,
            this);
    } while (S3StatusIsRetryable() && ShouldRetry(retry_times));

    if (m_completedStatus == OBS_STATUS_AccessDenied) {
        objectNum = 0;
        HCP_Log(DEBUG, LIBS3) << "Access Denied. GetBucketStorageInfoCA End, bucket name=" << bucketName
                              << ", duration=" << timer.Duration() << HCPENDLOG;
        return true;
    }
    if (m_completedStatus == OBS_STATUS_SignatureDoesNotMatch) {
        objectNum = 0;
        HCP_Log(DEBUG, LIBS3) << "OBS_STATUS_SignatureDoesNotMatch. GetBucketStorageInfoCA End, bucket name="
                              << bucketName << ", duration=" << timer.Duration() << HCPENDLOG;
        return true;
    }
    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("GetBucketObjNum");
        HCP_Log(DEBUG, LIBS3) << "GetBucketStorageInfoCA End, bucket name=" << bucketName
                              << ", duration=" << timer.Duration() << HCPENDLOG;
        return false;
    }
    try {
        objectNum = boost::lexical_cast<int64_t>(bucketObjectNum);
    } catch (exception &e) {
        PrintStatusErr("GetBucketObjNum");
        HCP_Log(DEBUG, LIBS3) << "GetBucketObjNum throw a exception " << WIPE_SENSITIVE(e.what()) << "." << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "GetBucketObjNum successfully. GetBucketStorageInfoCA End, bucket name=" << bucketName
                          << ", duration=" << timer.Duration() << HCPENDLOG;
    return true;
}

void libs3IO::JudgeStatus(obs_status status, uint64_t &capacity, bool &ret)
{
    if (status == OBS_STATUS_AccessDenied) {
        capacity = S3_BRICK_DEFAULT_SIZE;
        HCP_Log(WARN, LIBS3) << "Access Denied." << HCPENDLOG;
        ret = true;
    } else if (status == OBS_STATUS_SignatureDoesNotMatch) {
        capacity = S3_BRICK_DEFAULT_SIZE;
        HCP_Log(WARN, LIBS3) << "OBS_STATUS_SignatureDoesNotMatch." << HCPENDLOG;
        ret = true;
    } else {
        PrintStatusErr("GetCapacity");
    }
}

bool libs3IO::GetCapacity(uint64_t &capacity)
{
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    bucketCtx.PrintBucketContext();
    int retry_times = GetS3RetryTimeAndSetConnectTImeOut();
    m_completedStatus = OBS_STATUS_ErrorUnknown;

    uint64_t bucketquota = 0;
    do {
        string host = "";
        string user = "";
        FillHttpProxyParam(bucketCtx, host, user);
        bucketCtx.request_options.proxy_host = const_cast<char *>(host.c_str());
        bucketCtx.request_options.proxy_auth = const_cast<char *>(user.c_str());
        get_bucket_quota(&bucketCtx, &bucketquota, &responseHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retry_times));

    if (m_completedStatus != OBS_STATUS_OK) {
        bool ret = false;
        JudgeStatus(m_completedStatus, capacity, ret);
        return ret;
    }
    HCP_Log(DEBUG, LIBS3) << "GetCapacity successfully." << HCPENDLOG;

    try {
        capacity = boost::lexical_cast<uint64_t>(bucketquota);
    } catch (exception &e) {
        PrintStatusErr("GetCapacity");
        HCP_Log(ERR, LIBS3) << "GetCapacity throw a exception " << WIPE_SENSITIVE(e.what()) << "." << HCPENDLOG;
        return false;
    }

    if (capacity == 0) {
        capacity = S3_BRICK_DEFAULT_SIZE;
    }
    return true;
}

bool libs3IO::GetUsed(uint64_t &used)
{
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    S3BucketContextProxy bucketCtx(m_S3IOInfo);
    bucketCtx.PrintBucketContext();

    int retry_times = GetS3RetryTimeAndSetConnectTImeOut();

    m_completedStatus = OBS_STATUS_ErrorUnknown;

    char bucketSize[64] = {0}, bucketObjects[64] = {0};
    do {
        string host = "";
        string user = "";
        FillHttpProxyParam(bucketCtx, host, user);
        bucketCtx.request_options.proxy_host = const_cast<char *>(host.c_str());
        bucketCtx.request_options.proxy_auth = const_cast<char *>(user.c_str());
        get_bucket_storage_info(
            &bucketCtx, sizeof(bucketSize), bucketSize, sizeof(bucketObjects), bucketObjects, &responseHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retry_times));

    if (m_completedStatus == OBS_STATUS_AccessDenied) {
        used = 0;
        HCP_Log(DEBUG, LIBS3) << "Access Denied." << HCPENDLOG;
        return true;
    }

    if (m_completedStatus == OBS_STATUS_SignatureDoesNotMatch) {
        used = 0;
        HCP_Log(DEBUG, LIBS3) << "OBS_STATUS_SignatureDoesNotMatch." << HCPENDLOG;
        return true;
    }

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("GetCapacity");
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "GetCapacity successfully." << HCPENDLOG;

    try {
        used = boost::lexical_cast<uint64_t>(bucketSize);
    } catch (exception &e) {
        PrintStatusErr("GetCapacity");
        HCP_Log(DEBUG, LIBS3) << "GetCapacity throw a exception " << WIPE_SENSITIVE(e.what()) << "." << HCPENDLOG;
        return false;
    }

    return true;
}

bool libs3IO::CompleteMultipartUploadInternalList(
    S3BucketContextProxy &bucketCtx, const string &objName, const string &uploadTaskID)
{
    HCP_Log(INFO, LIBS3) << "enter CompleteMultipartUploadInternalList" << DBG(objName) << HCPENDLOG;
    int retryTimes = RetriesG;

    obs_put_properties stPutProperties;
    InitPutProperties(&stPutProperties);
    stPutProperties.content_type = const_cast<char *>("text/plain");

    boost::scoped_array<obs_complete_upload_Info> peUploadInfo(
        new (nothrow) obs_complete_upload_Info[m_PartsInfoVector.size()]);
    if (peUploadInfo == nullptr) {
        HCP_Log(ERR, LIBS3) << "begin complete multipart upload failed!" << DBG(objName) << HCPENDLOG;
        return false;
    }

    int i = 0;
    for (auto &sPart : m_PartsInfoVector) {
        peUploadInfo[i].part_number = sPart.partNumber;
        peUploadInfo[i].etag = const_cast<char *>(sPart.eTag.c_str());
        i++;
    }

    obs_complete_multi_part_upload_handler responseHandler = {
        {&ResponseCallback, &CompleteCallback}, &CompleteMultipartUploadCallback};

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(objName);
    do {
        complete_multi_part_upload(&bucketCtx,
            const_cast<char *>(objName.c_str()),
            const_cast<char *>(uploadTaskID.c_str()),
            (unsigned int)m_PartsInfoVector.size(),
            peUploadInfo.get(),
            &stPutProperties,
            &responseHandler,
            this);
        // 这个接口的回调函数没有写任何数据,所以不需要回滚数据;
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("CompleteMultipartUploadInternalList");
        return false;
    }

    HCP_Log(INFO, LIBS3) << "CompleteMultipartUploadInternalList successfully. " << DBG(objName) << HCPENDLOG;
    return true;
}

bool libs3IO::CompleteMultiPartUpload(
    S3BucketContextProxy &bucketCtx, const string &remote_objName, const string &uploadTaskID)
{
    HCP_Log(INFO, LIBS3) << "enter CompleteMultiPartUpload" << DBG(remote_objName) << HCPENDLOG;
    bool ret = false;

    // multi parts
    ret = CompleteMultipartUploadInternalList(bucketCtx, remote_objName, uploadTaskID);
    if (!ret) {
        HCP_Log(ERR, LIBS3) << "Begin CompleteMultipartUpload failed !" << HCPENDLOG;
        return false;
    }
    return true;
}

static int uploadMultiPartCallback(int bufferSize, char *buffer, void *callbackData)
{
    libs3IO *s3Obj = static_cast<libs3IO *>(callbackData);
    return s3Obj->CopyFromExternalBuf(buffer, bufferSize);
}

bool libs3IO::InitiateMultiPartUpload(
    S3BucketContextProxy &bucketCtx, const string &objName, string &uploadTaskID)
{
    HCP_Log(INFO, LIBS3) << "enter InitiateMultiPartUpload " << DBG(objName) << HCPENDLOG;
    char szUpload[256] = {0};
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    m_PartsInfoVector.clear();

    obs_put_properties stPutProperties;
    InitPutProperties(&stPutProperties);
    stPutProperties.content_type = const_cast<char *>("text/plain");

    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    Timer timer;
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(objName);
    do {
        initiate_multi_part_upload(&bucketCtx,
            const_cast<char *>(objName.c_str()),
            sizeof(szUpload),
            szUpload,
            &stPutProperties,
            0,
            &responseHandler,
            this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));
    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("InitiateMultipartUpload");
        HCP_Log(ERR, LIBS3) << "InitiateMultipartUpload End, object name=" << objName
                            << ", duration=" << timer.Duration() << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "szUpload=" << szUpload << ",szUpload size=" << sizeof(szUpload) << DBG(objName)
                          << HCPENDLOG;
    HCP_Log(INFO, LIBS3) << "InitiateMultipartUpload successfully." << DBG(objName) << ", duration=" << timer.Duration()
                         << HCPENDLOG;

    uploadTaskID = szUpload;
    return true;
}

bool libs3IO::CopyOnePart(S3BucketContextProxy &bucketCtx, const string &srcObjName, const string &destBucket,
    const string &destObjName, uint64_t startByte, uint64_t byteCount, const string &uploadTaskID,
    int partIndex)
{
    HCP_Log(DEBUG, LIBS3) << "enter CopyOnePart " << DBG(srcObjName) << DBG(partIndex) << DBG(destObjName) << HCPENDLOG;

    if (partIndex <= 0) {
        HCP_Log(ERR, LIBS3) << "partIndex:" << partIndex << " <= 0 is invalid" << HCPENDLOG;
        return false;
    }

    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    int64_t lastModified = -1;
    char eTag[256] = {0};
    obs_copy_destination_object_info object_info;
    memset_s(&object_info, sizeof(obs_copy_destination_object_info), 0, sizeof(obs_copy_destination_object_info));
    object_info.destination_bucket = const_cast<char *>(destBucket.c_str());
    object_info.destination_key = const_cast<char *>(destObjName.c_str());
    object_info.last_modified_return = &lastModified;
    object_info.etag_return = eTag;
    object_info.etag_return_size = sizeof(eTag);

    obs_upload_part_info copypart;
    memset_s(&copypart, sizeof(obs_upload_part_info), 0, sizeof(obs_upload_part_info));
    copypart.part_number = partIndex;
    copypart.upload_id = const_cast<char *>(uploadTaskID.c_str());

    obs_put_properties put_properties;
    InitPutProperties(&put_properties);
    put_properties.content_type = const_cast<char *>("text/plain");
    put_properties.start_byte = startByte;
    put_properties.byte_count = byteCount;

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(destObjName);
    do {
        HCP_Log(DEBUG, LIBS3) << "CopyPart ing... partNum=" << partIndex << DBG(srcObjName) << DBG(destObjName)
                              << HCPENDLOG;
        copy_part(&bucketCtx,
            const_cast<char *>(srcObjName.c_str()),
            &object_info,
            &copypart,
            &put_properties,
            0,
            &responseHandler,
            this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK || strlen(object_info.etag_return) == 0) {
        PrintStatusErr("CopyPart");
        HCP_Log(ERR, LIBS3) << "CopyPart failed or etag_return size is 0, " << DBG(srcObjName) << DBG(partIndex)
                            << DBG(destObjName) << HCPENDLOG;
        return false;
    }

    // save etag
    singlePart sPart;
    sPart.partNumber = m_PartsInfoVector.size() + 1;
    sPart.eTag = object_info.etag_return;
    m_PartsInfoVector.push_back(sPart);
    m_etag = string(object_info.etag_return);

    HCP_Log(DEBUG, LIBS3) << "CopyOnePart success!" << DBG(srcObjName) << DBG(partIndex) << DBG(destObjName)
                          << HCPENDLOG;
    return true;
}

obs_status libs3IO::CallbackListMultipartUploads(int isTruncated, const char *nextMarker,
    const char *nextUploadIdMarker, int uploadsCount, const obs_list_multipart_upload *uploads, int commonPrefixesCount,
    const char **commonPrefixes)
{
    list_multipart_uploads_callback_data *data = &m_ListPartUploadsInfo;
    HCP_Log(DEBUG, LIBS3) << "listMultipartUploadsCallback,isTruncated = " << isTruncated << HCPENDLOG;
    data->isTruncated = isTruncated;
    HCP_Log(DEBUG, LIBS3) << "nextMarker = " << nextMarker << HCPENDLOG;
    HCP_Log(INFO, LIBS3) << "nextUploadIdMarker = " << nextUploadIdMarker << HCPENDLOG;
    HCP_Log(DEBUG, LIBS3) << "uploadsCount = " << uploadsCount << HCPENDLOG;

    // This is tricky.  S3 doesn't return the NextMarker if there is no
    // delimiter.  Why, I don't know, since it's still useful for paging
    // through results.  We want NextMarker to be the last content in the
    // list, so set it to that if necessary.
    if ((!nextMarker || !nextMarker[0]) && uploadsCount) {
        nextMarker = uploads[uploadsCount - 1].key;
    }

    if (nextMarker) {
        if (sprintf_s(data->nextMarker, sizeof(data->nextMarker), "%s", nextMarker) == -1) {
            HCP_Log(ERR, LIBS3) << "sprintf_s failed" << HCPENDLOG;
        }
        if (sprintf_s(data->nextUploadIdMarker, sizeof(data->nextUploadIdMarker), "%s", nextUploadIdMarker) == -1) {
            HCP_Log(ERR, LIBS3) << "sprintf_s failed" << HCPENDLOG;
        }
    } else {
        data->nextMarker[0] = 0;
    }

    for (int i = 0; i < uploadsCount; i++) {
        const obs_list_multipart_upload *upload = &(uploads[i]);
        singleUploadTask sTask(upload->key, upload->upload_id, upload->initiated);
        data->uploadTaskMap.emplace(make_pair(data->uploadTasksCount + i + 1, sTask));
        HCP_Log(DEBUG, LIBS3) << "key: " << upload->key << ", uploadId: " << upload->upload_id << ", initiated"
                              << upload->initiated << HCPENDLOG;
    }

    for (int i = 0; i < commonPrefixesCount; i++) {
        HCP_Log(DEBUG, LIBS3) << "Common Prefix: " << commonPrefixes[i] << HCPENDLOG;
    }
    data->keyCount += uploadsCount;
    data->uploadTasksCount += uploadsCount;

    return OBS_STATUS_OK;
}

static obs_status listMultipartUploadsCallback(int isTruncated, const char *nextMarker, const char *nextUploadIdMarker,
    int uploadsCount, const obs_list_multipart_upload *uploads, int commonPrefixesCount, const char **commonPrefixes,
    void *callbackData)
{
    libs3IO *s3Obj = static_cast<libs3IO *>(callbackData);
    return s3Obj->CallbackListMultipartUploads(
        isTruncated, nextMarker, nextUploadIdMarker, uploadsCount, uploads, commonPrefixesCount, commonPrefixes);
}

bool libs3IO::ListMultiPartUploads(S3BucketContextProxy &bucketCtx, const string &prefix)
{
    HCP_Log(INFO, LIBS3) << "enter ListMultiPartUploads" << DBG(prefix) << HCPENDLOG;

    obs_list_multipart_uploads_handler listUploadsHandler = {
        {&ResponseCallback, &CompleteCallback}, &listMultipartUploadsCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(prefix);
    m_ListPartUploadsInfo = list_multipart_uploads_callback_data();
    int maxkeys = 0;

    do {
        m_ListPartUploadsInfo.isTruncated = 0;
        do {
            list_multipart_uploads(&bucketCtx,
                const_cast<char *>(prefix.c_str()),
                m_ListPartUploadsInfo.nextMarker,
                0,
                m_ListPartUploadsInfo.nextUploadIdMarker,
                maxkeys,
                &listUploadsHandler,
                this);
            // 此接口通过nextMark去记录位置，所以重试时，不会出现数据数据拼接问题;
        } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

        if (m_completedStatus != OBS_STATUS_OK) {
            PrintStatusErr("ListMultipartUploads");
            HCP_Log(ERR, LIBS3) << "ListMultiPartUploads failed! " << DBG(prefix) << HCPENDLOG;
            return false;
        }
    } while (m_ListPartUploadsInfo.isTruncated && (!maxkeys || (m_ListPartUploadsInfo.keyCount < maxkeys)));

    HCP_Log(INFO, LIBS3) << "ListMultipartUploads successfully. " << DBG(prefix)
                         << ", uploadTaskMap.size:" << m_ListPartUploadsInfo.uploadTaskMap.size() << HCPENDLOG;
    return true;
}

bool libs3IO::GetuploadTaskMapByPath(
    S3BucketContextProxy &bucketCtx, const string &prefix, map<int, singleUploadTask> &uploadTaskMap)
{
    HCP_Log(DEBUG, LIBS3) << "enter GetuploadTaskMapByPath" << DBG(prefix) << HCPENDLOG;
    bool res = ListMultiPartUploads(bucketCtx, prefix);
    if (!res) {
        HCP_Log(ERR, LIBS3) << "ListMultiPartUploads failed! " << DBG(prefix) << HCPENDLOG;
        return false;
    }

    uploadTaskMap = m_ListPartUploadsInfo.uploadTaskMap;
    HCP_Log(DEBUG, LIBS3) << "GetuploadTaskMapByPath successfully. " << DBG(prefix) << HCPENDLOG;
    return true;
}

bool libs3IO::UploadOnePart(S3BucketContextProxy &bucketCtx, const string &objName,
    const string &uploadTaskID, char *buffer, uint64_t bufferSize, int partIndex)
{
    HCP_Log(DEBUG, LIBS3) << "enter UploadOnePart " << DBG(objName) << DBG(partIndex) << HCPENDLOG;
    obs_upload_handler responseHandler = {{&ResponseCallback, &CompleteCallback}, &uploadMultiPartCallback};

    if (partIndex <= 0) {
        HCP_Log(ERR, LIBS3) << "partIndex:" << partIndex << " <= 0 is invalid" << HCPENDLOG;
        return false;
    }

    m_external_buf.buffer = buffer;
    m_external_buf.offset = 0;
    m_external_buf.buffer_size = bufferSize;

    obs_upload_part_info uploadPartInfo;
    memset_s(&uploadPartInfo, sizeof(obs_upload_part_info), 0, sizeof(obs_upload_part_info));
    uploadPartInfo.part_number = partIndex;
    uploadPartInfo.upload_id = const_cast<char *>(uploadTaskID.c_str());

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(objName);
    do {
        HCP_Log(DEBUG, LIBS3) << "UploadPart ing... partNum=" << partIndex << " objName=" << objName
                              << " bufferSize=" << bufferSize << HCPENDLOG;

        upload_part(
            &bucketCtx, const_cast<char *>(objName.c_str()), &uploadPartInfo, bufferSize, 0, 0, &responseHandler, this);

        if (S3StatusIsRetryable())
            m_external_buf.offset = 0;
        // 已排查，有重置offset。不会传坏数据;
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("UploadPart");
        HCP_Log(ERR, LIBS3) << "UploadOnePart failed, " << DBG(objName) << DBG(partIndex) << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, LIBS3) << "UploadOnePart success!" << DBG(objName) << DBG(partIndex) << HCPENDLOG;
    return true;
}

bool libs3IO::GetMetadata(
    S3BucketContextProxy &bucketCtx, const string &objName, vector<obj_metadata> &obj_metadata)
{
    HCP_Log(DEBUG, LIBS3) << "enter GetMetadata " << DBG(objName) << HCPENDLOG;

    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    obs_object_info objectinfo;
    memset_s(&objectinfo, sizeof(obs_object_info), 0, sizeof(obs_object_info));
    objectinfo.key = const_cast<char *>(objName.c_str());

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(objName);
    do {
        get_object_metadata(&bucketCtx, &objectinfo, 0, &responseHandler, this);
    } while (S3HeadStatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("GetObjectMetadata");
        return false;
    }

    obj_metadata.clear();
    obj_metadata = m_obj_metadata;

    HCP_Log(DEBUG, LIBS3) << "GetMetadata success!" << DBG(objName) << HCPENDLOG;
    return true;
}

bool libs3IO::ListObjectName(S3IOParams params, string prefix, int maxKeys, vector<string> &objectNames)
{
    bool ret = ListBucket(params, prefix.c_str(), nullptr, nullptr, maxKeys);
    if (!ret) {
        HCP_Log(ERR, LIBS3) << "List bucket error." << HCPENDLOG;
        return false;
    }
    for (auto object : m_objectInfoList) {
        objectNames.emplace_back(object.m_ObjectName);
    }
    return true;
}

#endif