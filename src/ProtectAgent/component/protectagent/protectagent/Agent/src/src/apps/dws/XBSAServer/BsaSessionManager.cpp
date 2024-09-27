#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include <atomic>
#include <list>
#include <string>
#include <sstream>
#include <fstream>
#include <libgen.h>
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "securecom/CryptAlg.h"
#include "apps/dws/XBSAServer/BsaSession.h"
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "apps/dws/XBSAServer/BsaIntfAdaptor.h"
#include "apps/dws/XBSAServer/BsaDb.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"

using namespace std;

namespace {
    const mp_long DEFAULT_INIT_TRANSID = 4999;
    const mp_long DEFAULT_INIT_SESSIONID = 9999;
    const mp_long MAX_LONG_ID = LONG_MAX;
    const mp_uint64 DEFAULT_INIT_COPYID = 999;
    const mp_uint64 MAX_COPY_ID = (mp_uint64)~0;
    const mp_string BSA_PROVIDER = "XBSA/ProtectAgent/1.2.1";
    const int INVALID_FILE_SIZE = -1;
    const mp_uint32 QUERY_LIMIT = 128;
    const mp_int32 DEFAUL_MAX_DWS_HOST_NUM = 2048;
    const mp_uint32 MB_TO_BYTE = 1024 * 1024;
}

BsaSessionManager BsaSessionManager::m_instance;

void BsaSessionManager::Split(const mp_string &s, char delimiter, std::vector<mp_string> &tokens)
{
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
}

// env字符串格式说明:key1=val1;key2=val2;
mp_void BsaSessionManager::ParseEnv(const mp_string &in, mp_string out[BSA_ENV_BUTT], const mp_int32 outSize)
{
    (mp_void)outSize;

    DBGLOG("Env:%s", in.c_str());
    std::vector<string> kvList;
    Split(in, ';', kvList);

    for (auto iter : kvList) {
        for (int i = 0; i < BSA_ENV_BUTT; i++) {
            if (iter.find(BsaEnvKeys[i]) == std::string::npos) {
                continue;
            }
            std::size_t pos = iter.find("=");
            if (pos != std::string::npos) {
                out[i] = iter.substr(pos + 1);
            }
        }
    }
}

mp_int32 BsaSessionManager::CheckNewSessionParam(const BsaObjectOwner& objectOwner,
    const mp_string env[BSA_ENV_BUTT], const mp_int32 envSize)
{
    (mp_void)envSize;

    if (!BsaIntfAdaptor::BsaObjectOwnerValid(objectOwner.bsaObjectOwner, MP_FALSE) ||
        !BsaIntfAdaptor::AppObjectOwnerValid(objectOwner.appObjectOwner)) {
        ERRLOG("new session param check fail!bsaObjectOwner(%s) or appObjectOwner(%s) invalid!",
            objectOwner.bsaObjectOwner.c_str(), objectOwner.appObjectOwner.c_str());
        return SetLastErr(BSA_RC_INVALID_OBJECTDESCRIPTOR);
    }

    return BSA_RC_SUCCESS;
}

mp_void BsaSessionManager::NewSession(BSAInitResult& rsp, const BsaObjectOwner& objectOwner, const std::string& env,
                                      int32_t appType)
{
    mp_string parsedEnv[BSA_ENV_BUTT];
    ParseEnv(env, parsedEnv, BSA_ENV_BUTT);

    rsp.handle = 0;
    mp_int32 ret = CheckNewSessionParam(objectOwner, parsedEnv, BSA_ENV_BUTT);
    if (ret != BSA_RC_SUCCESS) {
        rsp.response = ret;
        return;
    }

    mp_long bsaHandle = NewSessionId();
    BsaSession newSession(bsaHandle, static_cast<BSA_AppType>(appType));

    if (newSession.Init(objectOwner, parsedEnv, BSA_ENV_BUTT) != MP_SUCCESS) {
        rsp.response = SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutexSessionMapAndMount);
    m_sessionMap.insert(std::pair<mp_long, BsaSession>(bsaHandle, newSession));

    rsp.handle = bsaHandle;
    rsp.response = BSA_RC_SUCCESS;

    INFOLOG("new session suscc.bsaHandle=%lld,bsaObjectOwner(%s),appObjectOwner(%s)",
        bsaHandle, objectOwner.bsaObjectOwner.c_str(), objectOwner.appObjectOwner.c_str());
}

