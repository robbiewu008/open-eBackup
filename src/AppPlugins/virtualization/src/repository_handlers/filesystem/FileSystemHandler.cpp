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
#include "FileSystemHandler.h"
#include <cstdio>
#include <log/Log.h>
#include <boost/filesystem.hpp>
#include "securec.h"
#include "common/utils/Utils.h"
#include "common/utils/RetryOper.h"

namespace {
    const std::string MODULE_NAME = "FileSystemHandler";
    const int FILE_D_TYPE = 8;
}

namespace VirtPlugin {
int32_t FileSystemHandler::Open(const std::string &fileName, const std::string &mode)
{
    Utils::RetryOper<std::FILE*> retryOper;
    retryOper.SetOperName("fopen");
    retryOper.SetFailedChecker([](std::FILE *retV) { return retV == nullptr; });
    retryOper.SetOperator(std::bind(fopen, fileName.c_str(), mode.c_str()));
    retryOper.AddBlackList(ENOENT);
    auto [fp, errorNumber] = retryOper.Invoke();
    if (fp == std::nullopt || fp == nullptr) {
        ERRLOG("Open file %s failed. errno[%d]:[%s].", fileName.c_str(), errorNumber, strerror(errorNumber));
        return errorNumber;
    }
    m_fp = fp.value();
    m_mode = mode;
    m_fileName = fileName;
    return SUCCESS;
}

int32_t FileSystemHandler::Truncate(const uint64_t &size)
{
    if (m_fp == nullptr) {
        ERRLOG("File fd is null.");
        return FAILED;
    }

    Utils::RetryOper<int> retryOper;
    retryOper.SetOperName("ftruncate64");
    retryOper.SetFailedChecker([](int retV) { return retV != 0; });
    retryOper.SetOperator(std::bind(ftruncate64, fileno(m_fp), size));
    retryOper.AddBlackList(ENOENT);
    auto [ret, errorNumber] = retryOper.Invoke();
    if (ret == std::nullopt || ret != 0) {
        ERRLOG("Failed to truncate file, size:%llu, errno[%d]:[%s].", size, errorNumber, strerror(errorNumber));
        return errorNumber;
    }
    return SUCCESS;
}

int32_t FileSystemHandler::Close()
{
    if (m_fp != nullptr) {
        if (!Flush()) {
            ERRLOG("Flush failed.");
            return FAILED;
        }

        int retV = fclose(m_fp);
        if (retV != 0) {
            ERRLOG("Close file %s failed. errno[%d]:[%s].", m_fileName.c_str(), errno, strerror(errno));
            return FAILED;
        }
    }

    m_fp = nullptr;
    return SUCCESS;
}

size_t FileSystemHandler::Read(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    if (m_fp == nullptr) {
        ERRLOG("file fd is null.");
        return 0;
    }

    size_t rc = fread(buf.get(), 1, count, m_fp);
    if (rc != count) {
        ERRLOG("Read failed, rc=[%zu], count=[%zu], errno[%d]:[%s]", rc, count, errno, strerror(errno));
    }
    return rc;
}

size_t FileSystemHandler::Read(std::string &buf, size_t count)
{
    if (m_fp == nullptr) {
        ERRLOG("File fd is null.");
        return 0;
    }
    std::unique_ptr<char[]> tmpbuf = std::make_unique<char[]>(count + 1);
    if (tmpbuf.get() == nullptr) {
        ERRLOG("Allocate with new returns NULL.");
        return 0;
    }

    memset_s(tmpbuf.get(), count + 1, 0, count + 1);
    size_t rc = fread(tmpbuf.get(), 1, count, m_fp);
    if (rc != count) {
        DBGLOG("Read failed, rc=[%zu], count=[%zu], errno[%d]:[%s]", rc, count, errno, strerror(errno));
        return rc;
    }
    buf = std::string(tmpbuf.get(), count);
    return rc;
}

size_t FileSystemHandler::Write(const std::shared_ptr<uint8_t[]> &buf, size_t count)
{
    size_t rc = (m_fp == nullptr) ? 0 : fwrite(buf.get(), 1, count, m_fp);
    return rc;
}

size_t FileSystemHandler::Write(const std::string &str)
{
    size_t rc = (m_fp == nullptr) ? 0 : fwrite(str.c_str(), 1, str.length(), m_fp);
    return rc;
}

size_t FileSystemHandler::Append(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    if (m_fp == nullptr) {
        ERRLOG("File fd is null.");
        return 0;
    }

    if (m_mode != "a" && m_mode != "a+") {
        ERRLOG("Open mode is incorrect. mode=[%s].", m_mode.c_str());
        return 0;
    }

    return fwrite(buf.get(), 1, count, m_fp);
}

int64_t FileSystemHandler::Tell()
{
    if (m_fp == nullptr) {
        return -1;
    }
    Utils::RetryOper<int> retryOper;
    retryOper.SetOperName("ftell");
    retryOper.SetFailedChecker(nullptr);
    retryOper.SetOperator(std::bind(ftell, m_fp));
    retryOper.AddBlackList(ENOENT);
    auto [ret, errorNumber] = retryOper.Invoke();
    if (ret == std::nullopt) {
        return -1;
    }
    return ret.value();
}

int64_t FileSystemHandler::Seek(size_t offset, int origin)
{
    if (m_fp == nullptr) {
        return -1;
    }
    Utils::RetryOper<int> retryOper;
    retryOper.SetOperName("fseek");
    retryOper.SetFailedChecker(nullptr);
    retryOper.SetOperator(std::bind(fseek, m_fp, offset, origin));
    retryOper.AddBlackList(ENOENT);
    auto [ret, errorNumber] = retryOper.Invoke();
    if (ret == std::nullopt) {
        return -1;
    }
    return ret.value();
}

size_t FileSystemHandler::FileSize(const std::string &fileName)
{
    struct stat statInfo = {0};
    int32_t errNo = -1;
    if (Stat(fileName, statInfo, errNo) != SUCCESS) {
        ERRLOG("stat file. filename=[%s].", fileName.c_str());
        return 0;
    }
    return statInfo.st_size;
}

bool FileSystemHandler::Flush(bool sync)
{
    if (m_fp == nullptr) {
        ERRLOG("File fd is null.");
        return false;
    }

    Utils::RetryOper<int> retryOper;
    retryOper.SetOperName("fflush");
    retryOper.SetFailedChecker([](int retV) { return retV != 0; });
    retryOper.SetOperator(std::bind(fflush, m_fp));
    retryOper.AddBlackList(ENOENT);
    auto [ret, errorNumber] = retryOper.Invoke();
    if (ret == std::nullopt || ret != 0) {
        ERRLOG("fflush failed: errno[%d]:[%s]. filename=[%s].",
            errorNumber, strerror(errorNumber), m_fileName.c_str());
        return false;
    }

    if (sync) {
        Utils::RetryOper<int> retryOperfsync;
        retryOperfsync.SetOperName("fsync");
        retryOperfsync.SetFailedChecker([](int retV) { return retV != 0; });
        retryOperfsync.SetOperator(std::bind(fsync, fileno(m_fp)));
        retryOper.AddBlackList(ENOENT);
        auto [ret, errorNumber] = retryOperfsync.Invoke();
        if (ret == std::nullopt || ret != 0) {
            ERRLOG("fsync failed: errno[%d]:[%s]. filename=[%s].",
                errorNumber, strerror(errorNumber), m_fileName.c_str());
            return false;
        }
    }

    return true;
}

bool FileSystemHandler::Exists(const std::string &fileName)
{
    struct stat pathStat = {0};
    int32_t errNo = -1;
    if (Stat(fileName, pathStat, errNo) != SUCCESS) {
        INFOLOG("File/dir(%s) does not exist.", fileName.c_str());
        return false;
    }
    DBGLOG("File/dir(%s) exist.", fileName.c_str());
    return true;
}

bool FileSystemHandler::Rename(const std::string &oldName, const std::string &newName)
{
    if (oldName.empty() || newName.empty()) {
        ERRLOG("param invalid.");
        return false;
    }

    if (!Exists(oldName)) {
        ERRLOG("The old file or path: %s not exists.", oldName.c_str());
        return false;
    }

    Utils::RetryOper<int> retryOper;
    retryOper.SetOperName("rename");
    retryOper.SetFailedChecker([](int retV) { return retV != 0; });
    retryOper.SetOperator(std::bind(rename, oldName.c_str(), newName.c_str()));
    retryOper.AddBlackList(ENOENT);
    auto [ret, errorNumber] = retryOper.Invoke();
    if (ret == std::nullopt || ret != 0) {
        ERRLOG("rename failed: errno[%d]:[%s]", errorNumber, strerror(errorNumber));
        return false;
    }

    return true;
}

bool FileSystemHandler::CopyFile(const std::string &srcName, const std::string &destName)
{
    if (srcName.empty() || destName.empty()) {
        ERRLOG("param invalid");
        return false;
    }

    if (!Exists(srcName)) {
        ERRLOG("The old file or path: %s not exists.", srcName.c_str());
        return false;
    }

    try {
        boost::filesystem::path fromPath(srcName);
        boost::filesystem::path toPath(destName);
        if (boost::filesystem::is_directory(destName)) {
            std::string fileName = fromPath.filename().string();
            size_t pos = fileName.find_last_of("/");
            if (pos != std::string::npos) {
                fileName = fileName.substr(pos + 1);
            }
            toPath = toPath / fileName;
        }
        boost::filesystem::copy_file(fromPath, toPath, boost::filesystem::copy_option::overwrite_if_exists);
        DBGLOG("Succeeded in copying file from path: %s, to path: %s", srcName.c_str(), destName.c_str());
    } catch (const boost::filesystem::filesystem_error &e) {
        if (e.code() == boost::system::errc::permission_denied) {
            ERRLOG("Copy file failed: permission_denied. From: %s, to: %s, with error: %s", srcName.c_str(),
                destName.c_str(), WIPE_SENSITIVE(e.code().message()).c_str());
        } else {
            ERRLOG("Copy file failed. From: %s, to: %s, with error: %s", srcName.c_str(),
                destName.c_str(), WIPE_SENSITIVE(e.code().message()).c_str());
        }
        return false;
    }

    return true;
}

bool FileSystemHandler::IsDirectory(const std::string& path)
{
    struct stat pathStat = {0};
    int32_t errNo = -1;
    if (Stat(path, pathStat, errNo) != SUCCESS) {
        return false;
    }
    return S_ISDIR(pathStat.st_mode);
}

bool FileSystemHandler::IsRegularFile(const std::string& path)
{
    struct stat pathStat = {0};
    int32_t errNo = -1;
    if (Stat(path, pathStat, errNo) != SUCCESS) {
        return false;
    }
    return S_ISREG(pathStat.st_mode);
}

bool FileSystemHandler::Remove(const std::string &fileName)
{
    if (!Exists(fileName)) {
        DBGLOG("removing file, but not exists. filename=[%s]", fileName.c_str());
        return true;
    }

    Utils::RetryOper<int> retryOper;
    retryOper.SetOperName("remove");
    retryOper.SetFailedChecker([](int retV) { return retV != 0; });
    retryOper.SetOperator(std::bind(remove, fileName.c_str()));
    retryOper.AddBlackList(ENOENT);
    auto [ret, errorNumber] = retryOper.Invoke();
    if (ret == std::nullopt || ret != 0) {
        ERRLOG("Remove failed: errno[%d]:[%s]", errorNumber, strerror(errorNumber));
        return false;
    }

    return true;
}

bool FileSystemHandler::RemoveAll(const std::string &dirName)
{
    bool success = RemoveAllInner(dirName, ::remove);
    if (!success) {
        return false;
    }
    return Remove(dirName.c_str());
}

bool FileSystemHandler::CreateDirectory(const std::string &dirName)
{
    bool accessible = false;
    auto v = Tokenize(dirName, '/');
    if (!v.size()) {
        return false;
    }

    std::string dir;
    for (auto& t : v) {
        dir += "/" + t;

        struct stat statbuf = {0};
        int err = stat(dir.c_str(), &statbuf);
        if (err < 0) {
            if (!HandleStateError(errno, dir)) {
                /* error logged inside */
                return false;
            }
        }
        if (access(dir.c_str(), W_OK) != 0) {
            if (errno == EACCES) {
                DBGLOG("no permissions on directory, dir=[%s]", dir.c_str());
                // we handle a directory chain that has a non accessible component
                accessible = false;
                continue;
            }
            DBGLOG("directory %s is not accessible. errno[%d]:[%s]", dir.c_str(), errno, strerror(errno));
            return false;
        }
        accessible = true;  // we arrive if directory is accessible
    }

    return accessible;
}

void FileSystemHandler::GetFiles(std::string pathName, std::vector <std::string> &files)
{
    DIR *dir;
    struct dirent *ptr;
    if ((dir = opendir(pathName.c_str())) == NULL) {
        ERRLOG("Open dir error: errno[%d]:[%s]", errno, strerror(errno));
        return;
    }

    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == SUCCESS ||
            strcmp(ptr->d_name, "..") == SUCCESS) {    // current dir OR parrent dir
            continue;
        } else if (ptr->d_type == FILE_D_TYPE) {  // file
            std::string strFile = pathName;
            strFile += "/";
            strFile += ptr->d_name;
            files.push_back(strFile);
        } else {
            continue;
        }
    }
    closedir(dir);
}


