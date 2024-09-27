/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.security.permission;

import static openbackup.system.base.common.aspect.OperationLogAspect.TOKEN_BO;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.core.security.Evaluation;
import openbackup.system.base.common.aspect.DomainBasedOwnershipVerifier;
import openbackup.system.base.common.constants.AspectOrderConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.sdk.user.model.RolePo;
import openbackup.system.base.security.download.DownloadRightsControl;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.user.service.UserInternalService;
import openbackup.system.base.util.DefaultRoleHelper;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.Signature;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.reflect.MethodSignature;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.core.annotation.Order;
import org.springframework.expression.Expression;
import org.springframework.expression.spel.standard.SpelExpressionParser;
import org.springframework.expression.spel.support.StandardEvaluationContext;
import org.springframework.stereotype.Component;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

import javax.servlet.http.HttpServletRequest;

/**
 * Permission Aspect
 *
 * @author l00272247
 * @since 2021-12-11
 */
@Slf4j
@Aspect
@Component
@Order(AspectOrderConstant.PERMISSION_ASPECT_ORDER)
public class PermissionAspect {
    private static final String GET_REQUEST = "GET";

    private final ApplicationContext context;
    private final TokenVerificationService tokenVerificationService;
    private final List<UserTokenValidateService> userTokenValidateServices;
    private final List<DomainBasedOwnershipVerifier> verifiers;

    @Autowired
    private UserInternalService userInternalService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private ProviderManager providerManager;

    /**
     * constructor
     *
     * @param context context
     * @param tokenVerificationService tokenVerificationService
     * @param verifiers verifiers
     * @param userTokenValidateServices userTokenValidateServices
     */
    public PermissionAspect(
            ApplicationContext context,
            TokenVerificationService tokenVerificationService,
            List<DomainBasedOwnershipVerifier> verifiers,
            List<UserTokenValidateService> userTokenValidateServices) {
        this.context = context;
        this.tokenVerificationService = tokenVerificationService;
        this.verifiers = verifiers;
        this.userTokenValidateServices = userTokenValidateServices;
    }

    /**
     * aspect logging
     *
     * @param joinPoint join point
     * @param permission access
     * @return result
     * @throws Throwable throwable
     */
    @Around(value = "@annotation(permission)", argNames = "joinPoint,permission")
    public Object aspectAccess(ProceedingJoinPoint joinPoint, Permission permission) throws Throwable {
        TokenBo tokenBo = tokenVerificationService.parsingTokenFromRequest();
        if (tokenBo == null) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
        if (permission.enableCheckAuth()) {
            checkAuth(joinPoint, permission);
        }
        TokenBo.UserBo userBo = tokenBo.getUser();
        // 设备管理员角色不存在域-角色关联关系，直接放行
        if (StringUtils.equals(Constants.ROLE_DEVICE_MANAGER, userBo.getRoles().get(0).getId())) {
            assignTokenAsRequestAttribute(tokenBo);
            return joinPoint.proceed();
        }
        RolePo defaultRole = userInternalService.getDomainDefaultRoleSet(userBo.getDomainId());
        if (deployTypeService.isNotSupportRBACType()
            || permission.checkRolePermission()
            || DefaultRoleHelper.isBuiltInRole(defaultRole.getRoleId())) {
            intercept(joinPoint, permission, tokenBo);
        }
        assignTokenAsRequestAttribute(tokenBo);
        return joinPoint.proceed();
    }

    private void assignTokenAsRequestAttribute(TokenBo tokenBo) {
        RequestAttributes requestAttributes = Objects.requireNonNull(RequestContextHolder.getRequestAttributes());
        if (requestAttributes instanceof ServletRequestAttributes) {
            HttpServletRequest request = ((ServletRequestAttributes) requestAttributes).getRequest();

            request.setAttribute(TOKEN_BO, tokenBo);
        }
    }

    private void intercept(ProceedingJoinPoint joinPoint, Permission permission, TokenBo tokenBo) {
        TokenBo.UserBo userBo = tokenBo.getUser();
        for (UserTokenValidateService userTokenValidateService : userTokenValidateServices) {
            userTokenValidateService.validate(joinPoint, tokenBo);
        }
        checkRoles(userBo, permission.roles(), isForceSkipAuditor(joinPoint));
        checkResources(joinPoint, userBo, permission.resources());
    }

    private boolean isForceSkipAuditor(ProceedingJoinPoint joinPoint) {
        if (joinPoint.getSignature() instanceof MethodSignature) {
            MethodSignature signature = (MethodSignature) joinPoint.getSignature();
            Method method = signature.getMethod();
            return Arrays.stream(method.getDeclaredAnnotations())
                .anyMatch(annotation -> annotation.annotationType().equals(DownloadRightsControl.class));
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "point get signature failed");
    }