mp_int32 BsaSessionManager::CloseSession(mp_long bsaHandle)
{
    std::lock_guard<std::mutex> lock(m_mutexSessionMapAndMount);

    mp_int32 ret = BSA_RC_SUCCESS;
    auto iter = m_sessionMap.find(bsaHandle);
    if (iter == m_sessionMap.end()) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return ret; // 找不到session回复SUCCESS
    }

    iter->second.Finit();

    mp_string taskId = iter->second.GetTaskId();
    DwsCacheInfo cacheInfo = iter->second.GetCacheInfo();
    DwsXbsaSpeedInfo info;
    mp_string cachePath = cacheInfo.cacheRepoPath;
    info.totalSizeInMB = m_taskDataSize[taskId].dataSize / MB_TO_BYTE;
    if (info.totalSizeInMB == 0) {
        DBGLOG("task:%s,TotalSizeInMB is 0.no need to write speed file", taskId.c_str());
        return ret;
    }
    std::string jsonStr;
    if (!JsonHelper::StructToJsonString(info, jsonStr)) {
        ERRLOG("Struct to json string failed.");
        return ret;
    }
    DBGLOG("SpeedInfo:%s", jsonStr.c_str());
    WriteSpeedFile(info.totalSizeInMB, jsonStr, cacheInfo);
    m_sessionMap.erase(iter);

    INFOLOG("close session end.bsaHandle=%lld", bsaHandle);
    auto iter_t = m_taskDataSize.find(taskId);
    if (iter_t != m_taskDataSize.end()) {
        std::time_t timestamp = std::time(nullptr);
        iter_t->second.startTime = timestamp;
    }

    return ret;
}

mp_int32 BsaSessionManager::BeginTxn(mp_long bsaHandle)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    return pSession->BeginTxn();
}

mp_int32 BsaSessionManager::EndTxn(mp_long bsaHandle, mp_int32 vote)
{
    if (!BsaIntfAdaptor::VoteValid(vote)) {
        ERRLOG("vote invalid! bsaHandle=%lld,vote=%d", bsaHandle, vote);
        return SetLastErr(BSA_RC_INVALID_VOTE);
    }

    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    return pSession->EndTxn((BSA_Vote)vote);
}

mp_int32 BsaSessionManager::CheckCreateObjParam(const BsaObjectDescriptor& objDesc)
{
    if (!BsaIntfAdaptor::CopyTypeValid(objDesc.copyType, MP_FALSE) ||
        !BsaIntfAdaptor::ObjectTypeValid(objDesc.objectType, MP_FALSE)) {
        ERRLOG("new object param check fail!copyType(%d) or objectType(%d) invalid!",
            objDesc.copyType, objDesc.objectType);
        return SetLastErr(BSA_RC_INVALID_OBJECTDESCRIPTOR);
    }

    if (!BsaIntfAdaptor::BsaObjectOwnerValid(objDesc.objectOwner.bsaObjectOwner) ||
        !BsaIntfAdaptor::AppObjectOwnerValid(objDesc.objectOwner.appObjectOwner)) {
        ERRLOG("new object param check fail!bsaObjectOwner(%s) or appObjectOwner(%s) invalid!",
            objDesc.objectOwner.bsaObjectOwner.c_str(), objDesc.objectOwner.appObjectOwner.c_str());
        return SetLastErr(BSA_RC_INVALID_OBJECTDESCRIPTOR);
    }

    if (!BsaIntfAdaptor::ObjectSpaceNameValid(objDesc.objectName.objectSpaceName) ||
        !BsaIntfAdaptor::PathNameValid(objDesc.objectName.pathName, MP_FALSE)) {
        ERRLOG("new object param check fail!spaceName(%s) or pathName(%s) invalid!",
            objDesc.objectName.objectSpaceName.c_str(), objDesc.objectName.pathName.c_str());
        return SetLastErr(BSA_RC_INVALID_OBJECTDESCRIPTOR);
    }

    if (!BsaIntfAdaptor::ResourceTypeValid(objDesc.resourceType) ||
        !BsaIntfAdaptor::ObjectDescriptionValid(objDesc.objectDescription) ||
        !BsaIntfAdaptor::ObjectInfoValid(objDesc.objectInfo)) {
        ERRLOG("new object param check fail!resourceType(%s) or objectDescription(%s) or objectInfo(%s) invalid!",
            objDesc.resourceType.c_str(), objDesc.objectDescription.c_str(), objDesc.objectInfo.c_str());
        return SetLastErr(BSA_RC_INVALID_OBJECTDESCRIPTOR);
    }

    return BSA_RC_SUCCESS;
}

