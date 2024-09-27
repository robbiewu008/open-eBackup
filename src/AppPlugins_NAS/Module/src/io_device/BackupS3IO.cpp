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
#include "BackupS3IO.h"
#include "FileDef.h"
#include <fstream>
#include "IODeviceManager.h"

using namespace std;
using namespace Module;

namespace {
    const string MODULE = "BackupS3IO";
    constexpr uint32_t QOS_LIMIT = 2;
    constexpr int32_t READ_S3_DATA_FAILED = -1;
    constexpr uint64_t FILE_LEN_THRESHOLD = 100 * 1024 * 1024;
    constexpr uint64_t UPLOAD_FILE_SIZE_MAX = 49980ULL * 1024 * 1024 * 1024; // 49980G
};

BackupS3IO::BackupS3IO(const IODeviceInfo &info) : m_deviceInfo(info)
{}

BackupS3IO::~BackupS3IO()
{}

bool BackupS3IO::Initialize()
{
    bool ret = IODeviceManager::GetInstance().RegisterIODevice(
        m_deviceInfo, boost::bind(&S3SystemIO::CreateInstance, boost::placeholders::_1, boost::placeholders::_2));
    if (!ret) {
        HCP_Log(ERR, MODULE) << "RegisterIODevice failed. " << HCPENDLOG;
        return false;
    }
    m_snapGf = make_unique<GuardedFile>();
    if (m_snapGf == nullptr) {
        HCP_Log(ERR, MODULE) << "Create instance of GuardedFile failed. " << HCPENDLOG;
        return false;
    }
    return true;
}

void BackupS3IO::FormatFileName(string &s3path, const string &relativePath)
{
    string s3WorkPath = GetWorkSpace();
    if (!s3WorkPath.empty()) {
        s3path = m_deviceInfo.path_prefix + "/" + s3WorkPath;
    } else {
        s3path = m_deviceInfo.path_prefix;
    }
    if (relativePath[0] == '/') {
        s3path += relativePath;
    } else {
        s3path += "/" + relativePath;
    }
    HCP_Log(DEBUG, MODULE) << "Format s3 Path: " << s3path << HCPENDLOG;
}

bool BackupS3IO::GetFileContentLen(const string &fileName, uint64_t &contentLen)
{
    ifstream stream(fileName.c_str(), ios::in | ios::binary);
    if (!stream) {
        HCP_Log(ERR, MODULE) << "Open local file failed, name: " << fileName << HCPENDLOG;
        return false;
    }
    stream.seekg(0, ios_base::end);
    contentLen = stream.tellg();
    return true;
}

bool BackupS3IO::CreateDirectory(const string &directoryName)
{
    if (directoryName.empty()) {
        HCP_Log(ERR, MODULE) << "Directory name error." << HCPENDLOG;
        return false;
    }
    string dirName = directoryName;
    if (directoryName[directoryName.size()-1] != '/') {
        dirName += "/";
    }

    string s3Path;
    FormatFileName(s3Path, dirName);
    if (!m_snapGf->Open(s3Path.c_str(), "w", nullptr)) {
        HCP_Log(ERR, MODULE) << "Open the bucker file failed." << HCPENDLOG;
        return false;
    }
    if (!m_snapGf->Close()) {
        HCP_Log(ERR, MODULE) << "Close the bucker file failed." << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, MODULE) << "Create directory success." << HCPENDLOG;
    return true;
}

bool BackupS3IO::IsDirectory(const string &directoryName)
{
    string s3Path;
    FormatFileName(s3Path, directoryName);
    return m_snapGf->IsDirectory(s3Path.c_str());
}

bool BackupS3IO::IsFileExist(const string &fileName)
{
    string s3Path;
    FormatFileName(s3Path, fileName);
    return m_snapGf->Exists(s3Path.c_str());
}

bool BackupS3IO::GetDirectoryList(const string &directoryName, vector<string> &elementList)
{
    string s3Path;
    FormatFileName(s3Path, directoryName);
    return m_snapGf->GetDirectoryList(s3Path.c_str(), elementList);
}

bool BackupS3IO::GetFileListInDirectory(const string &directoryName, vector<string> &elementList)
{
    string s3Path;
    FormatFileName(s3Path, directoryName);
    return m_snapGf->GetFileListInDirectory(s3Path.c_str(), elementList);
}

bool BackupS3IO::GetObjectListWithMarker(const string &directoryName, string &marker, bool &isEnd,
    vector<string> &elementList, int maxSize)
{
    string s3Path;
    FormatFileName(s3Path, directoryName);
    return m_snapGf->GetObjectListWithMarker(s3Path, marker, isEnd, maxSize, elementList);
}