    private void checkRoles(TokenBo.UserBo userBo, String[] roles, boolean isForceSkipAuditor) {
        if (roles.length == 0) {
            return;
        }
        String role = userBo.getRoles().get(0).getName();
        // 审计员角色具有系统只读权限
        if (!isForceSkipAuditor && isGetRequest() && hasRoleAuditor(userBo)) {
            return;
        }
        if (StringUtils.equals(UserTypeEnum.HCS.getValue(), userBo.getUserType())) {
            if (!userBo.isHcsUserManagePermission() && !isGetRequest()) {
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
            }
        }
        if (Arrays.stream(roles).noneMatch(item -> Objects.equals(item, role))) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
    }

    private boolean hasRoleAuditor(TokenBo.UserBo userBo) {
        return userBo.getRoles().stream().anyMatch(roleBo -> Constants.Builtin.ROLE_AUDITOR.equals(roleBo.getName()));
    }

    private static boolean isGetRequest() {
        RequestAttributes requestAttributes = RequestContextHolder.getRequestAttributes();
        HttpServletRequest request;
        if (requestAttributes instanceof ServletRequestAttributes) {
            request = ((ServletRequestAttributes) requestAttributes).getRequest();
        } else {
            return false;
        }
        return GET_REQUEST.equalsIgnoreCase(request.getMethod());
    }

    private void checkResources(ProceedingJoinPoint joinPoint, TokenBo.UserBo userBo, String[] resources) {
        Map<String, List<String>> resolvedResources = resolveResources(joinPoint, resources);
        for (Map.Entry<String, List<String>> entry : resolvedResources.entrySet()) {
            getDomainBasedVerifierByType(entry.getKey())
                    .ifPresent(verifier -> verify(userBo, entry.getValue(), verifier));
        }
    }

    private void verify(TokenBo.UserBo userBo, List<String> resources, DomainBasedOwnershipVerifier verifier) {
        if (!DefaultRoleHelper.isAdminOrAudit(userBo.getId())) {
            verifier.verify(userBo, resources);
        }
    }

    private Optional<DomainBasedOwnershipVerifier> getDomainBasedVerifierByType(String type) {
        return verifiers.stream().filter(verifier -> Objects.equals(type, verifier.getType())).findFirst();
    }

    private Map<String, List<String>> resolveResources(ProceedingJoinPoint joinPoint, String[] resources) {
        Map<String, List<String>> map = new HashMap<>();
        for (String resource : resources) {
            int index = resource.indexOf(":");
            if (index == -1) {
                continue;
            }
            String type = resource.substring(0, index);
            String[] parts = resource.substring(index + 1).split(";");
            List<Object> items =
                    Arrays.stream(parts)
                            .map(String::trim)
                            .filter(item -> !item.isEmpty())
                            .map(item -> evaluate(joinPoint, item))
                            .filter(Objects::nonNull)
                            .collect(Collectors.toList());
            List<String> list = map.getOrDefault(type, new ArrayList<>());
            list.addAll(unfold(items));
            map.put(type, list);
        }
        return map;
    }

    private Object evaluate(ProceedingJoinPoint joinPoint, String item) {
        return new Evaluation(context, item)
                .evaluate(() -> Evaluation.castArrayToMap(Arrays.asList(joinPoint.getArgs())));
    }

    private List<String> unfold(Collection<?> items) {
        List<String> results = new ArrayList<>();
        for (Object item : items) {
            if (item instanceof Collection) {
                Collection collection = (Collection) item;
                results.addAll(unfold(collection));
                continue;
            }
            if (item != null) {
                results.add(item.toString());
            }
        }
        return results;
    }

    private void checkAuth(ProceedingJoinPoint joinPoint, Permission permission) {
        // 不支持RBAC的部署形态直接返回
        if (deployTypeService.isNotSupportRBACType()) {
            return;
        }
        // 从http请求头中获取token并解析
        TokenBo tokenBo = tokenVerificationService.parsingTokenFromRequest();
        if (tokenBo == null) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
        // 初始化Spel解析器及上下文
        SpelExpressionParser spelExpressionParser = new SpelExpressionParser();
        Signature signature = joinPoint.getSignature();
        MethodSignature methodSignature = null;
        if (signature instanceof MethodSignature) {
            methodSignature = (MethodSignature) signature;
        }
        Expression expression = null;
        if (StringUtils.isNotEmpty(permission.target())) {
            expression = spelExpressionParser.parseExpression(permission.target());
        }
        // 将入参传入Spel解析上下文中
        String[] parameterNames = methodSignature.getParameterNames();
        List<Object> args = Arrays.asList(joinPoint.getArgs());
        StandardEvaluationContext standardEvaluationContext = new StandardEvaluationContext();
        for (int i = 0; i < args.size(); i++) {
            standardEvaluationContext.setVariable(parameterNames[i], args.get(i));
        }
        TokenBo.UserBo userBo = tokenBo.getUser();
        // 从token中获取当前域id
        String domainId = userBo.getDomainId();

        AuthValidator chosenValidator = providerManager.findProvider(AuthValidator.class,
            permission.operation().getValue());
        chosenValidator.beforeBusinessLogic(domainId, permission, expression, standardEvaluationContext);
    }
}
