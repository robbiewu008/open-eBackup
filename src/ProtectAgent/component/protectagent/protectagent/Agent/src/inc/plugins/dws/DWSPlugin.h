/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DWSPlugin.h
 * @brief  Contains function declarations for DWSPlugin
 * @version 1.0.0
 * @date 2021-05-21
 * @author lixiang lws1045600
 */
#ifndef __AGENT_DWS_PLUGIN_H__
#define __AGENT_DWS_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/dws/XBSAServer/DwsTaskCommonDef.h"

class DWSPlugin : public CServicePlugin {
public:
    DWSPlugin();
    ~DWSPlugin();

#ifdef WIN32
    static DWORD WINAPI ScheduleThread(mp_void *object);
#else
    static mp_void *ScheduleThread(mp_void *object);
#endif

    mp_int32 Init(std::vector<mp_uint32> &cmds);
    mp_int32 DoAction(CDppMessage &reqMsg, CDppMessage &rspMsg);

private:
    mp_void ScheduleThreadRun();
    mp_void WriteSpeedFile(uint64_t totalSize, const std::string &output, const DwsCacheInfo &cacheInfo);

private:
    thread_id_t m_scheduleThreadId;
    volatile mp_bool m_schedulethreadExitFlag;
    mp_uint32 m_scheduleInterval;
};

#endif // __AGENT_DWS_PLUGIN_H__
