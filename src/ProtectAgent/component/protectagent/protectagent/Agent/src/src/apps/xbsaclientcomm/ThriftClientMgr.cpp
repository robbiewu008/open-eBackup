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
#include "xbsaclientcomm/ThriftClientMgr.h"
#include "securecom/SecureUtils.h"
#include "securecom/CryptAlg.h"
#include "common/Path.h"
#include "common/Ip.h"
#include "common/ConfigXmlParse.h"
#include "common/StackTracer.h"
#include "common/JsonUtils.h"
#include "common/JsonHelper.h"
#include "common/File.h"
#include "xbsaclientcomm/DataConversion.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/THeaderProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>

using namespace apache::thrift;
using namespace apache::thrift::async;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
namespace {
const std::string EMPY_STRING = "";
const std::string DEFAULT_IP = "127.0.0.1";
const std::string DEFAULT_XBSA_PATH = "/DataBackup/ProtectClient/interfaces/xbsa";
const std::string DEFAULT_CERT_HOSTNAME = "DataBackup-AGENT";
const std::string DEFAULT_CA_FILE_PATH = "thrift/client/ca.crt.pem";
const std::string DEFAULT_CRT_FILE_PATH = "thrift/client/client.crt.pem";
const std::string DEFAULT_KEY_FILE_PATH = "thrift/client/client.pem";
const std::string DEFAULT_ALGORITHM_SUITE = "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256";
const std::string DEFAULT_ROOT_PATH = "/usr/openv";
const int DEFAULT_HOSTPORT = 59560;
const int DATA_TIME_OUT = 30;
const int INVALID_FILE_SIZE = -1;
const int THRIFT_MAX_RETRY_TIMES = 3;
const mp_string IIF_CONF_FILE = "informix.conf";
const mp_string IIF_CONF_VERSION = "version";
};  // namespace

ThriftClientMgr::ThriftClientMgr()
{
    this->Init();
}

ThriftClientMgr::~ThriftClientMgr()
{}

int32_t ThriftClientMgr::InitModule()
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    mp_string customInstallPath;
    CIP::GetHostEnv("DATA_BACKUP_AGENT_HOME", customInstallPath);
    std::string strRootPath = customInstallPath + DEFAULT_XBSA_PATH;
    CPath::GetInstance().SetRootPath(strRootPath);
    CLogger::GetInstance().Init(XBSA_CLIENT_LOG_NAME.c_str(), CPath::GetInstance().GetXBSALogPath());
    int32_t iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetXBSAConfFilePath(AGENT_XML_CONF));
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init conf file %s failed.", AGENT_XML_CONF.c_str());
    }
    return iRet;
}

void ThriftClientMgr::Init()
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    int32_t iRet = this->InitModule();
    if (iRet != MP_SUCCESS) {
        return;
    }

    int32_t sslFlag = 1;
    this->Init(sslFlag);
}

