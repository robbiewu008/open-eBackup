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
package openbackup.data.protection.access.provider.sdk.util;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.system.base.common.model.host.AgentManagementDomain;
import openbackup.system.base.common.utils.VerifyUtil;

import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述 api工具类
 *
 * @author t30028453
 * @version [DataBackup 1.3.0]
 * @since 2023-05-31
 */
public class AgentApiUtil {
    /**
     * 获取agent的id集合
     *
     * @param agents agents
     * @return agent的id集合
     */
    public static List<String> getAgentIds(List<Endpoint> agents) {
        return Optional.ofNullable(agents)
                .map(item -> item.stream().map(Endpoint::getId).collect(Collectors.toList()))
                .orElse(new ArrayList<>());
    }

    /**
     * 选择一个pod
     * 原则：出现频率最高的，如果有多个出现频率一样的则随机选一个
     *
     * @param allDomain 所有可用pod
     * @return 选择一个pod，出现频率最高的，如果有多个出现频率一样的则随机选一个
     */
    public static AgentManagementDomain selectDomain(List<AgentManagementDomain> allDomain) {
        AgentManagementDomain finalDomain = new AgentManagementDomain();
        if (VerifyUtil.isEmpty(allDomain)) {
            return finalDomain;
        }

        List<String> allPods = allDomain.stream()
                .map(AgentManagementDomain::getDomain)
                .collect(Collectors.toList());

        Map<String, Long> countMap = allPods.stream()
                .collect(Collectors.groupingBy(e -> e, Collectors.counting()));
        // 找到出现最多的次数
        Long maxTime = countMap.entrySet().stream()
                .max(Map.Entry.comparingByValue())
                .map(Map.Entry::getValue)
                .get();
        // 出现次数最多的所有domain
        List<String> maxTimeDomains = countMap.entrySet().stream()
                .filter(entry -> entry.getValue() == maxTime)
                .map(Map.Entry::getKey)
                .collect(Collectors.toList());
        // 随机选择一个
        SecureRandom random = new SecureRandom();
        String randomDomain = maxTimeDomains.get(random.nextInt(maxTimeDomains.size()));

        finalDomain.setDomain(randomDomain);
        return finalDomain;
    }
}