mp_void BsaSessionManager::CreateObject(mp_long bsaHandle, const BsaObjectDescriptor &objDesc, CreateObjectResult &rsp)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return;
    }

    BsaTransaction *pTrans = pSession->GetTrans();
    if (pTrans == nullptr) {
        ERRLOG("trans not found! bsaHandle=%lld", bsaHandle);
        SetLastErr(BSA_RC_INVALID_HANDLE);
        return;
    }

    mp_int32 ret = CheckCreateObjParam(objDesc);
    if (ret != BSA_RC_SUCCESS) {
        ERRLOG("param check fail in create Object! bsaHandle=%lld", bsaHandle);
        rsp.response = ret;
        return;
    }

    if (pSession->UpdateTaskWhenCreateObject(objDesc) != MP_SUCCESS) {
        rsp.response = SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
        return;
    }

    BsaObjInfo objInfo;
    // 上面GetTrans不为空，这里GetSession肯定也不为空了
    BsaIntfAdaptor::ConvertCreateReqObj(objDesc, objInfo, pSession->GetOwner());

    ret = pTrans->CreateObj(objInfo);
    if (ret != BSA_RC_SUCCESS) {
        rsp.response = ret;
        return;
    }

    objInfo.copyId = NewCopyId();
    pSession->AllocFilesystem(objInfo);
    mp_string mountPath = BsaMountManager::GetInstance().GetMountPath(pSession->GetTaskId(),
                                                                      objInfo.fsDeviceId, objInfo.fsName);
    if (mountPath.empty()) {
        ERRLOG("Get MountPath fail! bsaHandle=%lld,fsDeviceId(%s),fsName(%s)",
            bsaHandle, objInfo.fsDeviceId.c_str(), objInfo.fsName.c_str());
        rsp.response = SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
        return;
    }

    rsp.objectDescriptor = objDesc;
    BsaIntfAdaptor::ConvertCreateRspObj(objInfo, rsp.objectDescriptor, GetSession(bsaHandle)->GetOwner());
    pSession->GenStorePath(objInfo);
    rsp.storePath = GenFullStorePath(mountPath, objInfo.storePath);
    rsp.response = BSA_RC_SUCCESS;

    pTrans->AddCreatList(objInfo);
    INFOLOG("Create obj succ.bsaHandle=%lld,copyId=%llu,storePath(%s)",
        bsaHandle, objInfo.copyId, rsp.storePath.c_str());
}

mp_int32 BsaSessionManager::CheckQueryObjParam(const BsaQueryDescriptor &queryDesc)
{
    if (!BsaIntfAdaptor::CopyTypeValid(queryDesc.copyType) ||
        !BsaIntfAdaptor::ObjectTypeValid(queryDesc.objectType) ||
        !BsaIntfAdaptor::ObjectStatusValid(queryDesc.objectStatus)) {
        ERRLOG("new object param check fail!copyType(%d) or objectType(%d) invalid!",
            queryDesc.copyType, queryDesc.objectType);
        return SetLastErr(BSA_RC_INVALID_OBJECTDESCRIPTOR);
    }

    if (!BsaIntfAdaptor::BsaObjectOwnerValid(queryDesc.objectOwner.bsaObjectOwner) ||
        !BsaIntfAdaptor::AppObjectOwnerValid(queryDesc.objectOwner.appObjectOwner)) {
        ERRLOG("new object param check fail!bsaObjectOwner(%s) or appObjectOwner(%s) invalid!",
            queryDesc.objectOwner.bsaObjectOwner.c_str(), queryDesc.objectOwner.appObjectOwner.c_str());
        return SetLastErr(BSA_RC_INVALID_OBJECTDESCRIPTOR);
    }

    if (!BsaIntfAdaptor::ObjectSpaceNameValid(queryDesc.objectName.objectSpaceName) ||
        !BsaIntfAdaptor::PathNameValid(queryDesc.objectName.pathName, MP_FALSE)) {
        ERRLOG("new object param check fail!spaceName(%s) or pathName(%s) invalid!",
            queryDesc.objectName.objectSpaceName.c_str(), queryDesc.objectName.pathName.c_str());
        return SetLastErr(BSA_RC_INVALID_OBJECTDESCRIPTOR);
    }

    return BSA_RC_SUCCESS;
}

