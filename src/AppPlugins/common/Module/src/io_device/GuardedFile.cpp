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
#include "GuardedFile.h"
#include "IODeviceManager.h"
#include "log/Log.h"

using namespace std;
using namespace Module;

namespace {
    const string GUARDED_FILE = "GUARDED_FILE";
}

GuardedFile::GuardedFile(OBJECT_TYPE fileType) : m_FileType(fileType)
{
    HCP_Log(DEBUG, GUARDED_FILE) << "Constructor of GuardedFile." << HCPENDLOG;
}

GuardedFile::~GuardedFile()
{
    HCP_Log(DEBUG, GUARDED_FILE) << "Destructor of GuardedFile." << HCPENDLOG;
    if (nullptr != m_IODevice) {
        m_IODevice->Close(FILE_CLOSE_STATUS_FINISH);
    }
}

string GuardedFile::GetErrorInfo()
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return "Unknown Error";
    }

    return m_IODevice->GetErrorInfo();
}

bool GuardedFile::GetNotExistError()
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return false;
    }

    return m_IODevice->GetNotExistError();
}

bool GuardedFile::Open(const char *filename, const char *mode, EBKFileHandle *handle)
{
    if (filename == nullptr || mode == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Open Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(filename);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->Open(filename, mode, handle);
}

bool GuardedFile::Close(FileCloseStatus status)
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return false;
    }

    m_IODevice->SetUpLoadRateLimit(m_UpLoadRateLimit);
    bool ret = m_IODevice->Close(status);
    return ret;
}

void GuardedFile::Reset(const char *fileName, const char *mode /* = "r" */)
{
    if (fileName == nullptr) {
        HCP_Log(WARN, GUARDED_FILE) << "Reset Param Null pointer." << HCPENDLOG;
        if (m_IODevice) {
            m_IODevice->Close(FILE_CLOSE_STATUS_FINISH);
        }
        return;
    }
    CreateInstance(fileName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return;
    }
    m_IODevice->Reset(fileName, mode);
}

size_t GuardedFile::Write(const void *ptr, size_t size, size_t count)
{
    if (m_IODevice == nullptr || ptr == nullptr || count <= 0) {
        HCP_Log(WARN, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return 0;
    }

    return m_IODevice->Write(ptr, size, count);
}

size_t GuardedFile::WriteFS(const void* ptr, size_t size, size_t count)
{
    if (m_IODevice == nullptr || ptr == nullptr || count <= 0) {
        return 0;
    }
    return m_IODevice->WriteFS(ptr, size, count);
}

bool GuardedFile::UseVpp()
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return false;
    }

    return m_IODevice->UseVpp();
}

size_t GuardedFile::Append(const void *ptr, size_t count)
{
    if (m_IODevice == nullptr || ptr == nullptr || count == 0) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return 0;
    }

    return m_IODevice->Append(ptr, count);
}

size_t GuardedFile::Read(void *ptr, size_t size, size_t count)
{
    if (m_IODevice == nullptr || ptr == nullptr || count == 0) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return 0;
    }
    return (m_IODevice->Read(ptr, size, count));
}

size_t GuardedFile::ReadFS(void *ptr, size_t size, size_t count)
{
    if (m_IODevice == nullptr || ptr == nullptr || count == 0) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return 0;
    }
    return (m_IODevice->ReadFS(ptr, size, count));
}

int GuardedFile::Seek(long int offset, int origin /* = SEEK_SET */)
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return -1;
    }
    return m_IODevice->Seek(offset, origin);
}

long int GuardedFile::Tell()
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return -1;
    }
    return m_IODevice->Tell();
}

bool GuardedFile::FileSize(uintmax_t &size, const char *filename)
{
    if (strlen(filename) != 0) {
        CreateInstance(filename);
    }

    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return false;
    }

    if (false == m_IODevice->FileSize(filename, size)) {
        HCP_Log(ERR, GUARDED_FILE) << "error when get file size" << HCPENDLOG;
        return false;
    }

    return true;
}

long int GuardedFile::DirSize(const char *filename /* = "" */)
{
    if (strlen(filename) != 0) {
        CreateInstance(filename);
    }

    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return 0;
    }

    return m_IODevice->DirSize(filename);
}
int GuardedFile::Truncate(long int position)
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return -1;
    }

    return m_IODevice->Truncate(position);
}

int GuardedFile::Flush(bool sync /* =true */)
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return -1;
    }
    return m_IODevice->Flush(sync);
}

bool GuardedFile::Loaded()
{
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Null pointer." << HCPENDLOG;
        return false;
    }

    return m_IODevice->Loaded();
}

