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
package openbackup.tdsql.resources.access.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;
import openbackup.tdsql.resources.access.service.TdsqlService;
import openbackup.tdsql.resources.access.util.TdsqlClusterGroupValidator;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.stream.Collectors;

/**
 * 功能描述 TDSQL 分布式实例 Provider
 *
 */
@Slf4j
@Component
public class TdsqlClusterGroupProvider implements ResourceProvider {
    private final TdsqlService tdsqlService;

    /**
     * 构造
     *
     * @param tdsqlService tdsqlService
     */
    public TdsqlClusterGroupProvider(TdsqlService tdsqlService) {
        this.tdsqlService = tdsqlService;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType().equals(object.getSubType());
    }

    /**
     * 检查受保护资源，创建逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(resource.getName());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(resource.getName());
        TdsqlClusterGroupValidator.checkTdsqlGroupParams(resource);

        checkLinkStatus(resource.getParentUuid());

        // 注册的时候校验实例唯一性
        String clusterGroupInfo = resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_GROUP_INFO);
        TdsqlGroup tdsqlGroup = JsonUtil.read(clusterGroupInfo, TdsqlGroup.class);
        if (resource.getExtendInfoByKey(TdsqlConstant.INSTANCE_ID) == null) {
            log.info("TDSQL group id check is unique " + tdsqlGroup.getId());
            List<ProtectedResource> instances = tdsqlService.getChildren(resource.getParentUuid(),
                ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
            if (instances.size() > 0) {
                List<String> collect = instances.stream()
                    .map(instance -> instance.getExtendInfoByKey(TdsqlConstant.INSTANCE_ID))
                    .collect(Collectors.toList());
                if (collect.contains(tdsqlGroup.getId())) {
                    throw new LegoCheckedException(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED,
                        "The instance has been registered！");
                }
            }
        }

        log.info("TDSQL Instance start to check beforeCreate.resourceName:{}", resource.getName());
        ProtectedEnvironment environment = BeanTools.copy(resource, ProtectedEnvironment::new);
        // 校验实例信息（下发到数据节点执行OSS查询实例数据和入参进行比较）
        if (!tdsqlService.checkGroupInfo(tdsqlGroup, environment)) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The TDSQL GroupInfoCheck failed.");
        }

        // 校验每个数据节点的ip是否属于所关联的代理主机
        tdsqlGroup.getGroup().getDataNodes().forEach(dataNode -> {
            if (!tdsqlService.checkDataNodeIsMatchAgent(dataNode, environment)) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                    "The TDSQL Group dataNode query failed.");
            }
        });
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        resource.setExtendInfoByKey(TdsqlConstant.INSTANCE_ID, tdsqlGroup.getId());
        // 注册分布式实例时， 添加Path属性
        setClusterGroupPath(resource);
    }

    /**
     * 校验应用集群是否在线;
     *
     * @param uuid 集群uuid
     */
    public void checkLinkStatus(String uuid) {
        ProtectedEnvironment clusterEnvironment = tdsqlService.getEnvironmentById(uuid);
        if (EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(clusterEnvironment)) {
            return;
        }
        log.error("tdsql  group Cluster {} Not Online", clusterEnvironment.getName());
        throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
            "tdsql group  cluster " + clusterEnvironment.getName() + " not online");
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        beforeCreate(resource);
    }

    /**
     * 设置集群实例的path，保证副本复制不会出错
     *
     * @param resource 集群实例资源
     */
    private void setClusterGroupPath(ProtectedResource resource) {
        List<String> managerNodeEndpoints = tdsqlService.getEnvironmentById(resource.getParentUuid())
            .getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(protectedResource -> tdsqlService.getEnvironmentById(protectedResource.getUuid()).getEndpoint())
            .collect(Collectors.toList());
        List<String> instanceNodeEndpoints = resource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(protectedResource -> tdsqlService.getEnvironmentById(protectedResource.getUuid()).getEndpoint())
            .collect(Collectors.toList());
        managerNodeEndpoints.addAll(instanceNodeEndpoints);
        String path = managerNodeEndpoints.stream().distinct().sorted().collect(Collectors.joining(","));
        resource.setPath(path);
    }

    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        ResourceDeleteContext resourceDeleteContext = ResourceDeleteContext.defaultValue();
        log.info("[Tdsql Delete] delete start, cluster uuid: {}", resource.getUuid());
        ProtectedResource protectedResource;
        try {
            protectedResource = tdsqlService.getResourceById(resource.getUuid());
        } catch (LegoCheckedException e) {
            log.warn("resource {} not exist", resource.getUuid());
            return resourceDeleteContext;
        }

        // 资源离线状态直接返回
        if (protectedResource == null || StringUtils.equals(
            protectedResource.getExtendInfoByKey(TdsqlConstant.LINKSTATUS),
            LinkStatusEnum.OFFLINE.getStatus().toString())) {
            log.warn("Tdsql cluster {} is offline", resource.getUuid());
            return resourceDeleteContext;
        }

        String clusterGroupInfo = resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_GROUP_INFO);
        TdsqlGroup tdsqlGroup = JsonUtil.read(clusterGroupInfo, TdsqlGroup.class);
        try {
            // 解除数据仓持续挂载
            tdsqlService.umountDataRepo(tdsqlGroup, resource);

            // 删除持久仓白名单
            tdsqlService.removeDataRepoWhiteListOfResource(protectedResource.getUuid());
        } catch (LegoUncheckedException | FeignException e) {
            log.error("tdsql Execute umount repo task failed.", e);
            return ResourceDeleteContext.defaultValue();
        }
        log.info("Execute umount repo task completed, resource id: {}.", resource.getUuid());
        return ResourceDeleteContext.defaultValue();
    }
}
