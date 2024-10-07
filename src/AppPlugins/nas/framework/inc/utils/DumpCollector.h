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
#ifndef _APP_DUMP_COLLECTOR_H_
#define _APP_DUMP_COLLECTOR_H_

#ifdef WIN32

#include <string>
#include <mutex>
#include <vector>
#include <functional>
#include "define/Defines.h"

namespace wincrash {

// store basic info of one frame of the stack trace dumped
class AGENT_API StackFrame {
public:
    std::string         file;
    std::string         module;
    std::string         function;
    uint64_t            address;
    uint64_t            line;
};

class AGENT_API DumpCollector {
public:
    // init collector, need to be called at main routine
    static void                         Init();
    // collect summary reason of exception
    static std::string                  GetExceptionInfo();
    // specify root path of dump file
    static bool                         SetDumpFileRoot(const std::string& dumpFileRootPath);
    // genterate a *.dmp file at target path, will return empty if failed
    static std::string                  CreateDumpFile();
    // generate detail info of stack trace (windows)
    static std::vector<StackFrame>      DumpWin32StackTrace();

    // util method, to generate a dump file path contain current datetime
    static std::string                  GenerateDumpFilePath();

    // store ptr of EXCEPTION_POINTER of exception caught
    static void*            exceptionPtr;
    // store the dump file root path specified
    static std::string      dumpFileRootPath;
    static std::mutex       lock;
    // callback function (dump path, crash cause, stacktrace)
    using WinCallbackType = void(const std::string&, const std::string&, const std::vector<StackFrame>& frames);
    static std::function<WinCallbackType> Win32CrashHandler;
};

}

#endif
#endif