bool BackupS3IO::GetSpaceInfo(const string &pathName, uint64_t &capacity, uint64_t &free)
{
    string s3Path;
    FormatFileName(s3Path, pathName);
    return m_snapGf->GetSpaceInfo(s3Path.c_str(), capacity, free);
}


bool BackupS3IO::Append(const string &remoteFile, const string& buffer)
{
    string s3Path;
    FormatFileName(s3Path, remoteFile);
    if (!m_snapGf->Open(s3Path.c_str(), "a", nullptr)) {
        HCP_Log(ERR, MODULE) << "Open the bucker file failed." << HCPENDLOG;
        return false;
    }
    size_t wSize = m_snapGf->Append(buffer.c_str(), buffer.size());
    if (wSize != buffer.size()) {
        HCP_Log(ERR, MODULE) << "Write buffer to bucker failed." << HCPENDLOG;
        return false;
    }
    if (!m_snapGf->Close()) {
        HCP_Log(ERR, MODULE) << "Close the bucker file failed." << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, MODULE) << "Write the bucker file success." << HCPENDLOG;
    return true;
}

bool BackupS3IO::ReadFile(const string &remoteFile, string &buffer)
{
    string s3Path;
    FormatFileName(s3Path, remoteFile);
    uintmax_t fileSize = 0;
    if (!m_snapGf->FileSize(fileSize, s3Path.c_str())) {
        HCP_Log(ERR, MODULE) << "Get file size failed." << HCPENDLOG;
        return false;
    }
    if (!m_snapGf->Open(s3Path.c_str(), "r", nullptr)) {
        HCP_Log(ERR, MODULE) << "Open file failed." << HCPENDLOG;
        return false;
    }
    unique_ptr<char[]> data = make_unique<char[]>(fileSize + 1);
    if (data == nullptr) {
        HCP_Log(ERR, MODULE) << "Malloc memory failed." << HCPENDLOG;
        return false;
    }
    if (!m_snapGf->Read(data.get(), fileSize, 1)) {
        HCP_Log(ERR, MODULE) << "Read file failed." << HCPENDLOG;
        return false;
    }
    buffer = string(data.get());
    if (!m_snapGf->Close()) {
        HCP_Log(ERR, MODULE) << "Close the bucker file failed." << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, MODULE) << "Read file to string buffer success." << HCPENDLOG;
    return true;
}

bool BackupS3IO::ReadFile(const string &remoteFile, const string &localFile)
{
    string s3workPath = GetWorkSpace();
    string objName = s3workPath.empty() ? remoteFile : (s3workPath + "/" + remoteFile);
    return m_snapGf->DownloadFile(m_deviceInfo.path_prefix, objName, localFile);
}

bool BackupS3IO::QueryFileInfo(const string &remoteFile, size_t &dataSize, size_t &offset)
{
    auto it = m_fileCache.find(remoteFile);
    if (it == m_fileCache.end()) {
        uintmax_t fileSize = 0;
        if (!m_snapGf->FileSize(fileSize, remoteFile.c_str())) {
            HCP_Log(ERR, MODULE) << "Get the bucker file size failed." << HCPENDLOG;
            return false;
        }
        FileInfo info;
        info.dataSize = fileSize;
        info.offset = 0;
        it = m_fileCache.insert(make_pair(remoteFile, info)).first;
    }
    dataSize = it->second.dataSize;
    offset = it->second.offset;
    return true;
}

bool BackupS3IO::UpdateFileInfo(const string &remoteFile, const size_t &offset)
{
    auto it = m_fileCache.find(remoteFile);
    if (it == m_fileCache.end()) {
        HCP_Log(ERR, MODULE) << "FileCache has no file info." << HCPENDLOG;
        return false;
    }
    FileInfo info;
    info.dataSize = it->second.dataSize;
    info.offset = it->second.offset + offset;
    m_fileCache[remoteFile] = info;
    return true;
}

void BackupS3IO::DeleteFileInfo(const string &remoteFile)
{
    auto it = m_fileCache.find(remoteFile);
    if (it == m_fileCache.end()) {
        HCP_Log(WARN, MODULE) << "FileCache has no file info." << HCPENDLOG;
        return;
    }
    m_fileCache.erase(it);
}