EXTER_ATTACK int32_t ThriftClientMgr::Init(bool ssl)
{
    if (ssl) {
        try {
            std::string hostName = EMPY_STRING;
            std::string algorithm_suite = EMPY_STRING;
            std::string caPath   = CPath::GetInstance().GetXBSAConfFilePath(DEFAULT_CA_FILE_PATH);
            std::string certPath = CPath::GetInstance().GetXBSAConfFilePath(DEFAULT_CRT_FILE_PATH);
            std::string keyPath  = CPath::GetInstance().GetXBSAConfFilePath(DEFAULT_KEY_FILE_PATH);

            if (InitCrypt(KMC_ROLE_TYPE_MASTER) != MP_SUCCESS) {
                ERRLOG("Init KMC failed.");
                return MP_FAILED;
            }

            int32_t iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION,
                CFG_ALGORITHM_SUITE, algorithm_suite);
            if (iRet != MP_SUCCESS || algorithm_suite.empty()) {
                ERRLOG("Get Algorithm suite failed.");
                algorithm_suite = DEFAULT_ALGORITHM_SUITE;
            }

            m_socketFactory = std::make_shared<TSSLSocketFactoryPassword>();
            m_socketFactory->ciphers(algorithm_suite);
            m_socketFactory->loadCertificate(certPath.c_str());
            m_socketFactory->overrideDefaultPasswordCallback();
            m_socketFactory->loadPrivateKey(keyPath.c_str());
            m_socketFactory->loadTrustedCertificates(caPath.c_str());
            m_socketFactory->authenticate(true);
        } catch (std::exception& tx) {
            ERRLOG("ERROR: %s", tx.what());
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

std::shared_ptr<apache::thrift::transport::TSocket> ThriftClientMgr::CreateSock(bool ssl)
{
    std::shared_ptr<TSocket> socket;
    if (ssl) {
        try {
            std::string hostName = DEFAULT_CERT_HOSTNAME;
            std::string certPath = CPath::GetInstance().GetXBSAConfFilePath(DEFAULT_CRT_FILE_PATH);

            SecureCom::GetHostFromCert(certPath, hostName);
            socket = m_socketFactory->createSocket(hostName, DEFAULT_HOSTPORT);
        } catch (std::exception& tx) {
            ERRLOG("ERROR: %s", tx.what());
            return nullptr;
        }
    } else {
        socket = std::make_shared<TSocket>(DEFAULT_IP, DEFAULT_HOSTPORT);
    }

    return socket;
}

std::shared_ptr<BSAServiceClient> ThriftClientMgr::GetClient()
{
    ThriftClientInfo client;
    auto socket = CreateSock(true);
    if (socket == nullptr) {
        ERRLOG("Create socket instance failed.");
        return nullptr;
    }
    client.m_transport = std::make_shared<TFramedTransport>(socket);
    if (client.m_transport == nullptr) {
        ERRLOG("Create transport instance failed.");
        return nullptr;
    }
    auto byProtocol = std::make_shared<TBinaryProtocol>(client.m_transport);
    if (byProtocol == nullptr) {
        ERRLOG("Create protocol instance failed.");
        return nullptr;
    }
    client.m_client = std::make_unique<BSAServiceClient>(byProtocol);
    if (client.m_client == nullptr) {
        ERRLOG("Create client instance failed.");
        return nullptr;
    }
    try {
        if (client.m_transport.get()) {
            client.m_transport->open();
        }
        DBGLOG("Create new client success.");
        return client.m_client;
    } catch (TTransportException& ex) {
        ERRLOG("Transport failed: %s", ex.what());
    } catch (const std::exception& ex) {
        ERRLOG("Standard C++ Exception. %s", ex.what());
    } catch (...) {
        ERRLOG("Unknown exception.");
    }

    return nullptr;
}

void ThriftClientMgr::SetThriftRetryInterval(uint32_t interval)
{
    m_thriftRetryInterval = interval;
}

void ThriftClientMgr::SetThriftRetryTimes(int32_t times)
{
    m_thriftRetryTimes = times;
}

bool ThriftClientMgr::IsInfomrix11()
{
        return m_isInformix11;
}

template<typename R, typename T, typename... ARGS1, typename... ARGS2>
inline R CallPointer(T* ptr, R (T::*f)(ARGS1...), ARGS2 &&... args)
{
    return (ptr->*f)(std::forward<ARGS2>(args) ...);
}

template<typename R, typename T, typename ... ARGS1, typename ... ARGS2>
inline R Call(std::shared_ptr<T> ptr, R (T::*f)(ARGS1...), ARGS2 &&... args)
{
    auto t = ptr.get();
    return CallPointer(t, f, std::forward<ARGS2>(args) ...);
}

template<typename T, typename ... ARGS1, typename ... ARGS2>
bool ThriftClientMgr::RetryThriftCall(std::shared_ptr<T> ptr,
    void (T::*MemFunc)(ARGS1 ...), long bsaHandle, ARGS2 &&... args)
{
    try {
        Call(ptr, MemFunc, std::forward<ARGS2>(args) ...);
        DBGLOG("Call XBSA RPC req success,bsaHandle=%ld.", bsaHandle);
        return true;
    } catch (apache::thrift::transport::TTransportException& ex) {
        ERRLOG("TTransportException. %s,bsaHandle=%ld.", ex.what(), bsaHandle);
    } catch (apache::thrift::protocol::TProtocolException& ex) {
        ERRLOG("TProtocolException. %s,bsaHandle=%ld.", ex.what(), bsaHandle);
    } catch (apache::thrift::TApplicationException& ex) {
        ERRLOG("TApplicationException. %s,bsaHandle=%ld.", ex.what(), bsaHandle);
    } catch (const std::exception& ex) {
        ERRLOG("Standard C++ Exception. %s,bsaHandle=%ld.", ex.what(), bsaHandle);
    } catch (...) {
        ERRLOG("Unknown exception.bsaHandle=%ld.", bsaHandle);
    }
    return false;
}

template<typename T, typename ... ARGS1, typename ... ARGS2>
bool ThriftClientMgr::ProtectServiceCall(void (T::*MemFunc)(ARGS1...), long bsaHandle, ARGS2 &&... args)
{
    int retry_times = m_thriftRetryTimes;
    while (retry_times--) {
        auto pClient = GetClient();
        if (pClient == nullptr) {
            ERRLOG("Get thrift client failed.bsaHandle=%ld.", bsaHandle);
            usleep(m_thriftRetryInterval);
            continue;
        }
        if (RetryThriftCall(pClient, MemFunc, bsaHandle, std::forward<ARGS2>(args)...)) {
            return true;
        }
        INFOLOG("RetryThriftCall retry, retry_times=%d, bsaHandle=%ld", retry_times, bsaHandle);
        usleep(m_thriftRetryInterval);
    }
    return false;
}

int32_t ThriftClientMgr::BSABeginTxnMgr(long bsaHandle)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif

    CallResult callRet;
    callRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSABeginTxn, bsaHandle, callRet, bsaHandle);
    if (callRet.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, callRet.response);
        return callRet.response;
    }

    INFOLOG("BSABeginTxn Success.bsaHandle=%ld.", bsaHandle);
    return callRet.response;
}