bool FileSystemHandler::HandleStateError(int errNo, const std::string &dir)
{
    if (errNo == ENOTDIR) {
        WARNLOG("path includes a file. dir=[%s], errno[%d]:[%s]", dir.c_str(), errNo, strerror(errNo));
        return false;
    }
    if (errNo == ENOENT) {
        DBGLOG("create directory, dir=[%s]", dir.c_str());
        Utils::RetryOper<int> retryOper;
        retryOper.SetOperName("mkdir");
        retryOper.SetFailedChecker([](int retV) { return retV != 0; });
        retryOper.SetOperator(std::bind(mkdir, dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO));
        retryOper.AddBlackList(ENOENT);
        auto [ret, errNumber] = retryOper.Invoke();
        if (ret == std::nullopt || ret != 0) {
            ERRLOG("create directory(%s) failed, errno[%d]:[%s]", dir.c_str(), errNumber, strerror(errNumber));
            return false;
        }
    }
    return true;
}

std::vector<std::string> FileSystemHandler::Tokenize(const std::string& str, char delimiter)
{
    std::stringstream s(str);
    std::string token;
    std::vector<std::string> result;

    while (std::getline(s, token, delimiter)) {
        if (token.size()) {
            result.push_back(token);
        }
    }
    return result;
}

static int FilterLocalAndParent(const struct dirent* entry)
{
    std::string tmp(entry->d_name);
    if (!tmp.compare(".")) {
        return 0;
    }
    if (!tmp.compare("..")) {
        return 0;
    }

    return 1;
}