int32_t BackupS3IO::ReadFile(const string &remoteFile, BinaryData &data)
{
    if (data.data == nullptr) {
        HCP_Log(ERR, MODULE) << "Buffer is nullptr." << HCPENDLOG;
        return READ_S3_DATA_FAILED;
    }

    string s3Path;
    FormatFileName(s3Path, remoteFile);
    size_t dataSize = 0;
    size_t offset = 0;
    if (!QueryFileInfo(s3Path, dataSize, offset)) {
        HCP_Log(ERR, MODULE) << "Query the bucker file info failed." << HCPENDLOG;
        return READ_S3_DATA_FAILED;
    }

    size_t readSize = 0;
    if (dataSize <= offset) {
        HCP_Log(DEBUG, MODULE) << "File size: " << dataSize << " is less than offset: " << offset << HCPENDLOG;
        DeleteFileInfo(s3Path);
        return readSize;
    }

    if (m_s3IO == nullptr) {
        m_s3IO = make_unique<S3SystemIO>(m_deviceInfo, OBJECT_NO_CACHE_DATA);
        if (m_s3IO == nullptr) {
            HCP_Log(ERR, MODULE) << "Create S3SystemIO instance failed." << HCPENDLOG;
            DeleteFileInfo(s3Path);
            return READ_S3_DATA_FAILED;
        }
    }
    if (!m_s3IO->ReadNoCache(s3Path.c_str(), "", offset, reinterpret_cast<char *>(data.data.get()),
                             data.length, readSize)) {
        HCP_Log(ERR, MODULE) << "Read the bucker file failed." << HCPENDLOG;
        DeleteFileInfo(s3Path);
        return READ_S3_DATA_FAILED;
    }
    HCP_Log(DEBUG, MODULE) << "Data[" << readSize << "] is read from the offset[" << offset << "] position of the file."
                           << HCPENDLOG;
    if (readSize < data.length) {
        HCP_Log(DEBUG, MODULE) << "The file is read completed, " << offset + readSize << " bytes has been read."
                               << HCPENDLOG;
        DeleteFileInfo(s3Path);
    } else {
        if (!UpdateFileInfo(s3Path, readSize)) {
            DeleteFileInfo(s3Path);
            HCP_Log(ERR, MODULE) << "Update file info failed." << HCPENDLOG;
            return READ_S3_DATA_FAILED;
        }
    }
    return readSize;
}

int32_t BackupS3IO::ReadFile(const string &remoteFile, uint64_t offset, BinaryData &data)
{
    if (data.data == nullptr) {
        HCP_Log(ERR, MODULE) << "Buffer is nullptr." << HCPENDLOG;
        return READ_S3_DATA_FAILED;
    }

    if (m_s3IO == nullptr) {
        m_s3IO = make_unique<S3SystemIO>(m_deviceInfo, OBJECT_NO_CACHE_DATA);
        if (m_s3IO == nullptr) {
            HCP_Log(ERR, MODULE) << "Create S3SystemIO instance failed." << HCPENDLOG;
            return READ_S3_DATA_FAILED;
        }
    }
    size_t readSize = 0;
    size_t dataSize = 0;
    string s3Path;
    FormatFileName(s3Path, remoteFile);
    size_t tempOffset = 0;
    if (!QueryFileInfo(s3Path, dataSize, tempOffset)) {
        HCP_Log(ERR, MODULE) << "Query the file info failed." << HCPENDLOG;
        return READ_S3_DATA_FAILED;
    }
    if (dataSize <= offset) {
        HCP_Log(DEBUG, MODULE) << "File size: " << dataSize << " is less than offset: " << offset << HCPENDLOG;
        DeleteFileInfo(s3Path);
        return readSize;
    }

    if (!m_s3IO->ReadNoCache(s3Path.c_str(), "", offset, reinterpret_cast<char *>(data.data.get()),
                             data.length, readSize)) {
        HCP_Log(ERR, MODULE) << "Read the bucker file failed." << HCPENDLOG;
        DeleteFileInfo(s3Path);
        return READ_S3_DATA_FAILED;
    }
    if (readSize < data.length) {
        HCP_Log(INFO, MODULE) << "The file is read completed, read: " << readSize << ", expect: " << data.length
                               << HCPENDLOG;
        DeleteFileInfo(s3Path);
    }
    HCP_Log(DEBUG, MODULE) << "Data[" << readSize << "] is read from the offset[" << offset << "] position of the file."
                           << HCPENDLOG;
    return readSize;
}

bool BackupS3IO::DownloadFile(const string &remoteFile, const string &localFile, ReadFileCallback &handle)
{
    string s3workPath = GetWorkSpace();
    string objName = s3workPath.empty() ? remoteFile : (s3workPath + "/" + remoteFile);
    m_snapGf->RegisterReadFileCallbackFun(m_deviceInfo.path_prefix, handle);
    return m_snapGf->DownloadFile(m_deviceInfo.path_prefix, objName, localFile);
}

