/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  thrift SecurityService handler
 * @version 1.0.0
 * @date 2022-2-15
 * @author twx949498
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