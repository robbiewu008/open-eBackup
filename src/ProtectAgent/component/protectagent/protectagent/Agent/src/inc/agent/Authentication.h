/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Authentication.h
 * @brief  The implemention about VMwareNativeCleanupVddkLibTask
 * @version 1.0.0.0
 * @date 2015-01-19
 * @author xuchong 00300551
 */
#ifndef _AGENT_AUTHENTICATION_H_
#define _AGENT_AUTHENTICATION_H_

#include "common/Types.h"
#include "common/CMpThread.h"
#include <vector>

namespace security {
// 连续失败次数 3次
static const mp_uchar MAX_TRY_TIME = 3;
// 在15分钟内连续失败3次，锁定15分钟 15 * 60
static const mp_int32 LOCKED_TIME  = 900;
// 两次失败登录登录如果超过30分钟，则不计算连续登录失败次数 15 * 60
static const mp_int32 CONTINUOUS_FAILURE_TIME = 900;

typedef struct st_locked_client_info_st {
    mp_bool isLocked;          // 是否锁定，true 锁定，false 未锁定
    mp_int64 failedTimes;      // 连续登录失败次数
    mp_uint64 lastFailedTime;  // 上一次鉴权失败时间
    mp_uint64 lockedTime;      // 锁定时间，非锁定情况下，其值是0
    mp_string strClientIP;     // 客户端ip地址
} locked_client_info;

class Authentication {
public:
    static Authentication& GetInstance()
    {
        return m_instance;
    }
    mp_int32 Init();
    EXTER_ATTACK mp_int32 Auth(mp_string& strClientIP, mp_string& strUsr, mp_string& strPw,
        const mp_string& strClientCertDN);
    ~Authentication()
    {
        CMpThread::DestroyLock(&m_lockedIPListMutex);
    }

private:
    Authentication()
    {
        CMpThread::InitLock(&m_lockedIPListMutex);
    }
    mp_bool IsLocked(const mp_string& strClientIP);
    mp_bool Check(const mp_string& strUsr, const mp_string& strPw) const;
    mp_void Lock(const mp_string& strClientIP);
    mp_void Unlock(const mp_string& strClientIP);
    mp_int32 CheckUserPwd(const mp_string& strClientIP, const mp_string& strUsr, const mp_string& strPw);
    mp_int32 CheckCert(const mp_string& strClientIP, const mp_string& strClientCertDN);

private:
    mp_string m_strUsr;                         // 保存的用户名，sha256
    mp_string m_strPwd;                         // 保存的pw，sha256
    static Authentication m_instance;          // 单例对象
    std::vector<locked_client_info> m_lockedIPList;  // 锁定http客户端ip地址列表
    thread_lock_t m_lockedIPListMutex;          // m_lockedIPList访问互斥锁
};
}
#endif
