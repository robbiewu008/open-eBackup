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
#include "device_access/oceanstor/OceanstorNas.h"
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "system/System.hpp"
#include "common/JsonUtils.h"
#include "common/Timer.h"
#include "device_access/Const.h"
#include "common/CleanMemPwd.h"
#include "config_reader/ConfigIniReaderImpl.h"
#include "device_access/oceanstor/OceanstorNasSnapshot.h"

namespace Module {
#define HOST_TYPE 21
#define LINUX_TYPE 0
    namespace {
        constexpr int NUM_2 = 2;
        constexpr int BASE = 10;
        constexpr int SNAPDIFF_DEFAULT_BASE_VAL = 255;
        constexpr int HUNDRED = 100;
        const std::string INNER_SAFE_IP = "protectengine-e-dma.dpa.svc.cluster.local";
        const std::string DEVICE_ID = "xxx";
        constexpr int SEND_REQ_RETRY_INTERVAL = 10;
    }  // namespace
    std::unique_ptr <SessionCache> OceanstorNas::m_oceanstorSessionCache = std::make_unique<SessionCache>(
            OCEANSTOR_MODULE_NAME);

/*
login Oceanstor and get token
Date : 2020/11/05
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.login Oceanstor and get token
             2.cache token for this instance
*/

    OceanstorNas::~OceanstorNas()
    {
        fileSystemId = "";
        if (useCache && m_oceanstorSessionCache != nullptr) {
            if (this->sessionPtr != nullptr) {
                DeleteDeviceSession();
            }
        } else {
            if (this->sessionPtr != nullptr) {
                SessionInfo sessionInfo;
                sessionInfo.token = this->sessionPtr->token;
                sessionInfo.cookie = this->sessionPtr->cookie;
                sessionInfo.device_id = this->sessionPtr->deviceId;
                Logout(sessionInfo);
            }
        }
        IHttpClient::ReleaseInstance(fs_pHttpCLient);
        CleanMemoryPwd(OceanstorPassword);
    }

    SessionInfo OceanstorNas::Login()
    {
        HttpRequest req;
        req.method = "POST";
        req.url = "sessions";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode;
        jsonValue["username"] = OceanstorUsername;
        jsonValue["password"] = OceanstorPassword;
        jsonValue["scope"] = "0";
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        SessionInfo sessionInfo{};
        int iRet = SendRequestEx(req, data, errorDes, errorCode, sessionInfo);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isMember("deviceid") && data.isMember("iBaseToken")) {
                sessionInfo.token = data["iBaseToken"].asString();
                sessionInfo.device_id = data["deviceid"].asString();
            } else {
                iRet = FAILED;
            }
        }
        for (std::string::iterator it = req.body.begin(); it != req.body.end(); ++it) {
            *it = 0;
        }
        return sessionInfo;
    }

