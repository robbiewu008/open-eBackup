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
#include <sstream>
#include <locale>
#include <ctime>
#include <fcntl.h>
#include <chrono>
#include <random>
#include <cerrno>
#include "log/Log.h"
#include "UniqueId.h"

using namespace GeneralDB;
namespace {
    const mp_string MODULE_NAME = "UniqueID";
}

mp_int32 UniqueId::GenerateUniqueID(mp_string &uniqueId)
{
    uniqueId = "";
    Module::CThreadAutoLock lock(&m_uniqueIDMutex);
    uniqueId = GetTimestamp() + "_";
    mp_uint64 randNum;
    if (GetRandom(randNum) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Get Random number failed." << HCPENDLOG;
        return MP_FAILED;
    }
    uniqueId += std::to_string(randNum);
    return MP_SUCCESS;
}

mp_string UniqueId::GetTimestamp()
{
    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    long long timeStamp = currentTime.count();
    return std::to_string(timeStamp);
}

mp_int32 UniqueId::GetRandom(mp_uint64& num)
{
#ifdef WIN32
    HCRYPTPROV hCryptProv;
    mp_uint64 lastCode;
    if (!::CryptAcquireContextW(&hCryptProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        lastCode = GetLastError();
        HCP_Log(ERR, MODULE_NAME) << "CryptAcquireContextW exec failed, errorcode = " << lastCode;
        return MP_FAILED;
    }

    if (!CryptGenRandom(hCryptProv, sizeof(mp_uint64), reinterpret_cast<BYTE*>(&num))) {
        lastCode = GetLastError();
        HCP_Log(ERR, MODULE_NAME) << "CryptGenRandom exec failed, errorcode = " << lastCode;
        ::CryptReleaseContext(hCryptProv, 0);
        return MP_FAILED;
    }

    if (!::CryptReleaseContext(hCryptProv, 0)) {
        lastCode = GetLastError();
        HCP_Log(ERR, MODULE_NAME) << "CryptReleaseContext exec failed, errorcode = " << lastCode;
        return MP_FAILED;
    }
#else
    mp_int32 fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        HCP_Log(ERR, MODULE_NAME) << "open /dev/random failed.strerrno = " << strerror(errno);
        return MP_FAILED;
    }

    if (read(fd, &num, sizeof(mp_uint64)) == -1) {
        close(fd);
        HCP_Log(ERR, MODULE_NAME) << "read file failed, strerrno = " << strerror(errno);
        return MP_FAILED;
    }
    close(fd);
#endif

    return MP_SUCCESS;
}