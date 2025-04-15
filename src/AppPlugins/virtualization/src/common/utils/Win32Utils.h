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
#ifndef WIN32_UTILS_H
#define WIN32_UTILS_H
#ifdef WIN32
#include <string>
#include <vector>
#include <iostream>
#include <log/Log.h>
#include <comdef.h>

std::wstring String2wstring(const std::string &s);
std::wstring String2WstringByUtf8(const std::string &s);
std::string BSTRToString(const BSTR &bstr);
BSTR StringToBSTR(const std::string& str);
#endif
#endif
