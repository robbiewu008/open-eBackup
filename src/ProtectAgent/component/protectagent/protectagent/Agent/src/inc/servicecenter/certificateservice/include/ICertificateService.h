/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ICertificateHandler.h
 * @brief  The implemention about ICertificateHandler.h
 * @version 1.1.0
 * @date 2021-11-1
 * @author caomin c00511255
 */

#ifndef ICERTIFICATE_SERVICE_H_
#define ICERTIFICATE_SERVICE_H_

#include <memory>
#include "servicecenter/servicefactory/include/IService.h"
#include "servicecenter/certificateservice/include/ICertificateComm.h"
#include "servicecenter/certificateservice/include/ICertificateHandler.h"
#include "servicecenter/certificateservice/include/ICertificatePathProxy.h"

namespace certificateservice {
class ICertificateService : public servicecenter::IService {
public:
    ICertificateService() = default;
    ~ICertificateService() = default;
    // 默认证书接口，使用nignx证书,使用CPATH和CConfigXmlParser，需提前初始化好
    virtual std::shared_ptr<ICertificateHandler> GetCertificateHandler() = 0;
    // 通过代理的方式获取自定义的证书路径和证书名
    virtual std::shared_ptr<ICertificateHandler> GetCertificateHandler(
        const std::shared_ptr<ICertificatePathProxy>& pathProxy) = 0;
};
}  // namespace certificateservice
#endif