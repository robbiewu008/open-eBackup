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
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <locale>
#include <codecvt>
#include "DumpCollector.h"
#include "common/EnvVarManager.h"
#include "log/Log.h"

#pragma comment(lib, "DbgHelp.lib")

#define CASE_RETURN(Error) case Error: { \
    return #Error; \
} \

using namespace wincrash;
using WinCallbackType = DumpCollector::WinCallbackType;

// init static member
std::string                     agentHomePath = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
std::string                     DumpCollector::dumpFileRootPath = agentHomePath; // default root path of win32 dump file
void*                           DumpCollector::exceptionPtr = nullptr;
std::mutex                      DumpCollector::lock {};
std::function<WinCallbackType>  DumpCollector::Win32CrashHandler = nullptr;

namespace {
    const std::string DEFAULT_DUMP_FILENAME = "a.dmp";
    const std::string LOG_FILE_NAME = "Plugin_lle.log";
    const int NUM255 = 255;
    const int NUM254 = 254;
};

static std::wstring Utf8ToUtf16(const std::string& str)
{
    using ConvertTypeX = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<ConvertTypeX> converterX;
    std::wstring wstr = converterX.from_bytes(str);
    return wstr;
}

static std::string Utf16ToUtf8(const std::wstring& wstr)
{
    using ConvertTypeX = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<ConvertTypeX> converterX;
    return converterX.to_bytes(wstr);
}

