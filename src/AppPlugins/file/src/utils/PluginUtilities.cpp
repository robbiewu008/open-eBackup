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
#include "PluginUtilities.h"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include "common/Thread.h"
#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/fs.h>
#endif
#include "system/System.hpp"
#include "common/CTime.h"
#include "common/Path.h"
#include "common/EnvVarManager.h"
#include "PluginConstants.h"
#include "Defines.h"
#include "common/FileSystemUtil.h"

#ifdef WIN32
#include <windows.h>
#include <winioctl.h>
#include "win32/BCD.h"
#include "win32/Registry.h"
#include "FileSystemUtil.h"
#endif

using namespace std;
using namespace Module;

namespace {
constexpr int TIME_STR_LEN = 80;
constexpr auto MODULE = "Utilities";

constexpr uint8_t NUMBER0 = 0;
constexpr uint8_t NUMBER1 = 1;
constexpr uint8_t NUMBER2 = 2;
constexpr uint8_t NUMBER3 = 3;
constexpr uint8_t NUMBER5 = 5;
constexpr uint32_t NUMBER4 = 4;
constexpr int32_t NUMBER6 = 6;
constexpr uint32_t NUMBER8 = 8;
constexpr uint32_t NUMBER64 = 64;
constexpr uint32_t NUMBER256 = 256;
constexpr uint16_t MP_FAILED = Module::FAILED;
constexpr uint16_t MP_SUCCESS = Module::SUCCESS;
const std::string DOUBLE_BACKSLASH = "\\\\";
const std::string DOUBLE_SLASH = "//";
const std::string SLASH = "/";
const std::string BACKSLASH = "\\";
const std::wstring UNC_PATH_PREFIX = LR"(\\?\)";
const std::string PATH_PREFIX = R"(\\?\)";
const std::string UEFI_BOOT = "UEFI";
const std::string BIOS_BOOT = "BIOS";
const std::string UNKNOW_BOOT = "UNKNOW";
const std::string SNAPSHOT_DIRNAME = ".snapshot";

}

namespace PluginUtils {
#ifdef WIN32
using namespace Win32;
#endif
// 输入是ip列表，以逗号分割。检查通过管理网络访问8088端口是否连通
bool CheckDeviceNetworkConnect(const std::string &managerIps)
{
    vector<string> output;
    vector<string> errOutput;
    vector<string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(managerIps);
#ifdef _AIX
    string cmd = "?/bin/checkIp.sh '?'";
#else
    string cmd = "sudo ?/bin/checkIp.sh '?'";
#endif
#ifndef WIN32
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, output, errOutput);
#else
    int ret = 1;
    ERRLOG("WIN32 NOT IMPLMENTED");
#endif
    if (ret != MP_SUCCESS) {
        string msg;
        for (auto &it : output) {
            msg += it + " ";
        }
        string errmsg;
        for (const auto &it : errOutput) {
            errmsg += it + " ";
        }
        HCP_Log(DEBUG, MODULE) << "run shell ret: " << ret << HCPENDLOG;
        HCP_Log(DEBUG, MODULE) << "run shell msg: " << WIPE_SENSITIVE(msg) << HCPENDLOG;
        HCP_Log(DEBUG, MODULE) << "run shell errmsg: " << WIPE_SENSITIVE(errmsg) << HCPENDLOG;
        HCP_Log(ERR, MODULE) << "Failed to visit all manager network." << HCPENDLOG;
        return false;
    }
    return true;
}

std::string FormatTimeToStrBySetting(time_t timeInSeconds, const std::string& timeFormat)
{
#ifndef WIN32
    struct tm *tmp = localtime(&timeInSeconds);
    char time[TIME_STR_LEN];
    strftime(time, sizeof(time), timeFormat.c_str(), tmp);
    string timeStr(time);
    return timeStr;
#else
    return "";
#endif
}

std::string FormatTimeToStr(time_t timeInSeconds)
{
    return FormatTimeToStrBySetting(timeInSeconds, "%Y-%m-%d-%H:%M:%S");
}

void LogCmdExecuteError(int retCode, const std::vector<std::string> &output, const std::vector<std::string> &errOutput)
{
    string stdoutMsg;
    for (auto &it : output) {
        stdoutMsg += it + " ";
    }
    string stderrMsg;
    for (const auto &it : errOutput) {
        stderrMsg += it + " ";
    }
    HCP_Log(DEBUG, MODULE) << "run shell failed with code:" << retCode
        << ", stdout: " << WIPE_SENSITIVE(stdoutMsg)
        << ", stderr: " << WIPE_SENSITIVE(stderrMsg) << HCPENDLOG;
}