bool BackupS3IO::WriteFile(const string &remoteFile, const string &buffer)
{
    string s3Path;
    FormatFileName(s3Path, remoteFile);
    if (!m_snapGf->Open(s3Path.c_str(), "w", nullptr)) {
        HCP_Log(ERR, MODULE) << "Open the bucker file failed." << HCPENDLOG;
        return false;
    }
    if (!m_snapGf->Write(buffer.c_str(), 1, buffer.size())) {
        HCP_Log(ERR, MODULE) << "Write str to bucker failed." << HCPENDLOG;
        return false;
    }
    if (!m_snapGf->Close()) {
        HCP_Log(ERR, MODULE) << "close the bucker file failed." << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, MODULE) << "Write str to bucket success." << HCPENDLOG;
    return true;
}

bool BackupS3IO::WriteFile(const BinaryData &packageData, const string &remoteFile, CallBackHandle &handle)
{
    string s3Path;
    FormatFileName(s3Path, remoteFile);
    m_snapGf->RegisterCallbackHandle(m_deviceInfo.path_prefix, handle);
    if (!m_snapGf->Open(s3Path.c_str(), "w", nullptr)) {
        HCP_Log(ERR, MODULE) << "Open the bucker file failed." << HCPENDLOG;
        return false;
    }
    m_snapGf->SetUpLoadRateLimit(m_UpLoadRateLimit / QOS_LIMIT);
    if (!m_snapGf->Write(packageData.data.get(), 1, packageData.length)) {
        HCP_Log(ERR, MODULE) << "Write packageData to bucker failed." << HCPENDLOG;
        return false;
    }
    if (handle.callBackFunc) {
        ((WriteFileCallback)handle.callBackFunc)(
            LayoutRetCode::SUCCESS, packageData.length, packageData.length, handle.callBackData);
    }
    if (!m_snapGf->Close()) {
        HCP_Log(ERR, MODULE) << "close the bucker file failed." << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, MODULE) << "Write packageData to bucket success." << HCPENDLOG;
    return true;
}

bool BackupS3IO::WriteFile(const string &localFile, const string &remoteFile, int threadCount,
    CallBackHandle &handle)
{
    uint64_t contentLen = 0;
    if (!GetFileContentLen(localFile, contentLen)) {
        HCP_Log(ERR, MODULE) << "Get file content length failed." << HCPENDLOG;
        return false;
    }
    if (contentLen > UPLOAD_FILE_SIZE_MAX) {
        HCP_Log(ERR, MODULE) << "This oversized file[len:" << contentLen << "] cannot be backed up to the cloud."
                             << HCPENDLOG;
        return false;
    }
    string s3workPath = GetWorkSpace();
    string objFilePath = s3workPath.empty() ? remoteFile : (s3workPath + "/" + remoteFile);
    if (contentLen > FILE_LEN_THRESHOLD) {
        m_upload = make_unique<UploadFile>(m_deviceInfo);
        if (m_upload == nullptr) {
            HCP_Log(ERR, MODULE) << "Create UploadFile instance failed." << HCPENDLOG;
            return false;
        }
        m_upload->SetUpLoadRateLimit(m_UpLoadRateLimit / QOS_LIMIT);
        if (!m_upload->ConcurrentUploadPart(objFilePath, localFile, threadCount, handle)) {
            HCP_Log(ERR, MODULE) << "Write file to bucket failed." << HCPENDLOG;
            return false;
        }
    } else {
        m_snapGf->RegisterCallbackHandle(m_deviceInfo.path_prefix, handle);
        m_snapGf->SetUpLoadRateLimit(m_UpLoadRateLimit);
        if (!m_snapGf->NormalFileUpload(m_deviceInfo.path_prefix, localFile, objFilePath)) {
            HCP_Log(ERR, MODULE) << "Write file to bucket failed." << HCPENDLOG;
            return false;
        }
    }
    HCP_Log(DEBUG, MODULE) << "Write file to bucket success." << HCPENDLOG;
    return true;
}

bool BackupS3IO::Delete(const string &remoteFile)
{
    string s3Path;
    FormatFileName(s3Path, remoteFile);
    return m_snapGf->Remove(s3Path.c_str());
}

bool BackupS3IO::DeleteAll(const string &directoryName, CallBackHandle &handle)
{
    string s3Path;
    FormatFileName(s3Path, directoryName);
    m_snapGf->RegisterCallbackHandle(m_deviceInfo.path_prefix, handle);
    return m_snapGf->RemoveAll(s3Path.c_str());
}

void BackupS3IO::SetUpLoadRateLimit(uint64_t qos)
{
    HCP_Log(DEBUG, MODULE) << "UpLoadRateLimit : " << qos << HCPENDLOG;
    m_UpLoadRateLimit = qos;
}
void BackupS3IO::SetDownLoadRateLimit(uint64_t qos)
{
    HCP_Log(DEBUG, MODULE) << "DownLoadRateLimit : " << qos << HCPENDLOG;
    m_DownLoadRateLimit = qos;
}