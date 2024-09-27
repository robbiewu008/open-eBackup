/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oceanbase.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.constants.OBErrorCodeConstants;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.dto.OBTenantInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-06
 */
@Slf4j
@Component
public class OceanBaseTenantProvider implements ResourceProvider {
    private final OceanBaseService oceanBaseService;

    public OceanBaseTenantProvider(OceanBaseService oceanBaseService) {
        this.oceanBaseService = oceanBaseService;
    }

    @Override
    public boolean applicable(ProtectedResource resourceSubType) {
        return ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType().equals(resourceSubType.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("start to check OceanBase tenant: {}. uuid: {}, parentUuid: {}", resource.getName(),
            resource.getUuid(), resource.getParentUuid());
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(resource.getName());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(resource.getName());
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(resource);

        List<OBTenantInfo> tenantNameList = clusterInfo.getTenantInfos()
            .stream()
            .filter(item -> StringUtils.isNotEmpty(StringUtils.trim(item.getName())))
            .collect(Collectors.toList());

        // 如果租户为空， 则返回异常
        if (CollectionUtils.isEmpty(tenantNameList)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The tenant in tenant set is empty");
        }

        List<String> existTenantList = oceanBaseService.getExistingOceanBaseTenant(resource.getParentUuid(),
            resource.getUuid());

        log.info("exist OceanBase tenant: [{}].", existTenantList);

        // 检查tenantNames是否已经注册过（同一个集群下）。参数中的租户名称是否重复。
        for (OBTenantInfo tenantInfo : tenantNameList) {
            if (existTenantList.contains(tenantInfo.getName())) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, new String[] {tenantInfo.getName()},
                    "The tenant name: " + tenantInfo.getName() + " already register.");
            }
        }

        // 需要检查连通性， resource的extendinfo中要添加租户连通信息
        List<String> notExistTenants = oceanBaseService.checkTenantSetConnect(resource);
        if (CollectionUtils.isNotEmpty(notExistTenants)) {
            String notExistTenantNames = StringUtils.join(notExistTenants, OBConstants.COMMA);
            throw new LegoCheckedException(OBErrorCodeConstants.TENANT_NOT_EXIST, new String[] {notExistTenantNames},
                "The tenant name: " + JsonUtil.json(notExistTenants) + " not exist.");
        }
        resource.setPath(oceanBaseService.getEnvironmentById(resource.getParentUuid()).getPath());
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        // 查询数据库该租户集信息， 确认parentUuid未改。
        ProtectedResource agentEnv = oceanBaseService.getResourceById(resource.getUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST,
                "Protected environment is not exists!"));
        if (!Objects.equals(agentEnv.getParentUuid(), resource.getParentUuid())) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Can not modify the parent cluster");
        }
        beforeCreate(resource);
    }
}
