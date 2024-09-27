/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.saphana.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.saphana.protection.access.util.SapHanaUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * SAPHANA数据库Provider
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-09
 */
@Component
@Slf4j
public class SapHanaDatabaseProvider implements ResourceProvider {
    private final ClusterEnvironmentService clusterEnvironmentService;

    private final SapHanaResourceService hanaResourceService;

    /**
     * SapHanaDatabaseProvider构造方法
     *
     * @param clusterEnvironmentService 集群环境业务类
     * @param hanaResourceService SAP HANA资源业务类
     */
    public SapHanaDatabaseProvider(ClusterEnvironmentService clusterEnvironmentService,
        SapHanaResourceService hanaResourceService) {
        this.clusterEnvironmentService = clusterEnvironmentService;
        this.hanaResourceService = hanaResourceService;
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.SAPHANA_DATABASE.equalsSubType(protectedResource.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("Start create sap hana database, resource name: {}", resource.getName());
        SapHanaUtil.checkResourceExtendInfoParam(resource);
        SapHanaUtil.checkDbTypeParam(resource);
        hanaResourceService.setDatabaseResourceInfo(resource);
        if (SapHanaUtil.isSystemDatabase(resource)) {
            checkHasOneHostOnline(resource);
        } else {
            checkAllHostsOnline(resource);
        }
        // 数据库名称不区分大小写，统一按大写处理
        SapHanaUtil.setDatabaseName(resource);
        // 检查是否已注册（sub_type、实例资源uuid、数据库名称）
        hanaResourceService.checkDbIsRegistered(resource);
        // 检查数据库是否在“通用数据库”应用中注册
        hanaResourceService.checkDbIsRegisteredInGeneralDb(resource);
        // 检查连通性
        hanaResourceService.checkDatabaseConnection(resource);
        ProtectedResource instResource = hanaResourceService.getResourceById(resource.getParentUuid());
        resource.setVersion(instResource.getVersion());
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End create sap hana database, resource name: {}, uuid: {}", resource.getName(), resource.getUuid());
    }

    private void checkHasOneHostOnline(ProtectedResource resource) {
        List<ProtectedEnvironment> agentEnvList = SapHanaUtil.parseHostProtectedEnvironmentList(resource);
        // 只要有一个主机在线，则允许注册或者修改
        boolean isOnline = agentEnvList.stream()
            .anyMatch(env -> LinkStatusEnum.ONLINE.getStatus().toString().equals(env.getLinkStatus()));
        if (!isOnline) {
            log.error("All hosts are offLine.");
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Select host is offLine.");
        }
    }

    private void checkAllHostsOnline(ProtectedResource resource) {
        List<ProtectedEnvironment> agentEnvList = SapHanaUtil.parseHostProtectedEnvironmentList(resource);
        // 只要有一个主机离线，则不允许注册或者修改
        boolean isOffline = agentEnvList.stream()
            .anyMatch(env -> LinkStatusEnum.OFFLINE.getStatus().toString().equals(env.getLinkStatus()));
        if (isOffline) {
            log.error("The sap hana tenant database(name={}) has offline host.", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Select host is offLine.");
        }
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update sap hana database, name: {}, uuid: {}.", resource.getName(), resource.getUuid());
        hanaResourceService.setDatabaseResourceInfo(resource);
        if (SapHanaUtil.isSystemDatabase(resource)) {
            checkHasOneHostOnline(resource);
        } else {
            checkAllHostsOnline(resource);
        }
        SapHanaUtil.setOperationTypeExtendInfo(resource, SapHanaConstants.MODIFY_OPERATION_TYPE);
        // 检查连通性
        hanaResourceService.checkDatabaseConnection(resource);
        SapHanaUtil.removeExtendInfoByKey(resource, SapHanaConstants.OPERATION_TYPE);
        ProtectedResource instResource = hanaResourceService.getResourceById(resource.getParentUuid());
        resource.setVersion(instResource.getVersion());
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End create sap hana database, name: {}, uuid: {}", resource.getName(), resource.getUuid());
    }

    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = new ResourceFeature();
        // SAP HANA数据库不检查名称重复
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }
}
