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
package openbackup.cnware.protection.access.livemount;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * 功能描述 Cnware 即时挂载provider
 *
 */
@Slf4j
@Component
public class CnwareLiveMountIntercreptorProvider implements LiveMountInterceptorProvider {
    private final CnwareCommonService cnwareCommonService;

    private final CopyRestApi copyRestApi;

    /**
     * 构造器注入
     *
     * @param copyRestApi copyRestApi
     * @param cnwareCommonService cnwareCommonService
     */
    public CnwareLiveMountIntercreptorProvider(CnwareCommonService cnwareCommonService, CopyRestApi copyRestApi) {
        this.cnwareCommonService = cnwareCommonService;
        this.copyRestApi = copyRestApi;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.CNWARE_VM.equalsSubType(subType);
    }

    @Override
    public Integer getLiveMountNumLimit() {
        return LegoNumberConstant.THIRTY_TWO;
    }

    @Override
    public void initialize(LiveMountCreateTask task) {
        log.info("Start live mount create task, requestId is {}", task.getRequestId());
        TaskResource targetObject = task.getTargetObject();
        String rootUuid = CnwareConstant.RES_TYPE_CNWARE_HOST.equals(targetObject.getSubType())
            ? targetObject.getRootUuid()
            : targetObject.getUuid();
        ProtectedEnvironment targetEnv = cnwareCommonService.getEnvironmentById(rootUuid);
        task.setTargetEnv(BeanTools.copy(targetEnv, TaskEnvironment::new));
        task.getTargetEnv().getExtendInfo().put(CnwareConstant.DEPLOY_TYPE, CnwareConstant.SINGLE);

        // 填充fileSystemShareInfo
        List<StorageRepository> repositories = task.getRepositories();
        Map<String, Object> advanceParams = task.getAdvanceParams();
        Object fileSystemShareInfo = advanceParams.get("fileSystemShareInfo");
        JSONArray fileSystemShareInfoList = JSONArray.fromObject(fileSystemShareInfo);
        for (StorageRepository repository : repositories) {
            repository.getExtendInfo().put("fileSystemShareInfo", fileSystemShareInfoList.get(0));
        }
        log.info("CNware live mount check success with repository, requestId is {}", task.getRequestId());
    }

    @Override
    public void finalize(LiveMountCancelTask task) {
        log.info("Start live mount cancel task, requestId is {}", task.getRequestId());
        // 根据targetObject，设置TargetEnv
        TaskResource targetObject = task.getTargetObject();
        String rootUuid = CnwareConstant.RES_TYPE_CNWARE_HOST.equals(targetObject.getSubType())
            ? targetObject.getRootUuid()
            : targetObject.getUuid();
        ProtectedEnvironment targetEnv = cnwareCommonService.getEnvironmentById(rootUuid);
        task.setTargetEnv(BeanTools.copy(targetEnv, TaskEnvironment::new));
        task.getTargetEnv().getExtendInfo().put(CnwareConstant.DEPLOY_TYPE, CnwareConstant.SINGLE);
    }

    @Override
    public boolean isSupportCheckCopyOperation() {
        return true;
    }

    /**
     * 是否刷新资源
     *
     * @return 要或者不
     */
    @Override
    public boolean isRefreshTargetEnvironment() {
        return false;
    }
}
