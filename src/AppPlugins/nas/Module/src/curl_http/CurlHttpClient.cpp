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
#include "curl_http/CurlHttpClient.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <ostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "define/Types.h"
#include "securec.h"
#include "curl_http/HttpStatus.h"
#include "common/CleanMemPwd.h"
#include "common/File.h"
#include "kmcv3.h"
#include "log/Log.h"

namespace {
const std::string HTTP_CLIENT_MODULE_NAME = "HttpClient";
const std::string SERVER_KEY_PASSWORD_FILE = "/opt/logpath/protectmanager/cert/internal/OpenAPI/OpenAPI.cnf";
const std::string CERT_FILE  = "/opt/logpath/protectmanager/cert/internal/OpenAPI/OpenAPI.crt.pem";
const std::string KEY_FILE = "/opt/logpath/protectmanager/cert/internal/OpenAPI/OpenAPI.pem";
const std::string CA_CERT_FILE = "/opt/logpath/protectmanager/cert/CA/certs/ca.crt.pem";

const std::string INTERNAL_CERT_FILE = "/opt/logpath/infrastructure/cert/internal/internal.crt.pem";
const std::string INTERNAL_KEY_FILE = "/opt/logpath/infrastructure/cert/internal/internal.pem";
const std::string INTERNAL_KEY_PASSWD_FILE = "/opt/logpath/infrastructure/cert/internal/internal_cert";
const std::string KMCLOGModuleName = "datamoveengine";
const std::string NONE_CERT_STRING = "none";

const int32_t MSG_BLK_SIZE = 2048;
const int32_t MAX_CURL_NUM = 100;
const int32_t MAX_REQUEST_NUM = 1024;
const int32_t LOOP_WAIT_MAX_MS = 100;
const int32_t SUBSTR_LENGTH5 = 5;
const int32_t SUBSTR_LENGTH6 = 6;
const int32_t NET_HTTP_HEAD_LEN = 3;
const uint32_t COUNT_NUM_TWO = 2;
const int32_t EXEC_SUCCESS = 1;
const int32_t RETURN_VALUE = 0;

struct FileStruct {
    const char* fileName;
    size_t fileSize;
};
}
namespace Module {
namespace {
    int32_t DecryptForInternal(std::string &plainText)
    {
        if (!CFile::FileExist(INTERNAL_KEY_PASSWD_FILE.c_str())) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "File is not exist, path is:"
                << INTERNAL_KEY_PASSWD_FILE << HCPENDLOG;
            return FAILED;
        }
        std::ifstream stream;
        stream.open(INTERNAL_KEY_PASSWD_FILE.c_str(), std::ios::in);
        if (!stream.is_open()) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Open file failed, path:" << INTERNAL_KEY_PASSWD_FILE << HCPENDLOG;
            return FAILED;
        }
        std::string cipherText;
        std::getline(stream, cipherText);
        stream.close();
        if (cipherText.empty()) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "CipherText is empty string." << HCPENDLOG;
            return FAILED;
        }
        static std::mutex mutex;
        std::lock_guard<std::mutex> lg(mutex);
        if (g_kmcInstance->Decrypt(plainText, cipherText) != SUCCESS) {
             // 多集群证书同步更新，重新加载初始化kmc
            if (!g_kmcInstance->InitKmc()) {
                HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "KMC Init Failed!" << HCPENDLOG;
                return FAILED;
            }
            if (g_kmcInstance->Decrypt(plainText, cipherText) == SUCCESS) {
                HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "KMC Decrypt Passwd success!" << HCPENDLOG;
                return SUCCESS;
            }
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "KMC decrypt passwd failed!" << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }
}

namespace CurlCertCommon {
CURLcode SSLTwoWayAuthenticationCallbackFunc(CURL* /* curl */, void* sslctx, void* parm)
{
    CURLcode retValue = CURLE_OK;
    MemberCertInfo memberCertInfo = *(static_cast<MemberCertInfo*>(parm));

    if (!memberCertInfo.m_strClusterCA.empty()) {
        retValue = SSLTAuthenticationCA(sslctx, memberCertInfo.m_strClusterCA);
    }

    if (!memberCertInfo.m_strClusterCrl.empty()) {
        retValue = SSLTAuthenticationCrl(sslctx, memberCertInfo.m_strClusterCrl);
    }

    if (!memberCertInfo.m_strClientCrt.empty()) {
        retValue = SSLClientCert(sslctx, memberCertInfo.m_strClientCrt);
    }

    if (!memberCertInfo.m_strClientKey.empty()) {
        retValue = SSLClientKey(sslctx, memberCertInfo.m_strClientKey);
    }

    return retValue;
}

CURLcode SSLTAuthenticationCA(void* &sslctx, const std::string& strClusterCA)
{
    CURLcode retValue = CURLE_ABORTED_BY_CALLBACK;
    X509_STORE* store = nullptr;
    X509* certCA = nullptr;
    BIO* bioCA = nullptr;

    ERR_clear_error();
    do {
        bioCA = BIO_new_mem_buf(strClusterCA.c_str(), -1);
        if (!bioCA) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "bioCA is null." << HCPENDLOG;
            break;
        }

        if (!PEM_read_bio_X509(bioCA, &certCA, nullptr, nullptr)) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "Get certCA is failed." << HCPENDLOG;
            break;
        }

