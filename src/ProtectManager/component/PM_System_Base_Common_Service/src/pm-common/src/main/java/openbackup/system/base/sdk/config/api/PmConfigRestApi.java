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
package openbackup.system.base.sdk.config.api;

import feign.Param;
import feign.RequestLine;
import openbackup.system.base.security.exterattack.ExterAttack;

import java.net.URI;
import java.util.List;

/**
 * PmConfigRestApi
 *
 */
public interface PmConfigRestApi {
    /**
     * 获取网络配置
     *
     * @param uri uri
     * @return 网络配置
     */
    @ExterAttack
    @RequestLine("GET /v1/internal/pm-config/network/local-network")
    List<String> getLocalNetwork(URI uri);

    /**
     * 检查ntp 服务器
     *
     * @param uri uri
     * @param ntpServerAddress ntp服务器地址
     * @return 是否检查通过
     */
    @ExterAttack
    @RequestLine("PUT /v1/internal/pm-config/system/ntp/check?server_ip={server_ip}")
    Boolean checkNtpConfig(URI uri, @Param("server_ip") String ntpServerAddress);

    /**
     * 保存ntp配置
     *
     * @param uri uri
     * @param ntpServerAddress ntp服务器地址
     * @return 是否同步成功
     */
    @ExterAttack
    @RequestLine("PUT /v1/internal/pm-config/system/ntp/update?server_ip={server_ip}")
    Boolean syncNtpConfig(URI uri, @Param("server_ip") String ntpServerAddress);

    /**
     * 回退ntp配置
     *
     * @param uri uri
     * @return 回退是否成功
     */
    @ExterAttack
    @RequestLine("PUT /v1/internal/pm-config/system/ntp/rollback")
    Boolean fallbackNtpConfig(URI uri);
}