int32_t ThriftClientMgr::BSACreateObjectMgr(
    long bsaHandle, BSA_ObjectDescriptor* objectDescriptorPtr, BSA_DataBlock32* dataBlockPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif

    CreateObjectResult createObjectPtr;
    BsaObjectDescriptor objectDescriptor;
    DataConversion::ConvertObjectDescriptorIn(objectDescriptorPtr, objectDescriptor);
    createObjectPtr.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSACreateObject, bsaHandle, createObjectPtr, bsaHandle, objectDescriptor);
    if (createObjectPtr.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, createObjectPtr.response);
        return createObjectPtr.response;
    }

    // 返回上层参数
    DataConversion::ConvertdataBlockOut(createObjectPtr.dataBlock, dataBlockPtr);
    if (!DataConversion::ConvertObjectDescriptorOut(objectDescriptor, objectDescriptorPtr)) {
        ERRLOG("[bsaHandle:%ld] covert data fail!", bsaHandle);
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    {
        BsaHandleContext context;
        context.workingObj.status = 0;
        context.workingObj.storgePath = createObjectPtr.storePath;
        context.workingObj.objectStatus = objectDescriptorPtr->objectStatus;
        std::lock_guard<std::mutex> lk(m_Mutex);
        m_writeMap[bsaHandle] = std::move(context);
    }

    m_writer.SetWriteStatus(FileIoStatus::CLOSE);
    INFOLOG("BSACreateObject Success.bsaHandle=%ld.response=%d,storePath=%s.",
        bsaHandle, createObjectPtr.response, createObjectPtr.storePath.c_str());
    return createObjectPtr.response;
}

int32_t ThriftClientMgr::BSADeleteObjectMgr(long bsaHandle, BSA_UInt64 copyId)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    BsaUInt64 mCopyId;
    mCopyId.left = copyId.left;
    mCopyId.right = copyId.right;

    CallResult callRet;
    callRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSADeleteObject, bsaHandle, callRet, bsaHandle, mCopyId);
    if (callRet.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, callRet.response);
        return callRet.response;
    }

    INFOLOG("BSADeleteObject Success.bsaHandle=%ld.", bsaHandle);
    return callRet.response;
}

bool ThriftClientMgr::GetEstimatedSize(long bsaHandle, BsaUInt64 &estimatedSize)
{
    mp_string filePath;
    {
        std::lock_guard<std::mutex> lk(m_Mutex);
        auto iter = m_writeMap.find(bsaHandle);
        if (iter == m_writeMap.end()) {
            ERRLOG("[bsaHandle:%ld] The handle is not found in the map.", bsaHandle);
            return false;
        }
        filePath = iter->second.workingObj.storgePath;
    }

    uint32_t fileSize = 0;
    if (CMpFile::FileSize(filePath.c_str(), fileSize) != MP_SUCCESS) {
        ERRLOG("[bsaHandle:%ld] get file size failed,filePath=%s.", bsaHandle, filePath.c_str());
        return false;
    }
    INFOLOG("bsaHandle=%ld,filePath=%s,fileSize=%u.", bsaHandle, filePath.c_str(), fileSize);
    DataConversion::U64ToBsaU64(fileSize, estimatedSize);
    return true;
}