        store = SSL_CTX_get_cert_store((SSL_CTX*)sslctx);
        if (!store) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "store is null." << HCPENDLOG;
            break;
        }

        if (X509_STORE_add_cert(store, certCA) == RETURN_VALUE) {
            unsigned long error = ERR_peek_last_error();
            if (ERR_GET_LIB(error) != ERR_LIB_X509 ||
                ERR_GET_REASON(error) != X509_R_CERT_ALREADY_IN_HASH_TABLE) {
                HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "X509_STORE_add_cert is failed." << HCPENDLOG;
                break;
            }
        }
        retValue = CURLE_OK;
    } while (0);

    BIO_free(bioCA);
    X509_free(certCA);

    ERR_clear_error();

    return retValue;
}

CURLcode SSLTAuthenticationCrl(void* &sslctx, const std::string& strClusterCrl)
{
    CURLcode retValue = CURLE_ABORTED_BY_CALLBACK;
    X509_STORE* store = nullptr;
    X509_CRL* certCrl = nullptr;
    BIO* bioCrl = nullptr;

    ERR_clear_error();
    do {
        bioCrl = BIO_new_mem_buf(strClusterCrl.c_str(), -1);
        if (!bioCrl) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "bioCrl is null." << HCPENDLOG;
            break;
        }

        if (!PEM_read_bio_X509_CRL(bioCrl, &certCrl, nullptr, nullptr)) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Get certCrl is failed." << HCPENDLOG;
            break;
        }

        store = SSL_CTX_get_cert_store((SSL_CTX*)sslctx);
        if (!store) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "store is null." << HCPENDLOG;
            break;
        }

        if (X509_STORE_add_crl(store, certCrl) == RETURN_VALUE) {
            unsigned long error = ERR_peek_last_error();
            if (ERR_GET_LIB(error) != ERR_LIB_X509 ||
                ERR_GET_REASON(error) != X509_R_CERT_ALREADY_IN_HASH_TABLE) {
                HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "X509_STORE_add_crl is failed." << HCPENDLOG;
                break;
            }
        }
        retValue = CURLE_OK;
    } while (0);

    if (BIO_free(bioCrl) == 0) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Biocrl free failed." << HCPENDLOG;
    }
    bioCrl = nullptr;

    X509_CRL_free(certCrl);
    certCrl = nullptr;

    ERR_clear_error();

    return retValue;
}

CURLcode SSLClientCert(void* &sslctx, const std::string& strClientCrt)
{
    CURLcode retValue = CURLE_ABORTED_BY_CALLBACK;
    X509* certClient = nullptr;
    BIO* bioClient = nullptr;

    do {
        bioClient = BIO_new_mem_buf(strClientCrt.c_str(), -1);
        if (bioClient == nullptr) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "bioClient is null.";
            break;
        }

        certClient = PEM_read_bio_X509(bioClient, nullptr, nullptr, nullptr);
        if (certClient == nullptr) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "PEM_read_bio_X509 is failed.";
            break;
        }

        if (SSL_CTX_use_certificate((SSL_CTX*)sslctx, certClient) != EXEC_SUCCESS) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "Use certificate failed";
            break;
        }

        retValue = CURLE_OK;
    } while (0);

    BIO_free(bioClient);
    X509_free(certClient);

    return retValue;
}

CURLcode SSLClientKey(void* &sslctx, const std::string& strClientKey)
{
    CURLcode retValue = CURLE_ABORTED_BY_CALLBACK;
    BIO* bioClientKey = nullptr;
    RSA* rsaClient = nullptr;

    do {
        bioClientKey = BIO_new_mem_buf(strClientKey.c_str(), -1);
        if (bioClientKey == nullptr) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "BIO_new_mem_buf failed,bioClientKey is null.";
            break;
        }

        rsaClient = PEM_read_bio_RSAPrivateKey(bioClientKey, nullptr, nullptr, nullptr);
        if (rsaClient == nullptr) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "The rsaClient is null.";
            break;
        }

        if (SSL_CTX_use_RSAPrivateKey((SSL_CTX*)sslctx, rsaClient) != EXEC_SUCCESS) {
            HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "SSL_CTX_use_RSAPrivateKey failed";
            break;
        }

        retValue = CURLE_OK;
    } while (0);

    BIO_free(bioClientKey);
    RSA_free(rsaClient);

    return retValue;
}
}

CurlHttpResponse::CurlHttpResponse()
    : IHttpResponse(),
      m_StatusCode(0),
      m_ErrorCode(CURLE_FAILED_INIT)
{
    m_Curl = curl_easy_init();
    m_stMemCertInfo.m_strClusterCA = "";
    m_stMemCertInfo.m_strClientCrt = "";
    m_stMemCertInfo.m_strClientKey = "";
}

CurlHttpResponse::~CurlHttpResponse()
{
    if (m_Curl != nullptr) {
        curl_easy_cleanup(m_Curl);
        m_Curl = nullptr;
    }
}

std::string CurlHttpGetPassword()
{
    return "";
}

