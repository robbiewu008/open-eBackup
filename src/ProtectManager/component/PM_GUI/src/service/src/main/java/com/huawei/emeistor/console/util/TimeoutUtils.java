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
package com.huawei.emeistor.console.util;

import com.huawei.emeistor.console.contant.NumberConstant;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.PropertySource;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 功能描述
 *
 * @since [生成时间]
 */
@Component
@PropertySource("classpath:/properties/security.properties")
public class TimeoutUtils {
    private Long timeout = -1L;

    private Map<String, Long> timeoutMap = new ConcurrentHashMap<>();

    @Value("#{'${security.escape.url}'.split(',')}")
    private List<String> whiteList;

    /**
     * 检查是否超时
     *
     * @param sessionId String
     * @param isRefresh boolean
     * @return boolean true代表超时
     */
    public boolean checkTimeout(String sessionId, boolean isRefresh) {
        long currTime = System.currentTimeMillis();
        Long firstTime = timeoutMap.get(sessionId);
        if (isRefresh) {
            timeoutMap.put(sessionId, currTime);
        }
        if (timeout == -1L) {
            return false;
        }

        boolean isTimeout = firstTime != null && currTime - firstTime > timeout;
        if (isTimeout) {
            timeoutMap.remove(sessionId);
        }

        return isTimeout;
    }

    /**
     * 功能描述
     *
     * @param sessionId String
     */
    public void destroy(String sessionId) {
        this.timeoutMap.remove(sessionId);
    }

    /**
     * 设置过期时间，单位为分钟
     *
     * @param timeout 分钟
     */
    public void setTimeout(Integer timeout) {
        this.timeout = timeout * (long) (NumberConstant.SIXTY * NumberConstant.THOUSAND);
    }

    public List<String> getWhiteList() {
        return whiteList;
    }
}
