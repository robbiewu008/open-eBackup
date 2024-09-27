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
package com.huawei.emeistor.console.listener;

import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SessionService;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.boot.ApplicationArguments;
import org.springframework.boot.ApplicationRunner;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.List;

/**
 * 处理升级问题，解决之前序列化到redis中的session数据，将其转换为json字符串保存
 *
 * @author hwx1144169
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-24
 */
@Component
@Slf4j
public class UpgradeOnLineSessionListener implements ApplicationRunner {
    private final SessionService sessionService;

    private final RedissonClient redissonClient;

    public UpgradeOnLineSessionListener(SessionService sessionService, RedissonClient redissonClient) {
        this.sessionService = sessionService;
        this.redissonClient = redissonClient;
    }

    @ExterAttack
    @Override
    public void run(ApplicationArguments args) throws Exception {
        List<String> onlineSessionIdList = sessionService.getOnlineSessionIdList();
        if (CollectionUtils.isEmpty(onlineSessionIdList)) {
            return;
        }
        for (String onLineSessionId : onlineSessionIdList) {
            RBucket<Object> rBucket = redissonClient.getBucket(onLineSessionId);
            if (rBucket.isExists()) {
                // 只处理之前序列化到redis中的session信息
                if (rBucket.get() instanceof SessionInfo) {
                    SessionInfo sessionInfo = (SessionInfo) rBucket.get();
                    rBucket.set(JSONObject.toJSONString(sessionInfo));
                    log.debug("deal history session info complete");
                }
            }
        }
    }
}
