/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.core.security.journal;

/**
 * Evaluate Service Interface
 *
 * @author l00272247
 * @since 2022-01-07
 */
public interface EvaluateService<T> {
    /**
     * evaluate before execute
     *
     * @param loggingContext logging context
     * @param param param
     */
    void evaluateBeforeExecute(LoggingContext loggingContext, T param);

    /**
     * evaluate after execute
     *
     * @param loggingContext logging context
     */
    void evaluateAfterExecute(LoggingContext loggingContext);
}