int32_t ThriftClientMgr::GetInformixVersion()
{
    mp_string iifConfPath =  CPath::GetInstance().GetXBSAConfFilePath(IIF_CONF_FILE);
    INFOLOG("The path of the Informix configuration file is %s.", iifConfPath.c_str());
    std::vector<mp_string> vecOutput;
    mp_int32 ret = CMpFile::ReadFile(iifConfPath, vecOutput);
    if (ret != MP_SUCCESS || vecOutput.empty()) {
        ERRLOG("Read %s failed!", iifConfPath.c_str());
        return MP_FAILED;
    }

    Json::Value iifConf;
    if (!JsonHelper::JsonStringToJsonValue(vecOutput[0], iifConf)) {
        ERRLOG("The configure file format is incorrect.");
        return MP_FAILED;
    }

    mp_string iifVersion;
    ret = CJsonUtils::GetJsonString(iifConf, IIF_CONF_VERSION, iifVersion);
    if (ret != MP_SUCCESS) {
        ERRLOG("The version field does not exist.");
        return MP_FAILED;
    }
    INFOLOG("The current infomrix version is %s.", iifVersion.c_str());
    if (iifVersion.find("11") == 0) {
        m_isInformix11 = true;
    }
    return MP_SUCCESS;
}

int32_t ThriftClientMgr::BSAEndDataMgr(long bsaHandle)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    m_writer.Close();
    m_reader.Close();

    BsaUInt64 estimatedSize;
    estimatedSize.left = INVALID_FILE_SIZE;
    estimatedSize.right = INVALID_FILE_SIZE;
    if (m_writer.GetWriteStatus() == FileIoStatus::OPEN && !GetEstimatedSize(bsaHandle, estimatedSize)) {
        ERRLOG("[bsaHandle:%ld] get file size failed.", bsaHandle);
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    m_writer.SetWriteStatus(FileIoStatus::CLOSE);
    m_reader.SetReadStatus(FileIoStatus::CLOSE);

    CallResult callRet;
    callRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
    int64_t totalDataSize = m_totalDataSize;
    ProtectServiceCall(&BSAServiceClient::BSAEndData, bsaHandle, callRet, bsaHandle, estimatedSize, totalDataSize);
    if (callRet.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, callRet.response);
        return callRet.response;
    }
    m_totalDataSize = 0;

    INFOLOG("BSAEndData Success.bsaHandle=%ld,totalDataSize=%ld.", bsaHandle, totalDataSize);
    return callRet.response;
}

int32_t ThriftClientMgr::BSAEndTxnMgr(long bsaHandle, BSA_Vote vote)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif

    int32_t iVote;
    if (vote == BSA_Vote::BSA_Vote_ABORT) {
        iVote = static_cast<int32_t>(BSA_Vote::BSA_Vote_ABORT);
    } else if (vote == BSA_Vote::BSA_Vote_COMMIT) {
        iVote = static_cast<int32_t>(BSA_Vote::BSA_Vote_COMMIT);
    } else {
        ERRLOG("Invalid vote=%d,bsaHandle=%ld.", vote, bsaHandle);
        return BSA_RC_INVALID_VOTE;
    }

    CallResult callRet;
    callRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAEndTxn, bsaHandle, callRet, bsaHandle, iVote);
    if (callRet.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, callRet.response);
        return callRet.response;
    }

    INFOLOG("BSAEndTxn Success.bsaHandle=%ld.", bsaHandle);
    return callRet.response;
}

int32_t ThriftClientMgr::BSAGetEnvironmentMgr(long bsaHandle, BSA_ObjectOwner* objectOwner, char** ptr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    return BSA_RC_SUCCESS;
}

int32_t ThriftClientMgr::BSAGetLastErrorMgr(BSA_UInt32 *sizePtr, char *errorCodePtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    long bsaHandle = 0;
    GetLastErrorResult lastError;
    lastError.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAGetLastError, bsaHandle, lastError, *sizePtr);
    if (lastError.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, lastError.response);
        return lastError.response;
    }

    if (lastError.bufferSize > *sizePtr) {
        ERRLOG("input size=%u too small, need size=%u.", *sizePtr, lastError.bufferSize);
        *sizePtr = lastError.bufferSize;
        return BSA_RC_BUFFER_TOO_SMALL;
    }

    (void)DataConversion::CopyStrToChar(lastError.detailError, errorCodePtr, *sizePtr);

    INFOLOG("BSAGetLastError Success.bsaHandle=%ld.", bsaHandle);
    return lastError.response;
}

