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
#ifndef _MS_REST_API_H_
#define _MS_REST_API_H_

#ifndef RETURN_ERROR
#define RETURN_ERROR (-1)
#endif

#ifndef RETURN_OK
#define RETURN_OK (0)
#endif

#include "json/json.h"
#include <set>
#include <memory>
#include "curl_http/HttpClientInterface.h"

namespace Module {
    const int HTTP_NETWORK_ERROR = 200000;

#define Token_Expired 100 // used for token expired time
#define Token_Invalid 101 // used for token invalid

    struct RestResult {
        int retCode;
        Json::Value data;

        RestResult()
        {
            retCode = 0;
        }
    };

    class MSRestApi {
    public:
        MSRestApi();

        virtual ~MSRestApi();

        virtual int Login(
                bool retry,
                const std::string &url,
                const std::string &userName,
                const std::string &password,
                const std::string &cert,
                std::string &errDesc,
                bool globalIAM = false,
                const std::string &domainID = "",
                const std::string &tenantID = "",
                const std::string &specifyNetwork = ""
        );

        virtual int Login(const std::string &url,
                          const Json::Value &loginBody,
                          const std::string &cert,
                          std::string &errDesc,
                          bool retry = true
        );

        virtual int Logout(const std::string &url,
                           const std::string &cert,
                           std::string &errDesc);

        virtual int SendRequest(const HttpRequest &req,
                                Json::Value &data, std::string &errorDes, bool retry = true);

        // send raw request information (with errorCode)
        virtual int SendRequest(const HttpRequest &req,
                                Json::Value &data, std::string &errorDes, std::string *token, int &errorCode);

        virtual int SendRequestDirectly(const HttpRequest &req,
                                        Json::Value &data, std::string &errorDes,
                                        bool retry = true);

        virtual int SendRequestDirectly(const HttpRequest &req,
                                        std::map<std::string, std::set<std::string> > &rspHeaders,
                                        std::string &rspData, std::string &errorDes,
                                        bool retry = true);

        virtual int SendRequestDirectly(const HttpRequest &req,
                                        std::string &rspData, std::string &errorDes,
                                        bool retry = true
        );

        virtual bool IsLogined();

        virtual int DownloadAttchment(const HttpRequest &req, std::string &errorDes);

        virtual int UploadAttachment(const HttpRequest &req, Json::Value &data, std::string &errorDes);

        virtual int UploadAttachmentAndParseRsp(const HttpRequest &req, Json::Value &data, std::string &errorDes);

        const std::string Token() const { return m_tokenID; }

        void CleanToken();

        static std::set<std::string>
        GetHeadByName(const std::map<std::string, std::set<std::string> > &headers, const std::string &header_name);

    private:
        //lint -e1735  //Virtual function
        //'MSRestApi::SendAndParseRsp(const HttpRequest &amp;, Json::Value &amp;,
        // std::basic_string&lt;char&gt; &amp;, bool)' has default parameter

        virtual int SendAndParseRsp(bool retry,
                                    const HttpRequest &req,
                                    Json::Value &data, std::string &errorDes,
                                    const bool is_login = false
        );

        //lint +e1735
        virtual int SendRequest(const HttpRequest &req,
                                std::map<std::string, std::set<std::string> > &rspHeaders,
                                std::string &rspData,
                                std::string &errorDes,
                                bool retry = true);

        int ParseRsp(const std::string &json_data,
                     Json::Value &data,
                     std::string &errorDes);

        int GetErrorCode(const std::string &json_data,
                         Json::Value &data,
                         std::string &errorDes,
                         int &errorCode);

        int ParseToken(const std::set<std::string> &tokens);

        MSRestApi(const MSRestApi &);

        const MSRestApi &operator=(const MSRestApi &);

    private:
        std::string m_tokenID;
        bool m_logined;
        IHttpClient *m_pHttpCLient;
    };
}
#endif
