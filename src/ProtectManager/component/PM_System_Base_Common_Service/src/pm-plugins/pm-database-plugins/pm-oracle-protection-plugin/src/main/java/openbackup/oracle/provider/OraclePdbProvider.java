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
package openbackup.oracle.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

/**
 * OraclePdbProvider
 *
 */
@Component
@Slf4j
public class OraclePdbProvider implements ResourceProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    private final OracleBaseService oracleBaseService;

    /**
     * oracle集群注册provider构造函数
     *
     * @param protectedEnvironmentService protectedEnvironmentService
     * @param instanceResourceService instanceResourceService
     * @param oracleBaseService baseService
     */
    public OraclePdbProvider(ProtectedEnvironmentService protectedEnvironmentService,
                             InstanceResourceService instanceResourceService, OracleBaseService oracleBaseService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
        this.oracleBaseService = oracleBaseService;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.ORACLE_PDB.getType().equals(object.getSubType());
    }

    @Override
    public void check(ProtectedResource resource) {
        log.info("Start create oracle pdb instance check. resource name: {}", resource.getName());
        // 获取数据库实例信息与环境信息
        ProtectedResource databaseResource = getDatabaseResource(resource);
        ProtectedEnvironment environment = protectedEnvironmentService
                .getEnvironmentById(resource.getExtendInfoByKey(OracleConstants.HOST_ID));
        databaseResource.setEnvironment(environment);
        databaseResource.getExtendInfo().put(DatabaseConstants.VIRTUAL_IP,
                databaseResource.getEnvironment().getEndpoint());
        // 检查刷新pdb集实例的状态
        oracleBaseService.checkPdbInstanceActiveStandby(databaseResource, environment);
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End create oracle pdb instance check. resource name: {}", resource.getName());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {

    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        instanceResourceService.healthCheckSingleInstance(getDatabaseResource(resource));
    }

    @Override
    public List<ProtectedResource> scan(ProtectedResource resource) {
        ProtectedResource databaseResource = getDatabaseResource(resource);
        log.info("Oracle single scan started, resource id :{}", resource.getUuid());
        oracleBaseService.checkPdbInstanceActiveStandby(databaseResource, databaseResource.getEnvironment());
        log.info("Oracle single scan finished, resource id :{}", resource.getUuid());
        return Collections.singletonList(resource);
    }

    /**
     * 获取数据库资源。
     *
     * @param resource 受保护的资源
     * @return 数据库资源对象
     */
    public ProtectedResource getDatabaseResource(ProtectedResource resource) {
        ProtectedResource databaseResource = instanceResourceService
                .getResourceById(resource.getExtendInfoByKey(OracleConstants.DB_UUID));
        databaseResource.setExtendInfoByKey(OracleConstants.PDB, resource.getExtendInfoByKey(OracleConstants.PDB));
        return databaseResource;
    }
}
