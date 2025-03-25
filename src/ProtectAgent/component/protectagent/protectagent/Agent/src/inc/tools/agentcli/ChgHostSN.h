/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file ChgHostSN.h
 * @brief  Contains function declarations ChgHostSN Operations
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef _AGENTCLI_CHGHOSTSN_H_
#define _AGENTCLI_CHGHOSTSN_H_

#include "common/Types.h"

#include <vector>

static const mp_uchar HOSTSN_LEN = 32;
#define INPUT_HOSTSN_CHG "Please Input HostSN:"

class ChgHostSN {
public:
    static mp_int32 Handle();

private:
    static mp_string m_ChghostsnFile;

private:
    static mp_int32 CheckUserPwd();
    static mp_int32 GetHostSNNum(mp_string& strInput);
    static mp_int32 ChownHostSn(mp_string& strInput);
    static mp_int32 ModifyHostSN(std::vector<mp_string>& vecResult, mp_string& strInput);
};

#endif  // _AGENTCLI_CHGHOSTSN_H_
