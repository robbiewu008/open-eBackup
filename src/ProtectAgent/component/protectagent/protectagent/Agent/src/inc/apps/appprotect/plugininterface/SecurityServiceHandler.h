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
#ifndef _SECURITY_SERVICE__HANDLER_HEADER_
#define _SECURITY_SERVICE__HANDLER_HEADER_


#include "apps/appprotect/plugininterface/SecurityService.h"
#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"
#include "servicecenter/messageservice/include/RpcPublishObserver.h"
#include "servicecenter/servicefactory/include/IService.h"
#include "common/Defines.h"
#include <string>
#include <algorithm>

namespace AppProtect {

class SecurityServiceHandler:
    public SecurityServiceIf,
    public messageservice::RpcPublishObserver,
    public servicecenter::IService {
public:
    SecurityServiceHandler();
    ~SecurityServiceHandler();

    EXTER_ATTACK void CheckCertThumbPrint(ActionResult& _return, const std::string& ip, const int32_t port,
        const std::string& thumbPrint);
    EXTER_ATTACK void RunCommand(CmdResult &_return, const std::string& cmdPara);
    bool Initailize() override;
    bool Uninitailize() override;

protected:
    void Update(std::shared_ptr<messageservice::RpcPublishEvent> event) override;
};
}
#endif