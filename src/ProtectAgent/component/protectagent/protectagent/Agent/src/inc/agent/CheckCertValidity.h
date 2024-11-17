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
#ifndef _AGENTCLI_CHECK_CERT_VALIDITY_H_
#define _AGENTCLI_CHECK_CERT_VALIDITY_H_

#include "message/curlclient/CurlHttpClient.h"
#include "common/Defines.h"
#include "common/JsonUtils.h"
#include "common/CMpThread.h"
#include "common/Log.h"
#include "openssl/ssl.h"

class CheckCertValidity {
public:
    mp_int32 Init();
    static CheckCertValidity& GetInstance()
    {
        return m_Instance;
    }
    ~CheckCertValidity();

private:
    CheckCertValidity()
    {
        m_bNeedExit = MP_FALSE;
    }
    CheckCertValidity(const CheckCertValidity &other);

// 线程启动函数
#ifdef WIN32
    static DWORD WINAPI CheckCertHandle(LPVOID param);
#else
    EXTER_ATTACK static mp_void* CheckCertHandle(mp_void* param);
#endif

    mp_int32 GetParameters();
    mp_int32 GetCertEndTime();
    time_t Asn1ToTimeT(ASN1_TIME *time);
    mp_int32 SendAlarm();
    mp_int32 SendClearAlarm();
    mp_int32 NewAlarmRecord();
    mp_int32 SendMessage(mp_bool isAlarmMsg);
    mp_int32 InitMessage(mp_bool isAlarmMsg, const mp_string &PmIp, HttpRequest &req);
    mp_int32 BuildMessageBody(Json::Value &reqBody);
    mp_int32 GetRequestPara();
    mp_int32 SendRequest(const HttpRequest& req);

private:
    static CheckCertValidity m_Instance;
    volatile mp_bool m_bNeedExit;
    thread_id_t m_CheckCertValidityThread;
    mp_int32 m_CheckCertCycle;
    mp_int32 m_WarningTime;
    time_t m_EndTime;
    mp_time m_NowTime;
    mp_time m_RemainTime;
    mp_int32 m_SecureChannel;
    std::vector<mp_string> m_PmIpVec;
    mp_string m_PmIP;
    mp_string m_PmPort;
    mp_string m_HostIp;
    mp_string m_AlarmIDString;
    mp_string m_AlarmID;
    mp_string m_CaInfo;
    mp_string m_SslCert;
    mp_string m_SslKey;
};

#endif