int32_t ThriftClientMgr::BSAGetNextQueryObjectMgr(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    GetNextQueryObjectResult nextQueryObject;
    nextQueryObject.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAGetNextQueryObject, bsaHandle, nextQueryObject, bsaHandle);

    if (!DataConversion::ConvertObjectDescriptorOut(nextQueryObject.objectDesc, objectDescriptorPtr)) {
        ERRLOG("[bsaHandle:%ld] covert data fail!", bsaHandle);
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    SaveObjIntoReadMap(bsaHandle, nextQueryObject);
    
    INFOLOG("BSAGetNextQueryObject Success,bsaHandle=%ld.response:%d, storePath=%s.",
        bsaHandle, nextQueryObject.response, nextQueryObject.storePath.c_str());
    return nextQueryObject.response;
}

bool ThriftClientMgr::SetWorkingObj(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    std::lock_guard<std::mutex> lk(m_Mutex);
    auto iter = m_readMap.find(bsaHandle);
    if (iter == m_readMap.end()) {
        ERRLOG("Query basHandle=%ld failed.", bsaHandle);
        return false;
    }
    auto obj = iter->second.queryMap.find(objectDescriptorPtr->objectName.pathName);
    if (obj == iter->second.queryMap.end()) {
        ERRLOG("Query basHandle=%ld,objectName=%s failed.", bsaHandle, objectDescriptorPtr->objectName.pathName);
        return false;
    }
    iter->second.workingObj = obj->second;
    INFOLOG("Set working obj, storgePath=%s.", obj->second.storgePath.c_str());
    return true;
}

int32_t ThriftClientMgr::BSAGetObjectMgr(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr,
                                         BSA_DataBlock32 *dataBlockPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    INFOLOG("bsaHandle=%ld,pathName=%s.", bsaHandle, objectDescriptorPtr->objectName.pathName);

    GetObjectResult dataBlock;
    BsaObjectDescriptor objectDescriptor;
    DataConversion::ConvertObjectDescriptorIn(objectDescriptorPtr, objectDescriptor);
    dataBlock.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAGetObject, bsaHandle, dataBlock, bsaHandle, objectDescriptor);
    if (dataBlock.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, dataBlock.response);
        return dataBlock.response;
    }

    dataBlockPtr->bufferLen = 0;
    dataBlockPtr->bufferPtr = NULL;
    dataBlockPtr->headerBytes = 0;
    dataBlockPtr->numBytes = 0;
    dataBlockPtr->shareOffset = 0;
    dataBlockPtr->shareId = 0;

    m_reader.SetReadStatus(FileIoStatus::CLOSE);
    if (!SetWorkingObj(bsaHandle, objectDescriptorPtr)) {
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    INFOLOG("BSAGetObject Success.bsaHandle=%ld.", bsaHandle);
    return dataBlock.response;
}

int32_t ThriftClientMgr::BSAInitMgr(long* bsaHandlePtr, BSA_SecurityToken* tokenPtr,
                                    BSA_ObjectOwner* objectOwnerPtr, char** environmentPtr, const int32_t appType)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    m_appType = appType;
    INFOLOG("Application type: %d.", m_appType);
    long bsaHandle = 0;
    BsaObjectOwner objectOwner;
    BSAInitResult initRet;
    objectOwner.bsaObjectOwner = objectOwnerPtr->bsa_ObjectOwner;
    objectOwner.appObjectOwner = objectOwnerPtr->app_ObjectOwner;

    DBGLOG("bsaObjectOwner=%s; appObjectOwner=%s", objectOwner.bsaObjectOwner.c_str(),
        objectOwner.appObjectOwner.c_str());

    // 当前支持的环境变量和版本
    std::string environment = "BSA_API_VERSION=1.1.0;";
    initRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAInit, bsaHandle, initRet, objectOwner, environment, appType);
    if (initRet.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, initRet.response);
        return initRet.response;
    }

    if (m_appType == BSA_AppType::BSA_INFORMIX) {
        if (GetInformixVersion() != MP_SUCCESS) {
            ERRLOG("Failed to obtain the Informix version.");
            return false;
        }
    }
    *bsaHandlePtr = initRet.handle;

    INFOLOG("BSAInit Success.bsaHandle=%ld.", initRet.handle);
    return initRet.response;
}

int32_t ThriftClientMgr::BSAQueryApiVersionMgr(BSA_ApiVersion *apiVersionPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif

    long bsaHandle = 0;
    QueryApiVersionResult apiVersion;
    apiVersion.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAQueryApiVersion, bsaHandle, apiVersion);
    if (apiVersion.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, apiVersion.response);
        return apiVersion.response;
    }

    apiVersionPtr->issue = apiVersion.version.issue;
    apiVersionPtr->level = apiVersion.version.level;
    apiVersionPtr->version = apiVersion.version.version;

    INFOLOG("BSAQueryApiVersion Success.response:%d:", apiVersion.response);
    return apiVersion.response;
}

