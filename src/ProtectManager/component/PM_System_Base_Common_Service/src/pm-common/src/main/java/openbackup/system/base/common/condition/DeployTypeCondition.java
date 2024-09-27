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
package openbackup.system.base.common.condition;

import openbackup.system.base.common.enums.DeployTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.boot.autoconfigure.condition.ConditionOutcome;
import org.springframework.boot.autoconfigure.condition.SpringBootCondition;
import org.springframework.context.annotation.ConditionContext;
import org.springframework.core.annotation.MergedAnnotation;
import org.springframework.core.annotation.MergedAnnotations;
import org.springframework.core.type.AnnotatedTypeMetadata;

import java.util.Arrays;
import java.util.Optional;

/**
 * 部署类型Condition, 与ConditionalOnDeployType结合使用
 *
 * @since 2023-01-03
 */
@Slf4j
class DeployTypeCondition extends SpringBootCondition {
    /**
     * 确定匹配的结果(是否生成bean)以及适当的日志输出。
     *
     * @param context 上下文
     * @param metadata 注解内容
     * @return ConditionOutcome.noMatch(提示信息): 不生成bean; ConditionOutcome.match(): 生成bean
     */
    @Override
    public ConditionOutcome getMatchOutcome(ConditionContext context, AnnotatedTypeMetadata metadata) {
        MergedAnnotations annotations = metadata.getAnnotations();
        MergedAnnotation<ConditionalOnDeployType> deployTypeMergedAnnotation =
            annotations.get(ConditionalOnDeployType.class);
        if (deployTypeMergedAnnotation.isPresent()) {
            Optional<DeployTypeEnum[]> defaultValue =
                deployTypeMergedAnnotation.getValue("value", DeployTypeEnum[].class);
            if (!defaultValue.isPresent()) {
                return ConditionOutcome.match();
            }
            DeployTypeEnum[] deployTypes = defaultValue.get();
            String productModel = System.getenv("DEPLOY_TYPE");
            if (productModel == null) {
                productModel = DeployTypeEnum.X8000.getValue();
            }
            DeployTypeEnum currentDeployType = DeployTypeEnum.getByValue(productModel);
            if (Arrays.stream(deployTypes).noneMatch(deployType -> deployType == currentDeployType)) {
                log.info("Class: {} does not support deploy type: {}", deployTypeMergedAnnotation.getSource(),
                    currentDeployType);
                return ConditionOutcome.noMatch(currentDeployType + " not supported.");
            }
            log.info("Class: {} support deploy type: {}", deployTypeMergedAnnotation.getSource(), currentDeployType);
        }
        return ConditionOutcome.match();
    }
}
