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
package openbackup.data.access.framework.core.security.permission.validator.impl;

import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaPageRequest;

import com.baomidou.mybatisplus.core.toolkit.StringUtils;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.security.permission.AuthValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.user.DomainResourceSetServiceApi;
import openbackup.system.base.sdk.user.ResourceSetResourceServiceApi;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.permission.Permission;

import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.expression.Expression;
import org.springframework.expression.spel.support.StandardEvaluationContext;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

/**
 * 功能描述
 *
 */
@Component
@Slf4j
public class QuerySingleValidator implements AuthValidator {
    @Autowired
    private DomainResourceSetServiceApi domainResourceSetServiceApi;

    @Autowired
    private ResourceSetResourceServiceApi resourceSetResourceServiceApi;

    @Autowired
    private SlaQueryService slaQueryService;

    private final List<String> requiredTypeList = Arrays.asList(ResourceSetTypeEnum.JOB.getType(),
        ResourceSetTypeEnum.COPY.getType(), ResourceSetTypeEnum.RESOURCE_GROUP.getType());

    @Override
    public boolean applicable(String operation) {
        return OperationTypeEnum.QUERY.getValue().equals(operation);
    }

    @Override
    public void beforeBusinessLogic(String domainId, Permission permission, Expression expression,
        StandardEvaluationContext standardEvaluationContext) {
        // 查询资源前遍历当前域所有角色及其下资源集查看是否有该资源的查询权限
        // 解析资源id集合
        Object val = expression.getValue(standardEvaluationContext);
        List<String> resourceIdList = new ArrayList<>();
        if (val instanceof String) {
            if (((String) val).contains(",")) {
                String[] splitVals = ((String) val).split(",");
                resourceIdList.addAll(Arrays.asList(splitVals));
            } else {
                resourceIdList.add((String) val);
            }
        }
        if (val instanceof Collection) {
            resourceIdList = JSONArray.fromObject(val).toBean(String.class);
        }
        if (VerifyUtil.isEmpty(resourceIdList)) {
            return;
        }
        resourceIdList.forEach(resourceId -> {
            String type = permission.resourceSetType().getType();
            // 对于内置的金银铜SLA 不需要校验权限 默认全授予
            if (isPublicResource(type, resourceId)) {
                return;
            }
            // 判断该资源是否在当前用户域内
            checkResourceSetActualType(type, domainId, resourceId);
        });
    }

    private boolean isPublicResource(String type, String resourceId) {
        // 当前只有sla类型的资源存在全局公共类型 不需要校验
        if (ResourceSetTypeEnum.SLA.getType().equals(type)) {
            SlaPageRequest pageRequest = new SlaPageRequest();
            pageRequest.setIsGlobal(true);
            List<String> slaIds = slaQueryService.filterSlaIds(pageRequest);
            return slaIds.contains(resourceId);
        }
        return false;
    }

    private void checkResourceSetActualType(String type, String domainId, String resourceId) {
        // 对于强制检查传入类型的资源 则直接判断是否在域内
        if (requiredTypeList.contains(type)) {
            if (StringUtils.isEmpty(domainResourceSetServiceApi.getResourceSetType(domainId, resourceId, type))) {
                log.error("Resource:{}, type:{} not in current user domain:{}.", resourceId, type, domainId);
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "Resource not in current user domain.");
            }
        } else {
            // 对于存储记录时资源的类型不一定为查询时所指定类型的资源 如agent类型资源查询时传入resources
            String actualType = domainResourceSetServiceApi.getResourceSetType(domainId, resourceId, Strings.EMPTY);
            if (StringUtils.isEmpty(actualType)) {
                log.error("Resource:{}, type:{} not in current user domain:{}.", resourceId, type, domainId);
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "Resource not in current user domain.");
            }
        }
    }
}
