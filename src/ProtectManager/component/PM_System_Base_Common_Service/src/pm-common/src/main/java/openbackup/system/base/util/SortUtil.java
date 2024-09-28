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
package openbackup.system.base.util;

import openbackup.system.base.bean.SortRule;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

/**
 * 排序工具类
 *
 */
@Slf4j
public class SortUtil {
    /**
     * list排序
     *
     * @param list 待排集合
     * @param sortRules 排序规则集
     * @param <T> 泛型
     */
    @SuppressWarnings("unchecked")
    public static <T> void sort(List<T> list, SortRule... sortRules) {
        List<Comparator<T>> comparators = new ArrayList<>();
        for (SortRule rule : sortRules) {
            if (CollectionUtils.isEmpty(rule.getOrder())) {
                if (rule.isReversed()) {
                    comparators.add(
                        (Comparator<T>) Comparator.comparing(obj -> getFieldValue(obj, rule.getField()).toString())
                            .reversed());
                    continue;
                }
                comparators.add(Comparator.comparing(obj -> getFieldValue(obj, rule.getField()).toString()));
                continue;
            }
            if (rule.isReversed()) {
                comparators.add((Comparator<T>) Comparator.comparing(
                    obj -> rule.getOrder().indexOf(getFieldValue(obj, rule.getField()))).reversed());
                continue;
            }
            comparators.add(Comparator.comparing(obj -> rule.getOrder().indexOf(getFieldValue(obj, rule.getField()))));
        }
        sort(list, comparators);
    }

    /**
     * list排序
     *
     * @param list 待排集合
     * @param comparators 排序规则集
     * @param <T> 泛型
     */
    public static <T> void sort(List<T> list, List<Comparator<T>> comparators) {
        list.sort((o1, o2) -> {
            for (Comparator<T> comparator : comparators) {
                int result = comparator.compare(o1, o2);
                if (result != 0) {
                    return result;
                }
            }
            return 0;
        });
    }

    @SuppressWarnings("unchecked")
    private static <R, U> R getFieldValue(U obj, String fieldName) {
        try {
            Field field = obj.getClass().getDeclaredField(fieldName);
            field.setAccessible(true);
            return (R) field.get(obj);
        } catch (NoSuchFieldException | IllegalAccessException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "sort failed at get filed value");
        }
    }
}

