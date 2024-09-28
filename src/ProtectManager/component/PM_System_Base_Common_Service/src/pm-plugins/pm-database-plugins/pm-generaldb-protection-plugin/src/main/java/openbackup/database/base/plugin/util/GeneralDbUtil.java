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
package openbackup.database.base.plugin.util;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * 通用数据库工具类
 *
 */
@Slf4j
public class GeneralDbUtil {
    /**
     * 将str的app conf转化为对象
     *
     * @param appConfStr conf字符串
     * @return conf map对象
     */
    public static Optional<AppConf> getAppConf(String appConfStr) {
        if (VerifyUtil.isEmpty(appConfStr)) {
            return Optional.empty();
        }
        AppConf appConf = JsonUtil.read(appConfStr, AppConf.class);
        return Optional.ofNullable(appConf);
    }

    /**
     * 获取host
     *
     * @param resource 资源
     * @return host
     */
    public static List<ProtectedEnvironment> getHosts(ProtectedResource resource) {
        // nodes
        List<ProtectedResource> nodes = getNodesFromDependency(resource);
        if (!VerifyUtil.isEmpty(nodes)) {
            return nodes.stream()
                .map(GeneralDbUtil::getHostsFromHostKey)
                .flatMap(Collection::stream)
                .collect(Collectors.toList());
        }
        return getHostsFromHostKey(resource);
    }

    /**
     * 获取host，仅从dependency结构获取
     *
     * @param resource 资源
     * @return host
     */
    public static List<ProtectedResource> getHostsByDependencyKey(ProtectedResource resource) {
        List<ProtectedResource> nodes = getNodesFromDependency(resource);
        if (!VerifyUtil.isEmpty(nodes)) {
            return nodes.stream()
                .map(GeneralDbUtil::getHostsFromDependency)
                .flatMap(Collection::stream)
                .collect(Collectors.toList());
        }
        return getHostsFromDependency(resource);
    }

    /**
     * 获取节点
     *
     * @param resource 资源
     * @return 节点
     */
    public static List<ProtectedResource> getNodesFromDependency(ProtectedResource resource) {
        return Optional.ofNullable(resource.getDependencies())
            .map(e -> e.get(GeneralDbConstant.DEPENDENCY_CLUSTER_NODE_KEY))
            .orElse(Collections.emptyList());
    }

    /**
     * 根据hostKey获取host
     *
     * @param resource 资源
     * @return host
     */
    public static List<ProtectedEnvironment> getHostsFromHostKey(ProtectedResource resource) {
        List<ProtectedResource> resources = getHostsFromDependency(resource);
        return resources.stream()
            .filter(e -> e instanceof ProtectedEnvironment)
            .map(e -> (ProtectedEnvironment) e)
            .collect(Collectors.toList());
    }

    /**
     * 查询依赖字段中的Host
     *
     * @param resource 资源
     * @return 主机
     */
    public static List<ProtectedResource> getHostsFromDependency(ProtectedResource resource) {
        return Optional.ofNullable(resource.getDependencies())
            .map(e -> e.get(GeneralDbConstant.DEPENDENCY_HOST_KEY))
            .orElse(Collections.emptyList());
    }

    /**
     * 字符串列表中是否有该字符串，忽略大小写
     *
     * @param list 字符串列表
     * @param elem 字符串
     * @return 是否有该字符串
     */
    public static boolean isListContainsElemWithoutCase(List<String> list, String elem) {
        if (VerifyUtil.isEmpty(list)) {
            return false;
        }
        for (String s : list) {
            if (s == null && elem == null) {
                return true;
            }
            if (s != null && s.equalsIgnoreCase(elem)) {
                return true;
            }
        }

        return false;
    }

    /**
     * 将ProtectedResource转化为AppEnv
     *
     * @param resource 资源
     * @param hostList 主机
     * @param originResource 原来的资源
     * @return AppEnv类型的资源
     */
    public static AppEnv resourceToAppEnv(ProtectedResource resource, List<ProtectedEnvironment> hostList,
        ProtectedResource originResource) {
        setProtectResourceFullHostInfo(resource, hostList);
        AppEnv appEnv = pureResourceToAppEnv(resource, originResource);

        List<ProtectedResource> nodes = getNodesFromDependency(resource);
        if (VerifyUtil.isEmpty(nodes)) {
            List<ProtectedEnvironment> hosts = getHostsFromHostKey(resource);
            hosts.forEach(host -> appEnv.getNodes().add(pureResourceToAppEnv(host, null)));
        } else {
            for (ProtectedResource node : nodes) {
                appEnv.getNodes().add(resourceToAppEnv(node, new ArrayList<>(), null));
            }
        }
        return appEnv;
    }

