/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbdws.protection.access.util;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.apache.commons.lang3.StringUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 环境变量工具类
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-01
 */
public class DwsTaskEnvironmentUtil {
    /**
     * 增加用户名
     *
     * @param protectEnv env
     * @param dwsUser 用户名
     */
    public static void initProtectEnvOfDwsUser(TaskEnvironment protectEnv, String dwsUser) {
        protectEnv.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_DWS_USER, dwsUser);
    }

    /**
     * 增加环境变量路径
     *
     * @param protectEnv env
     * @param envFilePath 文件路径
     */
    public static void initProtectEnvOfEnvFile(TaskEnvironment protectEnv, String envFilePath) {
        protectEnv.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_ENV_FILE, envFilePath);
    }

    /**
     * 增加主机类型
     *
     * @param protectEnv env
     * @param environment 环境信息
     */
    public static void initNodeType(TaskEnvironment protectEnv, ProtectedEnvironment environment) {
        List<String> clusterUuids = Optional.ofNullable(
            environment.getDependencies().get(DwsConstant.DWS_CLUSTER_AGENT))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_BUSY, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        List<String> hostUuids = Optional.ofNullable(environment.getDependencies().get(DwsConstant.HOST_AGENT))
            .orElse(new ArrayList<>())
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        protectEnv.getNodes().forEach(taskEnvironment -> initNodeType(clusterUuids, hostUuids, taskEnvironment));
    }

    /**
     * 增加主机类型
     *
     * @param clusterUuids 集群主键id列表
     * @param hostUuids 主机id列表
     * @param protectEnv 环境信息
     */
    public static void initNodeType(List<String> clusterUuids, List<String> hostUuids, TaskEnvironment protectEnv) {
        Map<String, String> protectEnvExtendInfo = Optional.ofNullable(protectEnv.getExtendInfo())
            .orElseGet(HashMap::new);
        if (clusterUuids.contains(protectEnv.getUuid())) {
            initNodeClusterType(protectEnvExtendInfo, protectEnv);
            return;
        }
        if (hostUuids.contains(protectEnv.getUuid())) {
            initNodeHostType(protectEnvExtendInfo, protectEnv);
            return;
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }

    private static void initNodeHostType(Map<String, String> protectEnvExtendInfo, TaskEnvironment protectEnv) {
        protectEnvExtendInfo.put(DwsConstant.EXTEND_INFO_KEY_NODE_TYPE, DwsConstant.EXTEND_INFO_VALUE_HOST_TYPE);
        protectEnv.setExtendInfo(protectEnvExtendInfo);
    }

    private static void initNodeClusterType(Map<String, String> protectEnvExtendInfo, TaskEnvironment protectEnv) {
        protectEnvExtendInfo.put(DwsConstant.EXTEND_INFO_KEY_NODE_TYPE, DwsConstant.EXTEND_INFO_VALUE_CLUSTER_TYPE);
        protectEnv.setExtendInfo(protectEnvExtendInfo);
    }

    /**
     * 获取主机的agentList信息
     *
     * @param protectedEnvironment 主机env
     * @return agentList信息
     */
    public static String getIps(ProtectedEnvironment protectedEnvironment) {
        Map<String, String> extendInfo = Optional.ofNullable(protectedEnvironment.getExtendInfo())
            .orElse(new HashMap<>());
        String ips = extendInfo.get(ResourceConstants.AGENT_IP_LIST);
        if (StringUtils.isEmpty(ips)) {
            return protectedEnvironment.getEndpoint();
        }
        return ips;
    }

    /**
     * 获取 环境的 dependencies信息
     *
     * @param environment 环境
     * @param agentKey dependencieskey值
     * @return 返回列表
     */
    public static List<ProtectedResource> getAgentResourcesByKey(ProtectedEnvironment environment, String agentKey) {
        Map<String, List<ProtectedResource>> dependencies = environment.getDependencies();
        // 获取集群key的集群列表
        return dependencies.get(agentKey);
    }

    /**
     * 构造环境的资源
     *
     * @param environment 环境对象
     * @return 资源对象
     */
    public static ProtectedResource getProtectedResource(ProtectedEnvironment environment) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(environment.getSubType());
        protectedResource.setType(environment.getType());
        protectedResource.setExtendInfo(environment.getExtendInfo());
        protectedResource.setAuth(environment.getAuth());
        protectedResource.setName(environment.getName());
        protectedResource.setDependencies(environment.getDependencies());
        return protectedResource;
    }
}
