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
package openbackup.kingbase.protection.access.provider.resource;

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

import org.springframework.stereotype.Component;

import java.util.Map;

/**
 * kingbase单实例provider
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-13
 */
@Component
@Slf4j
public class KingBaseInstanceProvider implements ResourceProvider {
    private final ProviderManager providerManager;

    private final InstanceResourceService instanceResourceService;

    /**
     * KingBaseInstanceProvider
     *
     * @param providerManager Provider管理器
     * @param instanceResourceService 实例资源服务
     */
    public KingBaseInstanceProvider(ProviderManager providerManager, InstanceResourceService instanceResourceService) {
        this.providerManager = providerManager;
        this.instanceResourceService = instanceResourceService;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("Start create kingbase instance parameters check. resource name: {}", resource.getName());
        // 校验实例是否已经存在
        instanceResourceService.checkSignalInstanceIsRegistered(resource);

        // 检查实例连通性
        String checkResult = checkConnection(resource);

        // 设置实例属性值
        setKingBaseInstanceProperties(resource, checkResult);
        log.info("End create kingbase instance parameters check. resource name: {}", resource.getName());
    }

    private String checkConnection(ProtectedResource resource) {
        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            resource);
        ResourceCheckContext context = provider.tryCheckConnection(resource);
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("Kingbase instance check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("Kingbase instance check connection failed. name: {}", resource.getName());
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), "check connection failed.");
        }
        return actionResult.getMessage();
    }

    private void setKingBaseInstanceProperties(ProtectedResource resource, String checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult).toMap(String.class);
        resource.setVersion(messageMap.get(DatabaseConstants.VERSION));
        resource.setPath(resource.getEnvironment().getEndpoint());
        resource.setExtendInfoByKey(DatabaseConstants.DATA_DIRECTORY, messageMap.get(DatabaseConstants.DATA_DIRECTORY));
        resource.setExtendInfoByKey(DatabaseConstants.DB_MODE_KEY, messageMap.get(DatabaseConstants.DB_MODE_KEY));
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update kingbase instance parameters check. resource name: {}", resource.getName());
        // check实例端口是否被修改
        instanceResourceService.checkSignalInstancePortIsChanged(resource);

        // 检查实例连通性
        String checkResult = checkConnection(resource);

        // 设置实例属性值
        setKingBaseInstanceProperties(resource, checkResult);
        log.info("End update kingbase instance parameters check. resource name: {}", resource.getName());
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        instanceResourceService.healthCheckSingleInstance(resource);
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.KING_BASE_INSTANCE.equalsSubType(protectedResource.getSubType());
    }
}
