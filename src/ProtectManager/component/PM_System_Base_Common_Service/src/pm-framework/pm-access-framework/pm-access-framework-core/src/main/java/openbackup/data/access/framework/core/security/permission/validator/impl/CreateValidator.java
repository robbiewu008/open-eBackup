package openbackup.data.access.framework.core.security.permission.validator.impl;

import openbackup.data.access.framework.core.security.permission.AuthValidator;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.user.AuthServiceApi;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.security.permission.Permission;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.expression.Expression;
import org.springframework.expression.spel.support.StandardEvaluationContext;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * 创建校验
 *
 * @author x30046484
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-16
 */
@Component
public class CreateValidator implements AuthValidator {
    @Autowired
    private AuthServiceApi authServiceApi;

    @Override
    public boolean applicable(String operation) {
        return OperationTypeEnum.CREATE.getValue().equals(operation);
    }

    @Override
    public void beforeBusinessLogic(String domainId, Permission permission, Expression expression,
        StandardEvaluationContext standardEvaluationContext) {
        // 创建资源前判断当前角色的默认角色资源集是否有创建权限（如果是系统管理员则一定有）
        List<String> authOperationList = Arrays.stream(permission.authOperations())
            .map(AuthOperationEnum::getAuthOperation)
            .collect(Collectors.toList());
        if (!authServiceApi.isDefaultRoleHasAuthOperation(domainId, authOperationList)) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
    }
}
