/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file CertificateType.h
 * @brief  The implemention about CertificateType.h
 * @version 1.1.0
 * @date 2021-11-1
 * @author caomin c00511255
 */

#ifndef ICERTIFICATECOMM_H_
#define ICERTIFICATECOMM_H_

namespace certificateservice {
enum class CertificateType {
    KEY_FILE,
    TRUSTE_CRETIFICATE_FILE,
    USE_CRETIFICATE_FILE
};

enum class CertificateConfig {
    PASSWORD,
    ALGORITEHM_SUITE,
    HOST_NAME
};
}

#endif