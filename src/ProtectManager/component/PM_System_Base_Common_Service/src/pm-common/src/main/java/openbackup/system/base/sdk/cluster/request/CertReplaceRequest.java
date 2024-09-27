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
package openbackup.system.base.sdk.cluster.request;

import lombok.Getter;
import lombok.Setter;

import org.springframework.web.multipart.MultipartFile;

/**
 * 证书替换对象
 *
 * @author fwx1022842
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/6/27
 */
@Getter
@Setter
public class CertReplaceRequest {
    /**
     * ca证书
     */
    MultipartFile caCertificate;

    /**
     * 服务端证书
     */
    MultipartFile serverCertificate;

    /**
     * 服务端私钥
     */
    MultipartFile serverKey;

    /**
     * keystore
     */
    MultipartFile keystore;
}