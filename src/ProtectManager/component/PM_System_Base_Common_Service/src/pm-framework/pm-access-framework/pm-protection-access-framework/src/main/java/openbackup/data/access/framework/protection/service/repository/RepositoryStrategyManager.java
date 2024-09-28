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
package openbackup.data.access.framework.protection.service.repository;

import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;

import com.google.common.base.CaseFormat;

import org.springframework.stereotype.Component;
import org.springframework.util.Assert;

import java.util.Locale;
import java.util.Map;

/**
 * 认证信息的策略管理器，后续不同协议
 *
 **/
@Component
public class RepositoryStrategyManager {
    /**
     * 认证策略后缀
     */
    private static final String STRATEGY_SUFFIX = "RepositoryStrategy";

    private final Map<String, RepositoryStrategy> strategyMap;

    public RepositoryStrategyManager(Map<String, RepositoryStrategy> strategyMap) {
        this.strategyMap = strategyMap;
    }

    /**
     * 根据协议枚举获取对应的认证策略类
     *
     * @param protocol 协议枚举对象
     * @return 认证策略类 {@code AuthStrategy}
     */
    public RepositoryStrategy getStrategy(RepositoryProtocolEnum protocol) {
        // 协议枚举名称大写下划线转小写驼峰风格
        String key = CaseFormat.UPPER_UNDERSCORE.to(CaseFormat.LOWER_CAMEL, protocol.name()) + STRATEGY_SUFFIX;
        Assert.isTrue(
                strategyMap.containsKey(key),
                String.format(Locale.ENGLISH, "protocol[%s] is not supported", protocol.name()));
        return strategyMap.get(key);
    }
}
