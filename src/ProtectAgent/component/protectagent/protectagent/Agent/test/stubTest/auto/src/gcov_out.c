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

// module system test ʱ��rdagent��monitor��������ֻ��ͨ��kill�ķ�ʽ�˳�
// Ĭ�ϵ�kill�˳��󣬲�������.gcda�ļ���������Ҫ���źŽ��в��񣬲�ʹ��exit�����˳�
// ʹ֮����.gcda�ļ�
// ���ļ�����gcov_out.so��ͨ��LD_PRELOAD�����������þ���·��
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