/*
logout at destruct this instance
Date : 2020/11/05
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.logout at destruct this instance
*/
    int OceanstorNas::Logout(SessionInfo sessionInfo)
    {
        HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Start Authentication Exit. " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "DELETE";
        req.url = "sessions";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequestEx(req, data, errorDes, errorCode, sessionInfo);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    void OceanstorNas::Clean()
    {
        if (fs_pHttpCLient) {
            fs_pHttpCLient = NULL;
        }
    }

    int OceanstorNas::ParseCookie(const std::set <std::string> &cookie_values, SessionInfo &sessionInfo)
    {
        if (cookie_values.empty()) {
            HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "Cookie is empty." << HCPENDLOG;
            return FAILED;
        }

        std::string cookie_value = *cookie_values.begin();

        std::vector <std::string> strs;
        (void) boost::split(strs, cookie_value, boost::is_any_of(";"));
        if (strs.size() > 0) {
            sessionInfo.cookie = strs[0];
            return SUCCESS;
        }
        return FAILED;
    }

    int OceanstorNas::SendHttpReq(std::shared_ptr <IHttpResponse> &rsp, const HttpRequest &req,
                                  std::string &errorDes, int &errorCode)
    {
        HttpRequest tempReq = req;
        tempReq.url = FormatFullUrl(tempReq.url);
        rsp = fs_pHttpCLient->SendRequest(tempReq, CurlTimeOut);
        if (NULL == rsp.get()) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return FAILED;
        }
        if (!rsp->Success()) {
            // 1.curl success,http response error with http status codes
            if (rsp->GetErrCode() == 0) {
                errorDes = rsp->GetHttpStatusDescribe(); // http status error description
                errorCode = rsp->GetErrCode();
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << tempReq.url << " Curl ok,HttpStatusCode: "
                                                    << rsp->GetHttpStatusCode() << ", Http response error. Error is "
                                                    << errorDes << HCPENDLOG;
                return rsp->GetHttpStatusCode(); // return http status code
                // 2.curl error,need directly retry
            } else {
                errorDes = rsp->GetErrString();
                errorCode = rsp->GetErrCode();
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << tempReq.url << " Curl error. errorCode: "
                                                    << errorCode << "errorDes:" << errorDes << HCPENDLOG;
            }
            SetErrorCode(errorCode);
            return FAILED;
            // 3. curl success, http response success
        } else {
            errorDes = rsp->GetErrString();
            errorCode = rsp->GetErrCode();
            return SUCCESS;
        }
        return SUCCESS;
    }

    int OceanstorNas::SendRequestOnce(HttpRequest req, Json::Value &data, std::string &errorDes, int &errorCode)
    {
        int iRet;
        HttpRequest request = req;
        request.url =
                curl_http + OceanstorIP + ":" + OceanstorPort + "/deviceManager/rest/" + this->sessionPtr->deviceId
                + "/" + req.url;
        request.enableProxy = GetEnableProxy();
        (void) request.heads.insert(std::make_pair(std::string("Cookie"), this->sessionPtr->cookie));
        (void) request.heads.insert(std::make_pair(std::string("iBaseToken"), this->sessionPtr->token));
        std::shared_ptr <IHttpResponse> rsp;
        iRet = SendHttpReq(rsp, request, errorDes, errorCode);
        if (iRet != SUCCESS) {
            // get when curl send success,http response error for httpstatuscodeforRetry
            if (errorCode == 0) {
                return iRet;
            }
            return FAILED;
        }
        iRet = ResponseSuccessHandle(req, rsp, data, errorDes, errorCode);
        return iRet;
    }

    int OceanstorNas::ResponseSuccessHandle(HttpRequest req,
                                            std::shared_ptr <IHttpResponse> &rsp, Json::Value &data,
                                            std::string &errorDes, int &errorCode)
    {
        int Ret = ParseResponse(rsp->GetBody(), data, errorDes, errorCode);
        if (errorCode == OceanstorErrorCode::UNAUTH ||
            errorCode == OceanstorErrorCode::NOUSERPERMISSION ||
            errorCode == OceanstorErrorCode::AUTHIPINCONSISTENCY) {
            SessionInfo sessionInfo = Login();
            if (sessionInfo.device_id.empty() || sessionInfo.token.empty() || sessionInfo.cookie.empty()) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Login Oceanstor Failed! deviceId: " << sessionInfo.device_id
                                                    << HCPENDLOG;
                return FAILED;
            }
            if (this->sessionPtr == nullptr) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Invalid session" << HCPENDLOG;
                return FAILED;
            }
            // Refresh session
            this->sessionPtr->deviceId = sessionInfo.device_id;
            this->sessionPtr->cookie = sessionInfo.cookie;
            this->sessionPtr->token = sessionInfo.token;
            HttpRequest request = req;
            request.url = curl_http + OceanstorIP + ":" + OceanstorPort + "/deviceManager/rest/" +
                          this->sessionPtr->deviceId + "/" + req.url;
            (void) request.heads.insert(std::make_pair(std::string("Cookie"), this->sessionPtr->cookie));
            (void) request.heads.insert(std::make_pair(std::string("iBaseToken"), this->sessionPtr->token));
            Ret = SendHttpReq(rsp, request, errorDes, errorCode);
            if (Ret != SUCCESS) {
                return FAILED;
            }

            Ret = ParseResponse(rsp->GetBody(), data, errorDes, errorCode);
        }
        return Ret;
    }

    void OceanstorNas::DelayTimeSendRequest()
    {
        auto now = std::chrono::steady_clock::now();
        while ((double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -
                                                                             now).count()) *
                std::chrono::microseconds::period::num / std::chrono::microseconds::period::den)
               < retryIntervalTime) {
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Waiting for storage device ... " << HCPENDLOG;
            sleep(SEND_REQ_RETRY_INTERVAL);
        }
        return;
    }

    void OceanstorNas::DeleteDeviceSession()
    {
        m_oceanstorSessionCache->DeleteSession(OceanstorIP, OceanstorUsername, OceanstorPort,
                                               [this](SessionInfo sesInfo) -> int { return Logout(sesInfo); });
    }

    void OceanstorNas::CreateDeviceSession()
    {
        this->sessionPtr = m_oceanstorSessionCache->CreateSession(OceanstorIP, OceanstorUsername, OceanstorPort,
                                                                  [this]() -> SessionInfo { return Login(); });
    }

    void OceanstorNas::LoginAndGetSessionInfo()
    {
        if (useCache && m_oceanstorSessionCache != nullptr) {
            CreateDeviceSession();
        } else {
            SessionInfo sessionInfo = Login();
            if (sessionInfo.token.empty() || sessionInfo.cookie.empty() || sessionInfo.device_id.empty()) {
                this->sessionPtr = nullptr;
            } else {
                this->sessionPtr = std::make_shared<Session>(sessionInfo.token, sessionInfo.device_id,
                                                             sessionInfo.cookie);
            }
        }
        return;
    }

    int OceanstorNas::SendRequest(HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode,
                                  bool lockSession)
    {
        // 检查存储设备是否含有证书和吊销列表信息
        if (!certification.empty()) {
            req.cert = certification;
            req.isVerify = VCENTER_VERIFY;
        }
        req.revocationList = !crl.empty() ? crl : req.revocationList;
        int retryNum = 0;
        bool needRetry = true;
        do {
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "send request for " << (retryNum + 1) << "time" << HCPENDLOG;
            int ret = SUCCESS;
            if (this->sessionPtr == nullptr) {
                if (OceanstorIP == INNER_SAFE_IP) {
                    this->sessionPtr = std::make_shared<Session>("", DEVICE_ID, "");
                } else {
                    LoginAndGetSessionInfo();
                }
                if (this->sessionPtr == nullptr) {
                    HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Invalid session" << HCPENDLOG;
                }
            }
            curl_http = "https://";
            if (OceanstorIP == INNER_SAFE_IP) {
                req.isVerify = INTERNAL_VERIFY;
            }
            req.enableProxy = GetEnableProxy();
            if (this->sessionPtr != nullptr) {
                if (lockSession) {
                    std::lock_guard <std::mutex> lock(this->sessionPtr->sessionMutex);
                    ret = SendRequestOnce(req, data, errorDes, errorCode);
                } else {
                    ret = SendRequestOnce(req, data, errorDes, errorCode);
                }
                if (ret == SUCCESS) {
                    HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "send requests success " << HCPENDLOG;
                    return SUCCESS;
                }
            }
            // 1.when curl success and ret not FAILED, ret is httpStatusCode,
            // so judge whether ret is in httpRspStatusCodeForRetry for retry.
            // 2.when when curl success and ret is FAILED,
            // OceanstorResposeNeedRetry, not judge http retry code, directly retry.
            // 3.when errorCode not 0,mean curl failed,directly retry.
            needRetry =  !(std::find(httpRspStatusCodeForRetry.begin(), httpRspStatusCodeForRetry.end(), ret)
                        == httpRspStatusCodeForRetry.end() && errorCode == 0 && !OceanstorResposeNeedRetry(ret));
            if (needRetry) {
                DelayTimeSendRequest();
                retryNum++;
            } else {
                HCP_Log(WARN, OCEANSTOR_MODULE_NAME) << "not retry send msg for httpstatuscode:" << ret << HCPENDLOG;
            }
        } while (retryNum < retryTimes && needRetry);
        HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "send request failed. " << HCPENDLOG;
        return FAILED;
    }

    int OceanstorNas::QueryFileSystem(DeviceDetails &info)
    {
        if (vstoreId.empty()) {
            GetVstoreId();
        }
        int ret = QueryFileSystem(ResourceName, info);
        if (ret != SUCCESS) {
            return ret;
        }
        fileSystemId = std::to_string(info.deviceId);
        return SUCCESS;
    }

    int OceanstorNas::QueryFileSystem(std::string fileName, DeviceDetails &info)
    {
        if (vstoreId.empty()) {
            GetVstoreId();
        }
        HttpRequest req;
        req.method = "GET";
        req.url = "filesystem?filter=NAME::" + fileName;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
        int errorCode;
        std::string errorDes;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.size() == 1) {
                std::string id = data[0]["ID"].asString();
                info.deviceId = std::atoi(id.c_str());
                info.deviceName = ResourceName;
                info.Compress = data[0]["ENABLECOMPRESSION"].asString() == "true" ? true : false;
                info.Dedup = data[0]["ENABLEDEDUP"].asString() == "true" ? true : false;
                std::istringstream capa(data[0]["CAPACITY"].asString());
                capa >> info.totalCapacity;
                info.usedCapacity = 0;
                if (data[0]["AVAILABLECAPCITY"].asString() != "") {
                    std::istringstream availCapa(data[0]["AVAILABLECAPCITY"].asString());
                    long long availableCapacity;
                    availCapa >> availableCapacity;
                    info.usedCapacity = info.totalCapacity - availableCapacity;
                }
                if (data[0]["MINSIZEFSCAPACITY"].asString() != "") {
                    std::istringstream minSize(data[0]["MINSIZEFSCAPACITY"].asString());
                    minSize >> info.minSizeOfFileSys;
                    HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "minSizeOfFileSys: " << info.minSizeOfFileSys <<
                                                          HCPENDLOG;
                }

                return iRet;
            } else {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "QueryFileSystem failed!" << HCPENDLOG;
                return FAILED;
            }
        } else {
            return iRet;
        }
    }

    int OceanstorNas::SendRequestEx(HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode,
                                    SessionInfo &sessionInfo)
    {
        int iRet = FAILED;
        if (req.method == "DELETE") {
            req.url = "https://" + OceanstorIP + ":" + OceanstorPort + "/deviceManager/rest/" +
                      sessionInfo.device_id + "/" + req.url;
        } else {
            req.url = "https://" + OceanstorIP + ":" + OceanstorPort + "/deviceManager/rest//" + req.url;
        }
        (void) req.heads.insert(std::make_pair(std::string("Cookie"), sessionInfo.cookie));
        (void) req.heads.insert(std::make_pair(std::string("iBaseToken"), sessionInfo.token));
        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "req: " << WIPE_SENSITIVE(req.url) << HCPENDLOG;
        req.url = FormatFullUrl(req.url);
        if (OceanstorIP == INNER_SAFE_IP) {
            req.isVerify = INTERNAL_VERIFY;
        }
        req.enableProxy = GetEnableProxy();
        std::shared_ptr <IHttpResponse> rsp = fs_pHttpCLient->SendRequest(req);
        if (NULL == rsp.get()) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return FAILED;
        }
        if (!rsp->Success()) {
            if (rsp->GetErrCode() != 0) {
                errorDes = rsp->GetHttpStatusDescribe();
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Http response error. Error  is " << errorDes
                                                    << ", ErrCode: " << rsp->GetErrCode() << HCPENDLOG;
            } else {
                errorDes = rsp->GetErrString();
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME)
                        << "Send http request occur network error. Error  is " << errorDes << HCPENDLOG;
            }
            return iRet;
        } else {
            if (req.method == "DELETE") {
                return ParseResponse(rsp->GetBody(), data, errorDes, errorCode);
            }
            iRet = ParseCookie(rsp->GetCookies(), sessionInfo);
            if (iRet != SUCCESS) {
                return iRet;
            }
            return ParseResponse(rsp->GetBody(), data, errorDes, errorCode);
        }
    }

    int OceanstorNas::isNeedRetryErrorCode(const int &errorCode)
    {
        int size = sizeof(g_noNeedRetryOceanstorErrorCode) / sizeof(int);
        for (int i = 0; i < size; i++) {
            if (errorCode == g_noNeedRetryOceanstorErrorCode[i]) {
                return FAILED;
            }
        }
        return SUCCESS;
    }

    void OceanstorNas::InitHttpStatusCodeForRetry()
    {
        ConfigReader::getIntValueVector("MicroService", "HttpStatusCodesForRetry", ",",
                                        httpRspStatusCodeForRetry);
    }

    bool OceanstorNas::OceanstorResposeNeedRetry(const int ret)
    {
        // when errorCode ==0 && ret == FAILED mean oceanstor response need retry
        if (ret == FAILED) {
            return true;
        }
        return false;
    }

    int OceanstorNas::ParseResponse(const std::string &json_data, Json::Value &data,
        std::string &errorDes, int &errorCode)
    {
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(json_data, jsonValue)) {
            errorDes = "Parse json string failed";
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Parse json string failed.Content is " << HCPENDLOG;
            return FAILED;
        }
        if (!jsonValue.isMember("error") || !jsonValue["error"].isMember("code") ||
            !jsonValue["error"].isMember("description")) {
            errorDes = "Json object format is error. ";
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Json object error. rsp is " << HCPENDLOG;
            return FAILED;
        }

        int oceanstorCode = jsonValue["error"]["code"].asInt();
        if (oceanstorCode != SUCCESS && isNeedRetryErrorCode(oceanstorCode) == SUCCESS) {
            HCP_Log(WARN, OCEANSTOR_MODULE_NAME)
                    << "code : " << jsonValue["error"]["code"].asInt()
                    << ", Describe : " << jsonValue["error"]["description"].asString() << HCPENDLOG;

            errorDes = jsonValue["error"]["description"].asString();
            errorCode = jsonValue["error"]["code"].asInt();
            SetErrorCode(errorCode);
            SetExtendInfo(errorDes);
            return FAILED;
        }

        if (jsonValue.isMember("data")) {
            data = jsonValue["data"];
            errorDes = jsonValue["error"]["description"].asString();
            errorCode = jsonValue["error"]["code"].asInt();
        }
        SetErrorCode(errorCode);
        return SUCCESS;
    }

    bool OceanstorNas::GetEnableProxy()
    {
        auto configReader = ConfigReaderImpl::instance();
        std::string backupScene = configReader->GetBackupSceneFromXml("backup_scene");
        return backupScene == "0" ? false : true;
    }

    bool OceanstorNas::GetJsonValue(const Json::Value &jsValue, std::string strKey, std::string &strValue)
    {
        if (jsValue.isArray()) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Json is Array." << HCPENDLOG;
            return false;
        }
        if (jsValue.isMember(strKey)) {
            if (jsValue[strKey].isString()) {
                strValue = jsValue[strKey].asString();
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Json value is:" << WIPE_SENSITIVE(strValue) << HCPENDLOG;
                return true;
            } else {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME)
                        << "The value Json key " << strKey << "is not string." << HCPENDLOG;
                return false;
            }
        } else {
            strValue = "";
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Json key " << strKey << "does not exist." <<
                                                HCPENDLOG;
            return false;
        }
    }

    int OceanstorNas::Query(DeviceDetails &info)
    {
        return QueryFileSystem(info);
    }

    int OceanstorNas::QuerySnapshot(std::string SnapshotName, std::string &id)
    {
        int iRet;
        DeviceDetails info;
        if (fileSystemId.empty()) {
            iRet = QueryFileSystem(info);
            if (iRet != SUCCESS) {
                return FAILED;
            }
        }
        HttpRequest req;
        req.method = "GET";
        req.url = "FSSNAPSHOT?PARENTID=" + fileSystemId + "&filter=NAME::" + SnapshotName;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.size() != 0 && errorCode == SUCCESS) {
            id = data[0]["ID"].asString();
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

/*
create snapshot for special volume
Date : 2020/11/05
out params:id -> snapshot ID
          :WWN -> snapshot WWN
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.create snapshot for special volume
*/
    std::unique_ptr <ControlDevice> OceanstorNas::CreateSnapshot(std::string SnapshotName, int &errorCode)
    {
        std::string id;
        ControlDeviceInfo deviceInfo;
        deviceInfo.deviceName = SnapshotName;
        deviceInfo.url = OceanstorIP;
        deviceInfo.port = OceanstorPort;
        deviceInfo.userName = OceanstorUsername;
        deviceInfo.password = OceanstorPassword;
        deviceInfo.poolId = OceanstorPoolId;
        deviceInfo.compress = Compress;
        deviceInfo.dedup = Dedup;

        int ret = QuerySnapshot(SnapshotName, id);
        if (ret == SUCCESS) {
            return std::make_unique<OceanstorNasSnapshot>(deviceInfo, fileSystemId, vstoreId, "/" + ResourceName + "/");
        }

        HttpRequest req;
        req.method = "POST";
        req.url = "FSSNAPSHOT";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        jsonValue["PARENTTYPE"] = OCEANSTOR_DEFAULT_PARENT_TYPE;
        jsonValue["NAME"] = SnapshotName;
        jsonValue["PARENTID"] = fileSystemId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return std::make_unique<OceanstorNasSnapshot>(deviceInfo, fileSystemId, vstoreId, "/" + ResourceName + "/");
        } else {
            return nullptr;
        }
    }

    int OceanstorNas::DeleteSnapshot(std::string SnapshotName)
    {
        std::string id;
        int ret = QuerySnapshot(SnapshotName, id);
        if (ret != SUCCESS) {
            return ret;
        }
        HttpRequest req;
        req.method = "DELETE";
        req.url = "FSSNAPSHOT/" + id;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        std::string errorDes;
        int errorCode;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == OceanstorErrorCode::FSSNAPSHOT_NOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNas::EndSnapshotDiffSession(std::string sessionId)
    {
        HttpRequest req;
        req.method = "DELETE";
        std::string errorDes;
        int errorCode;
        Json::Value data;
        req.url = "fs_snapchangelist_session/" + sessionId;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        }
        return iRet;
    }

    int OceanstorNas::StartSnapshotDiffSession(std::string BaseSnapshotName, std::string IncrementalSnapshotName,
                                               std::string &sessionId)
    {
        std::string base_id;
        std::string incremental_id;
        int iRet = QuerySnapshot(BaseSnapshotName, base_id);
        if (iRet != SUCCESS) {
            return iRet;
        }
        int iRetNew = QuerySnapshot(IncrementalSnapshotName, incremental_id);
        if (iRetNew != SUCCESS) {
            return iRetNew;
        }
        HttpRequest req;
        req.method = "POST";
        req.url = "fs_snapchangelist_session";
        std::string errorDes;
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        int errorCode;
        jsonValue["parentType"] = OCEANSTOR_DEFAULT_PARENT_TYPE;
        jsonValue["baseSnapshotId"] = base_id;
        jsonValue["incrementalSnapshotId"] = incremental_id;
        jsonValue["parentID"] = fileSystemId;
        if (!vstoreId.empty() && vstoreId != "0") {
            jsonValue["vstoreId"] = vstoreId;
        }
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.size() == 1 && errorCode == SUCCESS) {
            sessionId = data["id"].asString();
            return iRet;
        } else {
            return iRet;
        }
    }

    void CopyMetadtaResponse(SnapdiffMetadataInfo metadatInfo[], Json::Value data)
    {
        Json::Value metaDataArray;
        Json::Reader reader;
        if (reader.parse(data["changeList"].asString(), metaDataArray)) {
            for (int i = 0; i < metaDataArray.size(); i++) {
                metadatInfo[i].aTime = std::stoi(metaDataArray[i]["aTime"].asString(), nullptr, BASE);
                metadatInfo[i].cTime = std::stoi(metaDataArray[i]["cTime"].asString(), nullptr, BASE);
                metadatInfo[i].crTime = std::stoi(metaDataArray[i]["crTime"].asString(), nullptr, BASE);
                metadatInfo[i].mTime = std::stoi(metaDataArray[i]["mTime"].asString(), nullptr, BASE);
                metadatInfo[i].changeType = std::stoi(metaDataArray[i]["changeType"].asString(), nullptr, BASE);
                metadatInfo[i].fType = std::stoi(metaDataArray[i]["fType"].asString(), nullptr, BASE);
                metadatInfo[i].dosAttr = std::stoi(metaDataArray[i]["dosAttr"].asString(), nullptr, BASE);
                metadatInfo[i].fAttr = std::stoi(metaDataArray[i]["fAttr"].asString(), nullptr, BASE);
                metadatInfo[i].filePath = metaDataArray[i]["filePath"].asString();
                metadatInfo[i].group = std::stoi(metaDataArray[i]["group"].asString(), nullptr, BASE);
                metadatInfo[i].inode = std::stoi(metaDataArray[i]["inode"].asString(), nullptr, BASE);
                metadatInfo[i].links = std::stoi(metaDataArray[i]["links"].asString(), nullptr, BASE);
                metadatInfo[i].owner = std::stoi(metaDataArray[i]["owner"].asString(), nullptr, BASE);
                metadatInfo[i].size = std::stoi(metaDataArray[i]["size"].asString(), nullptr, BASE);
            }
        }
    }

    int OceanstorNas::GetSnapshotDiffChanges(std::string sessionId, SnapdiffInfo &snapdiffInfo,
                                             SnapdiffMetadataInfo metadatInfo[], int metadataListLen)
    {
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        int errorCode;
        Json::Value data;
        unsigned long long posType;
        unsigned long long posObj;
        unsigned long long posOffset;
        unsigned long long linkPosType;
        unsigned long long linkPos;
        if (snapdiffInfo.progress == "0") {
            posType = SNAPDIFF_DEFAULT_BASE_VAL;
            posObj = 0;
            posOffset = 0;
            linkPosType = SNAPDIFF_DEFAULT_BASE_VAL;
            linkPos = 0;
        } else {
            posType = snapdiffInfo.posType;
            posObj = snapdiffInfo.posObj;
            posOffset = snapdiffInfo.posOffset;
            linkPosType = snapdiffInfo.linkPosType;
            linkPos = snapdiffInfo.linkPos;
        }
        req.url = "fs_snapchangelist_result?sessionID=" + sessionId + "&parentType=" + "40" + "&posType=" +
                  std::to_string(posType) + "&posObj=" + std::to_string(posObj) + "&posOffset=" +
                  std::to_string(posOffset) +
                  "&linkPosType=" + std::to_string(linkPosType) + "&linkPos=" + std::to_string(linkPos);
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.size() > 0 && errorCode == SUCCESS) {
            snapdiffInfo.progress = data["progress"].asString();
            snapdiffInfo.posType = std::stoull(data["posType"].asString(), nullptr, BASE);
            snapdiffInfo.posObj = std::stoull(data["posObj"].asString(), nullptr, BASE);
            snapdiffInfo.posOffset = std::stoull(data["posOffset"].asString(), nullptr, BASE);
            snapdiffInfo.linkPosType = std::stoull(data["linkPosType"].asString(), nullptr, BASE);
            snapdiffInfo.linkPos = std::stoull(data["linkPos"].asString(), nullptr, BASE);
            snapdiffInfo.numChanges = std::stoi(data["numChanges"].asString(), nullptr, BASE);
            if (metadataListLen < snapdiffInfo.numChanges) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "metadata List size is small than diff change = " <<
                                                    snapdiffInfo.numChanges << HCPENDLOG;
                return FAILED;
            }
            if (snapdiffInfo.numChanges > 0) {
                CopyMetadtaResponse(metadatInfo, data);
            }
        } else {
            return FAILED;
        }
        return SUCCESS;
    }

    int OceanstorNas::GetVstoreId()
    {
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        std::string IPProtocol = (OceanstorServiceIP.find(':') != std::string::npos) ? "IPV6ADDR:" : "IPV4ADDR:";
        std::string serviceIP = (OceanstorServiceIP.find(':') != std::string::npos)
                                ? OceanstorServiceIP.substr(1, OceanstorServiceIP.size() - NUM_2) : OceanstorServiceIP;
        boost::replace_all(OceanstorServiceIP, ":", "\\:");
        req.url = "lif?range=[0-10000]&filter=" + IPProtocol + serviceIP;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.size() > 0 && errorCode == SUCCESS) {
            if (data[0].size() > 1) {
                if (data[0].isMember("vstoreId")) {
                    vstoreId = data[0]["vstoreId"].asString();
                } else {
                    vstoreId = "0";
                }
                return SUCCESS;
            }
        }
        return FAILED;
    }

    int OceanstorNas::DeleteFileSystem()
    {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "filesystem/" + fileSystemId;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS || errorCode == OceanstorErrorCode::FILESYSTEMIDNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNas::CreateCloneFileSystem(std::string volumeName, std::string &fsid)
    {
        HttpRequest req;
        req.method = "POST";
        req.url = "filesystem";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = volumeName;
        jsonValue["PARENTFILESYSTEMID"] = fileSystemId;
        jsonValue["ALLOCTYPE"] = "1";
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS &&
            (errorCode == SUCCESS || errorCode == OceanstorErrorCode::FILESYSTEMALREADYEXIST)) {
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Create clone file system finished!" << HCPENDLOG;
            DeviceDetails info;
            if (QueryFileSystem(volumeName, info) != SUCCESS) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Query Clone FileSystem Failed!" << HCPENDLOG;
                return FAILED;
            }
            fsid = std::to_string(info.deviceId);
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    void OceanstorNas::AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string volumeName)
    {
        deviceInfo.deviceName = volumeName;
        deviceInfo.url = OceanstorIP;
        deviceInfo.port = OceanstorPort;
        deviceInfo.userName = OceanstorUsername;
        deviceInfo.password = OceanstorPassword;
        deviceInfo.poolId = OceanstorPoolId;
    }

    int OceanstorNas::DeleteFileSystemAndParentSnapshot()
    {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "filesystem/" + fileSystemId + "?ISDELETEPARENTSNAPSHOT=true";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                   errorCode == OceanstorErrorCode::FILESYSTEMIDNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    void OceanstorNas::SetCurlTimeOut(uint64_t tmpTimeOut)
    {
        if (tmpTimeOut > MIN_CURL_TIME_OUT) {
            CurlTimeOut = tmpTimeOut;
        }
    }
}
