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
package openbackup.oceanprotect.k8s.protection.access.provider;

import openbackup.oceanprotect.k8s.protection.access.constant.K8sConstant;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * k8s Dataset资源类provider
 *
 * @author t30049904
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/24
 */
@Slf4j
@AllArgsConstructor
@Component
public class K8sDatasetResourceProvider extends DefaultResourceProvider {
    private final ResourceService resourceService;

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.equalsSubType(object.getSubType());
    }

    /**
     * 注册时检查资源
     *
     * @param resource 资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        // 检验数据集参数是否合法
        checkName(resource);
        checkParentUuid(resource);
        // 检查namespace下dataset数量是否合法
        checkK8sDatasetSetCount(resource);
        fillPath(resource);
    }

    /**
     * 更新时检查资源
     *
     * @param resource 资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        // 检验数据集参数是否合法
        checkName(resource);
        fillPath(resource);
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        return true;
    }

    private void checkK8sDatasetSetCount(ProtectedResource resource) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("parentUuid", resource.getParentUuid());
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = resourceService.basicQuery(false, 0,
                K8sConstant.DATASET_MAX_COUNT_IN_NAMESPACE, conditions);
        int count = protectedResourcePageListResponse.getTotalCount();
        if (count >= K8sConstant.DATASET_MAX_COUNT_IN_NAMESPACE) {
            log.error("K8s Dataset check, count Exceeded the maximum count: {}", count);
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_NUM_EXCEED_LIMIT,
                    new String[]{String.valueOf(K8sConstant.DATASET_MAX_COUNT_IN_NAMESPACE)},
                    "The number of K8s exceeds the maximum.");
        }
    }

    private void checkName(ProtectedResource resource) {
        if (VerifyUtil.isEmpty(resource.getName())
                || !ValidateUtil.match(RegexpConstants.NAME_STR, resource.getName())) {
            log.error("K8s Dataset check, name is illegal, name:{}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "K8s Dataset name is illegal.");
        }
    }

    private void checkParentUuid(ProtectedResource resource) {
        if (VerifyUtil.isEmpty(resource.getParentUuid())) {
            log.error("K8s Dataset check, K8s Dataset parentUuid is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "K8s Dataset parentUuid is empty.");
        }
    }

    private void fillPath(ProtectedResource dataset) {
        ProtectedResource ns = resourceService.getBasicResourceById(dataset.getParentUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Namespace not found."));
        dataset.setPath(ns.getPath() + File.separator + dataset.getName());
    }
}
