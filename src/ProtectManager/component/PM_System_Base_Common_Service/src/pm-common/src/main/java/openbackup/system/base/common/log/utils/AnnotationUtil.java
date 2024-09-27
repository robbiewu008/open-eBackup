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
package openbackup.system.base.common.log.utils;

import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.core.annotation.AnnotationUtils;

import java.lang.annotation.Annotation;
import java.lang.reflect.AnnotatedElement;
import java.lang.reflect.Method;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Annotation Util
 *
 * @author l00272247
 * @since 2019-11-14
 */
public class AnnotationUtil {
    /**
     * constructor
     */
    protected AnnotationUtil() {
    }

    /**
     * get annotations from annotated element
     *
     * @param annotatedElement        annotated element
     * @param annotationType          annotation type
     * @param annotationContainerType annotation container type
     * @param function                function
     * @param <A>                     a
     * @param <C>                     c
     * @return annotations
     */
    public static <A extends Annotation, C extends Annotation> List<A> getAnnotations(AnnotatedElement annotatedElement,
        Class<A> annotationType, Class<C> annotationContainerType, Function<C, A[]> function) {
        Stream<A> stream1 = Stream.of(annotatedElement.getAnnotation(annotationType));
        Stream<A> stream2 = Optional.ofNullable(annotatedElement.getAnnotation(annotationContainerType))
            .map(function)
            .map(Arrays::asList)
            .map(Collection::stream)
            .orElseGet(Stream::empty);
        return Stream.concat(stream1, stream2).filter(Objects::nonNull).collect(Collectors.toList());
    }

    /**
     * get annotations
     *
     * @param annotatedElement annotated element
     * @param annotationType   annotation type
     * @param scopes           scope
     * @param <A>              tempalte type
     * @return annotations
     */
    @SafeVarargs
    public static <A extends Annotation> Set<A> getAnnotations(AnnotatedElement annotatedElement,
        Class<A> annotationType, Class<? extends Annotation>... scopes) {
        boolean hasAnnotations;
        List<Class<? extends Annotation>> ranges;
        if (scopes != null) {
            hasAnnotations = scopes.length == 0;
            ranges = Arrays.stream(scopes).filter(Objects::nonNull) // 过滤非空元素
                .filter(limit -> AnnotationUtils.isAnnotationMetaPresent(limit, annotationType)) // 原标签过滤
                .collect(Collectors.toList());
        } else {
            hasAnnotations = true;
            ranges = Collections.emptyList();
        }
        Set<A> annotations = new HashSet<>();
        for (Annotation annotation : annotatedElement.getAnnotations()) {
            if (annotationType.isInstance(annotation)) {
                annotations.add(annotationType.cast(annotation));
                continue;
            }
            Class<? extends Annotation> type = annotation.annotationType();
            if (!AnnotationUtils.isAnnotationMetaPresent(type, annotationType)) {
                continue;
            }
            if (hasAnnotations || isRelatedAnnotatedElement(type, ranges)) {
                Set<A> items = AnnotatedElementUtils.getMergedRepeatableAnnotations(annotation.annotationType(),
                    annotationType);
                annotations.addAll(items);
            }
        }
        return annotations;
    }

    /**
     * get annotations of method parameters with index
     *
     * @param method method
     * @return annotations of method parameters
     */
    public static List<Map.Entry<Integer, List<Annotation>>> getParameterAnnotations(Method method) {
        List<Map.Entry<Integer, List<Annotation>>> parametersAnnotationList = new ArrayList<>();
        Annotation[][] annotations = method.getParameterAnnotations();
        for (int index = 0; index < annotations.length; index++) {
            Annotation[] items = annotations[index];
            parametersAnnotationList.add(new AbstractMap.SimpleEntry<>(index, Arrays.asList(items)));
        }
        return parametersAnnotationList;
    }

    private static boolean isRelatedAnnotatedElement(Class<? extends Annotation> annotationType,
        List<Class<? extends Annotation>> scopes) {
        for (Class<? extends Annotation> scope : scopes) {
            if (AnnotationUtils.isAnnotationMetaPresent(annotationType, scope)
                || AnnotationUtils.isAnnotationMetaPresent(scope, annotationType)) {
                return true;
            }
        }
        return false;
    }
}
