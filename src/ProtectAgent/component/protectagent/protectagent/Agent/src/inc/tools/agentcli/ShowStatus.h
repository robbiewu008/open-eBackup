/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file ShowStatus.h
 * @brief  The implemention about ShowStatus
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef _AGENTCLI_SHOWSTATUS_H_
#define _AGENTCLI_SHOWSTATUS_H_

#include "common/Types.h"

static const mp_string RUNNING_TAG = "RUNNING";
static const mp_string SVN_CONF = "svn";

typedef enum { PROCESS_RDAGENT = 0, PROCESS_NGINX, PROCESS_MONITOR } PROCESS_TYPE;

class ShowStatus {
public:
    static mp_int32 Handle();

private:
    static mp_bool IsStartted(PROCESS_TYPE eType);
    static mp_void ShowSvn();
};

#endif