template<typename T>
void ThriftClientMgr::SaveObjIntoReadMap(long bsaHandle, const T &objectDescriptor)
{
    BsaObjContext objCtx;
    objCtx.status = 0;
    objCtx.storgePath = objectDescriptor.storePath;
    objCtx.getDataType = objectDescriptor.getDataType;
    objCtx.archiveBackupId = objectDescriptor.archiveBackupId;
    objCtx.fsID = objectDescriptor.fsID;
    objCtx.archiveServerIp = objectDescriptor.archiveServerIp;
    objCtx.archiveServerPort = objectDescriptor.archiveServerPort;
    objCtx.archiveOpenSSL = objectDescriptor.archiveOpenSSL;
    objCtx.stFileInfo.offset = 0;
    objCtx.stFileInfo.fileSize = 0;
    objCtx.stFileInfo.readEnd = 0;

    auto &objName = objectDescriptor.objectDesc.objectName.pathName;
    std::lock_guard<std::mutex> lk(m_Mutex);
    auto iter = m_readMap.find(bsaHandle);
    if (iter == m_readMap.end()) {
        BsaHandleContext context;
        ArchiveStreamService streamService;
        context.streamService = streamService;
        context.workingObj = objCtx;
        if (!objCtx.storgePath.empty()) {
            context.queryMap[objName] = std::move(objCtx);
            INFOLOG("Add object into query map.bsaHandle=%ld,pathName=%s.", bsaHandle, objName.c_str());
        }
        m_readMap[bsaHandle] = std::move(context);
        INFOLOG("Add handle into query map.bsaHandle=%ld.", bsaHandle);
    } else {
        iter->second.workingObj = objCtx;
        if (!objCtx.storgePath.empty()) {
            iter->second.queryMap[objName] = std::move(objCtx);
            INFOLOG("Add object into query map.bsaHandle=%ld,pathName=%s.", bsaHandle, objName.c_str());
        }
    }
}

int32_t ThriftClientMgr::BSAQueryObjectMgr(
    long bsaHandle, BSA_QueryDescriptor* queryDescriptorPtr, BSA_ObjectDescriptor* objectDescriptorPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    QueryObjectResult objectDescriptor;
    BsaQueryDescriptor queryDescriptor;
    DataConversion::ConvertQueryObjectIn(queryDescriptorPtr, queryDescriptor);
    objectDescriptor.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAQueryObject, bsaHandle, objectDescriptor, bsaHandle, queryDescriptor);

    if (!DataConversion::ConvertObjectDescriptorOut(objectDescriptor.objectDesc, objectDescriptorPtr)) {
        ERRLOG("[bsaHandle:%ld] covert data fail!", bsaHandle);
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    SaveObjIntoReadMap(bsaHandle, objectDescriptor);

    INFOLOG("BSAQueryObject Success.bsaHandle=%ld.response=%d, storePath=%s.",
        bsaHandle, objectDescriptor.response, objectDescriptor.storePath.c_str());
    return objectDescriptor.response;
}

int32_t ThriftClientMgr::IifBSAQueryServiceProviderMgr(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    long bsaHandle = 0;
    QueryServiceProviderResult queryRet;
    queryRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAQueryServiceProvider, bsaHandle, queryRet, *sizePtr);
    if (queryRet.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, queryRet.response);
        return queryRet.response;
    }

    if (queryRet.retSize > *sizePtr) {
        ERRLOG("input size=%u too small, need size=%u.", *sizePtr, queryRet.retSize);
        *sizePtr = queryRet.retSize;
        return BSA_RC_BUFFER_TOO_SMALL;
    }
    queryRet.providerPtr = "IBM/PSM/12.10.FC12";
    (void)DataConversion::CopyStrToChar(queryRet.delimiter, delimiter, *sizePtr);
    (void)DataConversion::CopyStrToChar(queryRet.providerPtr, providerPtr, *sizePtr);

    INFOLOG("BSAQueryServiceProvider Success.response:%d:", queryRet.response);
    return queryRet.response;
}

