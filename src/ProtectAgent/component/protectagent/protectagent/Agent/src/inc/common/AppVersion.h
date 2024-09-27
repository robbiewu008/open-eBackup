#ifndef AGENT_APP_VERSION
#define AGENT_APP_VERSION

#include "common/Types.h"

static const mp_string AGENT_PACKAGE_VERSION = "8.0.0";
static const mp_string AGENT_HDRS_VERSION = "8.0.0";
static const mp_string AGENT_VERSION = "8.0.0";
static const mp_wstring RD_PROVIDER_VERSION = L"1.0.0.0";
static const mp_string COMPILE_TIME = "compile";
static const mp_int64 AGENT_UPDATE_VERSION = 1617181350;

// 变更规则"1.0.0" :
// 1.每次V或者R版本号的变更，第一个数字加1.
// 2.每次C版本号的变更，第二个数字加1.
// 3.每次交互版本时，第三个数字加1.
static const mp_string AGENT_BUILD_NUM = "1.9.0";

inline void AgentVersion()
{
    printf("Copyright 2013-2020 Huawei Technologies Co., Ltd.\n");
    printf("Version     : %s\n", AGENT_VERSION.c_str());
    printf("Build Number: %s\n", AGENT_BUILD_NUM.c_str());
}

#endif  // _AGENT_APP_VERSION_