void CurlHttpResponse::SetCert(int verifyCert, const std::string& cert)
{
    HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "VerifyCert:" << verifyCert << HCPENDLOG;
    std::string outStr;
    std::string plainText;
    if (verifyCert == INTERNAL_VERIFY) {
        if (DecryptForInternal(plainText) != SUCCESS) {
            HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "Failed to decrypt for internal."<< HCPENDLOG;
            return;
        }
    }
    // verfify server cerificate
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYPEER, (verifyCert == DO_NOT_VERIFY) ? false : true);
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYHOST, 0); // don't verify host name
    switch (verifyCert) {
        case DO_NOT_VERIFY:
            break;
        case AGENT_VERIFY:
            curl_easy_setopt(m_Curl, CURLOPT_CAINFO, CA_CERT_FILE.c_str()); // the path of CA
            curl_easy_setopt(m_Curl, CURLOPT_SSLCERT, CERT_FILE.c_str());   // client cert path
            curl_easy_setopt(m_Curl, CURLOPT_SSLCERTTYPE, "PEM");           // client cert type
            curl_easy_setopt(m_Curl, CURLOPT_SSLKEY, KEY_FILE.c_str());     // client cert private key path
            curl_easy_setopt(m_Curl, CURLOPT_SSLKEYTYPE, "PEM");            // client cert private key type
            outStr = CurlHttpGetPassword();
            curl_easy_setopt(m_Curl, CURLOPT_SSLKEYPASSWD, outStr.c_str());
            break;
        case VCENTER_VERIFY:
            curl_easy_setopt(m_Curl, CURLOPT_CAINFO, cert.c_str());
            break;
        case INTERNAL_VERIFY:
            curl_easy_setopt(m_Curl, CURLOPT_CAINFO, INTERNAL_CA_CERT_FILE.c_str());
            curl_easy_setopt(m_Curl, CURLOPT_SSLCERT, INTERNAL_CERT_FILE.c_str());
            curl_easy_setopt(m_Curl, CURLOPT_SSLCERTTYPE, "PEM");
            curl_easy_setopt(m_Curl, CURLOPT_SSLKEY, INTERNAL_KEY_FILE.c_str());
            curl_easy_setopt(m_Curl, CURLOPT_SSLKEYTYPE, "PEM");
            curl_easy_setopt(m_Curl, CURLOPT_SSLKEYPASSWD, plainText.c_str());
            break;
        default:
            break;
    }
    CleanMemoryPwd(plainText);
    return;
}

void CurlHttpResponse::SetTwoWayAuthentication(const HttpRequest& req)
{
    bool verifyCert = req.isVerify == DO_NOT_VERIFY ? false : true;
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYPEER, verifyCert);
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYHOST, 0);

    if (verifyCert) {
        CleanCertInfo();
        m_stMemCertInfo.m_strClusterCA = req.certCA;
        m_stMemCertInfo.m_strClusterCrl = req.revocationList;
        m_stMemCertInfo.m_strClientCrt = req.clientCert;
        m_stMemCertInfo.m_strClientKey = req.clientKey;
        curl_easy_setopt(m_Curl, CURLOPT_SSLCERTTYPE, "PEM");
        curl_easy_setopt(m_Curl, CURLOPT_SSLKEYTYPE, "PEM");
        curl_easy_setopt(m_Curl, CURLOPT_CAINFO, nullptr);
        curl_easy_setopt(m_Curl, CURLOPT_CAPATH, nullptr);
        curl_easy_setopt(m_Curl, CURLOPT_SSL_CTX_FUNCTION, *CurlCertCommon::SSLTwoWayAuthenticationCallbackFunc);
        curl_easy_setopt(m_Curl, CURLOPT_SSL_CTX_DATA, static_cast<void*>(&m_stMemCertInfo));
    }
}

void CurlHttpResponse::SendTwoWayCertRequest(const HttpRequest& req, const uint32_t timeOut)
{
    HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << WIPE_SENSITIVE(req.method) << "," << WIPE_SENSITIVE(req.url) << HCPENDLOG;

    char curl_error_str[CURL_ERROR_SIZE] = {0};
    curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, &CurlHttpResponse::GetDataCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_Curl, CURLOPT_HEADERFUNCTION, &CurlHttpResponse::GetHeaderCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEHEADER, this);
    if (req.specialNetworkCard != "") {
        curl_easy_setopt(m_Curl, CURLOPT_INTERFACE, req.specialNetworkCard.c_str());
    }
    // redirect
    curl_easy_setopt(m_Curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_Curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(m_Curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(m_Curl, CURLOPT_CONNECTTIMEOUT, static_cast<int>(timeOut));
    curl_easy_setopt(m_Curl, CURLOPT_TIMEOUT, static_cast<int>(timeOut));
    curl_easy_setopt(m_Curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(m_Curl, CURLOPT_ERRORBUFFER, curl_error_str);

    SetTwoWayAuthentication(req);
    curl_easy_setopt(m_Curl, CURLOPT_URL, req.url.c_str());
    SetMethod(req.method);
    curl_slist* headers = SetHeaders(req.heads);

    if (!req.body.empty()) {
        curl_easy_setopt(m_Curl, CURLOPT_POSTFIELDS, req.body.c_str());
    }
    uint64_t restryCount = 0;
    while (restryCount < NAME_NOT_RESOLV_RETRY_COUNT) {
        m_ErrorCode = curl_easy_perform(m_Curl);
        if (m_ErrorCode == CURLE_COULDNT_RESOLVE_HOST) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Retry." << HCPENDLOG;
            sleep(NAME_NOT_RESOLV_RETRY_INTERVAL);
            restryCount++;
            continue;
        }
        break;
    }
    if (m_ErrorCode != CURLE_OK) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Http send request failed. Error is"
            << WIPE_SENSITIVE(curl_error_str) << HCPENDLOG;
    }
    if (headers != nullptr) {
        curl_slist_free_all(headers);
    }

    CleanCertInfo();
}

