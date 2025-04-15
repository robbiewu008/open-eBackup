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
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

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

    private final ResourceService resourceService;

    /**
     * oracle集群注册provider构造函数
     *
     * @param resourceService             resourceService
     * @param protectedEnvironmentService protectedEnvironmentService
     * @param instanceResourceService     instanceResourceService
     * @param oracleBaseService           baseService
     */
    public OraclePdbProvider(ResourceService resourceService, ProtectedEnvironmentService protectedEnvironmentService,
                             InstanceResourceService instanceResourceService, OracleBaseService oracleBaseService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
        this.oracleBaseService = oracleBaseService;
        this.resourceService = resourceService;
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
        resource.setVersion(databaseResource.getVersion());
        resource.setParentName(databaseResource.getParentName());
        resource.setPath(databaseResource.getEnvironment().getEndpoint());
        log.info("End create oracle pdb instance check. resource name: {}", resource.getName());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("Start beforeCreate oracle pdb instance. resource name: {}", resource.getName());
        List<String> pdbList = getPdbList(resource);
        // 检查pdb集内是否存在已经注册的pdb
        checkPdbIsRegistered(pdbList, resource.getParentUuid());
    }

    private void checkPdbIsRegistered(List<String> pdbList, String databaseId) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.ORACLE_PDB.getType());
        conditions.put(DatabaseConstants.PARENT_UUID, databaseId);
        int pageNo = 0;
        PageListResponse<ProtectedResource> data;
        do {
            data = resourceService.query(pageNo, IsmNumberConstant.TEN, conditions);
            data.getRecords().forEach(registeredResource -> checkPdbList(registeredResource, pdbList));
            pageNo++;
        } while (data.getRecords().size() >= IsmNumberConstant.TEN);
    }

    private List<String> getPdbList(ProtectedResource resource) {
        String pdbList = resource.getExtendInfoByKey(OracleConstants.PDB);
        if (pdbList.startsWith("[") && pdbList.endsWith("]")) {
            pdbList = pdbList.substring(1, pdbList.length() - 1);
        }
        if (StringUtils.isBlank(pdbList)) {
            log.error("Select pdb is empty. resource id: {}, name: {}", resource.getUuid(), resource.getName());
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Select pdb is empty.");
        }
        return Arrays.asList(pdbList.split(DatabaseConstants.SPLIT_CHAR));
    }

    private void checkPdbList(ProtectedResource registeredResource, List<String> pdbList) {
        log.info("start check PdbList");
        String registeredPdb = registeredResource.getExtendInfoByKey(OracleConstants.PDB);
        if (registeredPdb.startsWith("[") && registeredPdb.endsWith("]")) {
            registeredPdb = registeredPdb.substring(1, registeredPdb.length() - 1);
        }
        List<String> registeredPdbList = Arrays.asList(registeredPdb.split(DatabaseConstants.SPLIT_CHAR));
        // 重复注册的PDB列表
        List<String> repeatedRegisteredPdb = pdbList.stream()
                .distinct()
                .filter(registeredPdbList::contains)
                .collect(Collectors.toList());
        if (!repeatedRegisteredPdb.isEmpty()) {
            log.error("The select pdb has been registered. repeated registered pdb: {}", repeatedRegisteredPdb);
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_PDB_REPEATED,
                    new String[]{repeatedRegisteredPdb.toString()}, "The pdb is registered");
        }
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
