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
#ifndef EBK_BASE_EBKTRACEPOINT_H
#define EBK_BASE_EBKTRACEPOINT_H

#include <csignal>
#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "common/JsonUtils.h"
#include "common/JsonHelper.h"

const int MAX_INIT_ACTIVE_NUM = 512;

enum TPStatus { TP_STAT_UNKNOWN = 0, TP_STAT_ACTIVE, TP_STAT_DEACTIVE };

enum RETURN_CODE_TRACE_POINT {
    TRACE_POINT_ERROR = -1,
    TRACE_POINT_SUCCESS = 0,
    TRACE_POINT_PARAM_ERR,
    TRACE_POINT_NOT_EXIST
};

enum TPType {
    TP_TYPE_CALLBACK = 0,  // 回调
    TP_TYPE_RESET,         // 复位
    TP_TYPE_BUTT
};

enum TPInitActiveStep {
    TP_INIT_ACTIVE_NAME = 0,
    TP_INIT_ACTIVE_ALIVE = 1,
    TP_INIT_ACTIVE_PARAM = 2
};

typedef void (*FnTpCommon)(std::string userparam, size_t param_num, ...);

struct TP {
    TP()
    {
        uiPid = 0;
        blockTimes = 0;
    }
    std::string szName;   // tracepoint的名字。
    std::string szDesc;   // tracepoint的描述字段。
    uint32_t uiPid;       // 模块Id
    std::atomic<int32_t> iActive;      // 用于识别该tracepoint 是否激活.
    int32_t type;         // tracepoint类型：callback/reset/pause.
    std::atomic<int32_t> aliveNum;    // travepoint有效次数，默认为0，0为无限次
    std::atomic<int32_t> timeCalled;  // tracepoint被调用次数统计
    FnTpCommon fnHook;    // tracepoint上的回调函数。
    std::string szParam;  // 用于存放回调函数的自定义参数。
    std::atomic<int32_t> blockTimes;  // 跳过blockTimes次后开始执行, 0为立即执行
};

