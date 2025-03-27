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
#ifndef AGENT_LOG_ROTATER_H
#define AGENT_LOG_ROTATER_H

#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <thread>
#include "common/Types.h"
#include "common/JsonUtils.h"

class AGENT_API LogRotater {
public:
    ~LogRotater();
    static LogRotater& GetInstance();
    mp_void Init(const mp_string& strFilePath, const mp_string& strFileName);
private:
    LogRotater();

    static LogRotater m_instance;
    // 日志最大大小
    mp_int32 m_iMaxSize;
    // 日志个数
    mp_int32 m_iLogCount;
    // 日志保留时间
    mp_int32 m_iLogKeepTime;
    // 日志目录
    mp_string m_strFilePath;
    // 日志名称
    mp_string m_strFileName;

    // 日志转储线程
    std::unique_ptr<std::thread> m_logRotateThread;
    // 日志转储线程停止条件变量
    std::condition_variable m_logRotateThreadCV;
    // 日志转储线程停止标志
    std::atomic<bool> m_stopFlag { false };
    // 日志转储线程停止条件变量互斥锁
    std::mutex m_logRotateThreadCVLock;

    mp_void LogRotateThread();
    mp_int32 HandlerAccess(mp_string &strFileName);
    mp_int32 LogRotateInner();
    mp_int32 LogRotate();

};

#endif // AGENT_LOG_ROTATER_H