int32_t ThriftClientMgr::BSAQueryServiceProviderMgr(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    long bsaHandle = 0;
    QueryServiceProviderResult queryRet;
    queryRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSAQueryServiceProvider, bsaHandle, queryRet, *sizePtr);
    if (queryRet.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, queryRet.response);
        return queryRet.response;
    }

    if (queryRet.retSize > *sizePtr) {
        ERRLOG("Input size=%u too small, need size=%u.", *sizePtr, queryRet.retSize);
        *sizePtr = queryRet.retSize;
        return BSA_RC_BUFFER_TOO_SMALL;
    }
    (void)DataConversion::CopyStrToChar(queryRet.delimiter, delimiter, *sizePtr);
    (void)DataConversion::CopyStrToChar(queryRet.providerPtr, providerPtr, *sizePtr);

    INFOLOG("BSAQueryServiceProvider Success.response:%d:", queryRet.response);
    return queryRet.response;
}

int32_t ThriftClientMgr::BSATerminateMgr(long bsaHandle)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    CallResult callRet;
    callRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
    ProtectServiceCall(&BSAServiceClient::BSATerminate, bsaHandle, callRet, bsaHandle);
    if (callRet.response != BSA_RC_SUCCESS) {
        ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, callRet.response);
        return callRet.response;
    }

    std::lock_guard<std::mutex> lk(m_Mutex);
    auto iter = m_readMap.find(bsaHandle);
    if (iter != m_readMap.end()) {
        m_readMap.erase(iter);
        INFOLOG("[bsaHandle:%ld] has been erased from read map.", bsaHandle);
    }
    INFOLOG("BSATerminate Success.bsaHandle=%ld.", bsaHandle);
    return callRet.response;
}

int32_t ThriftClientMgr::BSASendDataMgr(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif
    m_totalDataSize += dataBlockPtr->numBytes;
    uint64_t totalDataSize = m_totalDataSize;
    DBGLOG("[bsaHandle:%ld] bufferLen: %u, headerBytes: %u numBytes: %u totalDataSize: %lu", bsaHandle,
        dataBlockPtr->bufferLen, dataBlockPtr->headerBytes, dataBlockPtr->numBytes, totalDataSize);

    // 第一次通知server端，后续每隔大约30s通知一次状态
    if ((m_writer.GetWriteStatus() == FileIoStatus::CLOSE) ||
        ((m_writer.GetNowTime() - m_writer.GetLastTime()) > DATA_TIME_OUT)) {
        BsaDataBlock32 bsaDataBlock32;
        DataConversion::ConvertdataBlockIn(dataBlockPtr, bsaDataBlock32);
        bsaDataBlock32.numBytes = m_totalDataSize;
        CallResult callRet;
        callRet.response = BSA_RC_ABORT_SYSTEM_ERROR;
        ProtectServiceCall(&BSAServiceClient::BSASendData, bsaHandle, callRet, bsaHandle, bsaDataBlock32);
        if (callRet.response != BSA_RC_SUCCESS) {
            ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, callRet.response);
            return callRet.response;
        }
        m_totalDataSize = 0;
        m_writer.UpdateLastTime();
        INFOLOG("BSASendData Success.bsaHandle=%ld.", bsaHandle);
    }

    int32_t iRet = SendData(bsaHandle, dataBlockPtr);
    if (iRet != MP_SUCCESS) {
        ERRLOG("[bsaHandle:%ld] Failed to send data.", bsaHandle);
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    m_writer.SetWriteStatus(FileIoStatus::OPEN);
    return BSA_RC_SUCCESS;
}