void CurlHttpResponse::SendRequest(const HttpRequest& req, const uint32_t timeOut)
{
    HCP_Log(INFO, HTTP_CLIENT_MODULE_NAME) << "Method: " << req.method
                                        << ", url: " << req.url << ", special card: "
                                        << req.specialNetworkCard << "." << HCPENDLOG;
    char curl_error_str[CURL_ERROR_SIZE] = {0};

    curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, &CurlHttpResponse::GetDataCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_Curl, CURLOPT_HEADERFUNCTION, &CurlHttpResponse::GetHeaderCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEHEADER, this);
    if (req.specialNetworkCard != "") {
        curl_easy_setopt(m_Curl, CURLOPT_INTERFACE, req.specialNetworkCard.c_str());
    }
    // redirect
    curl_easy_setopt(m_Curl, CURLOPT_FOLLOWLOCATION, 1L);
    // set timeout parameters
    SetTimeOut(m_Curl, timeOut);
    curl_easy_setopt(m_Curl, CURLOPT_ERRORBUFFER, curl_error_str);
    if (req.enableProxy) {
        curl_easy_setopt(m_Curl, CURLOPT_PROXY, "protectengine-e-dma:30071");
    }
    // 设置是否验证对端证书
    SetCert(req.isVerify, req.cert);
    // 设置吊销列表
    if (!req.revocationList.empty()) {
        curl_easy_setopt(m_Curl, CURLOPT_CRLFILE, req.revocationList.c_str());
    }
    curl_easy_setopt(m_Curl, CURLOPT_URL, req.url.c_str());
    SetMethod(req.method);
    curl_slist* headers = SetHeaders(req.heads);

    if (!req.body.empty()) {
        if (req.method == "GET") {
            curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, "GET");
        }
        curl_easy_setopt(m_Curl, CURLOPT_POSTFIELDS, req.body.c_str());
    }
    uint64_t restryCount = 0;
    while (restryCount < NAME_NOT_RESOLV_RETRY_COUNT) {
        m_ErrorCode = curl_easy_perform(m_Curl);
        if (m_ErrorCode == CURLE_COULDNT_RESOLVE_HOST) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "retry" << HCPENDLOG;
            sleep(NAME_NOT_RESOLV_RETRY_INTERVAL);
            restryCount++;
            continue;
        }
        break;
    }
    if (m_ErrorCode != CURLE_OK) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Http send request failed. Error is"
            << curl_error_str << HCPENDLOG;
    }
    if (headers != nullptr) {
        curl_slist_free_all(headers);
    }
}

void CurlHttpResponse::SetTimeOut(CURL* curlPtr, const uint32_t timeOut)
{
    curl_easy_setopt(curlPtr, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curlPtr, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curlPtr, CURLOPT_CONNECTTIMEOUT, static_cast<int>(timeOut));
    curl_easy_setopt(curlPtr, CURLOPT_TIMEOUT, static_cast<int>(timeOut));
    curl_easy_setopt(curlPtr, CURLOPT_VERBOSE, 0);
}

bool CurlHttpResponse::PerformUpload(const std::string& attachmentPath)
{
    if (attachmentPath.empty()) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Attachment filepath is empty." << HCPENDLOG;
        return false;
    }

    char path[PATH_MAX + 1] = {0};
    if (attachmentPath.length() > PATH_MAX || realpath(attachmentPath.c_str(), path) == nullptr) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "PerformUpload() attachment file path not exist"
            << ", attachmentPath is:" << attachmentPath << ", path is:" << path << HCPENDLOG;
        return false;
    }
    path[PATH_MAX] = '\0';

    // open file to upload
    FILE *fd = fopen(path, "rb");
    if (fd == nullptr) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Open attachment file failed, path is:" << path << HCPENDLOG;
        return false;
    }

    // to get the file size
    struct stat fileInfo;
    if (fstat(fileno(fd), &fileInfo) != 0) {
        // can't continue
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Get the attachment file size failed, path is:" << path << HCPENDLOG;
        fclose(fd);
        return false;
    }
    // tell it to "upload" to the URL
    curl_easy_setopt(m_Curl, CURLOPT_UPLOAD, 1L);
    // set where to read from
    curl_easy_setopt(m_Curl, CURLOPT_READDATA, fd);
    // and give the size of the upload
    curl_easy_setopt(m_Curl, CURLOPT_INFILESIZE_LARGE,
        (curl_off_t)fileInfo.st_size);

    m_ErrorCode = curl_easy_perform(m_Curl);
    if (CURLE_OK != m_ErrorCode) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Http send request failed." << HCPENDLOG;
    }

    // close file and return
    fclose(fd);
    return true;
}

