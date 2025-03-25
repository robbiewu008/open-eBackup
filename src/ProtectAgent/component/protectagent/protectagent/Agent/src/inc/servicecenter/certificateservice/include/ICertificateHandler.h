/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ICertificateHandler.h
 * @brief  The implemention about ICertificateHandler.h
 * @version 1.1.0
 * @date 2021-11-1
 * @author caomin c00511255
 */

#ifndef ICERTIFICATEHADNDLER_H_
#define ICERTIFICATEHADNDLER_H_
#include <string>
#include "servicecenter/certificateservice/include/ICertificateComm.h"

namespace certificateservice {
class ICertificateHandler {
public:
    ICertificateHandler() = default;
    virtual ~ICertificateHandler() = default;
    virtual std::string GetCertificateFile(CertificateType type) = 0;
    virtual bool GetCertificateConfig(CertificateConfig config, std::string& value) = 0;
};
}  // namespace certificateservice
#endif