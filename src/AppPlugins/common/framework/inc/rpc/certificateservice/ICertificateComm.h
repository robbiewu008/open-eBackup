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
#ifndef ICERTIFICATECOMM_H
#define ICERTIFICATECOMM_H

namespace certificateservice {
enum class CertificateType {
    KEY_FILE,
    TRUSTE_CERTIFICATE_FILE,
    USE_CERTIFICATE_FILE
};

enum class CertificateConfig {
    PASSWORD,
    ALGORITHM_SUITE,
    HOST_NAME
};

enum class SecurityItemConfig {
    CRETIFICATE_ROOT_PATH = 0,
    KEY_FILE_NAME,
    TRUSTE_CRETIFICATE_FILE_NAME,
    USE_CRETIFICATE_FILE_NAME,
    PASSWORD,
    ALGORITEHM_SUITE,
    HOST_NAME
};
}

#endif