int RunShellCmd(const std::string& cmd, const vector<string>& paramList)
{
    vector<string> output;
    vector<string> erroutput;
    if (Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, output, erroutput) != MP_SUCCESS) {
        string msg;
        string errmsg;
        for (auto &it : output) {
            msg += it + " ";
        }
        ERRLOG("Run shell msg: %s", Module::WipeSensitiveDataForLog(msg).c_str());
        for (auto &it : erroutput) {
            errmsg += it + " ";
        }
        ERRLOG("Run shell error msg: %s", Module::WipeSensitiveDataForLog(errmsg).c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int RunShellCmd(const std::string& cmd, const vector<string>& paramList, std::vector<std::string>& output)
{
    vector<string> erroutput;
    if (Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, output, erroutput) != MP_SUCCESS) {
        string msg;
        for (auto &it : output) {
            msg += it + " ";
        }
        ERRLOG("Run shell command error message: %s", Module::WipeSensitiveDataForLog(msg).c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

std::string Base64Encode(const std::string &in)
{
    string out;
    // Characters involved in base64 encoding and decoding
    const string base64Array = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned int val = NUMBER0;
    int valb = -NUMBER6;
    for (unsigned char c : in) {
        val = (val << NUMBER8) + c;
        valb += NUMBER8;
        while (valb >= NUMBER0) {
            out.push_back(base64Array[(val >> valb) & 0x3F]);
            valb -= NUMBER6;
        }
    }
    if (valb > -NUMBER6) {
        out.push_back(base64Array[((val << NUMBER8) >> (valb + NUMBER8)) & 0x3F]);
    }
    while (out.size() % NUMBER4 != 0) {
        out.push_back('=');
    }
    return out;
}

std::string Base64Decode(const std::string &in)
{
    string out;
    vector<int> T(NUMBER256, -1);
    // Characters involved in base64 encoding and decoding
    const string base64Array = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for (uint32_t i = 0; i < NUMBER64; i++) {
        T[base64Array[i]] = i;
    }
    int val = NUMBER0;
    int valb = -NUMBER8;
    for (unsigned char c : in) {
        if (T[c] == -1) {
            break;
        }
        val = (val << NUMBER6) + T[c];
        valb += NUMBER6;
        if (valb >= NUMBER0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= NUMBER8;
        }
    }
    return out;
}

size_t GenerateHash(std::string jobId)
{
    boost::hash<string> stringHash;
    return stringHash(jobId);
}

time_t GetCurrentTimeInSeconds()
{
    time_t currTime;
    Module::CTime::Now(currTime);
    return currTime;
}

std::string ConvertToReadableTime(time_t time)
{
#ifndef WIN32
    const uint32_t timeBufferSize = 80;
    char buffer[timeBufferSize];
    struct tm timeinfo {};
    localtime_r(&time, &timeinfo);
    strftime(buffer, sizeof(buffer), "%d%m%Y%H%M%S", &timeinfo);
    string timeStr(buffer);
    return timeStr;
#else
    return "None";
    ERRLOG("WIN32 NOT IMPLMENTED");
#endif
}

time_t GetCurrentTimeInSeconds(std::string &dateAndTimeString)
{
    time_t currTime;
    Module::CTime::Now(currTime);
    dateAndTimeString = ConvertToReadableTime(currTime);
    return currTime;
}

bool WriteFile(const std::string &path, const std::string &data)
{
    INFOLOG("start_saveStatisticInfo");
    std::ofstream file(path);
    if (!file.is_open()) {
        ERRLOG("open file failed,path:%s", path.c_str());
        return false;
    }
    file << data;
    if (file.fail()) {
        ERRLOG("Failed to write path:%s", path.c_str());
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool ReadFile(const std::string &path, std::string &data)
{
    if (!IsFileExist(path)) {
        return false;
    }
    std::ifstream infoFile(path, std::ios::in);
    std::stringstream fileBuffer{};
    if (!infoFile.is_open()) {
#ifdef WIN32
        DWORD errcode = ::GetLastError();
        ERRLOG("Open file %s failed, errno[%lu]:%s.", path.c_str(), errcode, strerror(errcode));
#else
        ERRLOG("Open file %s failed, errno[%d]:%s.", path.c_str(), errno, strerror(errno));
#endif
        return false;
    }
    fileBuffer << infoFile.rdbuf();
    infoFile.close();
    data = fileBuffer.str();
    return true;
}

bool CreateDirectory(const std::string& path)
{
    INFOLOG("Enter Create CreateDirectory: %s", path.c_str());
    if (path.empty()) {
        return true;
    }
    std::string dirPath = path;
#ifdef WIN32
    dirPath = ReverseSlash(path);
#endif
    if (IsDirExist(dirPath)) {
        INFOLOG("CreateDirectory success, dir exist: %s", path.c_str());
        return true;
    }
    std::string parentDir = GetPathName(dirPath);
    if (!CreateDirectory(parentDir)) {
        return false;
    }
#ifndef WIN32
    int res = mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    if (res != Module::SUCCESS && errno != EEXIST) {
        ERRLOG("CreateDirectory fail for: %s, errno: %u", path.c_str(), errno);
        return false;
    }
    return true;
#else
    INFOLOG("call create_directory, %s", dirPath.c_str());
    std::wstring wDirPath = Module::FileSystemUtil::Utf8ToUtf16(dirPath);
    if (wDirPath.find(UNC_PATH_PREFIX) != 0) { // check UNC prefix
        wDirPath = UNC_PATH_PREFIX + wDirPath;
    }
    if (!::CreateDirectoryW(wDirPath.c_str(), nullptr)) {
        DWORD errorCode = ::GetLastError();
        if (errorCode == ERROR_ALREADY_EXISTS) {
            return true;
        }
        ERRLOG("dir %s create failed, errno %d", dirPath.c_str(), errorCode);
        return false;
    }
    return true;
#endif
}

bool SafeCreateDirectory(const std::string& path, const std::string& basePath)
{
    if (path.empty()) {
        return true;
    }

    if (IsDirExist(path)) {
        DBGLOG("CreateDirectory success, dir exist: %s", path.c_str());
        return true;
    }

    if (basePath == path) {
        WARNLOG("CreateDirectory failed for base path not exist! %s, %s", path.c_str(), basePath.c_str());
        return false;
    }
    std::string parentDir = GetPathName(path);
    if (!SafeCreateDirectory(parentDir, basePath)) {
        WARNLOG("Create dir failed! %s, %s", path.c_str(), basePath.c_str());
        return false;
    }

    return DoRealCreateDirectory(path);
}

bool DoRealCreateDirectory(const std::string& path)
{
    int retryTimes = 0;
    while (retryTimes < NUMBER3) {
#ifndef WIN32
        INFOLOG("call create_directory, %s", path.c_str());
        int res = mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
        if (res != Module::SUCCESS && errno != EEXIST) {
            ERRLOG("CreateDirectory fail for: %s, errno: %u, retryTimes: %d", path.c_str(), errno, retryTimes);
            retryTimes++;
            Module::SleepFor(std::chrono::seconds(NUMBER5));
            continue;
        }
        return true;
#else
        std::string dirPath = ReverseSlash(path);
        INFOLOG("call create_directory, %s", dirPath.c_str());
        std::wstring wDirPath = Module::FileSystemUtil::Utf8ToUtf16(dirPath);
        if (wDirPath.find(UNC_PATH_PREFIX) != 0) { // check UNC prefix
            wDirPath = UNC_PATH_PREFIX + wDirPath;
        }
        if (!::CreateDirectoryW(wDirPath.c_str(), nullptr)) {
            DWORD errorCode = ::GetLastError();
            if (errorCode == ERROR_ALREADY_EXISTS) {
                return true;
            }
            ERRLOG("dir %s create failed, errno %d, retryTimes: %d", dirPath.c_str(), errorCode, retryTimes);
            retryTimes++;
            Module::SleepFor(std::chrono::seconds(NUMBER5));
            continue;
        }
        return true;
#endif
    }
    return false;
}

bool Remove(std::string path)
{
    INFOLOG("Call Remove: %s", path.c_str());
    bool flag = false;
    try {
        flag = boost::filesystem::exists(path);
    }
    catch (const boost::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE) << "path exists() exeption: " << WIPE_SENSITIVE(e.code().message())
            << ", path: " << path << HCPENDLOG;
        return false;
    }
    if (!flag) {
        INFOLOG("Path(%s) not exist, needn't remove.", path.c_str());
        return true;
    }

    constexpr uint16_t maxRemoveDirRetryCnt = 3;
    int retry = 0;
    do {
        retry++;
        try {
            boost::filesystem::remove_all(path);
        }
        catch (const boost::filesystem::filesystem_error &e) {
            ERRLOG("remove_all() exeption: %s, path: %s", WIPE_SENSITIVE(e.code().message()).c_str(), path.c_str());
        }
        if (!boost::filesystem::exists(path)) {
            break;
        }
        sleep(retry);
    } while (retry < maxRemoveDirRetryCnt);

    if (boost::filesystem::exists(path)) {
        ERRLOG("Remove path fail for: %s", path.c_str());
        return false;
    }
    return true;
}

bool IsPathExists(const std::string &path)
{
#ifdef WIN32
    auto statResult = Module::FileSystemUtil::Stat(path);
    if (!statResult) {
        WARNLOG("win32 FileSystemUtil stat path %s failed, error: %d", path.c_str(), ::GetLastError());
        return false;
    }
    return true;
#else
    bool flag = false;
    try {
        flag = boost::filesystem::exists(path);
    }
    catch (const boost::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE) << "path exists() exeption: " << WIPE_SENSITIVE(e.code().message())
            << ", path: " << path << HCPENDLOG;
        return false;
    }
    if (!flag) {
        DBGLOG("path: %s doesn't exist", path.c_str());
        return false;
    }
    return true;
#endif
}

bool Rename(std::string srcPath, std::string dstPath)
{
    if (!IsPathExists(srcPath)) {
        ERRLOG("Can not rename, source path: %s doesn't exist", srcPath.c_str());
        return false;
    }

    if (IsPathExists(dstPath) && !Remove(dstPath)) {
        HCP_Log(ERR, MODULE) << "dst path: " << dstPath << " exist and remove failed" << HCPENDLOG;
        return false;
    }

    constexpr uint16_t maxRemoveDirRetryCnt = 3;
    int res = 0;
    int retry = 0;
    do {
        retry++;
        res = rename(srcPath.c_str(), dstPath.c_str());
        if (res == 0) {
            DBGLOG("Rename src path: %s to dst path: %s success", srcPath.c_str(), dstPath.c_str());
            break;
        }
        sleep(retry);
    } while (retry < maxRemoveDirRetryCnt);
    if (res != 0) {
#ifdef WIN32
        ERRLOG("Rename failed, src: %s, dst: %s, error: %d", srcPath.c_str(), dstPath.c_str(), ::GetLastError());
#else
        ERRLOG("Rename src path: %s to dst path: %s failed, error: %d", srcPath.c_str(), dstPath.c_str(), errno);
#endif
        return false;
    }
    return true;
}

std::string GetPathName(const std::string &filePath)
{
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif
    size_t fileoffset = filePath.rfind(sep, filePath.length());
    if (fileoffset != string::npos) {
        return filePath.substr(0, fileoffset);
    }

    return ("");
}

std::string GetFileName(const std::string& filePath)
{
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif
    size_t fileoffset = filePath.rfind(sep);
    if (fileoffset == string::npos) {
        return filePath;
    }
    size_t fileStarPos = fileoffset + 1;
    return filePath.substr(fileStarPos);
}

bool IsDirExist(const std::string& pathName)
{
    if (!IsPathExists(pathName)) {
        return false;
    }

    bool res = false;
    try {
        res = boost::filesystem::is_directory(pathName);
    }
    catch (const boost::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE) << "pathName is_directory() exeption: " << WIPE_SENSITIVE(e.code().message())
            << ", pathName: " << pathName << HCPENDLOG;
        return false;
    }
    return res;
}

bool IsFileExist(const std::string& fileName)
{
    if (!IsPathExists(fileName)) {
        return false;
    }

// to_do: windows归档恢复场景这里会报失败， 先注掉后面再看
#ifndef WIN32
    bool res = false;
    try {
        res = boost::filesystem::is_regular_file(fileName);
    }
    catch (const boost::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE) << "fileName is_regular_file() exeption: " << WIPE_SENSITIVE(e.code().message())
            << ", fileName: " << fileName << HCPENDLOG;
        return false;
    }
#endif
    return true;
}

std::string FormatCapacity(uint64_t capacity)
{
    const uint64_t NUM_1024 = 1024;
    const uint64_t KB_size = NUM_1024;
    const uint64_t MB_size = NUM_1024 * KB_size;
    const uint64_t GB_size = NUM_1024 * MB_size;
    const uint64_t TB_size = NUM_1024 * GB_size;
    const uint64_t PB_size = NUM_1024 * TB_size;
    float formatCapacity;

    if ((capacity >= MB_size) && (capacity < GB_size)) {
        formatCapacity = static_cast<float>(capacity) / MB_size;
        return FloatToString(formatCapacity) + "MB";
    } else if ((capacity >= GB_size) && (capacity < TB_size)) {
        formatCapacity = static_cast<float>(capacity) / GB_size;
        return FloatToString(formatCapacity) + "GB";
    } else if ((capacity >= TB_size) && (capacity < PB_size)) {
        formatCapacity = static_cast<float>(capacity) / TB_size;
        return FloatToString(formatCapacity, NUMBER2) + "TB";
    } else if ((capacity >= KB_size) && (capacity < MB_size)) {
        formatCapacity = static_cast<float>(capacity) / KB_size;
        return FloatToString(formatCapacity) + "KB";
    } else if (capacity >= PB_size) {
        formatCapacity = static_cast<float>(capacity) / PB_size;
        return FloatToString(formatCapacity) + "PB";
    } else {
        return to_string(capacity) + "B";
    }
}

bool RemoveFile(const std::string& path)
{
    constexpr uint16_t maxRemoveDirRetryCnt = 3;
    bool res = false;
    int retry = 0;

    try {
        if (!boost::filesystem::is_regular_file(path)) {
            WARNLOG("skip remove inregular file %s", path.c_str());
            return true;
        }

        do {
            retry++;
            res = boost::filesystem::remove(path);
            if (res) {
                INFOLOG("path:%s", path.c_str());
                break;
            }
            sleep(retry);
        } while (retry < maxRemoveDirRetryCnt);

        if (!res) {
            ERRLOG("Remove path fail for: %s", path.c_str());
            return false;
        }
    }
    catch (const boost::filesystem::filesystem_error &e) {
        ERRLOG("remove file %s caught exception: %s", path.c_str(), e.code().message());
        return false;
    }
    return true;
}

bool CopyFile(std::string srcfile, std::string dstfile)
{
    if (!IsFileExist(srcfile)) {
        HCP_Log(ERR, MODULE) << "src file: " << srcfile << " isn't a file" << HCPENDLOG;
        return false;
    }

    constexpr uint16_t maxRemoveDirRetryCnt = 3;
    bool res = false;
    int retry = 0;

    do {
        retry++;
        try {
            res = boost::filesystem::copy_file(
                srcfile, dstfile, boost::filesystem::copy_option::overwrite_if_exists);
        }
        catch (const boost::filesystem::filesystem_error &e) {
            HCP_Log(ERR, MODULE) << "copy_file() exeption: " << WIPE_SENSITIVE(e.code().message())
                << ", srcfile: " << srcfile << ", dstfile: " << dstfile << HCPENDLOG;
            return false;
        }
        if (res) {
            break;
        }
        sleep(retry);
    } while (retry < maxRemoveDirRetryCnt);

    if (!res)
        HCP_Log(ERR, MODULE) << "failed to copy file: " << WIPE_SENSITIVE(srcfile) << HCPENDLOG;
    return res;
}

bool GetFileListInDirectory(std::string dir, std::vector<string>& fileList)
{
#ifdef WIN32
    try {
        if (!std::filesystem::exists(dir)) {
            ERRLOG("GetFileListInDirectory failed, dir: %s is not exist", dir.c_str());
            return false;
        }
        std::filesystem::directory_iterator endIter;
        for (std::filesystem::directory_iterator iter(dir); iter != endIter; ++iter) {
            if (std::filesystem::is_regular_file(iter->status())) {
                fileList.push_back(iter->path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE) << "catch boost file system exception : " << e.code().message() << " "
            << dir << HCPENDLOG;
        return false;
    }
#else
    try {
        if (!boost::filesystem::exists(dir)) {
            ERRLOG("GetFileListInDirectory failed, dir: %s is not exist", dir.c_str());
            return false;
        }
        boost::filesystem::directory_iterator endIter;
        for (boost::filesystem::directory_iterator iter(dir); iter != endIter; ++iter) {
            if (boost::filesystem::is_regular_file(iter->status())) {
                fileList.push_back(iter->path().string());
            }
        }
    } catch (const boost::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE) << "catch boost file system exception : " << e.code().message() << " "
            << dir << HCPENDLOG;
        return false;
    }
#endif
    return true;
}

bool GetDirListInDirectory(std::string dir, std::vector<string>& dirList, bool skipSnapshot)
{
    try {
#ifdef WIN32
        if (!std::filesystem::exists(dir)) {
            ERRLOG("GetDirListInDirectory failed, dir: %s is not exist", dir.c_str());
            return false;
        }
        std::filesystem::directory_iterator endIter;
        for (std::filesystem::directory_iterator iter(dir); iter != endIter; ++iter) {
            if (std::filesystem::is_directory(iter->status()) &&
                !(skipSnapshot && GetFileName(iter->path().string()) == SNAPSHOT_DIRNAME)) {
                dirList.push_back(GetFileName(iter->path().string()));
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE) << "catch boost file system exception : " << e.code().message() << " " << dir << HCPENDLOG;
        return false;
    }
#else
        if (!boost::filesystem::exists(dir)) {
            ERRLOG("GetDirListInDirectory failed, dir: %s is not exist", dir.c_str());
            return false;
        }
        boost::filesystem::directory_iterator endIter;
        for (boost::filesystem::directory_iterator iter(dir); iter != endIter; ++iter) {
            if (boost::filesystem::is_directory(iter->status()) &&
                !(skipSnapshot && GetFileName(iter->path().string()) == SNAPSHOT_DIRNAME)) {
                dirList.push_back(GetFileName(iter->path().string()));
            }
        }
    } catch (const boost::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE) << "catch boost file system exception : " << e.code().message() << " " << dir << HCPENDLOG;
        return false;
    }
#endif

    return true;
}

string GetParentDirName(const std::string& dir)
{
    string parentRelName;
    if (dir.empty()) {
        return parentRelName;
    }
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif
    string parentPath = GetPathName(dir);
    size_t fileoffset = parentPath.rfind(sep);
    parentRelName = parentPath.substr(fileoffset + 1);
    return parentRelName;
}

std::string FloatToString(const float &val, const uint8_t &precisson)
{
    stringstream sstream;
    sstream << setiosflags(ios::fixed) << setprecision(precisson) << val;
    return sstream.str();
}

void StripWhiteSpace(string& str)
{
    auto it = str.begin();
    while (it != str.end()) {
        if ((*it) == '\n' ||
            (*it) == '\t') {
            it = str.erase(it);
        } else {
            ++it;
        }
    }
}

void StripEscapeChar(string& str)
{
    auto it = str.begin();
    while (it != str.end()) {
        if ((*it) == '\\') {
            it = str.erase(it);
        } else {
            ++it;
        }
    }
}

std::string ReverseSlash(const std::string& path)
{
    string tmp = path;
#ifdef WIN32
    for (char& c : tmp) {
        if (c == '/') {
            c = '\\';
        }
    }
#endif
    return tmp;
}

std::string ReverseSlashWithLongPath(const std::string &path)
{
    string tmp = path;
#ifdef WIN32
    for (char &c : tmp) {
        if (c == '/') {
            c = '\\';
        }
    }
    if (path.length() > PATH_PREFIX.length() && path.find(PATH_PREFIX) == 0) {
        return tmp;
    }
    return PATH_PREFIX + tmp;
#else
    return tmp;
#endif
}

std::string PathJoin(std::initializer_list<std::string> paths)
{
    std::string fullPath = "";
    if (paths.size() == 0) {
        return fullPath;
    }
    for (auto path : paths) {
        fullPath += !fullPath.empty() ? dir_sep : "";
        fullPath += path;
    }
#ifdef WIN32
    RemoveDoubleBackSlash(fullPath);
#else
    RemoveDoubleSlash(fullPath);
#endif
    return fullPath;
}

inline void RemoveDoubleSlash(std::string &str)
{
    std::size_t pos = 0;
    while ((pos = str.find(DOUBLE_SLASH)) != std::string::npos) {
        str.replace(pos, DOUBLE_SLASH.length(), SLASH);
    }
}

inline void RemoveDoubleBackSlash(std::string &str)
{
    std::size_t beginIndex = 0;
    if (str.find(PATH_PREFIX) == 0) {
        beginIndex = PATH_PREFIX.size();
    }
    std::size_t pos = 0;
    while ((pos = str.find(DOUBLE_BACKSLASH, beginIndex)) != std::string::npos) {
        str.replace(pos, DOUBLE_BACKSLASH.length(), BACKSLASH);
    }
}

std::string VolumeNameTransform(const std::string& mapperName)
{
    // /dev/mapper/lv1-testAll --> lv1/testAll : ({vol_group}/{logical_vol})
    std::vector<std::string> split_string;
    boost::split(split_string, mapperName, boost::is_any_of("/"), boost::token_compress_on);
    std::string lvmVolumeName = split_string.back();
    if (lvmVolumeName.empty()) {
        ERRLOG("Transform lvm volume name failed, it's mapper path: %s", mapperName.c_str());
        return lvmVolumeName;
    }
    std::string::size_type pos = lvmVolumeName.find("-");
    if (pos != std::string::npos) {
        lvmVolumeName.replace(pos, 1, "/");
    }
    return lvmVolumeName;
}

#ifdef WIN32
uint64_t GetWinVolumeSize(const std::string& devicePath)
{
    std::string winDevicePath = "\\\\.\\" + devicePath;
    HANDLE hDevice = CreateFileA(
        winDevicePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);
    if (hDevice == INVALID_HANDLE_VALUE) {
        ERRLOG("Unable to open device.Error code: %d", GetLastError());
        return 0;
    }
    GET_LENGTH_INFORMATION lengthInfo;
    DWORD bytesReturned;
    bool success = DeviceIoControl(
        hDevice,
        IOCTL_DISK_GET_LENGTH_INFO,
        nullptr,
        0,
        &lengthInfo,
        sizeof(lengthInfo),
        &bytesReturned,
        nullptr);
    if (!success) {
        ERRLOG("Unable to get device size.Error code: %d", GetLastError());
        CloseHandle(hDevice);
        return 0;
    }
    CloseHandle(hDevice);
    return lengthInfo.Length.QuadPart;
}

std::string GetWinSystemDriveForInd()
{
    std::string drive;
    static char buffer[MAX_PATH];
    DWORD ret = ::GetEnvironmentVariableA("WINDIR", buffer, MAX_PATH);
    if (ret == 0 || ret > MAX_PATH) {
        WARNLOG("Failed get environment variable 'WINDIR', errno: %d", ::GetLastError());
        drive = "";
    } else {
        DBGLOG("'WINDIR' environment variable is: %s", buffer);
        std::string sysDir(buffer);
        drive = sysDir.substr(0, 1);
    }
    if (drive.empty()) {
        drive = "C";
    }
    return drive;
}

std::wstring TrimTrailingSpaces(const std::wstring& str)
{
    size_t end = str.find_last_not_of(L" \t\n\r\f\v");
    if (end != std::wstring::npos) {
        return str.substr(0, end + 1);
    }
    return str;
}

std::wstring ConvertDevicePath(const std::wstring& devicePath)
{
    // 查找 "\Device" 并替换为 "\\.\"
    size_t pos = devicePath.find(L"\\Device");
    if (pos != std::wstring::npos) {
        // 跳过 "\Device"
        std::wstring temp = L"\\\\.\\" + devicePath.substr(pos + NUMBER8);
        return TrimTrailingSpaces(temp);
    }
    return TrimTrailingSpaces(devicePath);
}

std::string GetEFIDrivePath()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    BCDStore store = BCDStore::OpenStore();
    for (BCDObject obj : store.GetObjects()) {
        if (obj.GetType() == BCDObject::BCDObjectType::GLOBAL_SETTINGS) {
            std::wstring drivePath = FileSystemUtil::Utf8ToUtf16(obj.GetElement(
                Win32::BCDElementType::BCDLIBRARY_DEVICE_APPLICATIONDEVICE).ToDeviceData().GetPartitionPath());
            INFOLOG("Windows boot: %s", FileSystemUtil::Utf16ToUtf8(drivePath).c_str());
            std::wstring efiPartition = ConvertDevicePath(drivePath);
            return FileSystemUtil::Utf16ToUtf8(efiPartition);
        }
    }
    ERRLOG("Failed to Get EFI Drive path!");
    return "";
}

// 检查WinPE环境的引导方式
std::string GetBootTypeForWinPE()
{
    HKEY hKey;
    DWORD firmwareType = 0;
    DWORD size = sizeof(firmwareType);

    LONG result = ::RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control",
        0,
        KEY_READ,
        &hKey);
    if (result != ERROR_SUCCESS) {
        ERRLOG("Failed to RegOpenKeyExW, err: %ld", result);
        return "Unknown boot";
    }

    result = ::RegQueryValueExW(
        hKey,
        L"PEFirmwareType",
        NULL,
        NULL,
        reinterpret_cast<LPBYTE>(&firmwareType),
        &size);

    ::RegCloseKey(hKey);

    if (result != ERROR_SUCCESS) {
        ERRLOG("Failed to get PEFirmwareType from registry, err: %ld", result);
        return "Unknown boot";
    }

    switch (firmwareType) {
        case NUMBER1:
            return BIOS_BOOT;
        case NUMBER2:
            return UEFI_BOOT;
        default:
            return UNKNOW_BOOT;
    }
}

