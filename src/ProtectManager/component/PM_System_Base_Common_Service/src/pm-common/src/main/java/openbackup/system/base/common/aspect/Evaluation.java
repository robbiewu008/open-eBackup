/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.aspect;

/**
 * Evaluation
 *
 * @author l00272247
 * @since 2021-01-14
 */
public interface Evaluation {
    /**
     * evaluate
     *
     * @param data data
     * @return result
     */
    Object evaluate(Object data);

    /**
     * evaluate
     *
     * @return result
     */
    default String evaluate() {
        Object value = evaluate(null);
        return value != null ? value.toString() : null;
    }
}
