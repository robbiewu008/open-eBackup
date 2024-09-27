/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.framework.core.security.permission;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.security.permission.Permission;

import org.springframework.expression.Expression;
import org.springframework.expression.spel.support.StandardEvaluationContext;

/**
 * 功能描述
 *
 * @author x30046484
 * @since 2024-05-16
 */
public interface AuthValidator extends DataProtectionProvider<String> {
    /**
     * RBAC权限校验支持
     *
     * @param operation 支持的操作类型
     * @return 是否支持
     */
    boolean applicable(String operation);

    /**
     * 业务代码执行前逻辑
     *
     * @param permission permission
     * @param domainId 域id
     * @param expression expression
     * @param standardEvaluationContext standardEvaluationContext
     */
    void beforeBusinessLogic(String domainId, Permission permission, Expression expression,
        StandardEvaluationContext standardEvaluationContext);
}