struct TPInit {
    TPInit() : alive(true), aliveNum(0), blockTimes(0) {}
    virtual ~TPInit() {}

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(alive, Alive)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(tpName, TpName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(aliveNum, AliveNum)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(userParam, UserParam)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(blockTimes, BlockTimes)
    END_SERIAL_MEMEBER;

    bool alive;
    std::string tpName;
    uint32_t aliveNum;
    std::string userParam;
    uint32_t blockTimes;
};

class EbkTracePoint {
public:
    static EbkTracePoint& GetInstance();
    int32_t GetTP(const std::string& name, std::shared_ptr<TP>& tracepoint);
    int32_t RegTP(const std::string& name, const std::string& desc, TPType type, FnTpCommon fnHook, uint32_t pid = 0);
    int32_t DeleteTP(const std::string& name);
    int32_t ActiveTPImpl(const std::string &name, uint32_t aliveNum, const std::string &userParam, uint32_t blockTimes,
                         bool ignoreConfFile = false);
    int32_t DeactiveTPImpl(const std::string& name);
    int32_t ExportTP(std::string& exportFilePath);
    void ParseConfigFile(const std::string& filename);
    std::vector<std::string> Split(const std::string& str, const std::string& delim);

private:
    EbkTracePoint();
    ~EbkTracePoint(){};
    std::map<std::string, TPInit> m_initActiveTPs;
    int32_t ParseLine(std::string line);
    std::unordered_map<std::string, std::shared_ptr<TP>> m_hashMap;
    std::mutex m_mutex;
};

#ifdef EBK_TRACE_POINT

// 宏定义
#define TP_START(name, ...)                                                                                            \
    do {                                                                                                               \
        std::shared_ptr<TP> tp = nullptr;                                                                              \
        TP* tpItem = nullptr;                                                                                          \
        EbkTracePoint& m_tp = EbkTracePoint::GetInstance();                                                            \
        if (m_tp.GetTP(name, tp) != 0) {                                                                              \
            HCP_Log(ERR, "TracePoint") << "tracepoint " << name << " not registered" << HCPENDLOG;                    \
        } else {                                                                                                       \
            tpItem = tp.get();                                                                                         \
        }                                                                                                              \
        if (tpItem != nullptr && tpItem->iActive == TP_STAT_ACTIVE && tpItem->blockTimes > 0) {                        \
            tpItem->blockTimes--;                                                                                      \
        } else if (tpItem != nullptr && tpItem->iActive == TP_STAT_ACTIVE && tpItem->type == TP_TYPE_CALLBACK) {       \
            tpItem->fnHook(tpItem->szParam, __VA_ARGS__);                                                              \
            tpItem->timeCalled++;                                                                                      \
            if (tpItem->aliveNum > 0 && --(tpItem->aliveNum) == 0) {                                                   \
                m_tp.DeactiveTPImpl(name);                                                                            \
            }                                                                                                          \
        } else {                                                                                                       \
            if (tpItem != nullptr && tpItem->iActive == TP_STAT_ACTIVE && tpItem->type == TP_TYPE_RESET) {             \
                  std::raise(SIGKILL);                                                                                 \
            }

#define TP_NOPARAM_START(name)                                                                                         \
    do {                                                                                                               \
        std::shared_ptr<TP> tp = nullptr;                                                                              \
        TP* tpItem = nullptr;                                                                                          \
        EbkTracePoint& m_tp = EbkTracePoint::GetInstance();                                                            \
        if (m_tp.GetTP(name, tp) != 0) {                                                                              \
            HCP_Log(ERR, "TracePoint") << "tracepoint " << name << " not registered" << HCPENDLOG;                    \
        } else {                                                                                                       \
            tpItem = tp.get();                                                                                         \
        }                                                                                                              \
        if (tpItem != nullptr && tpItem->iActive == TP_STAT_ACTIVE && tpItem->blockTimes > 0) {                        \
            tpItem->blockTimes--;                                                                                      \
        } else if (tpItem != nullptr && tpItem->iActive == TP_STAT_ACTIVE && tpItem->type == TP_TYPE_CALLBACK) {       \
            tpItem->fnHook(tpItem->szParam, 0);                                                                        \
            tpItem->timeCalled++;                                                                                      \
            if (tpItem->aliveNum > 0 && --(tpItem->aliveNum) == 0) {                                                   \
                m_tp.DeactiveTPImpl(name);                                                                            \
            }                                                                                                          \
        } else {                                                                                                       \
            if (tpItem != nullptr && tpItem->iActive == TP_STAT_ACTIVE && tpItem->type == TP_TYPE_RESET) {             \
                raise(SIGSEGV);                                                                                        \
            }
#define TP_END                                                                                                         \
        }                                                                                                              \
    }                                                                                                                  \
    while (0);
#else

#define TP_START(name, ...)
#define TP_NOPARAM_START(name)
#define TP_END

#endif

#endif //EBK_BASE_EBKTRACEPOINT_H

void NullHook(std::string userparam, ...);

// make all params become -1
// e.g. SsizeReturnFailHook("", 2, &i, &j)
void SsizeReturnFailHook(std::string userparam, size_t paramNum, ...);

// make all params become -1
void IntReturnFailHook(std::string userparam, size_t paramNum, ...);
// make all params become false
void BoolReturnFailHook(std::string userparam, size_t paramNum, ...);

void PauseThreadHook(std::string userparam, uint32_t time);

void ResetReturnFailHook(std::string userparam, size_t paramNum, ...);

void ModifyConfFile(const std::string& tpName);

void ParseLineData(std::string str, const std::string& tpName, std::string &data);

void ModifyLineData(const std::string& fileName, const std::string& tpName);

std::vector<std::string> stringSplit(const std::string& str, char delim);

