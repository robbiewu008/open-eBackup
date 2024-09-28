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

import static openbackup.system.base.common.constants.Constants.Builtin.ONLY_IN_DOMAIN_RESOURCE_TYPE_LIST;

import openbackup.data.access.framework.core.security.permission.AuthValidator;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.user.DomainResourceSetServiceApi;
import openbackup.system.base.sdk.user.ResourceSetResourceServiceApi;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.permission.Permission;

import com.baomidou.mybatisplus.core.toolkit.StringUtils;

import lombok.extern.slf4j.Slf4j;

import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.expression.Expression;
import org.springframework.expression.spel.support.StandardEvaluationContext;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * 修改或删除校验
 *
 */
@Component
@Slf4j
public class OperationValidator implements AuthValidator {
    @Autowired
    private DomainResourceSetServiceApi domainResourceSetServiceApi;

    @Autowired
    private ResourceSetResourceServiceApi resourceSetResourceServiceApi;

    private final List<String> requiredTypeList = Arrays.asList(ResourceSetTypeEnum.JOB.getType(),
        ResourceSetTypeEnum.COPY.getType());

    @Override
    public boolean applicable(String operation) {
        return OperationTypeEnum.DELETE.getValue().equals(operation) || OperationTypeEnum.MODIFY.getValue()
            .equals(operation);
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
            // 判断该资源是否在当前用户域内
            String actualType = getResourceSetTypeByCurrentType(type, domainId, resourceId);

            // 只分域不分权资源类型 不校验删除或修改权限
            if (ONLY_IN_DOMAIN_RESOURCE_TYPE_LIST.contains(actualType)) {
                return;
            }

            // 分域分权资源类型 校验是否有删除权限
            List<String> authOperationList = setAuthOperationListByResourceType(actualType, permission);

            if (!resourceSetResourceServiceApi.checkHasResourceOperation(domainId, authOperationList, resourceId,
                actualType)) {
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED,
                    "Current user has no " + permission.operation() + "permission.");
            }
        });
    }

    private String getResourceSetTypeByCurrentType(String type, String domainId, String resourceId) {
        if (requiredTypeList.contains(type)) {
            if (StringUtils.isEmpty(domainResourceSetServiceApi.getResourceSetType(domainId, resourceId, type))) {
                log.error("Resource:{}, type:{} not in current user domain:{}.", resourceId, type, domainId);
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "Resource not in current user domain.");
            }
            return type;
        } else {
            if (StringUtils.isEmpty(
                domainResourceSetServiceApi.getResourceSetType(domainId, resourceId, Strings.EMPTY))) {
                log.error("Resource:{}, type:{} not in current user domain:{}.", resourceId, type, domainId);
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "Resource not in current user domain.");
            }
            return domainResourceSetServiceApi.getResourceSetType(domainId, resourceId, Strings.EMPTY);
        }
    }

    private List<String> setAuthOperationListByResourceType(String actualType, Permission permission) {
        if (StringUtils.equals(ResourceSetTypeEnum.RESOURCE.getType(), actualType)) {
            return Collections.singletonList(AuthOperationEnum.MANAGE_RESOURCE.getAuthOperation());
        } else if (StringUtils.equals(ResourceSetTypeEnum.AGENT.getType(), actualType)) {
            return Collections.singletonList(AuthOperationEnum.MANAGE_CLIENT.getAuthOperation());
        } else {
            return Arrays.stream(permission.authOperations())
                .map(AuthOperationEnum::getAuthOperation)
                .collect(Collectors.toList());
        }
    }
}
