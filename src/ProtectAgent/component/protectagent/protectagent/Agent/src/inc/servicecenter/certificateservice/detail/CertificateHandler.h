/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ICertificateHandler.h
 * @brief  The implemention about ICertificateHandler.h
 * @version 1.1.0
 * @date 2021-11-1
 * @author caomin c00511255
 */

#ifndef CERTIFICATEHANDLER_H_
#define CERTIFICATEHANDLER_H_
#include <memory>
#include "servicecenter/certificateservice/include/ICertificateHandler.h"
#include "servicecenter/certificateservice/include/ICertificatePathProxy.h"

namespace certificateservice {
namespace detail {
class CertificateHandler : public ICertificateHandler {
public:
    friend class CertificateService;
    CertificateHandler() = default;
    virtual ~CertificateHandler() = default;
    virtual std::string GetCertificateFile(CertificateType type);
    virtual bool GetCertificateConfig(CertificateConfig config, std::string& value);
private:
    std::shared_ptr<ICertificatePathProxy>  m_pathProxy;
};
}
}

#endif