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
#ifndef RESPONSE_MODEL_MGR_H
#define RESPONSE_MODEL_MGR_H

#include <map>
#include <set>
#include "common/Macros.h"


VIRT_PLUGIN_NAMESPACE_BEGIN
class ResponseModel {
public:
    ResponseModel() {}
    virtual ~ResponseModel() {}
    bool Success() const;
    bool Busy() const;
    uint32_t  GetHttpStatusCode() const;
    std::string GetHttpStatusDescribe() const;
    int32_t GetErrCode() const;
    std::string GetErrString() const;
    std::string GetBody() const;
    std::map<std::string, std::set<std::string> > GetHeaders() const;
    std::set<std::string> GetCookies() const;
    uint32_t GetStatusCode() const;
    void SetSuccess(const bool &value);
    void SetBusy(const bool &value);
    void SetHttpStatusCode(const uint32_t &value);
    void SetHttpStatusDescribe(const std::string &value);
    void SetErrCode(const int32_t &value);
    void SetErrString(const std::string &value);
    void SetGetBody(const std::string &value);
    void SetHeaders(const std::map<std::string, std::set<std::string> > &value);
    void SetCookies(const std::set<std::string> &value);
    void SetStatusCode(const uint32_t &value);

protected:
    bool m_success = false;
    bool m_busy = false;
    uint32_t m_httpStatusCode;
    std::string m_httpStatusDescribe;
    int32_t m_errCode;
    std::string m_errString;
    std::string m_body;
    std::map<std::string, std::set<std::string> > m_headers;
    std::set<std::string> m_cookies;
    uint32_t m_statusCode;
};
VIRT_PLUGIN_NAMESPACE_END

#endif