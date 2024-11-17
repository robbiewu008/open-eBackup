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
#include "AppStackTracer.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <ctime>

#include <sys/types.h>
#ifndef _AIX
#include <execinfo.h>
#endif
#include <unistd.h>
#include <cxxabi.h>
#include "securec.h"

namespace {
constexpr int NUM_64 = 64;
constexpr int NUM_32 = 32;
static void *g_btarray[NUM_32];
static std::string g_logPath;
const std::string LOG_FILE_NAME = "Plugin_lle.log";
}

using SignalHandlerFunc = void (*)(int, siginfo_t *, void *);

static void InstallSignalHandler(int signum, SignalHandlerFunc handler)
{
    struct sigaction sa;
    memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
    sigaction(signum, &sa, nullptr);
}

static void UninstallSignalHandler(int signum)
{
    struct sigaction sa;
    (void)sigaction(signum, nullptr, &sa);
    sa.sa_handler = SIG_DFL;
    (void)sigaction(signum, &sa, nullptr);
}

static std::string HandleFunctionName(std::string line)
{
    auto firstPos = line.find_first_of("(");
    auto lastPos = line.find_first_of("+");
    if (lastPos == std::string::npos) {
        lastPos = line.find_first_of(")");
    }
    std::string rawName = line.substr(firstPos + 1, lastPos - firstPos - 1);
    int status;
    char *newName =  abi::__cxa_demangle(rawName.c_str(), nullptr, nullptr, &status);
    if (newName != nullptr) {
        return std::string(newName);
    } else {
        return rawName;
    }
}

AppStackTracer::AppStackTracer(std::string path)
{
    g_logPath = path;
}

AppStackTracer::~AppStackTracer()
{
    UninstallHandler();
}

void AppStackTracer::Init()
{
    InstallHandler();

    std::string filePath = g_logPath + "/" + LOG_FILE_NAME;
    std::ofstream stream(filePath.c_str(), std::ios::app);

    std::time_t timenow = std::time(nullptr);
    char strtime[NUM_64] = { 0 };
    std::tm *localnow = std::localtime(&timenow);
    if (localnow != nullptr) {
        if (std::strftime(strtime, sizeof(strtime), "[%Y-%b-%d %H:%M:%S]", localnow) > 0) {
            stream << "\n\n\n" << std::string(strtime) << "[INFO]" <<"[program start]"<< std::endl;
        }
    }
    stream.close();
}

void AppStackTracer::InstallHandler()
{
    InstallSignalHandler(SIGSEGV, static_cast<SignalHandlerFunc>(AppStackTracer::SignalHandler));
    InstallSignalHandler(SIGFPE, static_cast<SignalHandlerFunc>(AppStackTracer::SignalHandler));
    InstallSignalHandler(SIGABRT, static_cast<SignalHandlerFunc>(AppStackTracer::SignalHandler));
}

void AppStackTracer::UninstallHandler()
{
    UninstallSignalHandler(SIGSEGV);
    UninstallSignalHandler(SIGFPE);
    UninstallSignalHandler(SIGABRT);
}

void AppStackTracer::SignalHandler(int signum, siginfo_t *siginfo, void *ucontext)
{
    if ((signum != SIGSEGV) && (signum != SIGFPE) && (signum != SIGABRT)) {
        return;
    }
    ucontext;
    siginfo;

    std::string filePath = g_logPath + "/" + LOG_FILE_NAME;
    std::ofstream stream(filePath.c_str(), std::ios::app);

    std::time_t timenow = std::time(nullptr);
    char strtime[NUM_64] = { 0 };
    std::tm *localnow = std::localtime(&timenow);
    if (localnow != nullptr) {
        if (std::strftime(strtime, sizeof(strtime), "[%Y-%b-%d %H:%M:%S]", localnow) > 0) {
            stream << strtime << "[ERROR][ signal is: " << signum << "]" << std::endl;
        }
    }

    std::vector<std::string> bt;
    size_t size = backtrace(g_btarray, NUM_32);
    char **strings = backtrace_symbols(g_btarray, size);

    if (strings != nullptr) {
        for (int i = 0; i < size; i++) {
            if (strings[i] != nullptr) {
                bt.emplace_back(strings[i]);
            }
        }

        for (auto line : bt) {
            stream << line << "[" + HandleFunctionName(line) + "]" << "\n";
        }
    }
    stream.close();
    UninstallSignalHandler(SIGABRT);
    std::abort();
    return;
}