void CurlHttpResponse::SetCertParam(const HttpRequest& req)
{
    if (req.ifConfirmbyCACert) {
        HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "Http url:" << req.url.c_str() << "], [domain is:"
            << req.domainInfo.c_str() << "]" << HCPENDLOG;
        curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYPEER, req.isVerify == DO_NOT_VERIFY ? false : true);
        curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYHOST, 0);
        if (req.isVerify == VCENTER_VERIFY) {
            // Convert domain
            struct curl_slist* host = curl_slist_append(nullptr, req.domainInfo.c_str());
            if (host != nullptr && req.ifCheckDomain) {
                curl_easy_setopt(m_Curl, CURLOPT_RESOLVE, host);
            }
            // CA cert confirm
            curl_easy_setopt(m_Curl, CURLOPT_CAINFO, req.cert.c_str());
        }
        // username confirm
        curl_easy_setopt(m_Curl, CURLOPT_USERPWD, req.vSphereLoginInfo.c_str());
    } else {
        SetMethod(req.method);
        SetHeaders(req.heads);
        if (req.url.find("https://") == std::string::npos) {
            SetCert(req.cert);
        }
    }
    if (req.enableProxy) {
        curl_easy_setopt(m_Curl, CURLOPT_PROXY, "protectengine-e-dma:30071");
    }
}

void CurlHttpResponse::DownloadAttchment(const HttpRequest& req, const uint32_t timeOut)
{
    HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "Method : " << req.method << ", url : "
        << WIPE_SENSITIVE(req.url) << HCPENDLOG;
    char curl_error_str[CURL_ERROR_SIZE] = { 0 };
    std::string fileSaveName = req.body;
    FileStruct fileStruct = { fileSaveName.c_str(), 0 };
    curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, &CurlHttpResponse::DownloadAttchmentCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &fileStruct);

    curl_easy_setopt(m_Curl, CURLOPT_HEADERFUNCTION, &CurlHttpResponse::GetHeaderCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEHEADER, this);
    curl_easy_setopt(m_Curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_Curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(m_Curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(m_Curl, CURLOPT_CONNECTTIMEOUT, (int)timeOut);
    curl_easy_setopt(m_Curl, CURLOPT_TIMEOUT, (int)timeOut);
    curl_easy_setopt(m_Curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(m_Curl, CURLOPT_ERRORBUFFER, curl_error_str);

    curl_easy_setopt(m_Curl, CURLOPT_URL, req.url.c_str());
    SetCertParam(req);
    m_ErrorCode = curl_easy_perform(m_Curl);
    if (m_ErrorCode != CURLE_OK) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Http send request failed. Error is"
            << WIPE_SENSITIVE(curl_error_str) << HCPENDLOG;
    }
}

bool CurlHttpResponse::UploadAttachment(const HttpRequest& req, const uint32_t timeOut)
{
    HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "Method : " << req.method
        << ", url : " << WIPE_SENSITIVE(req.url) << HCPENDLOG;

    char curl_error_str[CURL_ERROR_SIZE] = { 0 };
    curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, &CurlHttpResponse::GetDataCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(m_Curl, CURLOPT_HEADERFUNCTION, &CurlHttpResponse::GetHeaderCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEHEADER, this);
    curl_easy_setopt(m_Curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_Curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(m_Curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(m_Curl, CURLOPT_CONNECTTIMEOUT, (int)timeOut);
    curl_easy_setopt(m_Curl, CURLOPT_TIMEOUT, (int)timeOut);
    curl_easy_setopt(m_Curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(m_Curl, CURLOPT_ERRORBUFFER, curl_error_str);

    curl_easy_setopt(m_Curl, CURLOPT_URL, req.url.c_str());
    SetCertParam(req);
    if (PerformUpload(req.body) == false) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Perform attachment failed. Error is "
            << WIPE_SENSITIVE(curl_error_str) << HCPENDLOG;
        return false;
    }
    return true;
}

size_t CurlHttpResponse::DownloadAttchmentCallback(void* ptr, size_t size, size_t count, void* self)
{
    if (ptr == nullptr) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "GetDataCallbackDownload, ptr is null" << HCPENDLOG;
        return SUCCESS;
    }

    struct FileStruct* pThis = (struct FileStruct *)self;
    std::string filePath = pThis->fileName;
    std::ofstream fileStream(filePath.c_str(), std::ios_base::app | std::ios_base::binary);
    if (!fileStream) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Open attachment file failed." << HCPENDLOG;
        return SUCCESS;
    }
    char* buff = (char*)ptr;
    size_t fileLen = size * count;
    size_t loopCount = fileLen / MSG_BLK_SIZE;
    size_t restLen = fileLen % MSG_BLK_SIZE;
    for (size_t i = 0; i < loopCount; ++i) {
        fileStream.write(buff + i*MSG_BLK_SIZE, MSG_BLK_SIZE);
    }
    if (restLen) {
        fileStream.write(buff + loopCount*MSG_BLK_SIZE, restLen);
    }

    fileStream.close();
    return fileLen;
}

bool CurlHttpResponse::Success()
{
    return ((m_StatusCode == SC_OK) || (m_StatusCode == SC_CREATED)) && (m_ErrorCode == CURLE_OK);
}

bool CurlHttpResponse::Busy(void)
{
    return (m_StatusCode == SC_SERVICE_UNAVAILABLE);
}

uint32_t CurlHttpResponse::GetStatusCode()
{
    return m_StatusCode;
}


void CurlHttpResponse::SetMethod(const std::string& method)
{
    std::string str("GET");
    if (!method.empty()) {
        str = method;
    }

    (void)std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    if (str == "GET") {
        curl_easy_setopt(m_Curl, CURLOPT_HTTPGET, 1L);
    } else if (str == "POST") {
        curl_easy_setopt(m_Curl, CURLOPT_POST, 1L);
    } else {
        curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, str.c_str());
    }
}


