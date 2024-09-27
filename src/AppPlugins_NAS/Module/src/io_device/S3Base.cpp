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
#include "S3Base.h"

#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <fcntl.h>
#include <securec.h>
#include <ext/stdio_filebuf.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "LibS3IO.h"
#include "system/System.hpp"
#include "securec.h"
#include "config_reader/ConfigIniReader.h"
#include "common/Timer.h"
#include "sqlite3.h"
#include "log/Log.h"

using namespace std;
using namespace Module;

namespace {
const char *ALGORITHM = "AES256";
constexpr int NUM_2 = 2;
constexpr int NUM_4 = 4;
constexpr int NUM_6 = 6;
constexpr int NUM_1000 = 1000;
constexpr int REQUESTID_32 = 32;
constexpr int REQUESTID_64 = 64;
constexpr int OBJECT_NAME_LEN = 1024;
constexpr int BUFFER_SIZE = 1024;
constexpr int TCP_LISTEN_PORT = 32120;
const char *S3StatusString[] = {
    "OBS_STATUS_OK",
    "OBS_STATUS_InitCurlFailed",
    "OBS_STATUS_InternalError",
    "OBS_STATUS_OutOfMemory",
    "OBS_STATUS_Interrupted",
    "OBS_STATUS_QueryParamsTooLong",
    "OBS_STATUS_FailedToIInitializeRequest",
    "OBS_STATUS_MetadataHeadersTooLong",
    "OBS_STATUS_BadContentType",
    "OBS_STATUS_ContentTypeTooLong",
    "OBS_STATUS_BadMd5",
    "OBS_STATUS_Md5TooLong",
    "OBS_STATUS_BadCacheControl",
    "OBS_STATUS_CacheControlTooLong",
    "OBS_STATUS_BadContentDispositionFilename",
    "OBS_STATUS_ContentDispositionFilenameTooLong",
    "OBS_STATUS_BadContentEncoding",
    "OBS_STATUS_ContentEncodingTooLong",
    "OBS_STATUS_BadIfMatchEtag",
    "OBS_STATUS_IfMatchEtagTooLong",
    "OBS_STATUS_BadIfNotMatchEtag",
    "OBS_STATUS_IfNotMatchEtagTooLong",
    "OBS_STATUS_UriTooLong",
    "OBS_STATUS_XmlParseFailure",
    "OBS_STATUS_UserIdTooLong",
    "OBS_STATUS_UserDisplayNameTooLong",
    "OBS_STATUS_EmailAddressTooLong",
    "OBS_STATUS_GroupUriTooLong",
    "OBS_STATUS_PermissionTooLong",
    "OBS_STATUS_TooManyGrants",
    "OBS_STATUS_BadGrantee",
    "OBS_STATUS_BadPermission",
    "OBS_STATUS_XmlDocumentTooLarge",
    "OBS_STATUS_NameLookupError",
    "OBS_STATUS_FailedToConnect",
    "OBS_STATUS_ServerFailedVerification",
    "OBS_STATUS_ConnectionFailed",
    "OBS_STATUS_AbortedByCallback",
    "OBS_STATUS_PartialFile",
    "OBS_STATUS_InvalidParameter",
    "OBS_STATUS_NoToken",
    "OBS_STATUS_OpenFileFailed",
    "OBS_STATUS_EmptyFile",
    "OBS_STATUS_AccessDenied",
    "OBS_STATUS_AccountProblem",
    "OBS_STATUS_AmbiguousGrantByEmailAddress",
    "OBS_STATUS_BadDigest",
    "OBS_STATUS_BucketAlreadyExists",
    "OBS_STATUS_BucketAlreadyOwnedByYou",
    "OBS_STATUS_BucketNotEmpty",
    "OBS_STATUS_CredentialsNotSupported",
    "OBS_STATUS_CrossLocationLoggingProhibited",
    "OBS_STATUS_EntityTooSmall",
    "OBS_STATUS_EntityTooLarge",
    "OBS_STATUS_ExpiredToken",
    "OBS_STATUS_IllegalVersioningConfigurationException",
    "OBS_STATUS_IncompleteBody",
    "OBS_STATUS_IncorrectNumberOfFilesInPostRequest",
    "OBS_STATUS_InlineDataTooLarge",
    "OBS_STATUS_InvalidAccessKeyId",
    "OBS_STATUS_InvalidAddressingHeader",
    "OBS_STATUS_InvalidArgument",
    "OBS_STATUS_InvalidBucketName",
    "OBS_STATUS_InvalidKey",
    "OBS_STATUS_InvalidBucketState",
    "OBS_STATUS_InvalidDigest",
    "OBS_STATUS_InvalidLocationConstraint",
    "OBS_STATUS_InvalidObjectState",
    "OBS_STATUS_InvalidPart",
    "OBS_STATUS_InvalidPartOrder",
    "OBS_STATUS_InvalidPayer",
    "OBS_STATUS_InvalidPolicyDocument",
    "OBS_STATUS_InvalidRange",
    "OBS_STATUS_InvalidRedirectLocation",
    "OBS_STATUS_InvalidRequest",
    "OBS_STATUS_InvalidSecurity",
    "OBS_STATUS_InvalidSOAPRequest",
    "OBS_STATUS_InvalidStorageClass",
    "OBS_STATUS_InvalidTargetBucketForLogging",
    "OBS_STATUS_InvalidToken",
    "OBS_STATUS_InvalidURI",
    "OBS_STATUS_MalformedACLError",
    "OBS_STATUS_MalformedPolicy",
    "OBS_STATUS_MalformedPOSTRequest",
    "OBS_STATUS_MalformedXML",
    "OBS_STATUS_MaxMessageLengthExceeded",
    "OBS_STATUS_MaxPostPreDataLengthExceededError",
    "OBS_STATUS_MetadataTooLarge",
    "OBS_STATUS_MethodNotAllowed",
    "OBS_STATUS_MissingAttachment",
    "OBS_STATUS_MissingContentLength",
    "OBS_STATUS_MissingRequestBodyError",
    "OBS_STATUS_MissingSecurityElement",
    "OBS_STATUS_MissingSecurityHeader",
    "OBS_STATUS_NoLoggingStatusForKey",
    "OBS_STATUS_NoSuchBucket",
    "OBS_STATUS_NoSuchKey",
    "OBS_STATUS_NoSuchLifecycleConfiguration",
    "OBS_STATUS_NoSuchUpload",
    "OBS_STATUS_NoSuchVersion",
    "OBS_STATUS_NotImplemented",
    "OBS_STATUS_NotSignedUp",
    "OBS_STATUS_NotSuchBucketPolicy",
    "OBS_STATUS_OperationAborted",
    "OBS_STATUS_PermanentRedirect",
    "OBS_STATUS_PreconditionFailed",
    "OBS_STATUS_Redirect",
    "OBS_STATUS_RestoreAlreadyInProgress",
    "OBS_STATUS_RequestIsNotMultiPartContent",
    "OBS_STATUS_RequestTimeout",
    "OBS_STATUS_RequestTimeTooSkewed",
    "OBS_STATUS_RequestTorrentOfBucketError",
    "OBS_STATUS_SignatureDoesNotMatch",
    "OBS_STATUS_ServiceUnavailable",
    "OBS_STATUS_SlowDown",
    "OBS_STATUS_TemporaryRedirect",
    "OBS_STATUS_TokenRefreshRequired",
    "OBS_STATUS_TooManyBuckets",
    "OBS_STATUS_UnexpectedContent",
    "OBS_STATUS_UnresolvableGrantByEmailAddress",
    "OBS_STATUS_UserKeyMustBeSpecified",
    "OBS_STATUS_InsufficientStorageSpace",
    "OBS_STATUS_NoSuchWebsiteConfiguration",
    "OBS_STATUS_NoSuchBucketPolicy",
    "OBS_STATUS_NoSuchCORSConfiguration",
    "OBS_STATUS_InArrearOrInsufficientBalance",
    "OBS_STATUS_NoSuchTagSet",
    "OBS_STATUS_ErrorUnknown",
    "OBS_STATUS_HttpErrorMovedTemporarily",
    "OBS_STATUS_HttpErrorBadRequest",
    "OBS_STATUS_HttpErrorForbidden",
    "OBS_STATUS_HttpErrorNotFound",
    "OBS_STATUS_HttpErrorConflict",
    "OBS_STATUS_HttpErrorUnknown",
    "OBS_STATUS_BUTT"
};
}  // namespace

