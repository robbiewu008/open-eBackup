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
package openbackup.tidb.resources.access.util;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.service.TidbService;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Random;
import java.util.stream.Collectors;

/**
 * tidb 校验工具类
 *
 * @author w00426202
 * @since 2023-07-14
 */
@Slf4j
public class TidbUtil {
    private TidbUtil() {
    }

    /**
     * 创建/更新Tdsql集群时，校验参数
     *
     * @param protectedEnvironment 集群环境
     */
    public static void checkTidbReqParam(ProtectedResource protectedEnvironment) {
        verifyEnvName(protectedEnvironment.getName());
        Map<String, String> extendInfo = protectedEnvironment.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("TDSQL cluster extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "TDSQL cluster extendInfo is null.");
        }
    }

    /**
     * 创建/更新Tdsql集群时，校验ClusterInfoList参数
     *
     * @param protectedEnvironment 集群环境
     */
    public static void checkClusterInfoList(ProtectedResource protectedEnvironment) {
        Map<String, String> extendInfo = protectedEnvironment.getExtendInfo();
        String clusterInfoList = MapUtils.getString(extendInfo, TidbConstants.CLUSTER_INFO_LIST);
        JSONArray clusterInfoArray = JSONArray.parseArray(clusterInfoList);
        if (Objects.isNull(clusterInfoArray) || clusterInfoArray.size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "param illegal.");
        } else {
            for (int i = 0; i < clusterInfoArray.size(); i++) {
                // 遍历 jsonarray 数组，把每一个对象转成 json 对象
                JSONObject node = clusterInfoArray.getJSONObject(i);
                // 得到 每个对象中的属性值
                nodeCheck(node, TidbConstants.ID, TidbConstants.ROLE, TidbConstants.HOST, TidbConstants.STATUS,
                    TidbConstants.HOST_MANAGER_IP, TidbConstants.HOST_MANAGER_RESOURCE_UUID);
            }
        }
    }

    private static void nodeCheck(JSONObject node, String... keys) {
        for (String key : keys) {
            Object nameObject = node.get(key);
            if (Objects.isNull(nameObject) || StringUtils.isEmpty(nameObject.toString())) {
                log.error("TDSQL cluster extendInfo {} is null.", key);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "param illegal.");
            }
        }
    }

    private static void verifyEnvName(String name) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(name);
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(name);
    }

    /**
     * 包装目标resource
     *
     * @param targetResource targetResource
     * @param srcResource srcResource
     */
    public static void wrapExtendInfo2Add(ProtectedResource targetResource, ProtectedResource srcResource) {
        Map<String, String> extendInfoAdd = new HashMap<>();
        extendInfoAdd.put(TidbConstants.LOG_BACKUP_PATH,
            srcResource.getExtendInfo().get(TidbConstants.LOG_BACKUP_PATH));
        extendInfoAdd.put(TidbConstants.TIUP_PATH, srcResource.getExtendInfo().get(TidbConstants.TIUP_PATH));
        extendInfoAdd.put(TidbConstants.CLUSTER_INFO_LIST,
            srcResource.getExtendInfo().get(TidbConstants.CLUSTER_INFO_LIST));
        targetResource.getExtendInfo().putAll(extendInfoAdd);
    }

    /**
     * 找到一个可用的tiup节点
     *
     * @param protectObjectExtendInfo 保护对象的扩展参数
     * @param clusterUuid 集群的uuid
     * @param resourceService 资源服务
     * @param defaultSelector 默认的Selector
     * @param tidbService tidb服务
     */
    public static void setTiupUuid(Map<String, String> protectObjectExtendInfo, String clusterUuid,
        ResourceService resourceService, DefaultProtectAgentSelector defaultSelector, TidbService tidbService) {
        ProtectedResource protectedResource = resourceService.getResourceById(clusterUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        List<ProtectedResource> resourceList = protectedResource.getDependencies().get(DatabaseConstants.AGENTS);
        List<String> agentList = resourceList.stream().map(item -> item.getUuid()).collect(Collectors.toList());
        String agents = String.join(";", agentList);
        List<Endpoint> endpoints = new ArrayList<>();
        if (StringUtils.isNotEmpty(agents)) {
            endpoints = defaultSelector.selectByAgentParameter(agents, null);
        }
        if (CollectionUtils.isNotEmpty(endpoints)) {
            // 先随机找一个
            Endpoint chosenEndpoint = endpoints.stream()
                .skip(new Random().nextInt(endpoints.size()))
                .findFirst()
                .orElse(null);
            ProtectedResource agentResource = tidbService.getResourceByCondition(chosenEndpoint.getId());
            // 检查集群，看看是否正常
            try {
                tidbService.checkHealth(protectedResource, agentResource, ResourceSubTypeEnum.TIDB_CLUSTER.getType(),
                    TidbConstants.CHECK_CLUSTER);
                protectObjectExtendInfo.put(TidbConstants.TIUP_UUID, chosenEndpoint.getId());
            } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
                log.error("Get browse result failed: %s", ExceptionUtil.getErrorMessage(e));
                // 如果不正常，在endpoints中遍历找个正常的节点
                if (findEndpoint(protectObjectExtendInfo, tidbService, protectedResource, endpoints, chosenEndpoint)) {
                    return;
                }
                throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "No agent found");
            }
        }
    }

    private static boolean findEndpoint(Map<String, String> protectObjectExtendInfo, TidbService tidbService,
        ProtectedResource protectedResource, List<Endpoint> endpoints, Endpoint chosenEndpoint) {
        ProtectedResource agentResource;
        for (Endpoint endpoint : endpoints) {
            if (StringUtils.equals(endpoint.getId(), chosenEndpoint.getId())) {
                continue;
            }
            agentResource = tidbService.getResourceByCondition(endpoint.getId());
            try {
                tidbService.checkHealth(protectedResource, agentResource, ResourceSubTypeEnum.TIDB_CLUSTER.getType(),
                    TidbConstants.CHECK_CLUSTER);
                protectObjectExtendInfo.put(TidbConstants.TIUP_UUID, endpoint.getId());
                return true;
            } catch (LegoCheckedException | LegoUncheckedException | FeignException ex) {
                log.error("Get browse result failed: %s", ExceptionUtil.getErrorMessage(ex));
            }
        }
        return false;
    }

    /**
     * 找到一个可用的tiup节点
     *
     * @param clusterResource 集群
     * @param tidbService tidb服务
     * @return key为集群节点，value为集群节点对应的主机列表
     */
    public static Map<ProtectedResource, List<ProtectedEnvironment>> getProtectedResourceListMap(
        ProtectedResource clusterResource, TidbService tidbService) {
        List<ProtectedResource> agentList = clusterResource.getDependencies().get(DatabaseConstants.AGENTS);
        String agentUuid = agentList.get(0).getUuid();
        for (ProtectedResource agentResource : agentList) {
            // 检查集群
            try {
                tidbService.checkHealth(clusterResource, agentResource, ResourceSubTypeEnum.TIDB_CLUSTER.getType(),
                    TidbConstants.CHECK_CLUSTER);
                agentUuid = agentResource.getUuid();
                // 检查数据库，检查表
                break;
            } catch (LegoCheckedException e) {
                log.error("cluster health check failed code: {}, message: {}", e.getErrorCode(), e.getMessage());
            }
        }
        ProtectedResource agentResource = tidbService.getResourceByCondition(agentUuid);
        ProtectedEnvironment protectedEnvironment = BeanTools.copy(agentResource, ProtectedEnvironment::new);

        Map<ProtectedResource, List<ProtectedEnvironment>> resultMap = new LinkedHashMap<>();
        resultMap.put(clusterResource, Arrays.asList(protectedEnvironment));
        return resultMap;
    }
}