// Register windows crash handler
static LONG ApplicationCrashCollector(EXCEPTION_POINTERS *pException)
{
    // handler can only be used for one thread at a time
    std::lock_guard<std::mutex> lck(DumpCollector::lock);
    // set exception pointer
    DumpCollector::exceptionPtr = static_cast<void*>(pException);
    // prepare to collect dump info
    std::string dumpFilePath;
    std::string exceptionCause;
    std::vector<StackFrame> stacktrace;
    try {
        dumpFilePath = DumpCollector::CreateDumpFile();
        exceptionCause = DumpCollector::GetExceptionInfo();
        stacktrace = DumpCollector::DumpWin32StackTrace();
    } catch (...) {
        ERRLOG("another win32 exception caught when handling exception!");
    }
    if (DumpCollector::Win32CrashHandler == nullptr) {
        ERRLOG("missing win32 crash handler");
    } else {
        DumpCollector::Win32CrashHandler(dumpFilePath, exceptionCause, stacktrace);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

static void DefaultWin32ApplicationCrashHandler(
    const std::string& dumpFilePath,
    const std::string& crashCause,
    const std::vector<StackFrame>& stacktrace)
{
    std::string logFilePath = DumpCollector::dumpFileRootPath;
    if (!logFilePath.empty() && logFilePath.back() != '\\') {
        logFilePath.push_back('\\');
    }
    logFilePath += LOG_FILE_NAME;
    std::ofstream logstream(logFilePath.c_str(), std::ios::app);
    try {
        logstream << "\n========================== Windows Crash ===========================" << std::endl;
        char strtime[NUM255] = "";
        std::time_t timenow = std::time(nullptr);
        std::tm *localnow = std::localtime(&timenow);
        if (localnow != nullptr) {
            if (std::strftime(strtime, sizeof(strtime), "Time: %Y-%m-%d %H:%M:%S", localnow) > 0) {
                logstream << std::string(strtime) << std::endl;
            }
        }
        if (!dumpFilePath.empty()) {
            logstream << "Dump File Generated At: " << dumpFilePath << std::endl;
        }
        logstream << "Crash Reason: " << crashCause << std::endl;
        for (const StackFrame& frame : stacktrace) {
            logstream
                << frame.module << ","
                << frame.function << "[0x" << std::hex << frame.address << "]"
                << frame.file << ":" << std::dec << frame.line
                << std::endl;
        }
        logstream << "===================================================================\n" << std::endl;
    } catch (...) {
        logstream.close();
        ERRLOG("caught exception in DefaultWin32ApplicationCrashHandler");
        return;
    }
    logstream.close();
    return;
}

// Create dump file
std::string DumpCollector::CreateDumpFile()
{
    if (DumpCollector::exceptionPtr == nullptr) {
        ERRLOG("failed to create dump file, exception ptr is null");
        return "";
    }
    std::string dumpFilePath = DumpCollector::GenerateDumpFilePath();
    EXCEPTION_POINTERS *pException = static_cast<EXCEPTION_POINTERS*>(DumpCollector::exceptionPtr);
    std::wstring wDumpFilePath = Utf8ToUtf16(dumpFilePath);
    HANDLE hDumpFile = ::CreateFileW(
        wDumpFilePath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (hDumpFile == INVALID_HANDLE_VALUE) {
        return ""; // failed to create file
    }
    // fill dump information
    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = ::GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;
    // write dump information to file
    ::MiniDumpWriteDump(
        ::GetCurrentProcess(),
        ::GetCurrentProcessId(),
        hDumpFile,
        MiniDumpNormal,
        &dumpInfo,
        nullptr,
        nullptr);
    ::CloseHandle(hDumpFile);
    return dumpFilePath;
}

bool DumpCollector::SetDumpFileRoot(const std::string& dumpFileRootPath)
{
    // check if root exist
    std::wstring wDumpFileRootPath = Utf8ToUtf16(dumpFileRootPath);
    DWORD attribute = ::GetFileAttributesW(wDumpFileRootPath.c_str());
    if (attribute == INVALID_FILE_ATTRIBUTES // root directory path not exist
        || (attribute & FILE_ATTRIBUTE_DIRECTORY) == 0) { // not a directory
        return false; // root directory path not exist
    }
    DumpCollector::dumpFileRootPath = dumpFileRootPath;
    return true;
}

// using yyyy-dd-mm.hh.mm:ss.dmp as filename
std::string DumpCollector::GenerateDumpFilePath()
{
    std::string fileName = DEFAULT_DUMP_FILENAME;
    std::time_t timenow = std::time(nullptr);
    std::tm* localnow = std::localtime(&timenow);
    char strtime[FILENAME_MAX] = "";
    if (std::strftime(strtime, sizeof(strtime), "%Y-%m-%d.%H.%M.%S.dmp", localnow) > 0) {
        fileName = std::string(strtime);
    }
    if (DumpCollector::dumpFileRootPath.back() == '\\') {
        return DumpCollector::dumpFileRootPath + fileName;
    }
    return DumpCollector::dumpFileRootPath + '\\' + fileName;
}

// a util function to reduce complexity of DumpWin32StackTrace
static std::vector<StackFrame> WalkStacks(DWORD machineType, HANDLE hProcess,
    HANDLE hThread, STACKFRAME* stackframePtr, CONTEXT* contextPtr)
{
    bool first = true;
    std::vector<StackFrame> stackframes;
    while (::StackWalk(machineType, hProcess, hThread, stackframePtr, contextPtr,
        nullptr, SymFunctionTableAccess, SymGetModuleBase, nullptr)) {
        StackFrame f{};
        f.address = stackframePtr->AddrPC.Offset;

#if _WIN64
        DWORD64 moduleBase = SymGetModuleBase(hProcess, stackframePtr->AddrPC.Offset);
#else
        DWORD moduleBase = SymGetModuleBase(hProcess, stackframePtr->AddrPC.Offset);
#endif
        char moduelBuff[MAX_PATH];
        if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduelBuff, MAX_PATH)) {
            f.module = moduelBuff;
        }
#if _WIN64
        DWORD64 offset = 0;
#else
        DWORD offset = 0;
#endif
        char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + NUM255];
        PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
        symbol->SizeOfStruct = (sizeof IMAGEHLP_SYMBOL) + NUM255;
        symbol->MaxNameLength = NUM254;

        if (::SymGetSymFromAddr(hProcess, stackframePtr->AddrPC.Offset, &offset, symbol)) {
            f.function = symbol->Name;
        } // Failed to resolve address frame.AddrPC.Offset otherwise, default empty

        IMAGEHLP_LINE line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

        DWORD offsetln = 0;
        if (::SymGetLineFromAddr(hProcess, stackframePtr->AddrPC.Offset, &offsetln, &line)) {
            f.file = line.FileName;
            f.line = line.LineNumber;
        } // Failed to resolve line for frame.AddrPC.Offset otherwise, default 0

        if (!first) {
            stackframes.push_back(f);
        }
        first = false;
    }
    return stackframes;
}

