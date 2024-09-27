/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ICertificateHandler.cpp
 * @brief  The implemention about CertificateHandler.h
 * @version 1.1.0
 * @date 2021-11-1
 * @author caomin c00511255
 */

#include <certificateservice/detail/CertificateHandler.h>
namespace certificateservice {
namespace detail {
std::string CertificateHandler::GetCertificateFile(CertificateType type)
{
    return m_pathProxy->GetCertificateRootPath() + "/" + m_pathProxy->GetCertificateFileName(type);
}

bool CertificateHandler::GetCertificateConfig(CertificateConfig config, std::string& value)
{
    return m_pathProxy->GetCertificateConfig(config, value);
}
}
}