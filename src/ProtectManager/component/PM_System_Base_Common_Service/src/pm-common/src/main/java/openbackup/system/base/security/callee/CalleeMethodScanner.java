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
package openbackup.system.base.security.callee;

import org.springframework.beans.factory.annotation.AnnotatedBeanDefinition;
import org.springframework.context.annotation.ClassPathScanningCandidateComponentProvider;
import org.springframework.core.type.AnnotationMetadata;
import org.springframework.core.type.filter.AnnotationTypeFilter;

/**
 * Callee Method Scanner
 *
 * @author l00272247
 * @since 2021-12-14
 */
public class CalleeMethodScanner extends ClassPathScanningCandidateComponentProvider {
    /**
     * constructor
     */
    public CalleeMethodScanner() {
        super(false);
        addIncludeFilter(new AnnotationTypeFilter(CalleeMethods.class, true, true));
    }

    /**
     * 确定给定的bean定义是否符合发布条件
     *
     * @param beanDefinition beanDefinition
     * @return boolean
     */
    @Override
    protected boolean isCandidateComponent(AnnotatedBeanDefinition beanDefinition) {
        AnnotationMetadata metadata = beanDefinition.getMetadata();
        return metadata.isIndependent() && !metadata.isAnnotation();
    }
}
