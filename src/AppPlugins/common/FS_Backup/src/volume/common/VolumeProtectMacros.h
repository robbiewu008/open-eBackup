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
#ifndef VOLUMEBACKUP_PROTECT_MACROS_HEADER
#define VOLUMEBACKUP_PROTECT_MACROS_HEADER

// function as stdafx.h, include common used STL headers here
#include <string>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <thread>
#include <fstream>
#include <vector>
#include <atomic>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <memory>
#include <queue>
#include <map>
#include <unordered_map>

#include "securec.h"

using ErrCodeType = int;

 /*
 * @brief
 * add -DLIBRARY_EXPORT build param to export lib on Win32 MSVC
 * define LIBRARY_IMPORT before including VolumeProtector.h to add __declspec(dllimport) to use dll library
 * libvolumeprotect is linked static by default
 * (avoid using define/Defines.h)
 */

// define library export macro
#ifdef _WIN32
    #ifdef LIBRARY_EXPORT
        #define VOLUMEPROTECT_API __declspec(dllexport)
    #else
        #ifdef LIBRARY_IMPORT
            #define VOLUMEPROTECT_API __declspec(dllimport)
        #else
            #define VOLUMEPROTECT_API
        #endif
    #endif
#else
    #define VOLUMEPROTECT_API
#endif

// check platform macro conflict
#ifdef __linux__
#ifdef _WIN32
static_assert(false, "conflict macro, both __linux__ and _WIN32 defined!");
#endif
#endif

// check if any of the platform macro defined
#ifndef __linux__
#ifndef _WIN32
static_assert(false, "platform unsupported, none of __linux__ and _WIN32 defined!");
#endif
#endif


// check if make_unique defined
#ifndef __cpp_lib_make_unique
/**
 * @brief define extended std function
 */
namespace exstd {
template<typename T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}
#else
namespace exstd = std;
#endif

/**
 * @brief extended smart pointer module
 */
namespace mem {
template<typename TO, typename FROM>
std::unique_ptr<TO> static_unique_pointer_cast(std::unique_ptr<FROM>&& old)
{
    return std::unique_ptr<TO>(static_cast<TO*>(old.release()));
}
}

#endif