// 检查是否是WinPE环境
bool DetectWinPE()
{
    char buffer[MAX_PATH];
    DWORD ret = ::GetEnvironmentVariableA("WINDIR", buffer, MAX_PATH);
    if (ret == 0 || ret > MAX_PATH) {
        WARNLOG("Failed get environment variable 'WINDIR', errno: %d", ::GetLastError());
    } else {
        DBGLOG("'WINDIR' environment variable is: %s", buffer);
        if (_stricmp(buffer, "X:\\Windows") == 0) {
            INFOLOG("WinPE environment detected");
            return true;
        }
    }

    try {
        std::wstring regVal =
            Win32::RegGetString(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control", L"SystemStartOptions");
        INFOLOG("'SystemStartOptions' registry value is: %s", Module::FileSystemUtil::Utf16ToUtf8(regVal).c_str());
        std::transform(regVal.begin(), regVal.end(), regVal.begin(), ::toupper);
        if (regVal.find(L"MININT") != std::wstring::npos) {
            INFOLOG("WinPE environment detected");
            return true;
        }
    } catch (const Win32::RegistryError &e) {
        ERRLOG("Failed to get SystemStartOptions from registry, err: %d", e.ErrorCode());
        return false;
    }

    INFOLOG("No WinPE environment detected");
    return false;
}

// 判断Windows环境是否是UEFI引导
bool WinIsUEFIBoot()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    BCDStore store = BCDStore::OpenStore();
    std::wstring recoveryPartition;
    std::wstring efiPartition;
    for (BCDObject obj : store.GetObjects()) {
        if (obj.GetType() == BCDObject::BCDObjectType::GLOBAL_SETTINGS) {
            std::string efiPath = obj.GetElement(Win32::BCDElementType::BCDLIBRARY_STRING_APPLICATIONPATH).ToString();
            if (efiPath.find("bootmgfw.efi") != std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

bool IsUEFIBoot()
{
    if (!DetectWinPE()) {
        INFOLOG("Not WinPE environment");
        if (WinIsUEFIBoot()) {
            INFOLOG("Windows is boot by UEFI.");
            return true;
        } else {
            INFOLOG("Windows is boot by BIOS.");
            return false;
        }
    } else {
        INFOLOG("WinPE environment");
        std::string bootType = GetBootTypeForWinPE();
        if (bootType == UEFI_BOOT) {
            INFOLOG("winPE is boot by UEFI.");
            return true;
        } else if (bootType == BIOS_BOOT) {
            INFOLOG("winPE is boot by BIOS.");
            return false;
        } else {
            INFOLOG("winPE is boot by unknown.");
            return false;
        }
    }
}
#endif

uint64_t GetVolumeSize(const std::string& devicePath)
{
#ifdef __linux__
    int fd = ::open(devicePath.c_str(), O_RDONLY);
    if (fd < 0) {
        ERRLOG("failed to open volume device %s, errno = %d", devicePath.c_str(), errno);
        return 0;
    }
    uint64_t size = 0;
    if (::ioctl(fd, BLKGETSIZE64, &size) < 0) {
        close(fd);
        ERRLOG("failed to execute ioctl BLKGETSIZE64, device %s, errno = %d", devicePath.c_str(), errno);
        return 0;
    }
    ::close(fd);
    return size;
#elif defined(WIN32)
    return GetWinVolumeSize(devicePath);
#else
    WARNLOG("GetVolumeSize not implemented on this platform!");
    return 0;
#endif
}

string GetVolumeUuid(const std::string& devicePath)
{
#ifdef __linux__
    struct stat st;
    if (devicePath.empty()) {
        ERRLOG("devicePath is empty, cannot get its volumeId");
        return "";
    }
    if (stat(devicePath.c_str(), &st) < 0) {
        ERRLOG("cannot get stat res:%s", devicePath.c_str());
        return "";
    }
    return to_string(st.st_ino);
#else
    WARNLOG("GetVolumeUuid not implemented on this platform!");
    return "";
#endif
}

// return lvm vg-lv name
std::string GetVolumeName(const std::string& dmDevicePath)
{
    if (dmDevicePath.find("/dev/mapper/") != 0) {
        ERRLOG("invalid lvm dm device path %s", dmDevicePath.c_str());
        return dmDevicePath;
    }
    return dmDevicePath.substr(std::string("/dev/mapper/").length());
}


std::string LowerCase(const std::string& path)
{
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(),
        [](unsigned char c) { return ::tolower(c); });
    return lowerPath;
}

#ifndef WIN32
string GetRealPath(const string& path)
{
    string realPath;
    char normalizePath[PATH_MAX + 1] = { 0x00 };
    if (realpath(path.c_str(), normalizePath) != nullptr) {
        realPath = normalizePath;
    }
    return realPath;
}
#endif

#ifndef WIN32
string GetDirName(const string& path)
{
    vector<char> copy(path.c_str(), path.c_str() + path.size() + 1);
    return dirname(copy.data());
}
#endif

#ifndef WIN32
bool IsDir(const string& path)
{
    struct stat st {};
    return (lstat(path.c_str(), &st) == 0) && S_ISDIR(st.st_mode);
}
#endif

std::string GetFileNameOfPath(std::string filePath)
{
    while (!filePath.empty() && filePath.back() == dir_sep[0]) {
        filePath.pop_back();
    }
    auto pos = filePath.find_last_of(dir_sep);
    return pos == std::string::npos ? filePath : filePath.substr(pos + 1);
}

std::string ReadFileContent(const std::string fullpath)
{
    INFOLOG("read content of file %s", fullpath.c_str());
    std::string content;
    try {
        std::ifstream inFileStream {};
        std::stringstream fileBuffer {};
        inFileStream.open(fullpath.c_str(), std::ios::in);
        if (!inFileStream.is_open()) {
            ERRLOG("read from file open file failed %s, errno %d", fullpath.c_str(), errno);
            return "";
        }
        fileBuffer << inFileStream.rdbuf();
        content = fileBuffer.str();
        inFileStream.close();
    } catch (...) {
        ERRLOG("read system name record file got exception");
    }
    return content;
}

} // namespace
