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
#ifndef MOUDLE_DLIB_H
#define MOUDLE_DLIB_H

#include "define/Defines.h"
#include "common/Thread.h"

#ifdef _AIX
#define DFLG_LOCAL (RTLD_MEMBER | RTLD_NOW | RTLD_LOCAL)
#define DFLG_GLOBAL (RTLD_MEMBER | RTLD_NOW | RTLD_GLOBAL)
#else
#define DFLG_LOCAL (RTLD_NOW | RTLD_LOCAL)
#define DFLG_GLOBAL (RTLD_NOW | RTLD_GLOBAL)
#endif

namespace Module {

AGENT_API handle_t DlibOpen(const std::string& pszLibName);
AGENT_API handle_t DlibOpenEx(const std::string& pszLibName, bool bLocal);
AGENT_API void DlibClose(handle_t hDlib);
AGENT_API void* DlibDlsym(handle_t hDlib, const std::string& pszFname);
AGENT_API const char* DlibError(char szMsg[], uint32_t isz);
AGENT_API char* GetOSStrErr(int err, char buf[], std::size_t buf_len);

} // namespace Module

#endif // MOUDLE_DLIB_H