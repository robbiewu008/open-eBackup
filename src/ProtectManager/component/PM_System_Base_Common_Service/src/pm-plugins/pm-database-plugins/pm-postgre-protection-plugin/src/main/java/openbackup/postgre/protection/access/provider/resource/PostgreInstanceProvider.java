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
package openbackup.postgre.protection.access.provider.resource;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.cxf.common.util.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Map;

/**
 * postgre单实例provider
 *
 */
@Component
@Slf4j
public class PostgreInstanceProvider implements ResourceProvider {
    private final ProviderManager providerManager;

    private final InstanceResourceService instanceResourceService;

    /**
     * PostgreInstanceProvider构造方法
     *
     * @param providerManager  provider管理器
     * @param instanceResourceService 实例资源服务
     */
    public PostgreInstanceProvider(ProviderManager providerManager, InstanceResourceService instanceResourceService) {
        this.providerManager = providerManager;
        this.instanceResourceService = instanceResourceService;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("start create postgre instance parameters check. resource name: {}", resource.getName());
        // 校验实例是否已经存在
        instanceResourceService.checkSignalInstanceIsRegistered(resource);

        // 检查实例连通性
        String checkResult = checkConnection(resource);
        setPostgreInstance(resource, checkResult);

        // 设置path
        resource.setPath(resource.getEnvironment().getEndpoint());
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End create postgre instance parameters check. resource name: {}", resource.getName());
    }

    private String checkConnection(ProtectedResource resource) {
        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            resource);
        ResourceCheckContext context = provider.tryCheckConnection(resource);
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("postgre instance check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("postgre instance check connection failed. name: {}", resource.getName());
            String message = actionResult.getMessage();
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()),
                StringUtils.isEmpty(message) ? "check connection failed." : message);
        }
        return actionResult.getMessage();
    }

    private void setPostgreInstance(ProtectedResource resource, String checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult).toMap(String.class);
        resource.setVersion(messageMap.get(DatabaseConstants.VERSION));
        resource.setExtendInfoByKey(DatabaseConstants.DATA_DIRECTORY, messageMap.get(DatabaseConstants.DATA_DIRECTORY));
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("start update postgre instance parameters check. resource name: {}", resource.getName());
        // check实例端口是否被修改
        instanceResourceService.checkSignalInstancePortIsChanged(resource);

        // 检查实例连通性
        checkConnection(resource);
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        instanceResourceService.healthCheckSingleInstance(resource);
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.POSTGRE_INSTANCE.equalsSubType(protectedResource.getSubType());
    }
}
