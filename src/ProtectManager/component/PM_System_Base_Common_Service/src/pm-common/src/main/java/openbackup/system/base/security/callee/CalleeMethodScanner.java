/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
