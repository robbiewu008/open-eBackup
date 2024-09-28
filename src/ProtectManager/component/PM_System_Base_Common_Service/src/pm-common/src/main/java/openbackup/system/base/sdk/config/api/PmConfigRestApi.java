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

import openbackup.system.base.security.exterattack.ExterAttack;

import feign.RequestLine;

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
}