std::vector<StackFrame> DumpCollector::DumpWin32StackTrace()
{
#if _WIN64
    DWORD machine = IMAGE_FILE_MACHINE_AMD64;
#else
    DWORD machine = IMAGE_FILE_MACHINE_I386;
#endif
    HANDLE process = GetCurrentProcess();
    HANDLE thread  = GetCurrentThread();

    if (!::SymInitialize(process, nullptr, TRUE)) {
        ERRLOG("Failed to call SymInitialize");
        return std::vector<StackFrame>();
    }

    ::SymSetOptions(SYMOPT_LOAD_LINES);
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_FULL;
    ::RtlCaptureContext(&context);

    STACKFRAME frame {};
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;
#if _WIN64
    frame.AddrPC.Offset = context.Rip;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrStack.Offset = context.Rsp;
#else
    frame.AddrPC.Offset = context.Eip;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrStack.Offset = context.Esp;
#endif

    std::vector<StackFrame> stackframes = WalkStacks(machine, process, thread, &frame, &context);
    ::SymCleanup(process);
    return stackframes;
}


void DumpCollector::Init()
{
    ::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashCollector);
    DumpCollector::Win32CrashHandler = DefaultWin32ApplicationCrashHandler;
}

std::string DumpCollector::GetExceptionInfo()
{
    EXCEPTION_POINTERS* exceptionPtr = static_cast<EXCEPTION_POINTERS*>(DumpCollector::exceptionPtr);
    if (exceptionPtr == nullptr || exceptionPtr->ExceptionRecord == nullptr) {
        return "";
    }
    switch (exceptionPtr->ExceptionRecord->ExceptionCode) {
        CASE_RETURN(EXCEPTION_ACCESS_VIOLATION)
        CASE_RETURN(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
        CASE_RETURN(EXCEPTION_BREAKPOINT)
        CASE_RETURN(EXCEPTION_DATATYPE_MISALIGNMENT)
        CASE_RETURN(EXCEPTION_FLT_DENORMAL_OPERAND)
        CASE_RETURN(EXCEPTION_FLT_DIVIDE_BY_ZERO)
        CASE_RETURN(EXCEPTION_FLT_INEXACT_RESULT)
        CASE_RETURN(EXCEPTION_FLT_INVALID_OPERATION)
        CASE_RETURN(EXCEPTION_FLT_OVERFLOW)
        CASE_RETURN(EXCEPTION_FLT_STACK_CHECK)
        CASE_RETURN(EXCEPTION_FLT_UNDERFLOW)
        CASE_RETURN(EXCEPTION_ILLEGAL_INSTRUCTION)
        CASE_RETURN(EXCEPTION_IN_PAGE_ERROR)
        CASE_RETURN(EXCEPTION_INT_DIVIDE_BY_ZERO)
        CASE_RETURN(EXCEPTION_INT_OVERFLOW)
        CASE_RETURN(EXCEPTION_INVALID_DISPOSITION)
        CASE_RETURN(EXCEPTION_NONCONTINUABLE_EXCEPTION)
        CASE_RETURN(EXCEPTION_PRIV_INSTRUCTION)
        CASE_RETURN(EXCEPTION_SINGLE_STEP)
        CASE_RETURN(EXCEPTION_STACK_OVERFLOW)
    }
    return "Unknown Exception";
}

#endif