mp_void BsaSessionManager::QueryObject(mp_long bsaHandle, const BsaQueryDescriptor &queryDesc, QueryObjectResult &rsp)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return;
    }
    if (pSession->UpdateTaskWhenQueryObject(queryDesc) != MP_SUCCESS) {
        rsp.response = SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
        return;
    }

    BsaTransaction *pTrans = pSession->GetTrans();
    if (pTrans == nullptr) {
        ERRLOG("trans not found! bsaHandle=%lld", bsaHandle);
        SetLastErr(BSA_RC_INVALID_HANDLE);
        return;
    }

    mp_int32 ret = CheckQueryObjParam(queryDesc);
    if (ret != BSA_RC_SUCCESS) {
        ERRLOG("param check fail in create Object! bsaHandle=%lld", bsaHandle);
        rsp.response = SetLastErr(ret);
        return;
    }

    BsaObjInfo queryCond;
    BsaObjInfo queryReslt;
    // // 上面GetTrans不为空，这里GetSession肯定也不为空了
    BsaIntfAdaptor::ConvertQueryReqObj(bsaHandle, queryDesc, queryCond, GetSession(bsaHandle)->GetOwner());
    ret = pTrans->QueryObj(queryCond, queryReslt);
    if (ret != BSA_RC_SUCCESS) {
        rsp.response = ret;
        return;
    }

    BsaIntfAdaptor::ConvertQueryRspObj(queryReslt, rsp.objectDesc);
    if (pSession->FillQuryRsp(bsaHandle, queryReslt, rsp)) {
        rsp.response = BSA_RC_SUCCESS;
        INFOLOG("Query obj success.bsaHandle=%lld,copyId=%llu,storePath(%s)",
            bsaHandle, queryReslt.copyId, rsp.storePath.c_str());
    } else {
        rsp.response = SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
        ERRLOG("Fill Query rsp failed.bsaHandle=%lld,copyId=%llu", bsaHandle, queryReslt.copyId);
    }
}

mp_void BsaSessionManager::GetObject(mp_long bsaHandle, const BsaObjectDescriptor &objDesc, GetObjectResult &rsp)
{
    BsaTransaction *pTrans = GetTrans(bsaHandle);
    if (pTrans == nullptr) {
        ERRLOG("trans not found! bsaHandle=%lld", bsaHandle);
        rsp.response = SetLastErr(BSA_RC_INVALID_HANDLE);
        return;
    }
    // block当前未使用
    rsp.response = pTrans->GetObj();
}

mp_void BsaSessionManager::GetNextObj(mp_long bsaHandle, GetNextQueryObjectResult &rsp)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return;
    }

    BsaTransaction *pTrans = pSession->GetTrans();
    if (pTrans == nullptr) {
        ERRLOG("trans not found! bsaHandle=%lld", bsaHandle);
        SetLastErr(BSA_RC_INVALID_HANDLE);
        return;
    }

    BsaObjInfo queryReslt;
    mp_int32 ret = pTrans->GetNextObj(queryReslt);
    if (ret != BSA_RC_SUCCESS) {
        rsp.response = ret;
        return;
    }

    BsaIntfAdaptor::ConvertQueryRspObj(queryReslt, rsp.objectDesc);

    if (pSession->FillQuryRsp(bsaHandle, queryReslt, rsp)) {
        rsp.response = BSA_RC_SUCCESS;
        INFOLOG("Query obj success.bsaHandle=%lld,copyId=%llu,storePath(%s)",
            bsaHandle, queryReslt.copyId, rsp.storePath.c_str());
    } else {
        rsp.response = SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
        ERRLOG("Fill Query rsp failed.bsaHandle=%lld,copyId=%llu", bsaHandle, queryReslt.copyId);
    }
}

