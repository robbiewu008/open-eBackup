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

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.network.Ipv6AddressUtil;

import lombok.extern.slf4j.Slf4j;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Locale;

/**
 * 根据集群的IP/port生成对应URI
 *
 */
@Slf4j
public class UriUtil {
    /**
     * Get the storage device uri
     *
     * @param ip ip
     * @param port port
     * @return Storage device uri
     */
    public static URI getUri(String ip, int port) {
        try {
            String linkIp = Ipv6AddressUtil.isIpv6Address(ip) ? "[" + ip + "]" : ip;
            return new URI(String.format(Locale.ENGLISH, "https://%s:%s", linkIp, port));
        } catch (URISyntaxException e) {
            log.error("Get uri failed!", e);
        }
        throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "Get uri failed");
    }
}