boost::tribool GuardedFile::FileExists(const char *filename)
{
    if (filename == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Exists Null pointer." << HCPENDLOG;
        return boost::indeterminate;
    }
    CreateInstance(filename);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return boost::indeterminate;
    }
    return m_IODevice->FileExists(filename);
}

bool GuardedFile::FilePrefixExists(const char *filePrefixName, vector<string> &suffixs)
{
    if (filePrefixName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "FilePrefixExists Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(filePrefixName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->FilePrefixExists(filePrefixName, suffixs);
}

bool GuardedFile::Exists(const char *filename)
{
    if (filename == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Exists Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(filename);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->Exists(filename);
}

bool GuardedFile::Remove(const char *filename)
{
    if (filename == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Remove Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(filename);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->Remove(filename);
}

bool GuardedFile::RemoveAll(const char *directoryName)
{
    if (directoryName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "RemoveAll Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(directoryName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->RemoveAll(directoryName);
}

bool GuardedFile::Rename(const char *oldName, const char *newName)
{
    if (oldName == nullptr || newName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Rename Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(oldName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->Rename(oldName, newName);
}

bool GuardedFile::CreateDirectory(const char *directoryName)
{
    HCP_Log(ERR, GUARDED_FILE) << "FLOW: Entered GuardedFile::CreateDirectory" << directoryName <<HCPENDLOG;
    if (directoryName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "CreateDirectory Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(directoryName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->CreateDirectory(directoryName);
}

bool GuardedFile::CreateCifsDirectory(const char *directoryName)
{
    HCP_Log(DEBUG, GUARDED_FILE)
        << "FLOW: Entered GuardedFile::CreateDirectory" << directoryName << HCPENDLOG;
    if (directoryName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "CreateDirectory Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(directoryName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->CreateCifsDirectory(directoryName);
}

bool GuardedFile::IsDirectory(const char *directoryName)
{
    if (directoryName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "IsDirectory Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(directoryName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->IsDirectory(directoryName);
}

bool GuardedFile::Copy(const char *srcName, const char *destName)
{
    if (srcName == nullptr || destName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(srcName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->Copy(srcName, destName);
}

bool GuardedFile::CopyDirectory(const char *srcName, const char *destName)
{
    if (srcName == nullptr || destName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Create instance failed." << HCPENDLOG;
        return false;
    }
    CreateInstance(srcName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->CopyDirectory(srcName, destName);
}

bool GuardedFile::GetDirectoryList(const char *directoryName, vector<string> &elementList)
{
    if (directoryName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "GetDirectoryList Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(directoryName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->GetDirectoryList(directoryName, elementList);
}

bool GuardedFile::GetFileListInDirectory(const char *directoryName, vector<string> &elementList)
{
    if (directoryName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE)
                  << "GetFileListInDirectory Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(directoryName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->GetFileListInDirectory(directoryName, elementList);
}

bool GuardedFile::GetObjectListWithMarker(const string &dir, string &marker, bool &isEnd, int maxKeys,
    vector<string> &elementList)
{
    if (dir.empty()) {
        HCP_Log(ERR, GUARDED_FILE) << "GetFileListInDirectory Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(dir);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->GetObjectListWithMarker(dir, marker, isEnd, maxKeys, elementList);
}

bool GuardedFile::GetCommonPrefixList(const char *directoryName, const string &delimiter,
                                      vector<string> &elementList)
{
    if (directoryName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "GetCommonPrefixList Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(directoryName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->GetCommonPrefixList(directoryName, delimiter, elementList);
}

bool GuardedFile::TestConnect(const char *bucketName, const int retryTimes)
{
    if (bucketName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "TestConnect Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(bucketName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->TestConnect(bucketName, retryTimes);
}

bool GuardedFile::GetSpaceInfo(const char *pathName, uint64_t &capacity, uint64_t &free)
{
    if (pathName == nullptr) {
        HCP_Log(DEBUG, GUARDED_FILE) << "TestConnect Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(pathName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->GetSpaceInfo(pathName, capacity, free);
}

void GuardedFile::CreateInstance(const string &fileName)
{
    HCP_Log(DEBUG, GUARDED_FILE) << "FLOW : CreateInstance fn:" << fileName << HCPENDLOG;
    if (nullptr != m_IODevice) {
        HCP_Log(ERR, GUARDED_FILE) << "1No io device instance been created." << HCPENDLOG;
        return;
    }

    m_IODevice = IODeviceManager::GetInstance().CreateIODeviceByPath(fileName, m_FileType);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return;
    }
    m_IODevice->SetDek(m_dek);
}

bool GuardedFile::TestSubBucketIsNotExisted(const char *bucketName, const int retryTimes)
{
    if (bucketName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "TestConnect Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(bucketName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->TestSubBucketIsNotExisted(bucketName, retryTimes);
}

bool GuardedFile::GetBucketObjNum(const char *bucketName, int64_t &objectNum)
{
    if (bucketName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "GetBucketObjNum Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(bucketName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    return m_IODevice->GetBucketObjNum(bucketName, objectNum);
}

bool GuardedFile::ReadNoCache(const char *filename, const string &dek, size_t offset, char *buffer,
                              const size_t bufferLen, size_t &readedLen)
{
    if (filename == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "Open Param Null pointer." << HCPENDLOG;
        return false;
    }
    CreateInstance(filename);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }

    return m_IODevice->ReadNoCache(filename, dek, offset, buffer, bufferLen, readedLen);
}

// The function returns True when the bucket does not exist or exists and is accessible.
bool GuardedFile::TestSubBucketExisted(const char *bucketName)
{
    if (bucketName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "TestConnect Param Null pointer." << HCPENDLOG;
        return false;
    }

    CreateInstance(bucketName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }

    return m_IODevice->TestSubBucketExisted(bucketName);
}
// When the bucket explicitly does not exist, the function returns True, else returns false.
bool GuardedFile::TestBucketExisted(const char *bucketName)
{
    if (bucketName == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "TestConnect Param Null pointer." << HCPENDLOG;
        return false;
    }

    CreateInstance(bucketName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }

    return m_IODevice->TestBucketExisted(bucketName);
}

bool GuardedFile::RemoveObjs(const vector<string> &directoryNames,
                             vector<string> &objs, uintmax_t &size, const uint8_t &snapVersion)
{
    if (directoryNames.size() == 0) {
        HCP_Log(ERR, GUARDED_FILE) << "directoryNames size = 0. " << HCPENDLOG;
        return false;
    }
    string directoryName = directoryNames[0];
    CreateInstance(directoryName);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, GUARDED_FILE) << "begin remove objs" << HCPENDLOG;
    return m_IODevice->RemoveObjs(directoryNames, objs, size, snapVersion);
}

bool GuardedFile::DownloadFile(const string &s3Path, const string &objName,
                               const string &destFile)
{
    CreateInstance(s3Path);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }

    int cacheType = m_IODevice->GetCacheDataType();
    m_IODevice->SetCacheDataType(1);  // OBJECT_FILE_DATA
    bool ret = m_IODevice->DownloadFile(objName, destFile);

    m_IODevice->SetCacheDataType(cacheType);

    return ret;
}

bool GuardedFile::NormalFileUpload(const string &s3Path, const string &localFile,
                                   const string &remoteFile)
{
    CreateInstance(s3Path);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return false;
    }

    int cacheType = m_IODevice->GetCacheDataType();
    m_IODevice->SetCacheDataType(1);  // OBJECT_FILE_DATA
    m_IODevice->SetUpLoadRateLimit(m_UpLoadRateLimit);
    bool ret = m_IODevice->NormalFileUpload(localFile, remoteFile);

    m_IODevice->SetCacheDataType(cacheType);

    return ret;
}

void GuardedFile::RegisterCallbackHandle(const string &s3Path, CallBackHandle &handle)
{
    CreateInstance(s3Path);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return;
    }
    int cacheType = m_IODevice->GetCacheDataType();
    m_IODevice->SetCacheDataType(1);  // OBJECT_FILE_DATA
    m_IODevice->RegisterCallbackHandle(handle);
    m_IODevice->SetCacheDataType(cacheType);
}

void GuardedFile::RegisterReadFileCallbackFun(const string &s3Path, const ReadFileCallback &handle)
{
    CreateInstance(s3Path);
    if (m_IODevice == nullptr) {
        HCP_Log(ERR, GUARDED_FILE) << "No io device instance been created. " << HCPENDLOG;
        return;
    }
    int cacheType = m_IODevice->GetCacheDataType();
    m_IODevice->SetCacheDataType(1);  // OBJECT_FILE_DATA
    m_IODevice->RegisterReadFileCallbackFun(handle);
    m_IODevice->SetCacheDataType(cacheType);
}

void GuardedFile::SetUpLoadRateLimit(uint64_t qos)
{
    m_UpLoadRateLimit = qos;
}

void GuardedFile::SetDownLoadRateLimit(uint64_t qos)
{
    m_DownLoadRateLimit = qos;
}