mp_int32 BsaSessionManager::DeleteObject(mp_long bsaHandle, const BsaUInt64& copyId)
{
    BsaTransaction *pTrans = GetTrans(bsaHandle);
    if (pTrans == nullptr) {
        ERRLOG("trans not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    mp_uint64 copyId64 = BsaIntfAdaptor::BsaU64ToU64(copyId);
    INFOLOG("try to delete obj.bsaHandle=%lld,copyId(%llu)", bsaHandle, copyId64);
    return pTrans->DelObj(copyId64);
}

mp_int32 BsaSessionManager::SendData(mp_long bsaHandle, const BsaDataBlock32& dataBlock)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    BsaTransaction *pTrans = pSession->GetTrans();
    if (pTrans == nullptr) {
        ERRLOG("trans not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    std::lock_guard<std::mutex> lock(m_mutexTaskDataSize);
    mp_string tempTaskId = pSession->GetTaskId();
    auto iter = m_taskDataSize.find(tempTaskId);
    if (iter == m_taskDataSize.end()) {
        std::time_t timestamp = std::time(nullptr);
        struct taskInfo info = {0, timestamp};
        m_taskDataSize[tempTaskId] = info;
    }
    m_taskDataSize[tempTaskId].dataSize += dataBlock.numBytes;
    uint64_t totalDataSize = m_taskDataSize[tempTaskId].dataSize;
    DBGLOG("taskId:%s,TotalDataSize=%lu,numBytes=%lu.", tempTaskId.c_str(), totalDataSize, dataBlock.numBytes);

    return pTrans->SendData();
}

mp_int32 BsaSessionManager::GetData(mp_long bsaHandle, BsaDataBlock32& dataBlock)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    BsaTransaction *pTrans = pSession->GetTrans();
    if (pTrans == nullptr) {
        ERRLOG("trans not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    std::lock_guard<std::mutex> lock(m_mutexTaskDataSize);
    mp_string tempTaskId = pSession->GetTaskId();
    auto iter = m_taskDataSize.find(tempTaskId);
    if (iter == m_taskDataSize.end()) {
        std::time_t timestamp = std::time(nullptr);
        struct taskInfo info = {0, timestamp};
        m_taskDataSize[tempTaskId] = info;
    }
    m_taskDataSize[tempTaskId].dataSize += dataBlock.numBytes;
    uint64_t totalDataSize = m_taskDataSize[tempTaskId].dataSize;
    DBGLOG("taskId:%s,TotalDataSize=%lu,numBytes=%lu.", tempTaskId.c_str(), totalDataSize, dataBlock.numBytes);

    return pTrans->GetData();
}

mp_int32 BsaSessionManager::EndData(mp_long bsaHandle, const BsaUInt64 &estimatedSize, int64_t size)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    BsaTransaction *pTrans = pSession->GetTrans();
    if (pTrans == nullptr) {
        ERRLOG("trans not found! bsaHandle=%lld", bsaHandle);
        return SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    std::lock_guard<std::mutex> lock(m_mutexTaskDataSize);
    mp_string tempTaskId = pSession->GetTaskId();
    auto iter = m_taskDataSize.find(tempTaskId);
    if (iter == m_taskDataSize.end()) {
        std::time_t timestamp = std::time(nullptr);
        struct taskInfo info = {0, timestamp};
        m_taskDataSize[tempTaskId] = info;
    }

    m_taskDataSize[tempTaskId].dataSize += size;
    uint64_t totalDataSize = m_taskDataSize[tempTaskId].dataSize;
    DBGLOG("taskId:%s,TotalDataSize=%lu,size=%lu.", tempTaskId.c_str(), totalDataSize, size);

    // 只有备份对象时estimatedSize才是有效的，恢复对象时是无效的，不能更新
    if (estimatedSize.left != INVALID_FILE_SIZE && estimatedSize.right != INVALID_FILE_SIZE) {
        mp_uint64 estimatedSize64 = BsaIntfAdaptor::BsaU64ToU64(estimatedSize);
        INFOLOG("End data.bsaHandle=%lld,estimatedSize64(%llu)", bsaHandle, estimatedSize64);
        if (!pTrans->UpdateEstimatedSize(estimatedSize64)) {
            ERRLOG("UpdateEstimatedSize failed.bsaHandle=%lld", bsaHandle);
            return SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
        }
    }

    return pTrans->EndData();
}

BsaSession *BsaSessionManager::GetSession(mp_long bsaHandle)
{
    std::lock_guard<std::mutex> lock(m_mutexSessionMapAndMount);
    auto iter = m_sessionMap.find(bsaHandle);
    return ((iter != m_sessionMap.end()) ? &iter->second : nullptr);
}

BsaTransaction *BsaSessionManager::GetTrans(mp_long bsaHandle)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return nullptr;
    }

    return pSession->GetTrans();
}
namespace {
    const map<mp_int32, mp_string> BSA_ERR_CODE_MAP = {
        {BSA_RC_ABORT_SYSTEM_ERROR, "aborted due to an server internal system error"},
        {BSA_RC_ACCESS_FAILURE, "access failure"},
        {BSA_RC_AUTHENTICATION_FAILURE, "authentication failure"},
        {BSA_RC_BUFFER_TOO_SMALL, "buffer too small"},
        {BSA_RC_INVALID_CALL_SEQUENCE, "invalid call sequence"},
        {BSA_RC_INVALID_COPYID, "invalid copyId"},
        {BSA_RC_INVALID_DATABLOCK, "invalid datablock"},
        {BSA_RC_INVALID_ENV, "invalid env param"},
        {BSA_RC_INVALID_HANDLE, "invalid handle"},
        {BSA_RC_INVALID_OBJECTDESCRIPTOR, "invalid object descriptor"},
        {BSA_RC_INVALID_QUERYDESCRIPTOR, "invalid query descriptor"},
        {BSA_RC_INVALID_VOTE, "invalid vote type"},
        {BSA_RC_NO_MATCH, "no macth object"},
        {BSA_RC_NO_MORE_DATA, "no more data"},
        {BSA_RC_NULL_ARGUMENT, "some argument is null"},
        {BSA_RC_OBJECT_NOT_FOUND, "object not found"},
        {BSA_RC_TRANSACTION_ABORTED, "transaction is aborted"},
        {BSA_RC_VERSION_NOT_SUPPORTED, "xbsa version not supported"},
    };
}

mp_string BsaSessionManager::GetLastErr()
{
    auto iter =  BSA_ERR_CODE_MAP.find(m_lastErr);
    return ((iter != BSA_ERR_CODE_MAP.end()) ? iter->second : "unkown error");
}

mp_string BsaSessionManager::GetProvider()
{
    mp_string provider = "";
    mp_int32 ret = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_XBSA_PROVIDER, provider);
    if (ret != MP_SUCCESS || provider.empty()) {
        ERRLOG("get %s.%s cfg fail!provider(%s)",
            CFG_BACKUP_SECTION.c_str(), CFG_XBSA_PROVIDER.c_str(), provider.c_str());
        return BSA_PROVIDER;
    }

    return provider;
}

mp_int32 BsaSessionManager::SetLastErr(mp_int32 errNo)
{
    m_lastErr = errNo;
    return errNo;
}

mp_string BsaSessionManager::GenFullStorePath(const mp_string &mountPath, const mp_string &storePath)
{
    return mountPath + "/" + storePath;
}

mp_string BsaSessionManager::GetBsaDbFilePath(mp_long bsaHandle)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return "";
    }
    DwsCacheInfo cacheInfo = pSession->GetCacheInfo();
    mp_string dbFile = cacheInfo.metaRepoPath + "/meta/" + cacheInfo.copyId + "/objectmeta/"
        + cacheInfo.hostKey + "/" + cacheInfo.hostKey + ".db";
    DBGLOG("Db file path:%s", dbFile.c_str());
    return dbFile;
}

