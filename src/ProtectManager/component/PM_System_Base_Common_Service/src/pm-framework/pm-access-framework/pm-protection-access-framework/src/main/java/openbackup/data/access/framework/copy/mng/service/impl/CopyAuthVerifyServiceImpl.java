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
package openbackup.data.access.framework.copy.mng.service.impl;

import static openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum.COPY_GENERATED_BY_REPLICATION;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.service.CopyAuthVerifyService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.user.AuthServiceApi;
import openbackup.system.base.sdk.user.DomainResourceSetServiceApi;
import openbackup.system.base.sdk.user.ResourceSetResourceServiceApi;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 副本权限校验
 *
 */
@Slf4j
@Component
public class CopyAuthVerifyServiceImpl implements CopyAuthVerifyService {
    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private AuthServiceApi authServiceApi;

    @Autowired
    private ResourceSetResourceServiceApi resourceSetResourceServiceApi;

    @Autowired
    private DomainResourceSetServiceApi domainResourceSetServiceApi;

    @Override
    public void checkCopyQueryAuth(Copy copy) {
        if (deployTypeService.isNotSupportRBACType() || copy == null) {
            return;
        }
        checkCopyInDomain(copy.getUuid(), TokenBo.get().getUser().getDomainId());
    }

    @Override
    public void checkCopyOperationAuth(Copy copy, List<String> authOperationList) {
        if (deployTypeService.isNotSupportRBACType() || copy == null) {
            return;
        }
        String domainId = TokenBo.get().getUser().getDomainId();
        checkCopyInDomain(copy.getUuid(), domainId);
        if (COPY_GENERATED_BY_REPLICATION.contains(copy.getGeneratedBy())) {
            // 复制副本校验当前用户的默认角色是否有恢复权限
            if (!authServiceApi.isDefaultRoleHasAuthOperation(domainId, authOperationList)) {
                log.error("No permission of copy: {}.", copy.getUuid());
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "No restore permission.");
            }
            return;
        }
        if (!resourceSetResourceServiceApi.checkHasResourceOperation(domainId, authOperationList, copy.getUuid(),
            ResourceSetTypeEnum.COPY.getType())) {
            log.error("No operation permission of copy: {}.", copy.getUuid());
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "No copy operation permission.");
        }
    }

    private void checkCopyInDomain(String copyId, String domainId) {
        if (StringUtils.isEmpty(domainResourceSetServiceApi.getResourceSetType(domainId, copyId,
            ResourceSetTypeEnum.COPY.getType()))) {
            log.error("Copy: {} not in current user domain.", copyId);
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "Copy not in current user domain.");
        }
    }
}
