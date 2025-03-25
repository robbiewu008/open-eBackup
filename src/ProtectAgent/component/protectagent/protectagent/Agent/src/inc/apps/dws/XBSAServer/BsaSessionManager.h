/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @Description manage XBSA sessions.
 * @Create 2021-05-18
 * @Author wuchuan wwx563312
 */
#ifndef _BSA_SESSION_MANAGER_H_
#define _BSA_SESSION_MANAGER_H_

#include <atomic>
#include <map>
#include <mutex>
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/Ip.h"
#include "xbsa/xbsa.h"
#include "apps/dws/XBSAServer/xbsa_types.h"
#include "apps/dws/XBSAServer/BsaSession.h"
#include "apps/dws/XBSAServer/BsaMountManager.h"
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "apps/dws/XBSAServer/BsaObjManager.h"
#include "apps/dws/XBSACom/TSSLSocketFactoryPassword.h"
#include "apps/dws/XBSAServer/DwsTaskInfoParser.h"

struct taskInfo {
    uint64_t dataSize;
    std::time_t startTime;
};

class BsaSessionManager {
public:
    static BsaSessionManager &GetInstance()
    {
        return m_instance;
    }
    ~BsaSessionManager()
    {
        m_sessionMap.clear();
    }

    mp_void NewSession(BSAInitResult &rsp, const BsaObjectOwner &objectOwner, const mp_string &env, int32_t appType);
    mp_int32 CloseSession(mp_long bsaHandle);
    mp_int32 BeginTxn(mp_long bsaHandle);
    mp_int32 EndTxn(mp_long bsaHandle, mp_int32 vote);
    mp_void CreateObject(mp_long bsaHandle, const BsaObjectDescriptor &objDesc, CreateObjectResult &rsp);
    mp_void QueryObject(mp_long bsaHandle, const BsaQueryDescriptor &queryDesc, QueryObjectResult &rsp);
    mp_void GetObject(mp_long bsaHandle, const BsaObjectDescriptor &objDesc, GetObjectResult &rsp);
    mp_void GetNextObj(mp_long bsaHandle, GetNextQueryObjectResult &rsp);
    mp_int32 DeleteObject(mp_long bsaHandle, const BsaUInt64 &copyId);
    mp_int32 SendData(mp_long bsaHandle, const BsaDataBlock32 &dataBlock);
    mp_int32 GetData(mp_long bsaHandle, BsaDataBlock32& dataBlock);
    mp_int32 EndData(mp_long bsaHandle, const BsaUInt64 &estimatedSize, int64_t size);
    mp_string GetLastErr();
    mp_string GetProvider();

    mp_int32 SetLastErr(mp_int32 errNo);
    mp_long NewTransId();
    mp_uint64 NewCopyId();
    mp_string GetBsaDbFilePath(mp_long bsaHandle);
    const DwsCacheInfo &GetSessionCacheInfo(mp_long bsaHandle);
    int32_t GetSessionAppType(mp_long bsaHandle);
    std::map<mp_string, taskInfo> GetTaskDataSize();
    mp_int32 GetTaskCacheInfo(const mp_string &taskId, DwsCacheInfo &cacheInfo);
    mp_void ClearTaskDataSizeByTaskId(const mp_string &taskId);

private:
    BsaSessionManager() : m_lastErr(BSA_RC_SUCCESS) { };
    mp_string GenFullStorePath(const mp_string &mountPath, const mp_string &storePath);

    BsaSession *GetSession(mp_long bsaHandle);
    BsaTransaction *GetTrans(mp_long bsaHandle);

    mp_int32 CheckNewSessionParam(const BsaObjectOwner& objectOwner,
        const mp_string env[BSA_ENV_BUTT], const mp_int32 envSize);
    mp_int32 CheckCreateObjParam(const BsaObjectDescriptor& objDesc);
    mp_int32 CheckQueryObjParam(const BsaQueryDescriptor &queryDesc);

    mp_void ParseEnv(const mp_string &in, mp_string out[BSA_ENV_BUTT], const mp_int32 outSize);
    template<typename T>
    T InitId(const T defaultVal, const T maxVal);
    template<typename T>
    T NewId(T &last);
    mp_long NewSessionId();
    mp_void WriteSpeedFile(uint64_t totalSize, const std::string &output, const DwsCacheInfo &cacheInfo);
    mp_bool GetSpeedFilePath(const DwsCacheInfo &cacheInfo, mp_string &speedFilePath);
    void Split(const mp_string &s, char delimiter, std::vector<mp_string> &tokens);

    static BsaSessionManager m_instance;
    mp_int32 m_lastErr;
    std::mutex m_mutexBsaHandle;
    std::mutex m_mutexTransId;
    std::mutex m_mutexSessionMapAndMount;
    std::map<mp_long, BsaSession> m_sessionMap; // 会话列表
    std::mutex m_mutexTaskDataSize;
    std::map<mp_string, taskInfo> m_taskDataSize; // 每个任务累计传输的数据量，以任务id为关键字
};

#endif // _BSA_SESSION_MANAGER_H_