template<typename T>
T BsaSessionManager::InitId(const T defaultVal, const T maxVal)
{
    mp_uint64 num;
    mp_int32 ret = GetRandom(num);
    if (ret != MP_SUCCESS) {
        num = defaultVal;
    }
    return (num % maxVal);
}

template<typename T>
T BsaSessionManager::NewId(T &last)
{
    T newId = ++last;
    if (newId <= 0) {
        newId = 1;
        last = 1;
    }
    return newId;
}

mp_long BsaSessionManager::NewSessionId()
{
    static mp_long lastBsaHandle = InitId(DEFAULT_INIT_SESSIONID, MAX_LONG_ID);
    std::lock_guard<std::mutex> lock(m_mutexBsaHandle);
    return NewId(lastBsaHandle);
}

mp_long BsaSessionManager::NewTransId()
{
    static mp_long lastTransId = InitId(DEFAULT_INIT_TRANSID, MAX_LONG_ID);
    std::lock_guard<std::mutex> lock(m_mutexTransId);
    return NewId(lastTransId);
}

mp_uint64 BsaSessionManager::NewCopyId()
{
    static std::atomic<mp_uint64> lastCopyId{InitId(DEFAULT_INIT_COPYID, MAX_COPY_ID)};
    return lastCopyId++;
}

