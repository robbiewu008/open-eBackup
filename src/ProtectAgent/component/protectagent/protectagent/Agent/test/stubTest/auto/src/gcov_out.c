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

// module system test 时，rdagent和monitor两个程序都只能通过kill的方式退出
// 默认的kill退出后，不会生成.gcda文件，所以需要对信号进行捕获，并使用exit函数退出
// 使之生成.gcda文件
// 本文件生成gcov_out.so并通过LD_PRELOAD环境变量设置绝对路径
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
void sighandler(int signo)
{
    exit(signo);
}
__attribute__ ((constructor))
void ctor()
{
    int sigs[] = {
        SIGILL, SIGFPE, SIGABRT, SIGBUS,
        SIGSEGV, SIGHUP, SIGINT, SIGQUIT,
        SIGTERM
    };
    int i;
    struct sigaction sa;
    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    for(i = 0; i < sizeof(sigs)/sizeof(sigs[0]); ++i) {
        if (sigaction(sigs[i], &sa, NULL) == -1) {
            perror("Could not set signal handler");
        }
    }
}