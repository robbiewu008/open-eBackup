/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.common;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * openstack 公共检查服务
 *
 * @author t30049904
 * @version [OceanProtect DataBack 1.5.0]
 * @since 2023/9/19
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