curl_slist* CurlHttpResponse::SetHeaders(const std::set<std::pair<std::string, std::string> >& heads)
{
    struct curl_slist* slist = nullptr;

    for (std::set<std::pair<std::string, std::string> >::const_iterator it = heads.begin();
            it != heads.end(); ++it) {
        std::string heard_string = it->first + ": " + it->second;
        slist = curl_slist_append(slist, heard_string.c_str());
    }

    std::string type = "Content-Type: application/json";
    slist = curl_slist_append(slist, type.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, slist);
    return slist;
}

void CurlHttpResponse::SetCert(const std::string& cert)
{
    // not move
}

size_t CurlHttpResponse::GetDataCallback(void* ptr, size_t size, size_t count, void* self)
{
    if ((ptr == nullptr) || (self == nullptr)) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Ptr or self is null." << HCPENDLOG;
        return SUCCESS;
    }

    CurlHttpResponse* pThis = (CurlHttpResponse*)self;
    pThis->RecieveData(std::string((char*)ptr, size * count));
    return size * count;
}

size_t CurlHttpResponse::GetHeaderCallback(void* ptr, size_t size, size_t count, void* self)
{
    if ((ptr == nullptr) || (self == nullptr)) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Ptr or self is null." << HCPENDLOG;
        return SUCCESS;
    }
    CurlHttpResponse* pThis = (CurlHttpResponse*)self;
    std::string data((char*)ptr, size * count);

    if (data.find_first_of(":") == std::string::npos) {
        pThis->ParseStatusLine(data);
    } else {
        pThis->RecieveHeader(data);
    }
    return size * count;
}

void CurlHttpResponse::CleanCertInfo()
{
    CleanMemoryPwd(m_stMemCertInfo.m_strClusterCA);
    CleanMemoryPwd(m_stMemCertInfo.m_strClientCrt);
    CleanMemoryPwd(m_stMemCertInfo.m_strClientKey);
    m_stMemCertInfo.m_strClusterCA.clear();
    m_stMemCertInfo.m_strClientCrt.clear();
    m_stMemCertInfo.m_strClientKey.clear();
}

void CurlHttpResponse::ParseStatusLine(const std::string& status_line)
{
    std::vector<std::string> strs;

    (void)boost::split(strs, status_line, boost::is_any_of(" "));

    if (strs.size() > 1) {
        try {
            boost::trim(strs[1]);
            m_StatusCode = boost::lexical_cast<uint32_t>(strs[1]);

            for (size_t index = 2; index < strs.size(); ++index) {
                m_StatusDescribe.append(" " + strs[index]);
            }
            boost::trim(m_StatusDescribe);
        }
        catch(std::exception &e) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Parse status failed.Status line:" << status_line << HCPENDLOG;
        }
    }
}

void CurlHttpResponse::RecieveHeader(const std::string& header)
{
    std::vector<std::string> strs;
    (void)boost::split(strs, header, boost::is_any_of(":"));
    if (strs.size() == COUNT_NUM_TWO) {
        std::string key(strs[0]), value(strs[1]);
        boost::trim(key);
        boost::trim(value);
        (void)m_Headers[key].insert(value);
    }
}

uint32_t CurlHttpResponse::GetHttpStatusCode()
{
    return m_StatusCode;
}

std::string CurlHttpResponse::GetHttpStatusDescribe()
{
    return m_StatusDescribe;
}

int32_t CurlHttpResponse::GetErrCode()
{
    return m_ErrorCode;
}

std::string CurlHttpResponse::GetErrString()
{
    std::string strErr("Unknow error!");
    const char* errDes = curl_easy_strerror((CURLcode)m_ErrorCode);
    if (errDes != nullptr) {
        strErr = errDes;
    }
    return strErr;
}

std::set<std::string> CurlHttpResponse::GetHeadByName(const std::string& header_name)
{
    std::map<std::string, std::set<std::string> >::iterator it = m_Headers.find(header_name);
    if (it != m_Headers.end()) {
        return it->second;
    }
    return std::set<std::string>();
}


const CURL* CurlHttpResponse::GetPtr()
{
    return m_Curl;
}

std::set<std::string> CurlHttpResponse::GetCookies()
{
    std::set<std::string> cookies = GetHeadByName("Set-Cookie");
    if (cookies.empty()) {
        cookies = GetHeadByName("set-cookie");
    }
    return cookies;
}
void CurlHttpResponse::RecieveData(const std::string& data)
{
    m_Body.append(data);
}

std::string CurlHttpResponse::GetBody()
{
    return m_Body;
}
std::map<std::string, std::set<std::string> > CurlHttpResponse::GetHeaders()
{
    return m_Headers;
}

