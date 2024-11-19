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
package openbackup.system.base.service;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.config.configmap.ConfigMapService;
import openbackup.system.base.sdk.cluster.model.ClusterComponentPwdInfo;
import openbackup.system.base.sdk.cluster.request.ClusterComponentPwdInfoRequest;
import openbackup.system.base.sdk.infrastructure.model.InfrastructureResponse;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 提供获取configMap或secret中的数据能力
 *
 */
@Slf4j
@Service
public class ConfigMapServiceImpl {
    @Autowired
    private ConfigMapService configMapService;

    /**
     * 解密
     *
     * @param key common-secret中的键
     * @return 查询到secret中的值
     */
    public String getValueFromSecretByKey(String key) {
        try {
            JSONArray dataSourceConfig = configMapService.getCommonSecreteMapData().getData();
            log.info("get regis information from infra.");
            for (Object obj : dataSourceConfig) {
                JSONObject object = obj instanceof JSONObject ? (JSONObject) obj : new JSONObject();
                if (object.containsKey(key)) {
                    String password = object.getString(key);
                    log.info("set redis auth info success.");
                    return password;
                }
            }
        } catch (FeignException.FeignClientException exception) {
            log.error("wrong with get redis auth info from inf.");
        }
        return "";
    }

    /**
     * 获取内部组件密码
     *
     * @return common-secret Map
     */
    public Map<String, String> getCommonSecreteMapData() {
        JSONArray component = configMapService.getCommonSecreteMapData().getData();
        List<Map> clusterComponentMaps = JSONArray.toCollection(component, Map.class);
        Map<String, String> clusterComponentMap = new HashMap<>();
        for (Map<String, String> componentMap : clusterComponentMaps) {
            clusterComponentMap.putAll(componentMap);
        }
        return clusterComponentMap;
    }

    /**
     * 替换内部组件database/kafka/redis/cifs/dataturbo密码
     *
     * @param clusterComponentPwdInfo 内部组件key和password值
     * @param isAssemble 是否正在组建多集群
     * @param roleType 节点角色
     * @return InfrastructureResponse
     */
    public InfrastructureResponse replaceInternalComponentPassword(
        List<ClusterComponentPwdInfo> clusterComponentPwdInfo, boolean isAssemble, Integer roleType) {
        ClusterComponentPwdInfoRequest request = new ClusterComponentPwdInfoRequest();
        request.setClusterComponentPwdInfoList(clusterComponentPwdInfo);
        request.setAssemble(isAssemble);
        if (!isAssemble) {
            request.setRoleType(roleType);
        }
        return configMapService.replaceComponentPassword(request);
    }

    /**
     * 更新内部组件database/kafka/redis/cifs/dataturbo密码
     *
     * @return InfrastructureResponse
     */
    public InfrastructureResponse updateInternalComponentPassword() {
        return configMapService.updateComponentPassword();
    }
}
