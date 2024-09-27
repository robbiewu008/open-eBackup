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
package openbackup.saphana.protection.access.util;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * SAP HANA工具类
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-10
 */
@Slf4j
public class SapHanaUtil {
    /**
     * 检查环境扩展信息参数
     *
     * @param environment 环境信息
     */
    public static void checkEnvironmentExtendInfoParam(ProtectedEnvironment environment) {
        Optional.ofNullable(environment.getExtendInfo())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The environment extendInfo is empty"));
    }

    /**
     * 检查资源扩展信息参数
     *
     * @param resource 资源信息
     */
    public static void checkResourceExtendInfoParam(ProtectedResource resource) {
        Optional.ofNullable(resource.getExtendInfo())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The resource extendInfo is empty"));
    }

    /**
     * 检查SAP HANA的数据库类型参数
     *
     * @param resource 资源信息
     */
    public static void checkDbTypeParam(ProtectedResource resource) {
        String hanaDbType = resource.getExtendInfo().get(SapHanaConstants.SAP_HANA_DB_TYPE);
        if (hanaDbType == null || !Arrays.asList(SapHanaConstants.SYSTEM_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE)
            .contains(hanaDbType)) {
            log.error("The sap hana database type is invalid, type: {}.", hanaDbType);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The database type is invalid");
        }
    }

    /**
     * 设置SAP HANA的System ID为全小写
     *
     * @param environment 环境信息
     */
    public static void setSystemId(ProtectedEnvironment environment) {
        String systemId = environment.getExtendInfo().get(SapHanaConstants.SYSTEM_ID);
        environment.setExtendInfoByKey(SapHanaConstants.SYSTEM_ID, systemId.toLowerCase(Locale.ROOT));
    }

    /**
     * 设置SAP HANA的数据库名称为全大写
     *
     * @param resource 资源信息
     */
    public static void setDatabaseName(ProtectedResource resource) {
        resource.setName(resource.getName().toUpperCase(Locale.ROOT));
    }

    /**
     * 设置连通测试的action扩展信息
     *
     * @param resource 资源信息
     * @param operationType 操作类型
     */
    public static void setOperationTypeExtendInfo(ProtectedResource resource, String operationType) {
        resource.setExtendInfoByKey(SapHanaConstants.OPERATION_TYPE, operationType);
    }

    /**
     * 设置节点角色
     *
     * @param env 环境信息
     * @param nodeRole 节点角色
     */
    public static void setNodeRole(ProtectedEnvironment env, String nodeRole) {
        Map<String, String> nodeExtInfo = Optional.ofNullable(env.getExtendInfo()).orElseGet(HashMap::new);
        nodeExtInfo.put(DatabaseConstants.ROLE, nodeRole);
        env.setExtendInfo(nodeExtInfo);
    }

    /**
     * 设置实例的NODES扩展信息
     *
     * @param env 实例信息
     * @param nodes 实例节点信息列表
     * @return ProtectedEnvironment
     */
    public static ProtectedEnvironment setInstanceEnvExtendInfoNodes(ProtectedEnvironment env,
        List<ProtectedEnvironment> nodes) {
        Map<String, String> envExtendInfo = Optional.ofNullable(env.getExtendInfo()).orElseGet(HashMap::new);
        envExtendInfo.put(SapHanaConstants.NODES, JSONObject.stringify(nodes));
        envExtendInfo.remove(SapHanaConstants.OPERATION_TYPE);
        env.setExtendInfo(envExtendInfo);
        return env;
    }

