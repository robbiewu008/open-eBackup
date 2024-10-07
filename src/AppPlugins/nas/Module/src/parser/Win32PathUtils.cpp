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
#include "Win32PathUtils.h"
#include <cstring>
#include <algorithm>

namespace {
    const std::string DOUBLE_BACKSLASH = "\\\\";
    const std::string DOUBLE_SLASH = "//";
    const std::string SLASH = "/";
    const std::string BACKSLASH = "\\";
    const char WIN_SEPARATOR = '\\';
    const char POSIX_SEPARATOR = '/';
    constexpr auto NUM2 = 2;
}

namespace Module {
namespace Win32PathUtil {

inline bool IsValidWindowDriver(const char driver)
{
    return ::toupper(driver) >= 'A' && ::toupper(driver) <= 'Z';
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
    std::size_t pos = 0;
    while ((pos = str.find(DOUBLE_BACKSLASH)) != std::string::npos) {
        str.replace(pos, DOUBLE_BACKSLASH.length(), BACKSLASH);
    }
}

std::string LowerCase(const std::string& path)
{
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(),
        [](unsigned char c) { return ::tolower(c); });
    return lowerPath;
}

/**
 * Map a posix path to a windows path, map first directory as driver
 * Example: "/C/User" => "C:\User"
 */
std::string PosixToWin32(const std::string &path, bool forceLowerCase)
{
    if (path.size() <= 1 || path.front() != POSIX_SEPARATOR) {
        return ""; // invalid path
    }
    std::string driver{};
    std::string subPath{};
    std::size_t pos = path.find(POSIX_SEPARATOR, 1);
    if (pos == std::string::npos) {
        driver = path.substr(1);
    } else {
        driver = path.substr(1, pos - 1);
        if (pos + 1 < path.length()) {
            subPath = path.substr(pos + 1);
        }
    }
    // to resolve subPath neither begin nor end with separator
    if (!subPath.empty() && subPath.back() == POSIX_SEPARATOR) {
        subPath.pop_back();
    }
    if (forceLowerCase) {
        std::transform(driver.begin(), driver.end(), driver.begin(),
            [](unsigned char c) { return ::tolower(c); });
        std::transform(subPath.begin(), subPath.end(), subPath.begin(),
            [](unsigned char c) { return ::tolower(c); });
    }
    std::replace(subPath.begin(), subPath.end(), POSIX_SEPARATOR, WIN_SEPARATOR);
    if (driver.length() == 1 && IsValidWindowDriver(driver[0])) {
        return driver + ":\\" + subPath;
    } else {
        return DOUBLE_BACKSLASH + driver + "\\" + subPath;
    }
}

/**
 * Map a windows path to a posix path, map driver as the first directory
 * Example: "C:\User" => "/c/user"  -- if forceLowerCase set to true
 */
std::string Win32ToPosix(const std::string &path, bool forceLowerCase)
{
    std::string posixPath = path;
    std::replace(posixPath.begin(), posixPath.end(),
        WIN_SEPARATOR, POSIX_SEPARATOR); // replace all backslash to slash
    RemoveDoubleSlash(posixPath);
    if (path.size() < NUM2 || !IsValidWindowDriver(path[0]) || path[1] != ':') {
        // resolve windows path which don't have a driver
        if (posixPath.length() > 1 && posixPath.back() == POSIX_SEPARATOR) {
            posixPath.pop_back();
        }
        if (!posixPath.empty() && posixPath[0] != POSIX_SEPARATOR) {
            posixPath = SLASH + posixPath;
        }
        return posixPath; // invalid windows path
    }
    char driver = path[0];
    posixPath[0] = POSIX_SEPARATOR;
    posixPath[1] = driver;
    if (forceLowerCase) {
        std::transform(posixPath.begin(), posixPath.end(), posixPath.begin(),
            [](unsigned char c) { return ::tolower(c); });
    }
    if (posixPath.length() > 1 && posixPath.back() == POSIX_SEPARATOR) {
        posixPath.pop_back();
    }
    return posixPath;
}

/**
 * Recover a VSS copy mounted path into it's origin path using specified driver,
 * (will force to transform to lower case before matching prefix)
 * Example:
 *			path            =   D:\vssmount\114514-191980-114514-1919810\Users\Administrator\Desktop
 *			prefix          =	D:\vssmount\114514-191980-114514-1919810
 *			orifunDriver    =   C
 *			return          =	C:\Users\Administrator\Desktop
 */
std::string Win32PathRecoverPrefix(const std::string& path, const std::string& prefix, const char originDriver)
{
    if (prefix.empty()) { // prefix disabled, "path" is orgin path
        return path;
    }
    if (!IsValidWindowDriver(originDriver) || prefix.length() > path.length()) {
        return ""; // invalid origin driver or prefix
    }
    std::string lowerCasePath = path;
    std::string lowerCasePrefix = prefix;
    std::transform(lowerCasePath.begin(), lowerCasePath.end(), lowerCasePath.begin(),
        [](unsigned char c) { return ::tolower(c); });
    std::transform(lowerCasePrefix.begin(), lowerCasePrefix.end(), lowerCasePrefix.begin(),
        [](unsigned char c) { return ::tolower(c); });
    if (lowerCasePath.find(lowerCasePrefix) != 0) {
        return ""; // invalid prefix
    }
    std::string actualSubPath = path.substr(prefix.length()); // orgin path without driver
    if (!actualSubPath.empty() && actualSubPath.front() == WIN_SEPARATOR) {
        actualSubPath = actualSubPath.substr(1);
    }
    std::string driverStr;
    driverStr.push_back(originDriver);
    return driverStr + ":\\" + actualSubPath;
}

/**
 * Recover a VSS copy mounted path into it's origin path without specifing driver,
 * originDriver will deduce from the first level directory after removed prefix from path
 * (will force to transform to lower case before matching prefix)
 * Example:
 *			path        =   D:\vssmount\114514-191980-114514-1919810\C\Users\Administrator\Desktop
 *			prefix      =	D:\vssmount\114514-191980-114514-1919810
 *			return      =	C:\Users\Administrator\Desktop
 */
std::string Win32PathRecoverPrefix(const std::string& path, const std::string& prefix)
{
    if (prefix.empty()) { // prefix disabled, "path" is orgin path
        return path;
    }
    std::string lowerCasePath = path;
    std::string lowerCasePrefix = prefix;
    std::transform(lowerCasePath.begin(), lowerCasePath.end(), lowerCasePath.begin(),
        [](unsigned char c) { return ::tolower(c); });
    std::transform(lowerCasePrefix.begin(), lowerCasePrefix.end(), lowerCasePrefix.begin(),
        [](unsigned char c) { return ::tolower(c); });
    if (lowerCasePath.find(lowerCasePrefix) != 0) {
        return ""; // invalid prefix
    }

    std::string actualSubPath = path.substr(lowerCasePrefix.length()); // orgin path with driver
    if (!actualSubPath.empty() && actualSubPath.front() == WIN_SEPARATOR) {
        actualSubPath = actualSubPath.substr(1);
    }
    if (actualSubPath.empty()) {
        return ""; // invalid origin driver (empty)
    }
    std::string driver;
    std::size_t pos;
    if ((pos = actualSubPath.find(WIN_SEPARATOR)) == std::string::npos) {
        driver = actualSubPath;
        actualSubPath = "";
    } else {
        driver = actualSubPath.substr(0, pos);
        actualSubPath = actualSubPath.substr(pos + 1);
    }
    if (driver.size() != 1 || !IsValidWindowDriver(driver[0])) { // not regular driver
        return DOUBLE_BACKSLASH + driver + BACKSLASH + actualSubPath;
    } else {
        return driver + ":\\" + actualSubPath;
    }
}

/**
 * compare if two path is the same (case insenstive)
 */
bool Win32PathEquals(const std::string& path1, const std::string& path2)
{
    std::string path1Lower = path1;
    std::string path2Lower = path2;
    std::transform(path1Lower.begin(), path1Lower.end(), path1Lower.begin(),
        [](unsigned char c) { return ::tolower(c); });
    std::transform(path2Lower.begin(), path2Lower.end(), path2Lower.begin(),
        [](unsigned char c) { return ::tolower(c); });
    return path1Lower == path2Lower;
}

std::string ConcatWin32Path(const std::string& path1, const std::string& path2)
{
    std::string win32Path1 = path1;
    std::string win32Path2 = path2;
    // replace all slash to backslash
    std::replace(win32Path1.begin(), win32Path1.end(), POSIX_SEPARATOR, WIN_SEPARATOR);
    std::replace(win32Path2.begin(), win32Path2.end(), POSIX_SEPARATOR, WIN_SEPARATOR);
    std::string concatPath = win32Path1 + "\\" + win32Path2;
    RemoveDoubleBackSlash(concatPath);
    return concatPath;
}

std::string GetParentDir(const std::string& path)
{
    std::string standardWin32Path = path;
    std::replace(standardWin32Path.begin(), standardWin32Path.end(), POSIX_SEPARATOR, WIN_SEPARATOR);
    RemoveDoubleBackSlash(standardWin32Path);
    if (!standardWin32Path.empty() && standardWin32Path.back() == WIN_SEPARATOR) {
        standardWin32Path.pop_back();
    }
    std::string ret = standardWin32Path.substr(0, standardWin32Path.find_last_of(WIN_SEPARATOR));
    if (ret.length() == NUM2 && IsValidWindowDriver(ret[0]) && ret[1] == ':') {
        ret.push_back(WIN_SEPARATOR);
    }
    return ret;
}

std::string GetFileName(const std::string& path)
{
    std::string standardWin32Path = path;
    std::replace(standardWin32Path.begin(), standardWin32Path.end(), POSIX_SEPARATOR, WIN_SEPARATOR);
    RemoveDoubleBackSlash(standardWin32Path);
    if (!standardWin32Path.empty() && standardWin32Path.back() == WIN_SEPARATOR) {
        standardWin32Path.pop_back();
    }
    std::string ret = standardWin32Path.substr(0, standardWin32Path.find_last_of(WIN_SEPARATOR));
    if (ret.length() == NUM2 && IsValidWindowDriver(ret[0]) && ret[1] == ':') {
        ret.push_back(WIN_SEPARATOR);
    }

    auto pos = path.find_last_of(BACKSLASH);
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return ""; /* invalid filename */
}

}

}