const DwsCacheInfo &BsaSessionManager::GetSessionCacheInfo(mp_long bsaHandle)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return DwsCacheInfo();
    }
    return pSession->GetCacheInfo();
}

int32_t BsaSessionManager::GetSessionAppType(mp_long bsaHandle)
{
    BsaSession *pSession = GetSession(bsaHandle);
    if (pSession == nullptr) {
        ERRLOG("session not found! bsaHandle=%lld", bsaHandle);
        return BSA_AppType::BSA_UNKNOWN;
    }
    return pSession->GetAppType();
}

std::map<mp_string, taskInfo> BsaSessionManager::GetTaskDataSize()
{
    return m_taskDataSize;
}

mp_int32 BsaSessionManager::GetTaskCacheInfo(const mp_string &taskId, DwsCacheInfo &cacheInfo)
{
    std::lock_guard<std::mutex> lock(m_mutexSessionMapAndMount);
    for (auto &iter : m_sessionMap) {
        if (taskId == iter.second.GetTaskId()) {
            DBGLOG("Get task id %s cache info. ", taskId.c_str());
            cacheInfo = iter.second.GetCacheInfo();
            return MP_SUCCESS;
        }
    }
    WARNLOG("There is no session belongs to task id(%s) running.", taskId.c_str());
    // 清理挂载信息
    BsaMountManager::GetInstance().ClearMountInfoByTaskId(taskId);
    return MP_FAILED;
}

mp_void BsaSessionManager::ClearTaskDataSizeByTaskId(const mp_string &taskId)
{
    std::lock_guard<std::mutex> lock(m_mutexTaskDataSize);
    auto iter = m_taskDataSize.find(taskId);
    if (iter != m_taskDataSize.end()) {
        m_taskDataSize.erase(iter);
        INFOLOG("Clear task data size map for taskid(%s).", taskId.c_str());
    }
}

mp_void BsaSessionManager::WriteSpeedFile(uint64_t totalSize, const std::string &output, const DwsCacheInfo &cacheInfo)
{
    mp_string speedFile;
    if (!GetSpeedFilePath(cacheInfo, speedFile)) {
        WARNLOG("get speedFile path failed!");
        return;
    }
    std::ofstream outfile(speedFile, std::ios::trunc | std::ios_base::binary);
    if (!outfile.is_open()) {
        // speed file directory will be deleted in post job,so this could happen in normal case.
        WARNLOG("Open speed file(%s) failed!errno=%d.", speedFile.c_str(), errno);
        return;
    }
    outfile.write(output.c_str(), output.length());
    if (outfile.fail()) {
        ERRLOG("Write speed file(%s) fail!totalSizeInMB=%llu,errno=%d.", speedFile.c_str(), totalSize, errno);
    } else {
        INFOLOG("Write speed file(%s) success!totalSizeInMB=%llu.", speedFile.c_str(), totalSize);
    }
}

mp_bool BsaSessionManager::GetSpeedFilePath(const DwsCacheInfo &cacheInfo, mp_string &speedFilePath)
{
    mp_string hostKey = cacheInfo.hostKey;
    if (hostKey.empty()) {
        WARNLOG("cacheInfo hostKey is empty!");
        return false;
    }
    std::vector<mp_string> hostKeys;
    Split(hostKey, ',', hostKeys);
    mp_string hostIp;
    mp_string listenPort;
    mp_int32 iRet = CIP::GetListenIPAndPort(hostIp, listenPort);
    if (iRet != MP_SUCCESS) {
        WARNLOG("get hostIp failed!");
        return false;
    }
    if (std::find(hostKeys.begin(), hostKeys.end(), hostIp) != hostKeys.end()) {
        speedFilePath = cacheInfo.cacheRepoPath + "/tmp/" + cacheInfo.copyId + "/speed/" + hostKey + "/xbsa_speed_" +
                        hostIp + ".txt";
        return true;
    }
    return false;
}