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
package openbackup.system.base.common.rest;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.scurity.IBcmX509TrustHandler;
import openbackup.system.base.common.utils.ExceptionUtil;

import java.security.cert.CertificateException;

/**
 * SSL证书校验handler类
 *
 */
@Slf4j
public class CommonX509TrustHandler implements IBcmX509TrustHandler {
    /**
     * handle error
     *
     * @param phase 错误码
     */
    @Override
    public void handle(long phase) {}

    /**
     * handle certificate verification exceptions
     *
     * @param exception received exceptions
     * @throws CertificateException 认证异常
     */
    @Override
    public void handle(Exception exception) throws CertificateException {
        if (exception == null) {
            return;
        }
        // 证书校验失败的话，就抛出
        if (exception instanceof CertificateException) {
            throw (CertificateException) exception;
        }
        log.error("Unexpected exception happen in certificate verify.", ExceptionUtil.getErrorMessage(exception));
    }
}
