package openbackup.data.access.framework.core.security.permission.validator;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.security.permission.validator.impl.OperationValidator;
import openbackup.system.base.sdk.user.DomainResourceSetServiceApi;
import openbackup.system.base.sdk.user.ResourceSetResourceServiceApi;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;

import openbackup.system.base.security.permission.Permission;

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

/**
 * 功能描述
 *
 * @author x30046484
 * @since 2024-05-20
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {})
public class DeleteValidatorTest {
    @InjectMocks
    private OperationValidator operationValidator;

    @Mock
    private DomainResourceSetServiceApi domainResourceSetServiceApi;

    @Mock
    private ResourceSetResourceServiceApi resourceSetResourceServiceApi;

    @Test
    public void should_support_delete_operation(){
        boolean sup = operationValidator.applicable(OperationTypeEnum.DELETE.getValue());
        Assert.assertTrue(sup);
    }

    @Test
    public void should_pass_check_before_bussiness_logic_only_domain_resource(){
        SpelExpressionParser spelExpressionParser = new SpelExpressionParser();
        Expression expression = spelExpressionParser.parseExpression("#resourceId");
        StandardEvaluationContext standardEvaluationContext = new StandardEvaluationContext();
        standardEvaluationContext.setVariable("resourceId","resourceId");

        Mockito.when(resourceSetResourceServiceApi.checkHasResourceOperation(anyString(), any(),
            anyString(),anyString())).thenReturn(true);
        Mockito.when(domainResourceSetServiceApi.getResourceSetType(any(), any(), any())).thenReturn("true");
        operationValidator.beforeBusinessLogic("domainId",new Permission(){
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
                return ResourceSetTypeEnum.COPY;
            }

            @Override
            public OperationTypeEnum operation() {
                return OperationTypeEnum.DELETE;
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

            {

        }},expression,standardEvaluationContext);
        Assert.assertTrue(true);
    }

    @Test
    public void should_pass_check_before_bussiness_logic(){
        SpelExpressionParser spelExpressionParser = new SpelExpressionParser();
        Expression expression = spelExpressionParser.parseExpression("#resourceId");
        StandardEvaluationContext standardEvaluationContext = new StandardEvaluationContext();
        standardEvaluationContext.setVariable("resourceId","resourceId");
        Mockito.when(domainResourceSetServiceApi.getResourceSetType(any(), any(), any())).thenReturn("true");
        Mockito.when(resourceSetResourceServiceApi.checkHasResourceOperation(anyString(), any(),
            anyString(), anyString())).thenReturn(true);
        operationValidator.beforeBusinessLogic("domainId", new Permission(){
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
                return OperationTypeEnum.DELETE;
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
            public AuthOperationEnum[] authOperations() {
                return new AuthOperationEnum[]{AuthOperationEnum.MANAGE_RESOURCE};
            }

            {

            }},expression,standardEvaluationContext);
        Assert.assertTrue(true);
    }
}
