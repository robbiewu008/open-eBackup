/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file CertificateService.h
 * @brief  The implemention about ICertificateService.h
 * @version 1.1.0
 * @date 2021-11-1
 * @author caomin c00511255
 */

#ifndef CRETIFICATE_SERVICE_H_
#define CRETIFICATE_SERVICE_H_

#include "servicecenter/certificateservice/include/ICertificateService.h"

namespace certificateservice {
namespace detail {
class CertificateService : public ICertificateService {
public:
    CertificateService();
    virtual ~CertificateService();
    virtual bool Initailize();
    virtual bool Uninitailize();
    virtual std::shared_ptr<ICertificateHandler> GetCertificateHandler();
    // 通过代理的方式获取自定义的证书路径和证书名
    virtual std::shared_ptr<ICertificateHandler> GetCertificateHandler(
        const std::shared_ptr<ICertificatePathProxy>& pathProxy);

private:
    std::shared_ptr<ICertificatePathProxy> m_pathProxy;
};
}  // namespace detail
}  // namespace certificateservice
#endif