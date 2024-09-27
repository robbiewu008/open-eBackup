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

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2InstanceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Map;
import java.util.Objects;

/**
 * DB2集群实例provider
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-27
 */
@Component
@Slf4j
public class Db2ClusterInstanceProvider implements ResourceProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final Db2InstanceService db2instanceService;

    private final InstanceResourceService instanceResourceService;

    private final EncryptorService encryptorService;

    public Db2ClusterInstanceProvider(ProtectedEnvironmentService protectedEnvironmentService,
        Db2InstanceService db2instanceService, InstanceResourceService instanceResourceService,
        EncryptorService encryptorService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.db2instanceService = db2instanceService;
        this.instanceResourceService = instanceResourceService;
        this.encryptorService = encryptorService;
    }

    /**
     * 资源重名检查配置
     *
     * @return DB2集群实例注册重名检查配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = new ResourceFeature();
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }

    private ProtectedEnvironment buildEnvironment(String environmentId) {
        return protectedEnvironmentService.getEnvironmentById(environmentId);
    }

    private void setDb2ClusterInstance(ProtectedResource resource, AgentBaseDto checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult.getErrorMessage()).toMap(String.class);
        resource.setVersion(messageMap.get(DatabaseConstants.VERSION));
        resource.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY, messageMap.get(DatabaseConstants.DEPLOY_OS_KEY));
        resource.setExtendInfoByKey(DatabaseConstants.DATABASE_BITS_KEY,
            messageMap.get(DatabaseConstants.DATABASE_BITS_KEY));
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("Start create db2 cluster instance check. resource name: {}", resource.getName());
        resource.setEnvironment(buildEnvironment(resource.getParentUuid()));
        db2instanceService.filterClusterInstance(resource);
        decryptPwd(resource);
        db2instanceService.checkClusterInstanceIsRegistered(resource);
        checkIsClusterInstance(resource);
        resource.setPath(resource.getEnvironment().getEndpoint());
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End create db2 cluster instance check. resource name: {}", resource.getName());
    }

    private void decryptPwd(ProtectedResource resource) {
        resource.getDependencies().get(DatabaseConstants.CHILDREN).stream().forEach(subInstance -> {
            String pwd = subInstance.getAuth().getAuthPwd();
            if (subInstance.getAuth().getAuthType() == Authentication.APP_PASSWORD && StringUtils.isNotBlank(pwd)) {
                subInstance.getAuth().setAuthPwd(encryptorService.decrypt(pwd));
            }
        });
    }

    private void checkIsClusterInstance(ProtectedResource resource) {
        if (Objects.equals(resource.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE),
            Db2ClusterTypeEnum.HADR.getType())) {
            db2instanceService.checkHadrClusterInstance(resource);
            return;
        }
        AgentBaseDto checkResult = db2instanceService.checkIsClusterInstance(resource);
        setDb2ClusterInstance(resource, checkResult);
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update db2 cluster instance check. resource name: {}", resource.getName());
        resource.setEnvironment(buildEnvironment(resource.getParentUuid()));
        db2instanceService.filterClusterInstance(resource);
        fillInstancePwd(resource);
        db2instanceService.checkClusterInstanceNameIsChanged(resource);
        checkIsClusterInstance(resource);
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End update db2 cluster instance check. resource name: {}", resource.getName());
    }

    private void fillInstancePwd(ProtectedResource resource) {
        resource.getDependencies().get(DatabaseConstants.CHILDREN).forEach(instance -> {
            if (VerifyUtil.isEmpty(instance.getAuth().getAuthPwd())) {
                instance.getAuth()
                    .setAuthPwd(instanceResourceService.getResourceById(instance.getUuid()).getAuth().getAuthPwd());
            }
        });
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.DB2_CLUSTER_INSTANCE.equalsSubType(protectedResource.getSubType());
    }
}
