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
#include "Uuid.h"
#ifdef WIN32
#include <objbase.h>
#include <cstdio>
#include "securec.h"
#else
#include <uuid/uuid.h>
#endif

namespace {
const int UUID_BUF_LENGTH = 40;
const int POS_0 = 0;
const int POS_1 = 1;
const int POS_2 = 2;
const int POS_3 = 3;
const int POS_4 = 4;
const int POS_5 = 5;
const int POS_6 = 6;
const int POS_7 = 7;
}

namespace VirtPlugin {
#ifdef WIN32
std::string GuidToString(const GUID &guid)
{
    char guidCstr[UUID_BUF_LENGTH] = {0};
    snprintf_s(guidCstr, sizeof(guidCstr), sizeof(guidCstr) - 1,
             "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             guid.Data1, guid.Data2, guid.Data3,
             guid.Data4[POS_0], guid.Data4[POS_1], guid.Data4[POS_2], guid.Data4[POS_3],
             guid.Data4[POS_4], guid.Data4[POS_5], guid.Data4[POS_6], guid.Data4[POS_7]);
    return std::string(guidCstr);
}
#endif

std::string Uuid::GenerateUuid()
{
    std::string strUuid;
#ifdef WIN32
    GUID uuid;
    ::CoCreateGuid(&uuid);
    strUuid = GuidToString(uuid);
#else
    uuid_t uuid;
    char buf[UUID_BUF_LENGTH] = {0};

    uuid_generate(uuid);
    uuid_unparse_upper(uuid, buf);
    strUuid = buf;
#endif
    return strUuid;
}
}