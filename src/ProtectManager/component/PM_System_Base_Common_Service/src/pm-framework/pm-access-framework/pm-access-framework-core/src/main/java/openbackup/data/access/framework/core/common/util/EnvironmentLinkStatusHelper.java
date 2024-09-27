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
package openbackup.data.access.framework.core.common.util;

import static openbackup.data.protection.access.provider.sdk.backup.ResourceExtendInfoConstants.CONNECTION_RESULT_KEY;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentConnectionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.util.BeanTools;

import com.fasterxml.jackson.core.type.TypeReference;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Map;
import java.util.Objects;

/**
 * 资源受保护状态获取工具类
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-27
 */
@Component
@Slf4j
public class EnvironmentLinkStatusHelper {
    private static MemberClusterService memberClusterService;

    private EnvironmentLinkStatusHelper(MemberClusterService memberClusterService) {
        EnvironmentLinkStatusHelper.memberClusterService = memberClusterService;
    }

    /**
     * 获取受保护环境的连接状态，适配多集群场景，获取单个节点对环境的连通状态
     *
     * @param protectedEnvironment 受保护环境
     * @return 连接状态
     */
    public static String getLinkStatusAdaptMultiCluster(ProtectedEnvironment protectedEnvironment) {
        if (memberClusterService.clusterEstablished()) {
            String resultString = protectedEnvironment.getExtendInfoByKey(CONNECTION_RESULT_KEY);
            if (VerifyUtil.isEmpty(resultString)) {
                log.error("get env link status by ext info failed,env id: {}", protectedEnvironment.getUuid());
                return protectedEnvironment.getLinkStatus();
            }
            try {
                Map<String, EnvironmentConnectionResult> connectionResult = JsonUtil.read(resultString,
                    new TypeReference<Map<String, EnvironmentConnectionResult>>() {
                    });
                String localEsn = memberClusterService.getCurrentClusterEsn();
                EnvironmentConnectionResult connectionResultOfLocal = connectionResult.get(localEsn);
                if (!VerifyUtil.isEmpty(connectionResultOfLocal) && !VerifyUtil.isEmpty(
                    connectionResultOfLocal.getLinkStatus())) {
                    return String.valueOf(connectionResultOfLocal.getLinkStatus());
                } else {
                    return protectedEnvironment.getLinkStatus();
                }
            } catch (DataMoverCheckedException e) {
                log.error("get env link status by ext info failed(json parsing failed),env id: {}",
                    protectedEnvironment.getUuid());
                return protectedEnvironment.getLinkStatus();
            }
        }
        return protectedEnvironment.getLinkStatus();
    }

    /**
     * 适配多集群场景，判断单个节点对环境的连通状态是否是ONLINE
     *
     * @param protectedEnvironment 受保护环境
     * @return 连通状态是否是ONLINE
     */
    public static boolean isOnlineAdaptMultiCluster(ProtectedEnvironment protectedEnvironment) {
        return Objects.equals(LinkStatusEnum.ONLINE.getStatus().toString(),
            getLinkStatusAdaptMultiCluster(protectedEnvironment));
    }

    /**
     * 适配多集群场景，判断单个节点对环境的连通状态是否是ONLINE
     *
     * @param targetEnv 受保护环境
     * @return 连通状态是否是ONLINE
     */
    public static boolean isOnlineAdaptMultiCluster(TaskEnvironment targetEnv) {
        ProtectedEnvironment protectedEnvironment = BeanTools.copy(targetEnv, ProtectedEnvironment::new);
        return Objects.equals(LinkStatusEnum.ONLINE.getStatus().toString(),
            getLinkStatusAdaptMultiCluster(protectedEnvironment));
    }
}
