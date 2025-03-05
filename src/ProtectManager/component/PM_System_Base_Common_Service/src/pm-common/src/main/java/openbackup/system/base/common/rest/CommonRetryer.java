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
package openbackup.system.base.common.rest;

import feign.RetryableException;
import feign.Retryer;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.CollectionUtils;
import openbackup.system.base.common.utils.CommonUtil;
import openbackup.system.base.common.utils.ExceptionUtil;

import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.BiPredicate;

/**
 * Common Retryer
 *
 */
@Slf4j
public class CommonRetryer<T> implements Retryer {
    private final CommonRetryPolicy policy;
    private final BiPredicate<List<T>, Throwable> predicate;
    private final List<T> evidences;
    private final AtomicInteger counter;

    /**
     * constructor
     *
     * @param predicate predicate
     * @param evidences evidences
     */
    public CommonRetryer(BiPredicate<List<T>, Throwable> predicate, T... evidences) {
        this(new CommonRetryPolicy(), predicate, evidences);
    }

    /**
     * constructor
     *
     * @param policy policy
     * @param predicate predicate
     * @param evidences evidences
     */
    public CommonRetryer(CommonRetryPolicy policy, BiPredicate<List<T>, Throwable> predicate, T... evidences) {
        this(policy, predicate, CollectionUtils.list(evidences));
    }

    private CommonRetryer(CommonRetryPolicy policy, BiPredicate<List<T>, Throwable> predicate, List<T> evidences) {
        this.policy = policy;
        this.predicate = predicate;
        this.evidences = evidences;
        this.counter = new AtomicInteger();
    }

    /**
     * retry on special codes
     *
     * @param codes codes
     * @return retryer
     */
    public static CommonRetryer<Long> create(Long... codes) {
        return new CommonRetryer<>((items, error) -> true, codes);
    }

    /**
     * retry on special policy
     *
     * @param policy policy
     * @return retryer
     */
    public static CommonRetryer<Long> create(CommonRetryPolicy policy) {
        return new CommonRetryer<>(policy, (items, error) -> true);
    }

    /**
     * continue or propagate
     *
     * @param ex exception
     */
    @Override
    public void continueOrPropagate(RetryableException ex) {
        if (predicate.test(evidences, ex)) {
            int count = counter.getAndIncrement();
            if (count < policy.getAttempts()) {
                log.error("rest error. url is {}, start retry, count is {}", ex.request().url(), count + 1,
                        ExceptionUtil.getErrorMessage(ex));
                sleep();
                return;
            }
        }
        if (ex instanceof CommonRetryableException) {
            Exception exception = ((CommonRetryableException) ex).getException();
            if (exception instanceof RuntimeException) {
                throw (RuntimeException) exception;
            } else {
                throw new LegoUncheckedException(exception);
            }
        }
        throw ex;
    }

    private void sleep() {
        if (policy.getWaitTime() > 0) {
            CommonUtil.sleep(policy.getWaitTime());
        }
    }

    /**
     * clone
     *
     * @return clone
     */
    @Override
    public CommonRetryer<T> clone() {
        return new CommonRetryer<>(policy, predicate, evidences);
    }
}
