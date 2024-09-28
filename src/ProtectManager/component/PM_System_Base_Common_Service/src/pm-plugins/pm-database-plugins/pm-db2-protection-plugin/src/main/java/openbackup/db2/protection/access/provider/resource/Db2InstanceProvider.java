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
package openbackup.db2.protection.access.provider.resource;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Map;

/**
 * DB2单实例provider
 *
 */
@Component
@Slf4j
public class Db2InstanceProvider implements ResourceProvider {
    private final ProviderManager providerManager;

    private final Db2InstanceService db2InstanceService;

    private final InstanceResourceService instanceResourceService;

    public Db2InstanceProvider(ProviderManager providerManager, Db2InstanceService db2InstanceService,
        InstanceResourceService instanceResourceService) {
        this.providerManager = providerManager;
        this.db2InstanceService = db2InstanceService;
        this.instanceResourceService = instanceResourceService;
    }

    /**
     * 资源重名检查配置
     *
     * @return DB2实例注册重名检查配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = new ResourceFeature();
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("Start create db2 instance parameters check. resource name: {}, uuid: {}", resource.getName(),
            resource.getExtendInfoByKey(DatabaseConstants.HOST_ID));
        db2InstanceService.checkSingleInstanceIsRegistered(resource);
        String checkResult = checkConnection(resource);
        setDb2Instance(resource, checkResult);
        resource.setPath(resource.getEnvironment().getEndpoint());
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End create db2 instance parameters check. resource name: {}, uuid: {}", resource.getName(),
            resource.getExtendInfoByKey(DatabaseConstants.HOST_ID));
    }

    private String checkConnection(ProtectedResource resource) {
        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            resource);
        ResourceCheckContext context = provider.tryCheckConnection(resource);
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("Db2 instance check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("Db2 instance check connection failed. name: {}", resource.getName());
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), "check connection failed.");
        }
        return actionResult.getMessage();
    }

    private void setDb2Instance(ProtectedResource resource, String checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult).toMap(String.class);
        resource.setVersion(messageMap.get(DatabaseConstants.VERSION));
        resource.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY, messageMap.get(DatabaseConstants.DEPLOY_OS_KEY));
        resource.setExtendInfoByKey(DatabaseConstants.DATABASE_BITS_KEY,
            messageMap.get(DatabaseConstants.DATABASE_BITS_KEY));
        resource.setExtendInfoByKey(DatabaseConstants.ROLE, messageMap.get(DatabaseConstants.ROLE));
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update db2 instance parameters check. resource name: {}, uuid: {}", resource.getName(),
            resource.getExtendInfoByKey(DatabaseConstants.HOST_ID));
        db2InstanceService.checkSingleInstanceNameIsChanged(resource);
        String checkResult = checkConnection(resource);
        setDb2Instance(resource, checkResult);
        resource.setPath(resource.getEnvironment().getEndpoint());
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End update db2 instance parameters check. resource name: {}, uuid: {}", resource.getName(),
            resource.getExtendInfoByKey(DatabaseConstants.HOST_ID));
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        instanceResourceService.healthCheckSingleInstance(resource);
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.DB2_INSTANCE.equalsSubType(protectedResource.getSubType());
    }
}
