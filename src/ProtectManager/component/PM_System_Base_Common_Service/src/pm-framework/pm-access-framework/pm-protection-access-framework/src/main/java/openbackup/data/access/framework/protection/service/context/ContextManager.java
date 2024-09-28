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
package openbackup.data.access.framework.protection.service.context;

import openbackup.data.access.framework.protection.service.archive.ArchiveContext;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.stereotype.Component;

/**
 * 业务中上下文管理器，抽象用于统一对上下文的获取
 *
 **/
@Component
public class ContextManager {
    private final RedissonClient redissonClient;

    /**
     * 上下文管理器的构造函数
     *
     * @param redissonClient redisson客户端
     */
    public ContextManager(RedissonClient redissonClient) {
        this.redissonClient = redissonClient;
    }

    /**
     * 获取当前归档任务的上下文
     *
     * @param requestId 当前任务的请求id
     * @return 归档上下文对象 {@code ArchiveContext}
     */
    @ExterAttack
    public ArchiveContext getArchiveContext(String requestId) {
        if (StringUtils.isBlank(requestId)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "requestId is empty, can not get context.");
        }
        final RMap<String, String> contextMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        return new ArchiveContextImpl(contextMap);
    }
}