std::shared_ptr<IHttpResponse> CurlHttpClient::SendRequest(const HttpRequest& req, const uint32_t timeOut /* = 90 */)
{
    std::shared_ptr<CurlHttpResponse> rsp = std::make_shared<CurlHttpResponse>();
    if (rsp == nullptr || rsp->GetPtr() == nullptr) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Create curlhttpclient object failed." << HCPENDLOG;
        return std::shared_ptr<CurlHttpResponse>();
    }
    rsp->SendRequest(req, timeOut);
    return rsp;
}

std::shared_ptr<IHttpResponse> CurlHttpClient::SendMemCertRequest(const HttpRequest& req, const uint32_t timeOut)
{
    std::shared_ptr<CurlHttpResponse> rsp = std::make_shared<CurlHttpResponse>();
    if (rsp == nullptr || rsp->GetPtr() == nullptr) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Create curlhttpclient object failed." << HCPENDLOG;
        return std::shared_ptr<CurlHttpResponse>();
    }
    rsp->SendTwoWayCertRequest(req, timeOut);
    return rsp;
}

bool SendHttpRequest(const HttpRequest &req, Json::Value &rsp, uint32_t timeout, bool ifRetry, bool ifCheckBody)
{
    uint32_t retryTime = 0;
    uint32_t retryTimes = ifRetry ? NAME_NOT_RESOLV_RETRY_COUNT : 0;
    do {
        IHttpClient *pHttpClient = IHttpClient::GetInstance();
        if (!pHttpClient) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Get instance prt is empty. " << HCPENDLOG;
            return false;
        }
        std::shared_ptr<IHttpResponse> httpRsp = pHttpClient->SendRequest(req, timeout);
        if (httpRsp == nullptr) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return false;
        }
        std::string errorDes;
        if (!httpRsp->Success()) {
            if (httpRsp->GetErrCode() == 0) {
                errorDes = httpRsp->GetHttpStatusDescribe();
                HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "StatusCode: " << httpRsp->GetHttpStatusCode() <<
                    " , Http response error. Error is " << errorDes << HCPENDLOG;
            } else {
                errorDes = httpRsp->GetErrString();
                HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "StatusCode: " << httpRsp->GetHttpStatusCode() <<
                    " , Send http request occur network error. Error is " << errorDes << HCPENDLOG;
            }
            continue;
        }

        if (httpRsp->GetHttpStatusCode() != SC_OK) {
            HCP_Log(WARN, HTTP_CLIENT_MODULE_NAME) << "HttpStatus:" << httpRsp->GetHttpStatusCode() << "retryTime:" <<
                retryTime << HCPENDLOG;
            sleep(HTTPS_ABNORMAL_RETRY_INTERVAL);
            continue;
        }

        if (ifCheckBody && !JsonHelper::JsonStringToJsonValue(httpRsp->GetBody(), rsp)) {
            HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Parse json failed!" << HCPENDLOG;
            return false;
        }

        return true;
    } while (++retryTime < retryTimes);

    HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "The retryTime exceeds the NAME_NOT_RESOLV_RETRY_COUNT." << HCPENDLOG;
    return false;
}

std::shared_ptr<IHttpResponse> CurlHttpClient::DownloadAttchment(const HttpRequest& req, const uint32_t timeOut)
{
    std::shared_ptr<CurlHttpResponse> rsp = nullptr;
    try {
        rsp = std::make_shared<CurlHttpResponse>();
    } catch (std::exception &e) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME)
            << "DownloadAttchment, create  curlhttpclient object failed." << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
        return std::shared_ptr<CurlHttpResponse>();
    }

    rsp->DownloadAttchment(req, timeOut);
    return rsp;
}

bool CurlHttpClient::UploadAttachment(const HttpRequest& req, std::shared_ptr<IHttpResponse>& rsp,
    const uint32_t timeOut /* = 90 */)
{
    std::shared_ptr<CurlHttpResponse> curlResp = nullptr;
    try {
        curlResp = std::make_shared<CurlHttpResponse>();
    } catch (std::exception &e) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Create curltttpclient object failed."
            << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
        return false;
    }

    rsp = curlResp;
    return curlResp->UploadAttachment(req, timeOut);
}


std::shared_ptr<IHttpClient> IHttpClient::CreateClient()
{
    return std::make_shared<CurlHttpClient>();
}


IHttpClient* IHttpClient::GetInstance()
{
    IHttpClient* pHttpClient = new (std::nothrow) CurlHttpClient;
    if (pHttpClient == nullptr) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Get curlhttpclient instance failed." << HCPENDLOG;
        return nullptr;
    }
    return pHttpClient;
}

void IHttpClient::ReleaseInstance(IHttpClient* pClient)
{
    if (pClient != nullptr) {
        delete pClient;
        pClient = nullptr;
    }
}

CurlHttpClient::~CurlHttpClient()
{
}

CurlHttpClient::CurlHttpClient()
{
}