EnableVpp::EnableVpp() : m_enableVppSpeedUp(false)
{
    HCP_Log(DEBUG, "EnableVpp") << "EnableVpp constructor Enter." << HCPENDLOG;
}

void EnableVpp::SetEnableVppSpeedUp(bool enableVppSpeedUp)
{
    m_enableVppSpeedUp = enableVppSpeedUp;
}

bool EnableVpp::GetEnableVppSpeedUp()
{
    return m_enableVppSpeedUp;
}

EnableVpp &EnableVpp::GetInstance()
{
    static EnableVpp enableVpp;
    return enableVpp;
}

// functions of S3BucketContextProxy
string S3BucketContextProxy::GetPlainPasswd(const string &secureKey)
{
    string plainPasswd = secureKey;
    return plainPasswd;
}

char *S3BucketContextProxy::ReadCertificateInfo(const string &CertificatePath)
{
    string realPath;
    try {
        boost::system::error_code errcode;
        realPath = boost::filesystem::canonical(boost::filesystem::path(CertificatePath), errcode).string();
        if (errcode.value() != 0) {
            HCP_Log(ERR, LIBS3) << "Get Certificate file real path failed. FileName :" << CertificatePath
                                << ", errcode:" << WIPE_SENSITIVE(errcode.value()) << HCPENDLOG;
            return nullptr;
        }
    } catch (const boost::filesystem::filesystem_error &errExp) {
        HCP_Log(ERR, LIBS3) << "Get Certificate file real path failed. FileName :" << CertificatePath
                            << ", exception:" << WIPE_SENSITIVE(errExp.what()) << HCPENDLOG;
        return nullptr;
    }
    FILE *fp = nullptr;
    if ((fp = fopen(realPath.c_str(), "rb")) == nullptr) {
        HCP_Log(ERR, LIBS3) << "Certificate file can not open or" << realPath.c_str() << "do not exist" << HCPENDLOG;
        return nullptr;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        HCP_Log(ERR, LIBS3) << "fseek failed!" << HCPENDLOG;
        fclose(fp);
        fp = nullptr;
        return nullptr;
    }

    long lFileLength = ftell(fp);
    if (lFileLength <= 0) {
        HCP_Log(ERR, LIBS3) << "fseek failed." << HCPENDLOG;
        fclose(fp);
        return nullptr;
    }

    size_t nFileLength = static_cast<size_t>(lFileLength);
    rewind(fp);
    m_SpCertificate.reset(new (nothrow) char[nFileLength + 1]);
    if (m_SpCertificate == nullptr) {
        HCP_Log(ERR, LIBS3) << "Reset certificate file failed!" << HCPENDLOG;
        fclose(fp);
        return nullptr;
    }

    size_t nReadSize = fread(m_SpCertificate.get(), 1, nFileLength, fp);
    if (nReadSize != nFileLength) {
        HCP_Log(ERR, LIBS3) << "Read uds certificate file failed!" << HCPENDLOG;
        fclose(fp);
        return nullptr;
    }
    fclose(fp);
    m_SpCertificate[nReadSize] = '\0';
    return m_SpCertificate.get();
}

void S3BucketContextProxy::PrintBucketContext() const
{
    HCP_Logger_noid(DEBUG, LIBS3) << "domain:" << m_Host << HCPENDLOG;
    HCP_Logger_noid(DEBUG, LIBS3) << "protocol:" << m_Protocol << HCPENDLOG;
    HCP_Logger_noid(DEBUG, LIBS3) << "bucketname:" << m_Bucket << HCPENDLOG;
    HCP_Logger_noid(DEBUG, LIBS3) << "hostname:" << m_Proxy.HostName << HCPENDLOG;
    HCP_Logger_noid(DEBUG, LIBS3) << "Port:" << m_Proxy.Port << HCPENDLOG;
}

S3BucketContextProxy::S3BucketContextProxy()
{
    init_obs_options(this);
    m_Protocol = OBS_PROTOCOL_HTTPS;
    bucket_options.certificate_info = 0;
    bucket_options.uri_style = OBS_URI_STYLE_PATH;
    bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    int timeOut = 60000;
    int connectTimeOut = 0;

    GetS3TimeOut(timeOut, connectTimeOut);
    // Connection timed out default 1min
    request_options.connect_time = connectTimeOut;
    // Receiving data times out default 5min
    request_options.max_connected_time = timeOut;
}

S3BucketContextProxy::S3BucketContextProxy(const S3IOParams &params)
    : m_Host(params.host),
      m_Bucket(params.bucket),
      m_UserName(params.userName),
      m_Protocol(params.protocol),
      m_Proxy(params.HttpProxyInfo),
      m_Speed(params.SpeedUpInfo)
{
    init_obs_options(this);
    m_PassWord = GetPlainPasswd(params.passWord);
    bucket_options.certificate_info = 0;
    bucket_options.uri_style = OBS_URI_STYLE_PATH;
    bucket_options.protocol = OBS_PROTOCOL_HTTPS;
    request_options.http2_switch = OBS_HTTP2_CLOSE;

    bucket_options.host_name = const_cast<char *>(m_Host.c_str());
    bucket_options.bucket_name = const_cast<char *>(m_Bucket.c_str());
    bucket_options.protocol = m_Protocol;
    bucket_options.access_key = const_cast<char *>(m_UserName.c_str());
    bucket_options.secret_access_key = const_cast<char *>(m_PassWord.c_str());
    bucket_options.uri_style = params.uriStyle;
    bucket_options.bucket_type = OBS_BUCKET_OBJECT;
    bucket_options.bucket_list_type = OBS_BUCKET_LIST_OBJECT;

    int timeOut = 60000;
    int connectTimeOut = 0;
    GetS3TimeOut(timeOut, connectTimeOut);
    request_options.connect_time = connectTimeOut;
    request_options.max_connected_time = timeOut;
    request_options.uploadRateLimit = params.uploadRateLimit;
    request_options.downloadRateLimit = params.downloadRateLimit;

    if (m_Proxy.Enable) {
        m_FullHost = m_Proxy.HostName + ":" + m_Proxy.Port;
        m_FullAuth = m_Proxy.UserName + ":" + m_Proxy.UserPassword;
        request_options.proxy_host = const_cast<char *>(m_FullHost.c_str());
        request_options.proxy_auth = const_cast<char *>(m_FullAuth.c_str());
    }

    request_options.bbr_switch = OBS_BBR_CLOSE;
    if (m_Speed.Enable) {
        if (m_Speed.Method == "bbr") {
            request_options.bbr_switch = OBS_BBR_OPEN;
        }
    }

    if (params.cert.size() > 0 && params.cert != "none") {
        m_SpCertificate.reset(new (nothrow) char[params.cert.size() + 1]);
        if (m_SpCertificate.get() == nullptr) {
            return;
        }
        if (strncpy_s(m_SpCertificate.get(), params.cert.size() + 1, params.cert.c_str(), params.cert.size()) != 0) {
            HCP_Log(ERR, LIBS3) << "strncpy_s failed!" << HCPENDLOG;
        }
    }
    if (m_SpCertificate.get() == nullptr) {
        return;
    }
    bucket_options.certificate_info = m_SpCertificate.get();
}

S3BucketContextProxy::~S3BucketContextProxy()
{
    CleanMemoryPwd(m_PassWord);
}

bool S3BucketContextProxy::Init(const string &host, const string &bucket, const string &accessKey,
    const string &secureKey, const string &certificatePath, obs_protocol prot, const obs_uri_style &style)
{
    init_obs_options(this);
    m_Host = host;
    m_Bucket = bucket;
    m_UserName = accessKey;
    m_PassWord = secureKey;
    m_Protocol = prot;

    bucket_options.certificate_info = 0;
    bucket_options.uri_style = OBS_URI_STYLE_PATH;
    bucket_options.protocol = OBS_PROTOCOL_HTTPS;
    bucket_options.host_name = const_cast<char *>(m_Host.c_str());
    bucket_options.bucket_name = const_cast<char *>(m_Bucket.c_str());
    bucket_options.protocol = m_Protocol;
    bucket_options.access_key = const_cast<char *>(m_UserName.c_str());
    bucket_options.secret_access_key = const_cast<char *>(m_PassWord.c_str());
    bucket_options.uri_style = style;

    if (strlen(m_PassWord.c_str()) == 0 || strlen(m_Host.c_str()) == 0) {
        return false;
    }
    if (m_Protocol == OBS_PROTOCOL_HTTPS) {
        char *certficate = ReadCertificateInfo(certificatePath);
        if (certficate == nullptr) {
            return false;
        }
        bucket_options.certificate_info = certficate;
    }
    return true;
}

