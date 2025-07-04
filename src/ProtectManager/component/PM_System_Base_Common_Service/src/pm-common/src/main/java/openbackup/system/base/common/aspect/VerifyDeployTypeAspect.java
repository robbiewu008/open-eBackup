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
package openbackup.system.base.common.aspect;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.annotation.VerifyDeployType;
import openbackup.system.base.common.constants.AspectOrderConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.service.DeployTypeService;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.springframework.core.annotation.Order;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 验证部署形态的切面
 *
 */
@Aspect
@Component
@Slf4j
@Order(AspectOrderConstant.DEPLOY_TYPE_ASPECT_ORDER)
public class VerifyDeployTypeAspect {
    private final DeployTypeService deployTypeService;

    public VerifyDeployTypeAspect(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    /**
     * 校验部署方式注解
     *
     * @param verifyDeployType 校验部署方式注解
     */
    @Before(value = "(execution(* com.huawei..*(..))) && @annotation(verifyDeployType)")
    public void processCheckDeployTypeSupport(VerifyDeployType verifyDeployType) {
        DeployTypeEnum deployType = deployTypeService.getDeployType();
        log.info("start check deploy type.deployType is : {}", deployType);
        Set<DeployTypeEnum> notSupportedDeployTypes = Arrays.stream(verifyDeployType.notSupportedDeployTypes())
                .collect(Collectors.toSet());
        if (notSupportedDeployTypes.contains(deployType)) {
            throw new LegoCheckedException(CommonErrorCode.NOT_SUPPORT_FUNCTION,
                    "Deploy type does not support this operation", log);
        }
    }
}
