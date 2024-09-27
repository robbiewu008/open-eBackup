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
#ifndef DME_NAS_UTILITIES_H
#define DME_NAS_UTILITIES_H

#include <string>
#include <set>
#include <shared_mutex>
#include <ctime>
#include <fstream>
#ifndef WIN32
#include <libgen.h>
#endif
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "define/Types.h"
#include "log/Log.h"

namespace PluginUtils {
    constexpr uint8_t NUMBER1  = 1;
    constexpr uint32_t NUMBER64 = 64;
    bool CheckDeviceNetworkConnect(const std::string &managerIps);
    std::string FormatTimeToStrBySetting(time_t timeInSeconds, const std::string& timeFormat);
    std::string FormatTimeToStr(time_t timeInSeconds);
    std::string Base64Encode(const std::string &in);
    std::string Base64Decode(const std::string &in);
    void LogCmdExecuteError(int retCode, const std::vector<std::string> &output,
                            const std::vector<std::string> &errOutput);
    int RunShellCmd(const std::string& cmd, const std::vector<std::string>& paramList);
    int RunShellCmd(const std::string& cmd, const std::vector<std::string>& paramList,
        std::vector<std::string>& output);

    time_t GetCurrentTimeInSeconds();
    time_t GetCurrentTimeInSeconds(std::string &dateAndTimeString);
    std::string ConvertToReadableTime(time_t time);
    std::string GetPathName(const std::string &filePath);
    std::string GetFileName(const std::string &filePath);
    bool WriteFile(const std::string &path, const std::string &data);
    bool ReadFile(const std::string &path, std::string &data);
    bool CreateDirectory(const std::string& path);
    bool Remove(std::string path);
    bool IsPathExists(const std::string &path);
    bool Rename(std::string srcPath, std::string dstPath);

    bool IsDirExist(const std::string& pathName);
    bool IsFileExist(const std::string& fileName);
    bool CopyFile(std::string srcfile, std::string dstfile);
    bool RemoveFile(const std::string& path);
    bool GetFileListInDirectory(std::string dir, std::vector<std::string>& fileList);
    bool GetDirListInDirectory(std::string dir, std::vector<std::string>& fileList, bool skipSnapshot = false);
    std::string GetParentDirName(const std::string& dir);

    size_t GenerateHash(std::string jobId);
    std::string FormatCapacity(uint64_t capacity);
    std::string FloatToString(const float &val, const uint8_t &precisson = NUMBER1);
    // remove \n \t in str
    void StripWhiteSpace(std::string& str);
    // remove \(escape charactor) in str
    void StripEscapeChar(std::string& str);
    std::string ReverseSlash(const std::string& path);
    std::string ReverseSlashWithLongPath(const std::string& path);
    std::string PathJoin(std::initializer_list<std::string> paths);
    inline void RemoveDoubleSlash(std::string &str);
    inline void RemoveDoubleBackSlash(std::string &str);
    std::string VolumeNameTransform(const std::string& mapperName);
    uint64_t GetVolumeSize(const std::string& devicePath);
    std::string GetVolumeUuid(const std::string& devicePath);
    std::string GetLvmVolumeName(const std::string& dmDevicePath);

    std::string LowerCase(const std::string& path);
#ifndef WIN32
    std::string GetRealPath(const std::string& path);
    std::string GetDirName(const std::string& path);
    bool IsDir(const std::string& path);
#endif
    template<class ... T>
    std::string PathJoin(T&&... paths)
    {
        return PathJoin({ paths... });
    }

    std::string GetFileNameOfPath(std::string filePath);

    std::string ReadFileContent(const std::string fullpath);
}

#endif  // DME_NAS_UTILITIES_H