int32_t ThriftClientMgr::BSAGetDataMgr(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
#ifndef WIN32
    StackTracer stackTracer;
#endif

    if ((m_reader.GetReadStatus() == FileIoStatus::CLOSE) ||
        ((m_reader.GetNowTime() - m_reader.GetLastTime()) > DATA_TIME_OUT)) {
        BsaDataBlock32 bsaDataBlock32;
        DataConversion::ConvertdataBlockIn(dataBlockPtr, bsaDataBlock32);
        GetDataResult getDataResult;
        bsaDataBlock32.numBytes = m_totalDataSize;
        getDataResult.response = BSA_RC_ABORT_SYSTEM_ERROR;
        ProtectServiceCall(&BSAServiceClient::BSAGetData, bsaHandle, getDataResult, bsaHandle, bsaDataBlock32);
        if (getDataResult.response != BSA_RC_SUCCESS) {
            ERRLOG("RPC failed!bsaHandle=%ld,response=%d.", bsaHandle, getDataResult.response);
            return getDataResult.response;
        }
        m_totalDataSize = 0;
        m_reader.UpdateLastTime();
        INFOLOG("BSAGetData Success.bsaHandle=%ld.", bsaHandle);
    }

    int32_t iRet = GetData(bsaHandle, dataBlockPtr);
    if (iRet != MP_SUCCESS) {
        ERRLOG("[bsaHandle:%ld] Failed to get data.", bsaHandle);
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    m_reader.SetReadStatus(FileIoStatus::OPEN);
    m_totalDataSize += dataBlockPtr->numBytes;
    uint64_t totalDataSize = m_totalDataSize;
    DBGLOG("[bsaHandle:%ld] bufferLen: %u, headerBytes: %u numBytes: %u totalDataSize: %lu", bsaHandle,
        dataBlockPtr->bufferLen, dataBlockPtr->headerBytes, dataBlockPtr->numBytes, totalDataSize);
    return ((dataBlockPtr->numBytes >= dataBlockPtr->bufferLen) ? BSA_RC_SUCCESS : BSA_RC_NO_MORE_DATA);
}

int32_t ThriftClientMgr::SendData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    mp_string filePath;
    BSA_ObjectStatus bsaWriteStatus;
    {
        std::lock_guard<std::mutex> lk(m_Mutex);
        auto iter = m_writeMap.find(bsaHandle);
        if (iter == m_writeMap.end()) {
            ERRLOG("[bsaHandle:%ld] The handle is not found in the map.", bsaHandle);
            return MP_FAILED;
        }
        filePath = iter->second.workingObj.storgePath;
        bsaWriteStatus = iter->second.workingObj.objectStatus;
    }
    INFOLOG("Application type: %d.", m_appType);
    if (m_appType == BSA_AppType::BSA_INFORMIX) {
        bsaWriteStatus = BSA_OBJECTSTATUS_OVERWRITE_WRITE;
    }

    int32_t iRet;
    INFOLOG("ThriftClientMgr SendData bsaWriteStatus=%d.", bsaWriteStatus);
    if (bsaWriteStatus == BSA_OBJECTSTATUS_OVERWRITE_WRITE) {
        // 1、覆盖写
        iRet = m_writer.OpenForWriteWithSameFile(bsaHandle, filePath, "wb+");
    } else if (bsaWriteStatus == BSA_OBJECTSTATUS_APPEND_WRITE) {
        // 2、追加写
        iRet = m_writer.OpenForWriteWithSameFile(bsaHandle, filePath, "ab+");
    } else {
        // 3、默认写
        iRet = m_writer.OpenForWrite(bsaHandle, filePath);
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("[bsaHandle:%ld] Failed to open disk backup file.", bsaHandle);
        return iRet;
    }

    iRet = m_writer.Write(bsaHandle, dataBlockPtr);
    if (iRet != MP_SUCCESS) {
        ERRLOG("[bsaHandle:%ld] Failed to write disk backup file.", bsaHandle);
        return iRet;
    }

    return MP_SUCCESS;
}

int32_t ThriftClientMgr::GetData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    std::map<long, BsaHandleContext>::iterator iter;
    {
        std::lock_guard<std::mutex> lk(m_Mutex);
        iter = m_readMap.find(bsaHandle);
        if (iter == m_readMap.end()) {
            ERRLOG("[bsaHandle:%ld] The handle is not found in the map.", bsaHandle);
            return MP_FAILED;
        }
    }
    if (iter->second.workingObj.getDataType == BSA_GET_DATA_FROM_NAS) {
        int32_t iRet = m_reader.OpenForRead(bsaHandle, iter->second.workingObj.storgePath);
        if (iRet != MP_SUCCESS) {
            ERRLOG("[bsaHandle:%ld] Failed to open disk backup file.", bsaHandle);
            return iRet;
        }

        iRet = m_reader.Read(bsaHandle, dataBlockPtr);
        if (iRet != MP_SUCCESS) {
            ERRLOG("[bsaHandle:%ld] Failed to write disk backup file.", bsaHandle);
            return iRet;
        }
    } else if (iter->second.workingObj.getDataType = BSA_GET_DATA_FROM_ARCHIVE) {
        BsaHandleContext &context = iter->second;
        int32_t iRet = m_reader.ConnectArchive(bsaHandle, context);
        if (iRet != MP_SUCCESS) {
            ERRLOG("[bsaHandle:%ld] Failed to connect aechive.", bsaHandle);
            return iRet;
        }

        iRet = m_reader.ReadFromArchive(bsaHandle, dataBlockPtr, context);
        if (iRet != MP_SUCCESS) {
            ERRLOG("[bsaHandle:%ld] Failed to get data from archive.", bsaHandle);
        }
    }
    return MP_SUCCESS;
}