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
package openbackup.system.base.common.utils;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.sdk.system.SystemConfigService;
import openbackup.system.base.util.SpringBeanUtils;

/**
 * 功能描述
 *
 */
@Slf4j
public class SecurityUtil {
    /**
     * SSL类型
     */
    private static final String SSL_CONTEXT = "ssl_context";

    /**
     * TLS类型
     */
    private static final String TLS_CONTEXT = "tls_context";

    /**
     * 可选协议，支持多种加密连接方式
     */
    private static final String PROTOCOLS = "protocols";

    /**
     * 数据库获取ssl
     *
     * @return String
     */
    public static String getSslContext() {
        return SpringBeanUtils.getBean(SystemConfigService.class).getConfigValue(SSL_CONTEXT);
    }

    /**
     * 数据库获取tls
     *
     * @return String
     */
    public static String getTlsContext() {
        return SpringBeanUtils.getBean(SystemConfigService.class).getConfigValue(TLS_CONTEXT);
    }

    /**
     * 数据库获取protocols
     *
     * @return String[]
     */
    public static String[] getProtocols() {
        String protocols = SpringBeanUtils.getBean(SystemConfigService.class).getConfigValue(PROTOCOLS);
        return protocols.split(",");
    }
}