    /**
     * 设置数据库资源的状态信息
     *
     * @param resource 数据库资源信息
     * @param linkStatus 数据库状态
     * @return ProtectedResource
     */
    public static ProtectedResource setDatabaseResourceLinkStatus(ProtectedResource resource, String linkStatus) {
        Map<String, String> envExtendInfo = Optional.ofNullable(resource.getExtendInfo()).orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.LINK_STATUS_KEY, linkStatus);
        envExtendInfo.remove(SapHanaConstants.OPERATION_TYPE);
        resource.setExtendInfo(envExtendInfo);
        return resource;
    }

    /**
     * 从数据库资源信息中获取nodes的值
     *
     * @param resources 数据库资源信息
     * @return nodes信息
     */
    public static String getResourceExtendNodesInfo(ProtectedResource resources) {
        String tmpNodesInfo = resources.getExtendInfoByKey(SapHanaConstants.NODES);
        if (StringUtils.isEmpty(tmpNodesInfo)) {
            log.warn("The nodes parameter of sap hana resource is empty, uuid: {}.", resources.getUuid());
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "The nodes parameter of sap hana resource is empty.");
        }
        return tmpNodesInfo;
    }

    /**
     * 解析实例或者数据库资源的主机环境信息列表
     *
     * @param resource 数据库资源信息
     * @return ProtectedEnvironment列表
     */
    public static List<ProtectedEnvironment> parseHostProtectedEnvironmentList(ProtectedResource resource) {
        String tmpNodesInfo = getResourceExtendNodesInfo(resource);
        JSONArray tmpNodesJsonArray = JSONArray.fromObject(tmpNodesInfo);
        return JSONArray.toCollection(tmpNodesJsonArray, ProtectedEnvironment.class);
    }

    /**
     * 解析数据库主机资源信息列表
     *
     * @param dbResource 数据库资源信息
     * @return ProtectedResource列表
     */
    public static List<ProtectedResource> parseDbHostProtectedResourceList(ProtectedResource dbResource) {
        String tmpNodesInfo = getResourceExtendNodesInfo(dbResource);
        JSONArray tmpNodesJsonArray = JSONArray.fromObject(tmpNodesInfo);
        return JSONArray.toCollection(tmpNodesJsonArray, ProtectedResource.class);
    }

    /**
     * 将ProtectedEnvironment信息列表转换为TaskEnvironment信息列表
     *
     * @param protectedEnvList ProtectedEnvironment信息列表
     * @return TaskEnvironment信息列表
     */
    public static List<TaskEnvironment> convertEnvListToTaskEnvList(List<ProtectedEnvironment> protectedEnvList) {
        return protectedEnvList.stream().map(SapHanaUtil::covertEnvToTaskEnv).collect(Collectors.toList());
    }

    /**
     * 将ProtectedEnvironment信息转换为TaskEnvironment信息
     *
     * @param protectedEnv ProtectedEnvironment信息
     * @return TaskEnvironment信息
     */
    public static TaskEnvironment covertEnvToTaskEnv(ProtectedEnvironment protectedEnv) {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(protectedEnv, taskEnvironment);
        return taskEnvironment;
    }

    /**
     * 将ProtectedEnvironment信息列表转换为Endpoint信息列表
     *
     * @param protectedEnvList ProtectedEnvironment信息列表
     * @return Endpoint信息列表
     */
    public static List<Endpoint> convertEnvListToEndpointList(List<ProtectedEnvironment> protectedEnvList) {
        return protectedEnvList.stream()
            .map(host -> new Endpoint(host.getUuid(), host.getEndpoint(), host.getPort()))
            .collect(Collectors.toList());
    }

    /**
     * 从操作结果中获取指定键的值
     *
     * @param actionResult 操作结果
     * @param key 键
     * @return 值
     */
    public static String getValueFromActionResultByKey(ActionResult actionResult, String key) {
        Map<String, String> messageMap = JSONObject.fromObject(actionResult.getMessage()).toMap(String.class);
        return messageMap.getOrDefault(key, "");
    }

    /**
     * 从实例资源信息中获取NODES的值
     *
     * @param environment 实例资源信息
     * @return NODES信息
     */
    public static String getInstanceExtendNodesInfo(ProtectedEnvironment environment) {
        String tmpNodesInfo = environment.getExtendInfoByKey(SapHanaConstants.NODES);
        if (StringUtils.isEmpty(tmpNodesInfo)) {
            log.warn("The nodes parameter of sap hana instance is empty, uuid: {}.", environment.getUuid());
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "The nodes parameter of sap hana instance is empty.");
        }
        return tmpNodesInfo;
    }

    /**
     * 解析实例的主机环境信息列表
     *
     * @param environment 实例资源信息
     * @return 主机环境信息列表
     */
    public static List<ProtectedEnvironment> parseInstanceHostEnvList(ProtectedEnvironment environment) {
        String tmpNodesInfo = getInstanceExtendNodesInfo(environment);
        JSONArray tmpNodesJsonArray = JSONArray.fromObject(tmpNodesInfo);
        return JSONArray.toCollection(tmpNodesJsonArray, ProtectedEnvironment.class);
    }

    /**
     * 解析实例的主机资源信息列表
     *
     * @param environment 实例资源信息
     * @return 主机资源信息列表
     */
    public static List<ProtectedResource> parseInstanceHostResourceList(ProtectedEnvironment environment) {
        String tmpNodesInfo = getInstanceExtendNodesInfo(environment);
        JSONArray tmpNodesJsonArray = JSONArray.fromObject(tmpNodesInfo);
        return JSONArray.toCollection(tmpNodesJsonArray, ProtectedResource.class);
    }

    /**
     * 是否系统数据库
     *
     * @param resource 资源信息
     * @return 是否系统数据库
     */
    public static boolean isSystemDatabase(ProtectedResource resource) {
        String hanaDbType = Optional.ofNullable(resource.getExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_TYPE))
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The sapHanaDbType parameter is empty."));
        if (!Arrays.asList(SapHanaConstants.SYSTEM_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE).contains(hanaDbType)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The sapHanaDbType parameter is invalid.");
        }
        return SapHanaConstants.SYSTEM_DB_TYPE.equals(hanaDbType);
    }

    /**
     * 获取数据库部署类型
     *
     * @param hostsNum 数据库所在主机数目
     * @param hanaDbType SAP HANA数据库类型
     * @return 数据库部署类型
     */
    public static String getDeployType(int hostsNum, String hanaDbType) {
        String deployType = DatabaseDeployTypeEnum.SINGLE.getType();
        if (hostsNum > IsmNumberConstant.ONE) {
            if (SapHanaConstants.SYSTEM_DB_TYPE.equals(hanaDbType)) {
                // 系统数据库集群是主备
                deployType = DatabaseDeployTypeEnum.AP.getType();
            } else {
                // 租户数据库集群是分布式
                deployType = DatabaseDeployTypeEnum.DISTRIBUTED.getType();
            }
        }
        return deployType;
    }

    /**
     * 从通用数据库SAP HANA数据库的自定义参数中获取system id
     *
     * @param customParams SAP HANA数据库的自定义参数
     * @return system id
     */
    public static String getSystemIdFromCustomParams(String customParams) {
        String systemId = "";
        if (StringUtils.isEmpty(customParams)) {
            return systemId;
        }
        for (String param : customParams.split(",")) {
            if (!param.contains("=")) {
                continue;
            }
            String[] tmpSplits = param.split("=");
            if (tmpSplits.length != IsmNumberConstant.TWO) {
                continue;
            }
            if (StringUtils.equals(SapHanaConstants.SYSTEM_ID, tmpSplits[0])) {
                systemId = tmpSplits[1];
                break;
            }
        }
        return systemId;
    }

    /**
     * 从资源的扩展信息中移除指定key
     *
     * @param resource 资源信息
     * @param key 扩展参数中key
     */
    public static void removeExtendInfoByKey(ProtectedResource resource, String key) {
        Map<String, String> envExtendInfo = Optional.ofNullable(resource.getExtendInfo()).orElseGet(HashMap::new);
        envExtendInfo.remove(key);
        resource.setExtendInfo(envExtendInfo);
    }
}
