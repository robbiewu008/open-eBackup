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
package openbackup.system.base.common.scurity;

import java.security.cert.CertificateException;

/**
 * 用于验证服务器证书的TrustManager
 *
 * @author l90005176
 * @version [OceanStor BCManager V200R001C00, 2016年2月16日]
 * @since 2019-10-25
 */
public interface IBcmX509TrustHandler {
    /**
     * 处理错误码
     *
     * @param phase 错误码
     */
    void handle(long phase);

    /**
     * 处理错误
     *
     * @param exception 异常
     * @throws CertificateException 证书校验失败异常
     */
    void handle(Exception exception) throws CertificateException;
}