std::string CurlHttpClient::FormatUrl(const std::string& fullUrl)
{
    std::string baseUrl = "";
    std::string headUrl = "";
    std::string::size_type head = fullUrl.find("://");
    if (head != std::string::npos) {
        baseUrl = fullUrl.substr(head + NET_HTTP_HEAD_LEN);
        headUrl = fullUrl.substr(0, head + NET_HTTP_HEAD_LEN);
    } else {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Url without http or https." << HCPENDLOG;
        return fullUrl;
    }

    std::string::size_type checkend = fullUrl.find('/', head + NET_HTTP_HEAD_LEN);
    if (checkend == std::string::npos) {
        return fullUrl;
    }
    std::string iPAndPortString = fullUrl.substr(head + NET_HTTP_HEAD_LEN, checkend - head - NET_HTTP_HEAD_LEN);

    unsigned int count = 0;
    for (unsigned int i = 0; i < iPAndPortString.length(); i++) {
        if (iPAndPortString[i] == ':') {
            count ++;
            if (count >= COUNT_NUM_TWO) {
                break;
            }
        }
    }
    if (count <= 1) {
        // this is IPV4
        return fullUrl;
    } else {
        // this is IPV6
        if (iPAndPortString.find('[') != std::string::npos) {
            return fullUrl;
        }
        std::string::size_type last_delim = iPAndPortString.find_last_of(':');
        std::string ipstr = baseUrl.substr(0, last_delim);
        std::string portstr = baseUrl.substr(last_delim + 1);
        std::string ipv6str = headUrl + "[" + ipstr + "]" + ":" + portstr;
        return ipv6str;
    }
}

bool CurlHttpClient::TestConnect(const std::string& oldurl, const uint32_t timeOut /* = 90 */)
{
    // not move
    return true;
}

static size_t WriteData(char* d, size_t n, size_t l, void *p)
{
    (void)d;
    (void)p;
    return n * l;
}

std::string CurlHttpClient::GetCertificate(const std::string& oldurl, const uint32_t timeOut /* = 90 */)
{
    CURLcode res;
    char curl_error_str[CURL_ERROR_SIZE] = {0};
    std::string certStr = "";
    union {
        struct curl_slist *to_info;
        struct curl_certinfo *to_certinfo;
    } ptr;
    ptr.to_info = nullptr;
    ptr.to_certinfo = nullptr;

    std::string url = FormatUrl(oldurl);
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "Connectiong failed." << WIPE_SENSITIVE(url) << HCPENDLOG;
        return "";
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (int)timeOut);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (int)timeOut);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_error_str);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Failed, res:" << res << "," << curl_error_str << HCPENDLOG;
        curl_easy_cleanup(curl);
        return "";
    }

    res = curl_easy_getinfo(curl, CURLINFO_CERTINFO, &ptr.to_info);
    if (res != CURLE_OK) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Curl_easy_getinfo return. res:" << res << HCPENDLOG;
        curl_easy_cleanup(curl);
        return "";
    }
    if (ptr.to_info && ptr.to_certinfo->num_of_certs != 0) {
        struct curl_slist* curlSlist = nullptr;
        for (curlSlist = ptr.to_certinfo->certinfo[0]; curlSlist; curlSlist = curlSlist->next) {
            certStr += "\n" + std::string(curlSlist->data);
        }
        curl_slist_free_all(curlSlist);
    }
    curl_easy_cleanup(curl);
    return certStr;
}

uint32_t CurlHttpClient::GetThunmPrint(const std::string& url, std::string& thunmPrint, const uint32_t timeOut)
{
    return SUCCESS;
}

CurlKmcManagerInterface::CurlKmcManagerInterface()
{
}

CurlKmcManagerInterface::~CurlKmcManagerInterface()
{
    UnInitKmc();
}

bool CurlKmcManagerInterface::InitKmc(const std::string& kmcFile, const std::string& kmcBackupFile)
{
    KMC_RET iRet = InitKMCV3c(kmcFile.c_str(), kmcBackupFile.c_str(), KMCLOGModuleName.c_str());
    if (iRet != KMC_SUCESS) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "InitKmc failed."  << HCPENDLOG;
        m_isInited = false;
        return false;
    }
    m_isInited = true;
    return true;
}

void CurlKmcManagerInterface::UnInitKmc()
{
    if (m_isInited) {
        if (DeInitKmc() != KMC_SUCESS) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Deinit KMC failed." << HCPENDLOG;
        }
    }
}

int32_t CurlKmcManagerInterface::Decrypt(std::string &plainText, const std::string &cipherText)
{
    HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "Enter Decrypt."  << HCPENDLOG;
    char* secertInfo = nullptr;
    if (DecryptV3c(KMC_SHARE_INNER_DOMAIN, &secertInfo, cipherText.c_str()) != KMC_SUCESS) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "Decrypt failed." << HCPENDLOG;
        return FAILED;
    }
    if (secertInfo != nullptr) {
        plainText = secertInfo;
        int len = plainText.length();
        memset_s(secertInfo, len, 0, len);
    }
    if (plainText.empty()) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "PlainText is empty string." << HCPENDLOG;
        return FAILED;
    }
    KmcFree(secertInfo);
    return SUCCESS;
}

int32_t CurlKmcManagerInterface::DecryptV1(std::string &plainText, const std::string &cipherText)
{
    HCP_Log(DEBUG, HTTP_CLIENT_MODULE_NAME) << "Enter DecryptV1."  << HCPENDLOG;
    if (DecryptPwdV1(KMC_SHARE_INNER_DOMAIN, plainText, cipherText) != KMC_SUCESS) {
        HCP_Log(ERR, HTTP_CLIENT_MODULE_NAME) << "DecryptV1 failed." << HCPENDLOG;
        return FAILED;
    }
    return SUCCESS;
}
}