    private static AppEnv pureResourceToAppEnv(ProtectedResource resource, ProtectedResource originResource) {
        AppEnv appEnv = new AppEnv();
        appEnv.setUuid(resource.getUuid());
        appEnv.setName(resource.getName());
        appEnv.setType(resource.getType());
        appEnv.setSubType(resource.getSubType());
        appEnv.setAuth(resource.getAuth());
        appEnv.setExtendInfo(resource.getExtendInfo());
        appEnv.setNodes(new ArrayList<>());
        if (resource instanceof ProtectedEnvironment) {
            appEnv.setEndpoint(((ProtectedEnvironment) resource).getEndpoint());
            appEnv.setPort(((ProtectedEnvironment) resource).getPort());
        }
        if (originResource != null) {
            appEnv.getExtendInfo().put(GeneralDbConstant.EXTEND_ORIGIN_RESOURCE_KEY, JsonUtil.json(originResource));
        }
        return appEnv;
    }

    /**
     * 将AppResource转化为ProtectedResource
     *
     * @param protectedResource 资源
     * @param hostList 主机
     * @param appResource AppResource类型的资源
     */
    public static void appResourceToProtectedResource(ProtectedResource protectedResource,
        List<ProtectedEnvironment> hostList, AppResource appResource) {
        protectedResource.setUuid(appResource.getUuid());
        Optional.ofNullable(appResource.getExtendInfo())
            .map(e -> e.get(GeneralDbConstant.EXTEND_VERSION_KEY))
            .ifPresent(protectedResource::setVersion);
        protectedResource.setExtendInfo(appResource.getExtendInfo());
        setProtectResourceOnlyHostUuid(protectedResource, hostList);
    }

    /**
     * 设置资源的主机信息为详细信息
     *
     * @param resource 资源
     * @param hostList 主机列表
     */
    public static void setProtectResourceFullHostInfo(ProtectedResource resource, List<ProtectedEnvironment> hostList) {
        Map<String, ProtectedEnvironment> hostMap = hostList.stream()
            .collect(Collectors.toMap(ResourceBase::getUuid, Function.identity()));

        List<ProtectedResource> nodes = getNodesFromDependency(resource);
        if (nodes.isEmpty()) {
            List<ProtectedResource> hosts = getHostsFromDependency(resource);
            setHostFullHostInfo(hosts, hostMap);
        } else {
            for (ProtectedResource node : nodes) {
                List<ProtectedResource> nodeHosts = getHostsFromDependency(node);
                setHostFullHostInfo(nodeHosts, hostMap);
            }
        }
    }

    /**
     * 检查版本信息是否满足
     *
     * @param version 应用的版本信息
     * @param minVersion 配置的最小版本
     * @param maxVersion 配置的最大版本
     * @return 应用版本信息是否符合配置的版本信息
     */
    public static boolean checkVersion(String version, String minVersion, String maxVersion) {
        if (VerifyUtil.isEmpty(maxVersion) && VerifyUtil.isEmpty(minVersion)) {
            return true;
        }
        if (VerifyUtil.isEmpty(version)) {
            return false;
        }
        boolean shouldPass = true;
        if (!VerifyUtil.isEmpty(maxVersion)) {
            shouldPass = version.compareTo(maxVersion) <= 0;
        }
        if (!VerifyUtil.isEmpty(minVersion)) {
            shouldPass = shouldPass && version.compareTo(minVersion) >= 0;
        }
        return shouldPass;
    }

    private static void setHostFullHostInfo(List<ProtectedResource> hosts,
        Map<String, ProtectedEnvironment> hostMap) {
        for (int i = 0; i < hosts.size(); i++) {
            ProtectedEnvironment host = hostMap.get(hosts.get(i).getUuid());
            if (host != null) {
                hosts.set(i, host);
            }
        }
    }

    private static void setProtectResourceOnlyHostUuid(ProtectedResource resource,
        List<ProtectedEnvironment> hostList) {
        Map<String, ProtectedEnvironment> hostMap = hostList.stream()
            .collect(Collectors.toMap(ResourceBase::getUuid, Function.identity()));

        List<ProtectedResource> nodes = getNodesFromDependency(resource);
        if (nodes.isEmpty()) {
            List<ProtectedResource> hosts = getHostsFromDependency(resource);
            setHostOnlyHostUuid(hosts, hostMap);
        } else {
            for (ProtectedResource node : nodes) {
                List<ProtectedResource> nodeHosts = getHostsFromDependency(node);
                setHostOnlyHostUuid(nodeHosts, hostMap);
            }
        }
    }

    private static void setHostOnlyHostUuid(List<ProtectedResource> hosts,
        Map<String, ProtectedEnvironment> hostMap) {
        for (int i = 0; i < hosts.size(); i++) {
            ProtectedEnvironment host = hostMap.get(hosts.get(i).getUuid());
            if (host != null) {
                ProtectedResource protectedResource = new ProtectedResource();
                protectedResource.setUuid(hosts.get(i).getUuid());
                hosts.set(i, protectedResource);
            }
        }
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
            if (StringUtils.equals(GeneralDbConstant.SYSTEM_ID, tmpSplits[0])) {
                systemId = tmpSplits[1];
                break;
            }
        }
        return systemId;
    }
}
