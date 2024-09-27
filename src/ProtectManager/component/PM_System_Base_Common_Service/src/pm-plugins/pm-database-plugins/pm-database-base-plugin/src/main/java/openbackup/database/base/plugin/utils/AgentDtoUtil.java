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
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;

import java.util.Map;

/**
 * AgentDtoUtil
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/9/2
 */
public class AgentDtoUtil {
    /**
     * host dto to  TaskEnvironment
     *
     * @param hostDto hostDto
     * @return TaskEnvironment
     */
    public static TaskEnvironment toTaskEnvironment(HostDto hostDto) {
        String extendInfo = hostDto.getExtendInfo();
        hostDto.setExtendInfo(null);
        TaskEnvironment environment = JsonUtil.read(JsonUtil.json(hostDto), TaskEnvironment.class);
        if (!VerifyUtil.isEmpty(extendInfo)) {
            Map<String, String> extendInfoObj = (Map<String, String>) JsonUtil.read(extendInfo, Map.class);
            environment.setExtendInfo(extendInfoObj);
        }
        hostDto.setExtendInfo(extendInfo);
        return environment;
    }
}