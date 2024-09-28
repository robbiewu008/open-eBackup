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
 * HaCertRequest
 *
 */
@Setter
@Getter
public class HaCertRequest {
    /**
     * Ha Ca证书
     */
    MultipartFile caCert;

    /**
     * Ha Ca私钥
     */
    MultipartFile caPem;

    /**
     * Ha Ca私钥密码
     */
    MultipartFile caPwd;

    /**
     * Ha server证书
     */
    MultipartFile serverCert;

    /**
     * Ha server私钥
     */
    MultipartFile serverPem;

    /**
     * Ha server私钥密码
     */
    MultipartFile serverPwd;

    /**
     * 更新前的Ha Ca证书
     */
    MultipartFile oldCaCert;

    /**
     * 更新前的Ha Ca私钥
     */
    MultipartFile oldCaPem;

    /**
     * 更新前的Ha Ca私钥密码
     */
    MultipartFile oldCaPwd;

    /**
     * 更新前的Ha server证书
     */
    MultipartFile oldServerCert;

    /**
     * 更新前的Ha server私钥
     */
    MultipartFile oldServerPem;

    /**
     * 更新前的Ha server私钥密码
     */
    MultipartFile oldServerPwd;
}
