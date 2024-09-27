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
#ifndef TOKEN_MGR_H
#define TOKEN_MGR_H
#include <mutex>
#include "common/Macros.h"
#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "common/token_mgr/GetTokenResponse.h"

VIRT_PLUGIN_NAMESPACE_BEGIN
class BaseTokenMgr {
public:
    BaseTokenMgr() {};
    ~BaseTokenMgr() {};
    void DeleteToken(ModelBase& model);
    bool GetToken(ModelBase &model, std::string &tokenValue, std::string &endPoint);
    virtual bool ReacquireToken(ModelBase &model, std::string &tokenValue) = 0;

protected:
    void CheckToken();
    virtual std::string GetTokenKey(ModelBase &model) = 0;
    virtual bool ParseEndpoint(ModelBase &model, const TokenInfo &tokenInfo, std::string &endPoint) = 0;
    bool TokenIsExpired(std::string &date);
    std::string AddToken(ModelBase &model, std::shared_ptr<GetTokenResponse> getTokenResponse);
    bool GetTokenFromMap(ModelBase &model, std::string &tokenValue, std::string &endPoint, const bool &checkExpireFlag);
    bool DateTransToStamp(const std::string &date, std::string &timeStamp) const;
    virtual int GetTimeDiffWithGMT()
    {
        return 0;
    };
    std::mutex m_tokenMutex;
    std::map<std::string, TokenInfo> m_tokenMap;
};
VIRT_PLUGIN_NAMESPACE_END

#endif