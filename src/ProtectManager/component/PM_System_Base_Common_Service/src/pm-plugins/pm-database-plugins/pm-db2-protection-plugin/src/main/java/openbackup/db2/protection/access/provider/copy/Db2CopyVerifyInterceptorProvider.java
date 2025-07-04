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
package openbackup.db2.protection.access.provider.copy;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyHelper;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.protection.access.provider.sdk.copy.CopyVerifyInterceptor;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

/**
 * db2副本校验拦截器
 *
 */
@Component
@Slf4j
public class Db2CopyVerifyInterceptorProvider implements CopyVerifyInterceptor {
    private final InstanceResourceService instanceResourceService;

    private final Db2Service db2Service;

    private final CopyManagerService copyManagerService;

    private final CopyRestApi copyRestApi;

    public Db2CopyVerifyInterceptorProvider(InstanceResourceService instanceResourceService, Db2Service db2Service,
            CopyManagerService copyManagerService, CopyRestApi copyRestApi) {
        this.instanceResourceService = instanceResourceService;
        this.db2Service = db2Service;
        this.copyManagerService = copyManagerService;
        this.copyRestApi = copyRestApi;
    }

    @Override
    public CopyVerifyTask interceptor(CopyVerifyTask task) {
        log.info("Start set db2 copy verify param. id: {}", task.getTaskId());
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        task.setTargetObject(copyManagerService.buildTaskResource(copy));
        task.setTargetEnv(copyManagerService.buildTaskEnvironment(task.getTargetObject().getRootUuid()));
        ProtectedResource resource = instanceResourceService.getResourceById(task.getTargetObject().getUuid());
        task.getTargetEnv().setNodes(db2Service.getEnvNodesByInstanceResource(resource));
        task.setAgents(db2Service.getAgentsByInstanceResource(resource));
        setEnvExtendInfo(task);
        log.info("End set db2 copy verify param. id: {}", task.getTaskId());
        return task;
    }

    @Override
    public void checkIsSupportVerify(Copy copy) {
        CopyVerifyHelper.copyIsNotGeneratedByReplication(copy);
    }

    private void setEnvExtendInfo(CopyVerifyTask task) {
        Map<String, String> envExtendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, Db2ClusterTypeEnum.getDeployType(task.getTargetEnv()
            .getExtendInfo()
            .getOrDefault(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.SINGLE.getType())));
        task.getTargetEnv().setExtendInfo(envExtendInfo);
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.DB2_DATABASE.getType(), ResourceSubTypeEnum.DB2_TABLESPACE.getType())
            .contains(subType);
    }
}
