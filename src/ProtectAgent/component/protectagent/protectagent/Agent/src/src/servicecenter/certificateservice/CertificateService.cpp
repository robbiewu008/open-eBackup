/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file CertificateService.cpp
 * @brief  The implemention about CertificateService.h
 * @version 1.1.0
 * @date 2021-11-1
 * @author caomin c00511255
 */

#include <certificateservice/detail/CertificateService.h>
#include <certificateservice/detail/CertificateHandler.h>
#include <certificateservice/detail/NginxCertificatePathProxy.h>
#include <servicefactory/include/ServiceFactory.h>

using namespace servicecenter;
namespace certificateservice {
namespace detail {
CertificateService::CertificateService()
    : m_pathProxy(std::make_shared<NginxCertificatePathProxy>())
{
}

CertificateService::~CertificateService()
{
    m_pathProxy.reset();
}

bool CertificateService::Initailize()
{
    return true;
}

bool CertificateService::Uninitailize()
{
    m_pathProxy.reset();
    return true;
}

std::shared_ptr<ICertificateHandler> CertificateService::GetCertificateHandler()
{
    return GetCertificateHandler(m_pathProxy);
}

std::shared_ptr<ICertificateHandler> CertificateService::GetCertificateHandler(
    const std::shared_ptr<ICertificatePathProxy>& pathProxy)
{
    std::shared_ptr<ICertificateHandler> ret = nullptr;
    if (pathProxy) {
        auto handler = std::make_shared<CertificateHandler>();
        handler->m_pathProxy = pathProxy;
        ret = handler;
    }
    return ret;
}
}
}