int S3BucketContextProxy::GetBbrSwitch() const
{
    int bbrSwitch = 0;

    if (m_Speed.Enable) {
        if (m_Speed.Method == "bbr") {
            bbrSwitch = 1;
        }
    }
    return bbrSwitch;
}

void S3BucketContextProxy::GetS3TimeOut(int &timeOut, int &connectTimeOut)
{
    connectTimeOut = ConfigReader::getInt("BackupNode", "S3ConnectionTimeOut") * NUM_1000;
    timeOut = ConfigReader::getInt("BackupNode", "S3Timeout") * NUM_1000;
}

string S3BucketContextProxy::GetProxyAddress() const
{
    if (m_Proxy.Enable) {
        string setproxy = m_Proxy.HostName + ":" + m_Proxy.Port;
        return setproxy;
    }
    return "";
}

string S3BucketContextProxy::GetProxyUser() const
{
    if (m_Proxy.Enable) {
        string proxyauth = m_Proxy.UserName + ":" + m_Proxy.UserPassword;
        return proxyauth;
    }
    return "";
}

static obs_status ListBucketCallback(int isTruncated, const char *nextMarker, int contentsCount,
    const obs_list_objects_content *contents, int commonPrefixesCount, const char **commonPrefixes, void *callbackData)
{
    HCP_Log(DEBUG, LIBS3) << "Enter ListBucketCallback ." << HCPENDLOG;
    S3Base *s3Obj = static_cast<S3Base *>(callbackData);
    return s3Obj->ListBucketCallbackInternal(
        isTruncated, nextMarker, contentsCount, contents, commonPrefixesCount, commonPrefixes);
}

S3Base::S3Base()
    : m_completedStatus(OBS_STATUS_ErrorUnknown), m_contentLength(0), m_ObjectsTotalSize(0), m_useVpp(false)
{
    memset_s(m_requestID, sizeof(char) * REQUESTID_64, 0, sizeof(char) * REQUESTID_64);
    memset_s(m_objectName, sizeof(char) * OBJECT_NAME_LEN, 0, sizeof(char) * OBJECT_NAME_LEN);
    memset_s(m_objKey, sizeof(char) * OBJECT_NAME_LEN, 0, sizeof(char) * OBJECT_NAME_LEN);
}

S3Base::~S3Base()
{
    m_objectInfoList.clear();
    m_matchObjList.clear();
    m_matchDirList.clear();
}

obs_status S3Base::ResponseCallback(const obs_response_properties *properties, void *callbackData)
{
    S3Base *s3Obj = static_cast<S3Base *>(callbackData);
    if (s3Obj != nullptr) {
        s3Obj->CallbackResponse(properties);
    } else {
        HCP_Log(CRIT, LIBS3) << "ResponseCallback failed. " << HCPENDLOG;
    }
    return OBS_STATUS_OK;
}

void S3Base::CompleteCallback(obs_status status, const obs_error_details *error, void *callbackData)
{
    if (error != nullptr) {
        string errorDetailsG;
        for (uint32_t i = 0; i < error->extra_details_count; ++i) {
            errorDetailsG +=
                (string)error->extra_details[i].name + " " + (string)error->extra_details[i].value + "\n";
        }
        if (!errorDetailsG.empty()) {
            HCP_Log(ERR, LIBS3) << WIPE_SENSITIVE(errorDetailsG) << HCPENDLOG;
        }
    }

    const char *msg = obs_get_status_name(status);
    if (msg != nullptr) {
        HCP_Log(DEBUG, LIBS3) << "CompleteCallback status: " << msg << HCPENDLOG;
    }

    if (status != OBS_STATUS_OK) {
        HCP_Log(WARN, LIBS3) << "CompleteCallback error status: " << status << HCPENDLOG;
    }

    S3Base *s3Obj = static_cast<S3Base *>(callbackData);
    if (s3Obj != nullptr) {
        s3Obj->CallbackComplete(status);
    } else {
        HCP_Log(CRIT, LIBS3) << "CompleteCallback failed. " << HCPENDLOG;
    }
}

int S3Base::PutObjectDataCallback(int bufferSize, char *buffer, void *callbackData)
{
    HCP_Log(DEBUG, LIBS3) << "Enter PutObjectDataCallback, bufferSize = " << bufferSize << HCPENDLOG;

    libs3IO *s3Obj = static_cast<libs3IO *>(callbackData);
    if (s3Obj != nullptr) {
        size_t readSize = s3Obj->CallbackRead(buffer, bufferSize);
        HCP_Log(DEBUG, LIBS3) << "Expect bufferSize:" << bufferSize << ",Get object data, size = " << readSize
                              << HCPENDLOG;
        if (readSize != static_cast<size_t>(bufferSize)) {
            HCP_Log(ERR, LIBS3) << "the expect size if not the same read from buffer! " << HCPENDLOG;
        }
        S3Base *s3Base = static_cast<S3Base *>(callbackData);
        if (s3Base != nullptr && s3Base->m_callbackHandle.callBackFunc != nullptr) {
            ((WriteFileCallback)(s3Base->m_callbackHandle.callBackFunc))(
                LayoutRetCode::SUCCESS, readSize, 0, s3Base->m_callbackHandle.callBackData);
        }
        return static_cast<int>(readSize);
    } else {
        HCP_Log(CRIT, LIBS3) << "PutObjectDataCallback failed. " << HCPENDLOG;
    }

    return -1;
}

obs_status S3Base::GetObjectDataCallback(int bufferSize, const char *buffer, void *callbackData)
{
    HCP_Log(DEBUG, LIBS3) << "Enter GetObjectDataCallback, bufferSize = " << bufferSize << HCPENDLOG;

    S3Base *s3Obj = static_cast<S3Base *>(callbackData);
    obs_status status = OBS_STATUS_AbortedByCallback;
    if (s3Obj != nullptr) {
        ssize_t writtenSize = s3Obj->CallbackWrite(buffer, bufferSize);
        HCP_Log(DEBUG, LIBS3) << "Expect bufferSize:" << bufferSize << ",write data to buffer:" << writtenSize
                              << HCPENDLOG;
        if (writtenSize != (ssize_t)bufferSize) {
            HCP_Log(ERR, LIBS3) << "the expect size if not the same write to buffer!"
                << DBG(writtenSize) << DBG(bufferSize) << DBG(errno) << HCPENDLOG;
        } else {
            bool aborted = false;
            s3Obj->m_downloadedSize += writtenSize;
            if (s3Obj->m_readFileFun) {
                s3Obj->m_readFileFun(LayoutRetCode::SUCCESS, writtenSize, aborted);
                return aborted ? OBS_STATUS_AbortedByCallback : OBS_STATUS_OK;
            }
            status = OBS_STATUS_OK;
        }
    } else {
        HCP_Log(CRIT, LIBS3) << "GetObjectDataCallback failed. " << HCPENDLOG;
    }
    return status;
}

bool S3Base::IsS3Accelerator()
{
    ostringstream ostr;
    m_completedStatus = OBS_STATUS_HttpErrorNotFound;

    int isvpp = ConfigReader::getInt("Vpp", "ISVPP");
    if (isvpp <= 0) {
        HCP_Log(DEBUG, LIBS3) << "not use vpp from config file." << HCPENDLOG;
        return false;
    }

    if (!EnableVpp::GetInstance().GetEnableVppSpeedUp()) {
        HCP_Log(DEBUG, LIBS3) << "not use vpp from adminNode." << HCPENDLOG;
        return false;
    }

    int port = ConfigReader::getInt("Vpp", "TCPListenPort");
    if (port <= 0) {
        HCP_Log(DEBUG, LIBS3) << "use default port." << HCPENDLOG;
        port = TCP_LISTEN_PORT;
    }

    ostr << "127.0.0.1:" << port;
    m_accelerator_proxy = ostr.str();

    HCP_Log(DEBUG, LIBS3) << "use vpp speed up." << HCPENDLOG;

    return true;
}

