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
package openbackup.openstack.protection.access.common;

import openbackup.openstack.protection.access.constant.OpenstackConstant;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * openstack 公共检查服务
 *
 */
@Slf4j
@Component
public class OpenstackCommonService {
    private final ResourceService resourceService;

    public OpenstackCommonService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 检查openstack和domain数量和是否超出限制
     *
     */
    public void checkOpenStackAndDomainMaxNum() {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("subType", Arrays.asList(ResourceSubTypeEnum.OPENSTACK_DOMAIN.getType(),
                ResourceSubTypeEnum.OPENSTACK_CONTAINER.getType()));
        conditions.put("sourceType", ResourceConstants.SOURCE_TYPE_REGISTER);
        PageListResponse<ProtectedResource> response = resourceService.query(LegoNumberConstant.ZERO,
                LegoNumberConstant.ONE, conditions);
        log.info("openstack and domain count is:{}", response.getTotalCount());
        if (response.getTotalCount() >= OpenstackConstant.MAX_DOMAIN_COUNT) {
            throw new LegoCheckedException(CommonErrorCode.ENV_COUNT_OVER_LIMIT, new String[]{
                    String.valueOf(OpenstackConstant.MAX_DOMAIN_COUNT)}, "Domains and OpenStack count over limit");
        }
    }
}
