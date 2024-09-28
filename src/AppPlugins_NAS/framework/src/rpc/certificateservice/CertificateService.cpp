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
#include "CertificateService.h"
#include "CertificateHandler.h"
#include "PluginCertificatePathProxy.h"
#include "ServiceFactory.h"

using namespace servicecenter;
namespace certificateservice {
namespace detail {
#ifndef WIN32
static bool registerSerivce = ServiceFactory::GetInstance()->Register<CertificateService>("ICertificateService");
#endif
CertificateService::CertificateService()
    : m_pathProxy(std::make_shared<PluginCertificatePathProxy>())
{}

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