int S3Base::GetS3RetryTimeAndSetConnectTImeOut()
{
    int iS3RetryTimes = ConfigReader::getInt("BackupNode", "S3RetryTimes");
    return iS3RetryTimes;
}

bool S3Base::S3StatusIsRetryable()
{
    if (OBS_STATUS_HttpErrorNotFound == m_completedStatus) {
        HCP_Log(WARN, LIBS3) << "status:" << m_completedStatus << " need retry." << HCPENDLOG;
        return true;
    }
    return S3HeadStatusIsRetryable();
}

bool S3Base::S3HeadStatusIsRetryable()
{
    if (OBS_STATUS_InternalError == m_completedStatus || OBS_STATUS_ServiceUnavailable == m_completedStatus ||
        OBS_STATUS_SlowDown == m_completedStatus) {
        HCP_Log(WARN, LIBS3) << "status:" << m_completedStatus << " need retry." << HCPENDLOG;
        return true;
    }
    return ::obs_status_is_retryable(m_completedStatus);
}

void S3Base::PrintStatusErr(string funName)
{
    const char *statusMsg = obs_get_status_name(m_completedStatus);
    if (statusMsg == nullptr) {
        HCP_Log(ERR, LIBS3) << funName << " failed,request object name:" << m_objectName
                            << ",request ID:" << m_requestID << HCPENDLOG;
    } else {
        HCP_Log(WARN, LIBS3) << funName << " failed,request object name:" << m_objectName
                             << ",request ID:" << m_requestID << ",error message:" << statusMsg << HCPENDLOG;
    }
}

bool S3Base::ShouldRetry(int &retryTimes)
{
    HCP_Log(INFO, LIBS3) << "may will retry later, retryTimes: " << retryTimes << HCPENDLOG;

    if (retryTimes-- != 0) {
        int iS3RetryTimeout = ConfigReader::getInt("BackupNode", "S3RetryTimeout");
        HCP_Log(DEBUG, LIBS3) << "Get retry timeout =" << iS3RetryTimeout << HCPENDLOG;
        struct timespec timeout = {time_t(iS3RetryTimeout), 0};
        boost::this_thread::no_interruption_point::sleep_for(boost::chrono::seconds(timeout.tv_sec) +
                                                             boost::chrono::nanoseconds(timeout.tv_nsec));
        return true;
    }

    return false;
}

string S3Base::Base64Encode()
{
    BIO *bmem = nullptr;
    BIO *b64 = nullptr;
    BUF_MEM *bptr = nullptr;
    string temp;

    b64 = BIO_new(BIO_f_base64());
    if (b64 == nullptr) {
        HCP_Log(ERR, LIBS3) << "b64 is empty." << HCPENDLOG;
        return temp;
    }

    bmem = BIO_new(BIO_s_mem());
    if (bmem == nullptr) {
        HCP_Log(ERR, LIBS3) << "bmem is empty." << HCPENDLOG;
        BIO_free_all(b64);
        return temp;
    }

    b64 = BIO_push(b64, bmem);
    BIO_write(b64, m_dek.c_str(), m_dek.length());
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    unique_ptr<char[]> buff = make_unique<char[]>(bptr->length + 1);
    if (buff == nullptr) {
        HCP_Log(ERR, LIBS3) << "malloc buff is empty." << HCPENDLOG;
        BIO_free_all(b64);
        return temp;
    }
    memset_s(buff.get(), bptr->length + 1, 0, bptr->length + 1);
    int retVal = memcpy_s(buff.get(), bptr->length + 1, bptr->data, bptr->length);
    if (retVal != 0) {
        HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
    }
    buff[bptr->length] = '\0';
    temp = buff.get();
    BIO_free_all(b64);
    if (m_dek.length() == 0)
        temp = "";
    HCP_Log(DEBUG, LIBS3) << "dek size " << m_dek.length() << HCPENDLOG;

    string::size_type pos = 0;
    while ((pos = temp.find("\n", pos)) != string::npos) {
        temp.replace(pos, 1, "");
    }

    return temp;
}

string S3Base::DekToMd5()
{
    const int DECRYPT_SIZE = 16;
    unsigned char decrypt[DECRYPT_SIZE];
    BIO *bmem = nullptr;
    BIO *b64 = nullptr;
    BUF_MEM *bptr = nullptr;
    string temp;

    MD5_CTX md5Ctx;
    MD5_Init(&md5Ctx);
    MD5_Update(&md5Ctx, m_dek.c_str(), m_dek.length());
    MD5_Final(decrypt, &md5Ctx);

    b64 = BIO_new(BIO_f_base64());
    if (b64 == nullptr) {
        HCP_Log(ERR, LIBS3) << "b64 is empty." << HCPENDLOG;
        return temp;
    }

    bmem = BIO_new(BIO_s_mem());
    if (bmem == nullptr) {
        HCP_Log(ERR, LIBS3) << "bmem is empty." << HCPENDLOG;
        BIO_free_all(b64);
        return temp;
    }
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, (char *)decrypt, DECRYPT_SIZE);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    unique_ptr<char[]> buff = make_unique<char[]>(bptr->length + 1);
    if (buff == nullptr) {
        HCP_Log(ERR, LIBS3) << "malloc buff is empty." << HCPENDLOG;
        BIO_free_all(b64);
        return temp;
    }
    memset_s(buff.get(), bptr->length + 1, 0, bptr->length + 1);
    int retVal = memcpy_s(buff.get(), bptr->length + 1, bptr->data, bptr->length);
    if (retVal != 0) {
        HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
    }
    buff[bptr->length] = '\0';
    temp = buff.get();
    BIO_free_all(b64);
    if (m_dek.length() == 0)
        temp = "";

    string::size_type pos = 0;
    while ((pos = temp.find("\n", pos)) != string::npos) {
        temp.replace(pos, 1, "");
    }
    return temp;
}

size_t S3Base::WriteDataToS3(S3BucketContextProxy &bucketContext, const string objKey, size_t dataLen)
{
    obs_put_object_handler putObjectHandler = {{&ResponseCallback, &CompleteCallback}, &PutObjectDataCallback};
    return WriteDataToS3(bucketContext, putObjectHandler, objKey, dataLen);
}

void S3Base::CallbackComplete(obs_status status)
{
    m_completedStatus = status;
    HCP_Log(DEBUG, LIBS3) << "m_completedStatus:"<<m_completedStatus <<HCPENDLOG;
}

void S3Base::CallbackResponse(const obs_response_properties *properties)
{
    if (properties == nullptr) {
        HCP_Log(INFO, LIBS3) << "properties is nullptr." << HCPENDLOG;
        return;
    }
    if (properties->request_id != nullptr) {
        (void)strncpy_s(m_requestID, sizeof(m_requestID), properties->request_id, REQUESTID_32);
        m_requestID[REQUESTID_32] = '\0';
    }
    m_contentLength = properties->content_length;
    m_lastModified = properties->last_modified;

    m_obj_metadata.clear();
    for (int i = 0; i < properties->meta_data_count; i++) {
        obj_metadata metadata = {string(properties->meta_data->name), string(properties->meta_data->value)};
        m_obj_metadata.push_back(metadata);
    }

    if (properties->etag) {
        singlePart sPart;
        sPart.partNumber = m_PartsInfoVector.size() + 1;
        sPart.eTag = properties->etag;
        m_PartsInfoVector.push_back(sPart);
        m_etag = string(properties->etag);
    }
}

void S3Base::SetObjectName(const string &objName)
{
    if (memcpy_s(m_objectName, sizeof(m_objectName), objName.c_str(), objName.size()) != 0) {
        HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
    }
    uint64_t len = sizeof(m_objectName) - 1;
    if (objName.size() < len) {
        len = objName.size();
    }
    m_objectName[len] = '\0';
}

void S3Base::SetObjectKey(const string &objKey)
{
    if (memcpy_s(m_objKey, sizeof(m_objKey), objKey.c_str(), objKey.size()) != 0) {
        HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
    }
    uint64_t len = sizeof(m_objKey) - 1;
    if (objKey.size() < len) {
        len = objKey.size();
    }
    m_objKey[len] = '\0';
}

