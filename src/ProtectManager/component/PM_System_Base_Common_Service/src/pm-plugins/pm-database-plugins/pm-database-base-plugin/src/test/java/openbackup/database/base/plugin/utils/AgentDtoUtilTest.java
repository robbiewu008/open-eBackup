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
package openbackup.database.base.plugin.utils;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;

import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

/**
 * AgentDtoUtil Test
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/9/2
 */
public class AgentDtoUtilTest {
    /**
     * 用例场景：host dto转测env
     * 前置条件：1. 没有扩展字段
     * 检 查 点：1. 参数正确
     */
    @Test
    public void to_env_success_when_no_extendInfo() {
        HostDto hostDto = new HostDto();
        hostDto.setPort(2181);
        hostDto.setEndpoint("8.40.99.103");
        final TaskEnvironment taskEnvironment = AgentDtoUtil.toTaskEnvironment(hostDto);
        Assert.assertTrue(hostDto.getPort().equals(taskEnvironment.getPort()));
        Assert.assertTrue(hostDto.getEndpoint().equals(taskEnvironment.getEndpoint()));
    }

    /**
     * 用例场景：host dto转测env
     * 前置条件：1. 有扩展字段
     * 检 查 点：1. 参数正确
     */
    @Test
    public void to_env_success_when_have_extendInfo() {
        HostDto hostDto = new HostDto();
        hostDto.setPort(2181);
        hostDto.setEndpoint("8.40.99.103");
        Map<String, String> m = new HashMap<>();
        m.put("ip", "ips");
        hostDto.setExtendInfo(JsonUtil.json(m));
        final TaskEnvironment taskEnvironment = AgentDtoUtil.toTaskEnvironment(hostDto);
        Assert.assertTrue(hostDto.getPort().equals(taskEnvironment.getPort()));
        Assert.assertTrue(hostDto.getEndpoint().equals(taskEnvironment.getEndpoint()));
        Assert.assertTrue(hostDto.getExtendInfo().equals(JsonUtil.json(taskEnvironment.getExtendInfo())));
    }
}
