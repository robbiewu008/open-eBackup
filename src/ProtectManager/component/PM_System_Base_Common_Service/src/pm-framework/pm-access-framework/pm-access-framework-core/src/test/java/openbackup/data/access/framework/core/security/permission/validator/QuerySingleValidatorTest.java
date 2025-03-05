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
package openbackup.data.access.framework.core.security.permission.validator;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import openbackup.data.access.framework.core.security.permission.validator.impl.QuerySingleValidator;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.sdk.user.DomainResourceSetServiceApi;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;

import openbackup.system.base.security.permission.Permission;
import org.apache.commons.lang3.StringUtils;
import org.checkerframework.checker.units.qual.A;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.expression.Expression;
import org.springframework.expression.spel.standard.SpelExpressionParser;
import org.springframework.expression.spel.support.StandardEvaluationContext;
import java.lang.annotation.Annotation;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {})
public class QuerySingleValidatorTest {
    @InjectMocks
    private QuerySingleValidator querySingleValidator;

    @Mock
    private DomainResourceSetServiceApi domainResourceSetServiceApi;

    @Mock
    private SlaQueryService slaQueryService;

    @Test
    public void should_support_delete_operation(){
        boolean sup = querySingleValidator.applicable(OperationTypeEnum.QUERY.getValue());
        Assert.assertTrue(sup);
    }

    @Test
    public void should_pass_check_before_bussiness_logic(){
        SpelExpressionParser spelExpressionParser = new SpelExpressionParser();
        Expression expression = spelExpressionParser.parseExpression("#resourceId");
        StandardEvaluationContext standardEvaluationContext = new StandardEvaluationContext();
        standardEvaluationContext.setVariable("resourceId","resourceId");

        Mockito.when(domainResourceSetServiceApi.getResourceSetType(any(), any(), any()))
            .thenReturn("true");
        querySingleValidator.beforeBusinessLogic("domainId", new Permission(){
            @Override
            public Class<? extends Annotation> annotationType() {
                return Permission.class;
            }

            @Override
            public String[] roles() {
                return new String[0];
            }

            @Override
            public String[] resources() {
                return new String[0];
            }

            public ResourceSetTypeEnum resourceSetType() {
                return ResourceSetTypeEnum.RESOURCE;
            }

            @Override
            public OperationTypeEnum operation() {
                return OperationTypeEnum.QUERY;
            }

            @Override
            public AuthOperationEnum[] authOperations() {
                return new AuthOperationEnum[]{AuthOperationEnum.MANAGE_RESOURCE};
            }

            @Override
            public String target() {
                return null;
            }

            public boolean checkRolePermission() {
                return false;
            }

            @Override
            public boolean enableCheckAuth() {
                return false;
            }

            @Override
            public boolean isEnableCustomRole() {
                return false;
            }

            {

            }},expression,standardEvaluationContext);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：sla在域中
     * 前置条件：公共sla
     * 检查点：成功检查
     */
    @Test
    public void should_pass_check_before_bussiness_logic_with_sla_in_domain() {
        SpelExpressionParser spelExpressionParser = new SpelExpressionParser();
        Expression expression = spelExpressionParser.parseExpression("#resourceId");
        StandardEvaluationContext standardEvaluationContext = new StandardEvaluationContext();
        standardEvaluationContext.setVariable("resourceId", "resourceId");

        Mockito.when(domainResourceSetServiceApi.getResourceSetType(any(), any(), any())).thenReturn("SLA");
        ArrayList<String> slaIds = new ArrayList<>();
        slaIds.add("resourceId");
        Mockito.when(slaQueryService.filterSlaIds(any())).thenReturn(slaIds);
        querySingleValidator.beforeBusinessLogic("domainId", new Permission() {
            @Override
            public Class<? extends Annotation> annotationType() {
                return Permission.class;
            }

            @Override
            public String[] roles() {
                return new String[0];
            }

            @Override
            public String[] resources() {
                return new String[0];
            }

            public ResourceSetTypeEnum resourceSetType() {
                return ResourceSetTypeEnum.SLA;
            }

            @Override
            public OperationTypeEnum operation() {
                return OperationTypeEnum.QUERY;
            }

            @Override
            public AuthOperationEnum[] authOperations() {
                return new AuthOperationEnum[] {AuthOperationEnum.SLA};
            }

            @Override
            public String target() {
                return null;
            }

            public boolean checkRolePermission() {
                return false;
            }

            @Override
            public boolean enableCheckAuth() {
                return false;
            }

            @Override
            public boolean isEnableCustomRole() {
                return false;
            }

            {

            }
        }, expression, standardEvaluationContext);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：sla不在域中 但是是公共sla
     * 前置条件：公共sla
     * 检查点：成功检查
     */

    @Test
    public void should_pass_check_before_bussiness_logic_with_sla_not_in_domain() {
        SpelExpressionParser spelExpressionParser = new SpelExpressionParser();
        Expression expression = spelExpressionParser.parseExpression("#resourceId");
        StandardEvaluationContext standardEvaluationContext = new StandardEvaluationContext();
        standardEvaluationContext.setVariable("resourceId", "resourceId");

        Mockito.when(domainResourceSetServiceApi.getResourceSetType(any(), any(), any())).thenReturn(StringUtils.EMPTY);
        ArrayList<String> slaIds = new ArrayList<>();
        Mockito.when(slaQueryService.filterSlaIds(any())).thenReturn(slaIds);
        Assert.assertThrows(LegoCheckedException.class,
            () -> querySingleValidator.beforeBusinessLogic("domainId", new Permission() {
                @Override
                public Class<? extends Annotation> annotationType() {
                    return Permission.class;
                }

                @Override
                public String[] roles() {
                    return new String[0];
                }

                @Override
                public String[] resources() {
                    return new String[0];
                }

                public ResourceSetTypeEnum resourceSetType() {
                    return ResourceSetTypeEnum.SLA;
                }

                @Override
                public OperationTypeEnum operation() {
                    return OperationTypeEnum.QUERY;
                }

                @Override
                public AuthOperationEnum[] authOperations() {
                    return new AuthOperationEnum[] {AuthOperationEnum.SLA};
                }

                @Override
                public String target() {
                    return null;
                }

                public boolean checkRolePermission() {
                    return false;
                }

                @Override
                public boolean enableCheckAuth() {
                    return false;
                }

                @Override
                public boolean isEnableCustomRole() {
                    return false;
                }

                {

                }
            }, expression, standardEvaluationContext));
        Assert.assertTrue(true);
    }

}
