/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.security.permission;

import openbackup.system.base.common.constants.TokenBo;

import org.aspectj.lang.ProceedingJoinPoint;

/**
 * User Token Validate Service
 *
 * @author l00272247
 * @since 2021-12-14
 */
public interface UserTokenValidateService {
    /**
     * validate method
     *
     * @param joinPoint joint point
     * @param token token
     */
    void validate(ProceedingJoinPoint joinPoint, TokenBo token);
}