obs_status S3Base::ListBucketCallbackInternal(int isTruncated, const char *nextMarker, int contentsCount,
    const obs_list_objects_content *contents, int commonPrefixesCount, const char **commonPrefixes)
{
    HCP_Log(DEBUG, LIBS3) << "Enter ListBucketCallback ." << HCPENDLOG;
    list_bucket_callback_data *data = (list_bucket_callback_data *)&(m_ListBucketInfo);

    data->isTruncated = isTruncated;

    if ((!nextMarker || (nextMarker[0] == 0)) && (contentsCount != 0)) {
        nextMarker = contents[contentsCount - 1].key;
    }
    if (nextMarker) {
        if (sprintf_s(data->nextMarker, sizeof(data->nextMarker), "%s", nextMarker) == -1) {
            HCP_Log(ERR, LIBS3) << "sprintf_s failed" << HCPENDLOG;
        }
    } else {
        data->nextMarker[0] = 0;
    }

    vector<obs_list_objects_content> vecContents;
    for (int i = 0; i < contentsCount; ++i) {
        vecContents.push_back(contents[i]);
        AppendObjName(contents[i].key);
        m_ObjectsTotalSize += contents[i].size;
        HCP_Log(DEBUG, LIBS3) << "i: " << i << ", contents[i].key: " << contents[i].key << HCPENDLOG;
    }

    AppendObjectList(vecContents);
    data->keyCount += contentsCount;

    for (int i = 0; i < commonPrefixesCount; ++i) {
        AppendDirName(commonPrefixes[i]);
        HCP_Log(DEBUG, LIBS3) << "i: " << i << ",commonPrefixes[i]: " << commonPrefixes[i] << HCPENDLOG;
    }
    HCP_Log(DEBUG, LIBS3) << "exit ListBucketCallback. isTruncated is :" << data->isTruncated << HCPENDLOG;

    return OBS_STATUS_OK;
}

bool S3Base::ListBucket(
    const S3IOParams &params, const char *prefix, const char *marker, const char *delimiter, int maxkeys)
{
    S3BucketContextProxy bucketContext(params);

    bool bRet = ListBucket(bucketContext, prefix, marker, delimiter, maxkeys);
    return bRet;
}

int S3Base::CallbackUploadData(int bufferSize, char *buffer)
{
    if (buffer == nullptr) {
        HCP_Log(ERR, LIBS3) << "Param invalid." << HCPENDLOG;
        return 0;
    }

    return GetUploadFileData(buffer, bufferSize);
}

obs_status S3Base::CallbackListParts(
    int isTruncated, unsigned int nextPartNumberMarker, int partsCount, const obs_list_parts *parts)
{
    list_parts_callback_data *data = (list_parts_callback_data *)&m_ListPartsInfo;

    HCP_Log(DEBUG, LIBS3) << "listPartsCallback,isTruncated = " << isTruncated << HCPENDLOG;
    data->isTruncated = isTruncated;
    HCP_Log(DEBUG, LIBS3) << "keypartsCount = " << partsCount << HCPENDLOG;
    HCP_Log(INFO, LIBS3) << "total partsCount = " << data->partsCount << HCPENDLOG;
    HCP_Log(DEBUG, LIBS3) << "nextPartNumberMarker = " << nextPartNumberMarker << HCPENDLOG;
    data->nextPartNumberMarker = nextPartNumberMarker;

    for (int i = 0; i < partsCount; i++) {
        const obs_list_parts *onePartOfList = &(parts[i]);
        singlePart sPart;
        sPart.partNumber = onePartOfList->part_number;
        sPart.eTag = onePartOfList->etag;
        sPart.partSzie = (unsigned long long)onePartOfList->size;
        sPart.lastModified = onePartOfList->last_modified;

        data->partMap.insert(make_pair(data->partsCount + i + 1, sPart));
        HCP_Log(DEBUG, LIBS3) << "partNumber:"
                              << "partsize:" << (unsigned long long)onePartOfList->size << onePartOfList->part_number
                              << "-->eTag" << onePartOfList->etag << HCPENDLOG;
    }
    data->keyCount += partsCount;
    data->partsCount += partsCount;
    HCP_Log(DEBUG, LIBS3) << "partMapsize= " << data->partMap.size() << HCPENDLOG;

    return OBS_STATUS_OK;
}

void S3Base::AppendObjectList(vector<obs_list_objects_content> &vecContents)
{
    m_objectInfoList.insert(m_objectInfoList.end(), vecContents.begin(), vecContents.end());
}

void S3Base::AppendObjName(string objName)
{
    m_matchObjList.push_back(objName);
}

void S3Base::AppendDirName(string dirName)
{
    m_matchDirList.push_back(dirName);
}

#ifndef EBKSDK_LIBS3_COMPILE

bool S3Base::FillS3Variables(
    obs_object_info &object_info, obs_get_conditions &get_conditions, size_t offset, size_t dataLen)
{
    if (memset_s(&object_info, sizeof(obs_object_info), 0, sizeof(obs_object_info)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
        return false;
    }
    object_info.key = m_objKey;
    object_info.version_id = 0;

    if (memset_s(&get_conditions, sizeof(obs_get_conditions), 0, sizeof(obs_get_conditions)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
        return false;
    }
    init_get_properties(&get_conditions);
    get_conditions.start_byte = offset;
    get_conditions.byte_count = dataLen;
    return true;
}

void S3Base::PrintGetObjectStatusInfo()
{
    Timer timer;
    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("RecvData");
        HCP_Log(WARN, LIBS3) << "get_object object failed name=" << m_objectName << ", request ID:" << m_requestID
                             << ",duration = " << timer.Duration() << HCPENDLOG;
        return;
    }

    HCP_Log(DEBUG, LIBS3) << "get_object object name=" << m_objectName << ", request ID:" << m_requestID
                          << ",duration = " << timer.Duration() << HCPENDLOG;
}

void S3Base::ReadDataFromS3(S3BucketContextProxy &bucketContext, size_t offset, size_t dataLen)
{
    HCP_Log(DEBUG, LIBS3) << "FLOW: entered S3Base::ReadDataFromS3 " << HCPENDLOG;
    obs_get_object_handler getObjectHandler = {{&ResponseCallback, &CompleteCallback}, &GetObjectDataCallback};
    server_side_encryption_params serverSideEncryptionParams;
    InitEncryptionParams(serverSideEncryptionParams);

    obs_object_info object_info;
    obs_get_conditions get_conditions;
    if (!FillS3Variables(object_info, get_conditions, offset, dataLen)) {
        HCP_Log(ERR, LIBS3) << "AllocForS3Variables failed" << HCPENDLOG;
    }

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(m_objKey);

    RWBegin();
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    do {
        if (IsS3Accelerator()) {
            RWRetry(true);
            bucketContext.request_options.proxy_host = const_cast<char *>(m_accelerator_proxy.c_str());
            bucketContext.request_options.bbr_switch = OBS_BBR_CLOSE;
            get_object(
                &bucketContext, &object_info, &get_conditions, &serverSideEncryptionParams, &getObjectHandler, this);
            m_useVpp = true;
            if (bucketContext.GetProxyAddress() != "") {
                bucketContext.request_options.proxy_host = const_cast<char *>(bucketContext.GetProxyAddress().c_str());
            }

            if (bucketContext.GetProxyUser() != "") {
                bucketContext.request_options.proxy_auth = const_cast<char *>(bucketContext.GetProxyUser().c_str());
            }

            bucketContext.request_options.bbr_switch =
                (bucketContext.GetBbrSwitch() == 0) ? OBS_BBR_CLOSE : OBS_BBR_OPEN;
            if (S3StatusIsRetryable()) {
                m_useVpp = false;
                bucketContext.request_options.proxy_host = 0;
                bucketContext.request_options.bbr_switch = OBS_BBR_CLOSE;
                bucketContext.request_options.proxy_auth = 0;
                HCP_Log(WARN, LIBS3) << "get_object failed. object name=" << m_objectName
                                     << ", m_completedStatus=" << m_completedStatus << HCPENDLOG;
            }
        }

        if (!m_useVpp) {
            RWRetry(true);
            get_object(
                &bucketContext, &object_info, &get_conditions, &serverSideEncryptionParams, &getObjectHandler, this);
        }
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));
    PrintGetObjectStatusInfo();
}