bool FileSystemHandler::RemoveAllInner(const std::string& path, std::function<int(const char*)> callback)
{
    struct dirent** namelist;

    int n = scandir(path.c_str(), &namelist, FilterLocalAndParent, nullptr);
    if (n == -1) {
        ERRLOG("scandir() failed, errno[%d]:[%s]", errno, strerror(errno));
        return false;
    }
    DBGLOG("scandir() returned=[%d], path=[%s]", n, path.c_str());

    while (n--) {
        std::string fileName = path + "/" + namelist[n]->d_name;
        if (IsDirectory(fileName)) {
            if (!RemoveAllInner(fileName, ::remove)) {
                ERRLOG("remove(%s) failed, errno[%d]:[%s]", fileName.c_str(), errno, strerror(errno));
            }
        }
        int err = callback(fileName.c_str());
        if (err < 0) {
            ERRLOG("remove(%s) failed, errno[%d]:[%s]", fileName.c_str(), errno, strerror(errno));
        }
        free(namelist[n]);
    }
    free(namelist);

    return true;
}

bool FileSystemHandler::Stat(const std::string &target, struct stat &statInfo, int32_t &errNo)
{
    memset_s(&statInfo, sizeof (struct stat), 0, sizeof (struct stat));
    Utils::RetryOper<int> retryOper;
    retryOper.SetOperName("stat");
    retryOper.SetFailedChecker([](int retV) { return retV < 0; });
    retryOper.SetOperator(std::bind(stat, target.c_str(), &statInfo));
    retryOper.AddBlackList(ENOENT);
    auto [ret, errNum] = retryOper.Invoke();
    errNo = errNum;
    if (ret == std::nullopt || ret < 0) {
        WARNLOG("stat failed, file=[%s] errno[%d]: %s", target.c_str(), errNum, strerror(errNum));
        return FAILED;
    }
    DBGLOG("State success, file=[%s]", target.c_str());
    return SUCCESS;
}
}