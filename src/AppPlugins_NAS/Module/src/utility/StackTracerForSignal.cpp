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
#include "StackTracerForSignal.h"
#include <execinfo.h>
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include "securec.h"

using SignalHandlerT = void (*)(int, siginfo_t *, void *);

namespace {
constexpr int NUM_64 = 64;
constexpr int NUM_32 = 32;
}  // namespace

namespace Module {
static void InstallSignalHandler(int signum, SignalHandlerT handler)
{
    struct sigaction sa;
    memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
    (void)sigaction(signum, &sa, nullptr);
}

static void UninstallSignalHandler(int signum)
{
    struct sigaction sa;
    (void)sigaction(signum, nullptr, &sa);
    sa.sa_handler = SIG_DFL;
    (void)sigaction(signum, &sa, nullptr);
}

void *StackTracerForSignal::btarray[NUM_32];
StackTracerForSignal::StackTracerForSignal()
{
    InstallSignalHandler();
    // note: stand I/O and error I/O has been redirected to filename by script, so we use printf() will print to
    // filename instead of screen.

    std::time_t timenow = std::time(nullptr);
    char strtime[NUM_64] = { 0 };

    std::tm *localnow = std::localtime(&timenow);
    if (localnow != nullptr) {
        if (std::strftime(strtime, sizeof(strtime), "[%d-%b-%Y %H:%M:%S]", localnow)) {
            std::cout << "\n\n\n" << strtime << std::endl;
        }
    }
    backtrace(btarray, NUM_32);
    std::cout << "[start program] at:" << strtime << std::endl;
}

StackTracerForSignal::~StackTracerForSignal()
{
    UninstallSignalHandler();
}

void StackTracerForSignal::InstallSignalHandler()
{
    Module::InstallSignalHandler(SIGSEGV, StackTracerForSignal::SignalHandler);
    Module::InstallSignalHandler(SIGFPE, StackTracerForSignal::SignalHandler);
    Module::InstallSignalHandler(SIGABRT, StackTracerForSignal::SignalHandler);
}

void StackTracerForSignal::UninstallSignalHandler()
{
    Module::UninstallSignalHandler(SIGSEGV);
    Module::UninstallSignalHandler(SIGFPE);
    Module::UninstallSignalHandler(SIGABRT);
}

void StackTracerForSignal::SignalHandler(int signum, siginfo_t *siginfo, void *ucontext)
{
    (void)siginfo;   // unused parameters
    (void)ucontext;  // unused parameters
    if ((signum != SIGSEGV) && (signum != SIGFPE) && (signum != SIGABRT)) {
        return;
    }

    std::size_t size = backtrace(btarray, NUM_32);
    std::cout << "Error: signal " << signum << std::endl;
    backtrace_symbols_fd(btarray, size, STDOUT_FILENO);

    Module::UninstallSignalHandler(SIGABRT);
    std::abort();
    return;
}
}  // namespace Module