int S3Base::Md5Base64Encode(const unsigned char *in, int inLen, char *out)
{
    static const char *enc = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    char *originalOut = out;
    while (inLen > 0) {
        *out++ = enc[*in >> NUM_2];
        if (--inLen == 0) {
            *out++ = enc[(*in & 0x3) << NUM_4];
            *out++ = '=';
            *out++ = '=';
            break;
        }
        *out++ = enc[((*in & 0x3) << NUM_4) | (*(in + 1) >> NUM_4)];
        in++;
        if (--inLen == 0) {
            *out++ = enc[(*in & 0xF) << NUM_2];
            *out++ = '=';
            break;
        }
        *out++ = enc[((*in & 0xF) << NUM_2) | (*(in + 1) >> NUM_6)];
        in++;
        *out++ = enc[*in & 0x3F];
        in++, inLen--;
    }
    return (out - originalOut);
}

void S3Base::CalculationFileMd5()
{
    uint32_t destMax = 64;
    memset_s(m_uploadMd5, destMax, 0, destMax);
    const int decryptSize = 16;
    unsigned char decrypt[decryptSize] = {0};
    MD5_CTX md5Ctx;
    MD5_Init(&md5Ctx);

    __gnu_cxx::stdio_filebuf<char> filebuf(*(m_localFile.get()), ios::in);
    istream is(&filebuf);

    streamsize length;
    char buffer[BUFFER_SIZE];
    while (!is.eof()) {
        is.read(buffer, BUFFER_SIZE);
        length = is.gcount();
        if (length > 0) {
            MD5_Update(&md5Ctx, buffer, length);
        }
    }

    MD5_Final(decrypt, &md5Ctx);
    BIO *bmem = nullptr;
    BIO *b64 = nullptr;
    BUF_MEM *bptr = nullptr;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, reinterpret_cast<char *>(decrypt), decryptSize);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    int retVal = memcpy_s(m_uploadMd5, destMax, bptr->data, bptr->length);
    if (retVal != 0) {
        HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
    }
    m_uploadMd5[bptr->length] = 0;
    const int UPLOADMD5_SIZE = 24;
    m_uploadMd5[UPLOADMD5_SIZE] = 0;
    BIO_free_all(b64);
}

void S3Base::InitPutProperties(obs_put_properties *put_properties, bool isMd5Upload)
{
    init_put_properties(put_properties);
    int fullControl = ConfigReader::getInt("BackupNode", "S3BucketFullControl");
    if (fullControl > 0) {
        put_properties->canned_acl = OBS_CANNED_ACL_BUCKET_OWNER_FULL_CONTROL;
        put_properties->az_redundancy = OBS_REDUNDANCY_1AZ;
    }
}

void S3Base::InitEncryptionParams(server_side_encryption_params &serverSideEncryptionParams)
{
    if (memset_s(&serverSideEncryptionParams,
        sizeof(server_side_encryption_params),
        0,
        sizeof(server_side_encryption_params)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
        return;
    }
    string base = Base64Encode();
    if (m_dek.length() > 0) {
        serverSideEncryptionParams.encryption_type = OBS_ENCRYPTION_SSEC;
        serverSideEncryptionParams.ssec_customer_algorithm = const_cast<char *>(ALGORITHM);
        serverSideEncryptionParams.ssec_customer_key = const_cast<char *>(base.c_str());
    }
}

void S3Base::GetUploadPutProperties(obs_put_properties &put_properties, int &isMd5Upload, const string &objKey)
{
    if (isMd5Upload == 1 && string::npos != objKey.find("ChainDB.db") &&
        string::npos == objKey.find("ChainDB.db-")) {
        InitPutProperties(&put_properties, true);
    } else {
        InitPutProperties(&put_properties);
    }
}

void S3Base::InitS3AccessParameter(
    server_side_encryption_params &serverSideEncryptionParams, obs_put_properties &put_properties, const string &objKey)
{
    InitEncryptionParams(serverSideEncryptionParams);
    int isMd5Upload = ConfigReader::getInt("Md5", "UploadWithMd5");
    GetUploadPutProperties(put_properties, isMd5Upload, objKey);
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(m_objKey);
}

size_t S3Base::WriteDataToS3(
    S3BucketContextProxy &bucketContext, obs_put_object_handler &handler, const string objKey, size_t dataLen)
{
    server_side_encryption_params serverSideEncryptionParams;
    obs_put_properties put_properties;
    InitS3AccessParameter(serverSideEncryptionParams, put_properties, objKey);
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    RWBegin();
    // modify by w90006072 fix  DTS2019022612416
    Timer timer;

    do {
        if (IsS3Accelerator()) {
            bucketContext.request_options.proxy_host = const_cast<char *>(m_accelerator_proxy.c_str());
            bucketContext.request_options.bbr_switch = OBS_BBR_CLOSE;

            RWRetry(false);
            put_object(&bucketContext, const_cast<char *>(objKey.c_str()), dataLen, &put_properties,
                &serverSideEncryptionParams, &handler, this);
            m_useVpp = true;
            if (bucketContext.GetProxyAddress() != "") {
                bucketContext.request_options.proxy_host = const_cast<char *>(bucketContext.GetProxyAddress().c_str());
            }
            if (bucketContext.GetProxyUser() != "") {
                bucketContext.request_options.proxy_auth = const_cast<char *>(bucketContext.GetProxyUser().c_str());
            }
            bucketContext.request_options.bbr_switch =
                (bucketContext.GetBbrSwitch() == 0) ? OBS_BBR_CLOSE : OBS_BBR_OPEN;
            if (S3StatusIsRetryable()) {
                m_useVpp = false;
                bucketContext.request_options.proxy_host = 0;
                bucketContext.request_options.bbr_switch = OBS_BBR_CLOSE;
                bucketContext.request_options.proxy_auth = 0;
                HCP_Log(WARN, LIBS3) << "put_object failed. object name=" << m_objectName
                                     << ", m_completedStatus=" << m_completedStatus << HCPENDLOG;
            }
        }
        if (!m_useVpp) {
            RWRetry(false);
            put_object(&bucketContext, const_cast<char *>(objKey.c_str()), dataLen, &put_properties,
                &serverSideEncryptionParams, &handler, this);
        }
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    size_t sendSize = 0;
    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("SendData");
        HCP_Log(ERR, LIBS3) << "SendData failed, duration=" << timer.Duration() << HCPENDLOG;
    } else {
        sendSize = dataLen;
    }

    HCP_Log(DEBUG, LIBS3) << "sendSize: " << sendSize << ". WriteDataToS3 End, object name=" << m_objectName
                          << ", request ID:" << m_requestID << ", duration = " << timer.Duration() << HCPENDLOG;

    return sendSize;
}

boost::tribool S3Base::FileExists(S3BucketContextProxy &bucketContext, const string &fileName)
{
    string str;
    return FileExists(bucketContext, fileName, nullptr, nullptr, str);
}

boost::tribool S3Base::FileExists(S3BucketContextProxy &bucketContext, const string &fileName, uint64_t *contentLength,
    int64_t *lastModified, string &etag)
{
    HCP_Log(DEBUG, LIBS3) << "fileName = " << fileName << HCPENDLOG;

    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(fileName);

    server_side_encryption_params serverSideEncryptionParams;
    InitEncryptionParams(serverSideEncryptionParams);
    obs_object_info object_info;
    if (memset_s(&object_info, sizeof(obs_object_info), 0, sizeof(obs_object_info)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
    }
    object_info.key = const_cast<char *>(fileName.c_str());
    object_info.version_id = 0;

    Timer timer;

    do {
        get_object_metadata(&bucketContext, &object_info, &serverSideEncryptionParams, &responseHandler, this);
    } while (S3HeadStatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("FileExists");
        if (OBS_STATUS_NoSuchKey == m_completedStatus || OBS_STATUS_NoSuchBucket == m_completedStatus ||
            OBS_STATUS_HttpErrorNotFound == m_completedStatus) {
            HCP_Log(DEBUG, LIBS3) << "File of " << fileName << " is not exists,request ID : " << m_requestID
                                  << ",duration = " << timer.Duration() << HCPENDLOG;
            return false;
        }

        return boost::indeterminate;
    }

    if (contentLength) {
        *contentLength = m_contentLength;
    }
    if (lastModified) {
        *lastModified = m_lastModified;
    }
    etag = m_etag;

    HCP_Log(DEBUG, LIBS3) << "FileExists successfully ,request ID : " << m_requestID
                          << ",duration = " << timer.Duration() << HCPENDLOG;
    return true;
}

bool S3Base::ListBucket(
    S3BucketContextProxy &bucketContext, const char *prefix, const char *marker, const char *delimiter, int maxkeys)
{
    m_matchObjList.clear();
    m_matchDirList.clear();
    m_objectInfoList.clear();

    obs_list_objects_handler listBucketHandler = {{&ResponseCallback, &CompleteCallback}, &ListBucketCallback};

    HCP_Log(DEBUG, LIBS3) << "prefix = " << prefix << HCPENDLOG;
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(prefix);

    if (marker == nullptr) {
        if (sprintf_s(m_ListBucketInfo.nextMarker, sizeof(m_ListBucketInfo.nextMarker), "%s", "") == -1) {
            HCP_Log(ERR, LIBS3) << "sprintf_s failed" << HCPENDLOG;
        }
    } else {
        if (sprintf_s(m_ListBucketInfo.nextMarker, sizeof(m_ListBucketInfo.nextMarker), "%s", marker) == -1) {
            HCP_Log(ERR, LIBS3) << "sprintf_s failed" << HCPENDLOG;
        }
    }
    m_ListBucketInfo.keyCount = 0;

    do {
        m_ListBucketInfo.isTruncated = 0;
        Timer timer;
        do {
            list_bucket_objects(&bucketContext,
                const_cast<char *>(prefix),
                const_cast<char *>(m_ListBucketInfo.nextMarker),
                const_cast<char *>(delimiter),
                maxkeys,
                &listBucketHandler,
                this);
            // 此接口也是通过nextmarker去控制，不会出现数据拼接问题;
        } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));
        HCP_Log(DEBUG, LIBS3) << "ListObjects in bucket End, bucket name=" << bucketContext.bucket_options.bucket_name
                              << ", prefix=" << m_objectName << ", duration=" << timer.Duration() << HCPENDLOG;
        if (m_completedStatus != OBS_STATUS_OK) {
            PrintStatusErr("ListBucket");
            return false;
        }
    } while ((m_ListBucketInfo.isTruncated != 0) && ((maxkeys == 0) || (m_ListBucketInfo.keyCount < maxkeys)));

    HCP_Log(DEBUG, LIBS3) << "S3 list bucket exit successfully." << HCPENDLOG;

    return true;
}

