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
package openbackup.opengauss.resources.access.util;

import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;

import com.fasterxml.jackson.core.type.TypeReference;

import lombok.extern.slf4j.Slf4j;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * OpenGaussClusterUtil 工具类
 *
 */
@Slf4j
public class OpenGaussRestoreUtil {
    /**
     * 检查环境是否存在和设置nodes节点信息
     *
     * @param taskEnvironment 环境信息
     */
    public static void checkEnvironmentExistAndBuildNodes(TaskEnvironment taskEnvironment) {
        TaskEnvironment targetEnv = Optional.ofNullable(taskEnvironment)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "targetEnv is empty"));
        List<TaskEnvironment> nodes = getNodesFromEnv(targetEnv);
        targetEnv.setNodes(nodes);
    }

    /**
     * 从保护集群环境中获取任务环境nodes
     *
     * @param targetEnv 保护集群环境
     * @return List<TaskEnvironment>
     */
    public static List<TaskEnvironment> getNodesFromEnv(TaskEnvironment targetEnv) {
        Optional<String> nodeString = Optional.ofNullable(targetEnv.getExtendInfo().get(OpenGaussConstants.NODES));
        if (!nodeString.isPresent()) {
            log.warn("cluster nodes is empty.");
            return new ArrayList<>();
        }
        List<NodeInfo> nodeInfos = JsonUtil.read(nodeString.get(), new TypeReference<List<NodeInfo>>() {
        });
        return nodeInfos.stream()
            .map(OpenGaussRestoreUtil::convertNodeInfoToTaskEnvironment)
            .collect(Collectors.toList());
    }

    private static TaskEnvironment convertNodeInfoToTaskEnvironment(NodeInfo nodeInfo) {
        TaskEnvironment taskEnv = new TaskEnvironment();
        taskEnv.setName(nodeInfo.getName());
        taskEnv.setUuid(nodeInfo.getUuid());
        taskEnv.setType(nodeInfo.getType());
        taskEnv.setSubType(nodeInfo.getSubType());
        taskEnv.setEndpoint(nodeInfo.getEndpoint());

        taskEnv.setExtendInfo(Optional.ofNullable(nodeInfo.getExtendInfo()).orElse(new HashMap<>()));
        return taskEnv;
    }
}