bool S3Base::RemoveFile(S3BucketContextProxy &bucketContext, const string &fileName)
{
    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    obs_object_info object_info;
    if (memset_s(&object_info, sizeof(obs_object_info), 0, sizeof(obs_object_info)) != 0) {
        HCP_Log(ERR, LIBS3) << "memset_s failed" << HCPENDLOG;
    }
    object_info.key = const_cast<char *>(fileName.c_str());
    object_info.version_id = 0;

    m_completedStatus = OBS_STATUS_ErrorUnknown;
    SetObjectName(fileName);
    Timer timer;
    do {
        delete_object(&bucketContext, &object_info, &responseHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    bool ret = true;
    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("RemoveFile");
        if (m_completedStatus != OBS_STATUS_NoSuchBucket && m_completedStatus != OBS_STATUS_NoSuchKey &&
            m_completedStatus != OBS_STATUS_HttpErrorNotFound) {
            HCP_Log(INFO, LIBS3) << "The file of " << fileName << " is not on storage."
                                 << ". DeleteObject End, duration=" << timer.Duration() << HCPENDLOG;
            ret = false;
        }
    }
    HCP_Log(DEBUG, LIBS3) << "remove from UDS successfully. File: " << fileName
                          << ". DeleteObject End, duration=" << timer.Duration() << HCPENDLOG;

    return ret;
}

bool S3Base::BigFileUploadInit(S3BucketContextProxy &bucketCtx, const string &objName, string &uploadTaskID)
{
    HCP_Log(DEBUG, LIBS3) << "enter BigFileUploadInit" << HCPENDLOG;
    char szUpload[256] = {0};
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    m_UploadPartInfo.originalContentLength = 0;
    m_UploadPartInfo.currentReadLength = 0;
    m_UploadPartInfo.contentLength = 0;
    m_UploadPartInfo.fp = nullptr;
    m_UploadPartInfo.uploadpartNum = 0;

    m_PartsInfoVector.clear();

    obs_put_properties stPutProperties;
    InitPutProperties(&stPutProperties);
    stPutProperties.content_type = const_cast<char *>("text/plain");

    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};
    Timer timer;
    do {
        SetObjectName(objName);
        initiate_multi_part_upload(&bucketCtx,
            const_cast<char *>(objName.c_str()),
            sizeof(szUpload),
            szUpload,
            &stPutProperties,
            0,
            &responseHandler,
            this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));
    if (m_completedStatus != OBS_STATUS_OK) {
        PrintStatusErr("InitiateMultipartUpload");
        HCP_Log(ERR, LIBS3) << "InitiateMultipartUpload End, object name=" << objName
                            << ", duration=" << timer.Duration() << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "szUpload=" << szUpload << ",szUpload size=" << sizeof(szUpload) << HCPENDLOG;
    HCP_Log(INFO, LIBS3) << "InitiateMultipartUpload successfully."
                         << " object name=" << objName << ", duration=" << timer.Duration() << HCPENDLOG;

    uploadTaskID = szUpload;
    return true;
}

bool S3Base::MultipartUploadCancel(
    S3BucketContextProxy &bucketCtx, const string &remote_objName, const string &uploadTaskID)
{
    HCP_Log(DEBUG, LIBS3) << "enter MultipartUploadCancel" << DBG(remote_objName) << HCPENDLOG;

    obs_response_handler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    SetObjectName(remote_objName);
    do {
        abort_multi_part_upload(&bucketCtx,
            const_cast<char *>(remote_objName.c_str()),
            const_cast<char *>(uploadTaskID.c_str()),
            &responseHandler,
            this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != OBS_STATUS_OK && m_completedStatus != OBS_STATUS_NoSuchUpload) {
        PrintStatusErr("AbortMultipartUpload");
        HCP_Log(ERR, LIBS3) << "AbortMultipartUpload faied!" << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "AbortMultipartUpload success!!" << DBG(remote_objName) << HCPENDLOG;
    return true;
}

#else

void S3Base::ReadDataFromS3(S3BucketContextProxy &bucketContext, size_t offset, size_t dataLen)
{
    S3GetObjectHandler getObjectHandler = {{&ResponseCallback, &CompleteCallback}, &GetObjectDataCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    char *ssecCustomerAlgorithm = 0;
    char use_ssec = 0;
    string base = Base64Encode();
    string md5 = DekToMd5();
    if (m_dek.length() > 0) {
        use_ssec = 1;
        ssecCustomerAlgorithm = const_cast<char *>(ALGORITHM);
    }

    ServerSideEncryptionParams serverSideEncryptionParams = {
        0,
        use_ssec,
        0,
        0,
        0,
        ssecCustomerAlgorithm,
        const_cast<char *>(base.c_str()),
        const_cast<char *>(md5.c_str()),
        0,
        0,
        0
    };

    m_completedStatus = S3StatusErrorUnknown;
    SetObjectName(m_objKey);
    Timer timer;

    RWBegin();

    do {
        GetObjectWithServerSideEncryption(&bucketContext,
            (const char *)m_objKey,
            0,
            0,
            offset,
            dataLen,
            &serverSideEncryptionParams,
            0,
            &getObjectHandler,
            this);

        if (S3StatusIsRetryable()) {
            RWRetry(true);
        }
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != S3StatusOK) {
        PrintStatusErr("RecvData");
    }

    HCP_Log(DEBUG, LIBS3) << "ReadDataFromS3 object name=" << m_objectName << ", duration=" << timer.Duration()
                          << HCPENDLOG;
}

// 保留老的调用是因为DGW可能用的是老eSDK;
size_t S3Base::WriteDataToS3(
    S3BucketContextProxy &bucketContext, S3PutObjectHandler &handler, const string objKey, size_t dataLen)
{
    char *ssecCustomerAlgorithm = 0;
    char use_ssec = 0;
    string base = Base64Encode();
    string md5 = DekToMd5();
    if (m_dek.length() > 0) {
        use_ssec = 1;
        ssecCustomerAlgorithm = const_cast<char *>(ALGORITHM);
    }

    ServerSideEncryptionParams serverSideEncryptionParams = {
        0,
        use_ssec,
        0,
        0,
        0,
        ssecCustomerAlgorithm,
        const_cast<char *>(base.c_str()),
        const_cast<char *>(md5.c_str()),
        0,
        0,
        0
    };

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    m_completedStatus = S3StatusErrorUnknown;
    SetObjectName(m_objKey);
    RWBegin();
    do {
        RWRetry(false);
        PutObjectWithServerSideEncryption(
            &bucketContext, objKey.c_str(), dataLen, 0, &serverSideEncryptionParams, 0, &handler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    size_t sendSize = 0;
    if (m_completedStatus != S3StatusOK) {
        PrintStatusErr("SendData");
        HCP_Log(ERR, LIBS3) << "SendData failed." << HCPENDLOG;
    } else {
        sendSize = dataLen;
    }

    HCP_Log(DEBUG, LIBS3) << "sendSize: " << sendSize << HCPENDLOG;
    return sendSize;
}

boost::tribool S3Base::FileExists(S3BucketContextProxy &bucketContext, const string &fileName)
{
    HCP_Log(DEBUG, LIBS3) << "fileName = " << fileName << HCPENDLOG;

    S3ResponseHandler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    m_completedStatus = S3StatusErrorUnknown;
    SetObjectName(fileName);

    char *ssecCustomerAlgorithm = 0;
    char use_ssec = 0;
    string base = Base64Encode();
    string md5 = DekToMd5();
    if (m_dek.length() > 0) {
        use_ssec = 1;
        ssecCustomerAlgorithm = const_cast<char *>(ALGORITHM);
    }

    ServerSideEncryptionParams serverSideEncryptionParams = {
        0,
        use_ssec,
        0,
        0,
        0,
        ssecCustomerAlgorithm,
        const_cast<char *>(base.c_str()),
        const_cast<char *>(md5.c_str()),
        0,
        0,
        0
    };

    do {
        GetObjectMetadataWithServerSideEncryption(
            &bucketContext, fileName.c_str(), 0, &serverSideEncryptionParams, 0, &responseHandler, this);
    } while (S3HeadStatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != S3StatusOK) {
        PrintStatusErr("FileExists");
        if (S3StatusNoSuchKey == m_completedStatus || S3StatusNoSuchBucket == m_completedStatus ||
            S3StatusHttpErrorNotFound == m_completedStatus) {
            HCP_Log(DEBUG, LIBS3) << "File of " << fileName << " is not exists." << HCPENDLOG;
            return false;
        }

        return boost::indeterminate;
    }
    HCP_Log(DEBUG, LIBS3) << "GetObjectMetadata successfully." << HCPENDLOG;

    return true;
}

bool S3Base::ListBucket(
    S3BucketContextProxy &bucketContext, const char *prefix, const char *marker, const char *delimiter, int maxkeys)
{
    m_matchObjList.clear();
    m_matchDirList.clear();
    m_objectInfoList.clear();

    S3ListBucketHandler listBucketHandler = {{&ResponseCallback, &CompleteCallback}, &ListBucketCallback};

    HCP_Log(DEBUG, LIBS3) << "prefix = " << prefix << HCPENDLOG;
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    m_completedStatus = S3StatusErrorUnknown;
    SetObjectName(prefix);

    if (sprintf_s(m_ListBucketInfo.nextMarker, sizeof(m_ListBucketInfo.nextMarker), "%s", marker) == -1) {
        HCP_Log(ERR, LIBS3) << "sprintf_s failed" << HCPENDLOG;
    }
    m_ListBucketInfo.keyCount = 0;

    do {
        m_ListBucketInfo.isTruncated = 0;
        Timer timer;
        do {
            ListObjects(
                &bucketContext, prefix, m_ListBucketInfo.nextMarker, delimiter, maxkeys, 0, &listBucketHandler, this);
            // 此接口也是通过nextmarker去控制，不会出现数据拼接问题;
        } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));
        HCP_Log(DEBUG, LIBS3) << "ListObjects in bucket End, bucket name=" << bucketContext.bucketName
                              << ", prefix=" << m_objectName << ", duration=" << timer.Duration() << HCPENDLOG;
        if (m_completedStatus != S3StatusOK) {
            PrintStatusErr("ListBucket");
            return false;
        }
    } while (m_ListBucketInfo.isTruncated && (!maxkeys || (m_ListBucketInfo.keyCount < maxkeys)));

    HCP_Log(DEBUG, LIBS3) << "S3 list bucket exit successfully." << HCPENDLOG;

    return true;
}

bool S3Base::RemoveFile(S3BucketContextProxy &bucketContext, const string &fileName)
{
    S3ResponseHandler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    m_completedStatus = S3StatusErrorUnknown;
    SetObjectName(fileName);
    Timer timer;
    do {
        DeleteObject(&bucketContext, fileName.c_str(), 0, 0, &responseHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    bool ret = true;
    if (m_completedStatus != S3StatusOK) {
        PrintStatusErr("RemoveFromUDS");
        if (m_completedStatus != S3StatusNoSuchBucket && S3StatusNoSuchKey != m_completedStatus) {
            HCP_Log(INFO, LIBS3) << "The file of " << fileName << " is not on storage."
                                 << ". DeleteObject End, duration=" << timer.Duration() << HCPENDLOG;
            ret = false;
        }
    }
    HCP_Log(DEBUG, LIBS3) << "remove from UDS successfully. File: " << fileName
                          << ". DeleteObject End, duration=" << timer.Duration() << HCPENDLOG;

    return ret;
}

bool S3Base::BigFileUploadInit(S3BucketContextProxy &bucketCtx, const string &objName, string &uploadTaskID)
{
    HCP_Log(DEBUG, LIBS3) << "enter BigFileUploadInit" << HCPENDLOG;
    char szUpload[256] = {0};
    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();

    m_UploadPartInfo.originalContentLength = 0;
    m_UploadPartInfo.currentReadLength = 0;
    m_UploadPartInfo.contentLength = 0;
    m_UploadPartInfo.fp = 0;
    m_UploadPartInfo.uploadpartNum = 0;

    const char *pcCacheControl = 0
    const char *pcContentType = "text/plain";
    const char *pcMD5 = 0;
    const char *pcContentDispositionFilename = 0;
    const char *pcContentEncoding = 0;
    const char *pcWebrl = 0;
    S3PutProperties stPutProperties = {
        pcContentType, pcMD5, pcCacheControl, pcContentDispositionFilename, pcContentEncoding, 0, pcWebrl, 0, 0, 0, -1,
        S3CannedAclPrivate, 0, 0, 0
    };

    S3ResponseHandler responseHandler = {&ResponseCallback, &CompleteCallback};
    Timer timer;
    do {
        SetObjectName(objName);
        InitiateMultipartUpload(
            &bucketCtx, objName.c_str(), &stPutProperties, sizeof(szUpload), szUpload, 0, &responseHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));
    if (m_completedStatus != S3StatusOK) {
        PrintStatusErr("InitiateMultipartUpload");
        HCP_Log(DEBUG, LIBS3) << "InitiateMultipartUpload End, object name=" << objName
                              << ", duration=" << timer.Duration() << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "szUpload=" << szUpload << ",szUpload size=" << sizeof(szUpload) << HCPENDLOG;
    HCP_Log(INFO, LIBS3) << "InitiateMultipartUpload successfully."
                         << " object name=" << objName << ", duration=" << timer.Duration() << HCPENDLOG;

    uploadTaskID = szUpload;
    return true;
}

bool S3Base::MultipartUploadCancel(
    S3BucketContextProxy &bucketCtx, const string &remote_objName, const string &uploadTaskID)
{
    HCP_Log(DEBUG, LIBS3) << "enter MultipartUploadCancel" << HCPENDLOG;

    S3ResponseHandler responseHandler = {&ResponseCallback, &CompleteCallback};

    int retryTimes = GetS3RetryTimeAndSetConnectTImeOut();
    SetObjectName(remote_objName);
    do {
        AbortMultipartUpload(&bucketCtx, remote_objName.c_str(), uploadTaskID.c_str(), 0, &responseHandler, this);
    } while (S3StatusIsRetryable() && ShouldRetry(retryTimes));

    if (m_completedStatus != S3StatusOK) {
        PrintStatusErr("AbortMultipartUpload");
        HCP_Log(ERR, LIBS3) << "AbortMultipartUpload faied!" << HCPENDLOG;
        return false;
    }
    HCP_Log(DEBUG, LIBS3) << "AbortMultipartUpload success!!" << HCPENDLOG;
    return true;
}

#endif