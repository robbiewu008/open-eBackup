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
package openbackup.system.base.common.scurity;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Collection;
import java.util.Collections;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * FutureResult类
 *
 * @param <T> 模板类
 * @author l00272247
 * @version V200R001C50
 * @since 2019-10-30
 */
public class FutureResult<T> {
    private static final Logger LOGGER = LoggerFactory.getLogger(FutureResult.class);

    private final T data;

    private final Throwable cause;

    /**
     * 构造函数
     *
     * @param data 返回结果
     */
    public FutureResult(T data) {
        this(data, null);
    }

    /**
     * 构造函数
     *
     * @param cause 异常结果
     */
    public FutureResult(Throwable cause) {
        this(null, cause);
    }

    /**
     * 构造函数
     *
     * @param data  数据
     * @param cause 异常
     */
    public FutureResult(T data, Throwable cause) {
        this.data = data;
        this.cause = cause;
    }

    public T getData() {
        return data;
    }

    public Throwable getCause() {
        return cause;
    }

    /**
     * 判断是否成功
     *
     * @return 判断结果
     */
    public boolean success() {
        return cause == null;
    }

    /**
     * 判断是否失败
     *
     * @return 判断结果
     */
    public boolean failure() {
        if (cause != null) {
            return true;
        }
        if (data instanceof BaseTaskResult) {
            return !((BaseTaskResult) data).isSuccess();
        }
        return false;
    }

    /**
     * rethrow error if cause is not null, otherwise return the future result itself.
     *
     * @return this future result
     */
    public FutureResult<T> rethrow() {
        if (cause instanceof LegoCheckedException) {
            throw (LegoCheckedException) cause;
        }
        if (cause != null) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, cause);
        }
        return this;
    }

    /**
     * 判断结果集合中是否存在失败的结果
     *
     * @param results 结果集合
     * @param <E>     模板类
     * @return 判断结果
     */
    public static <E> boolean hasFailedResult(Collection<FutureResult<E>> results) {
        return !getFailedResults(results).isEmpty();
    }

    /**
     * 获取失败结果
     *
     * @param results 结果集合
     * @param <E>     模板类
     * @return 失败结果集合
     */
    public static <E> Collection<FutureResult<E>> getFailedResults(Collection<FutureResult<E>> results) {
        return Optional.ofNullable(results).orElseGet(Collections::emptyList).stream()
            .filter(FutureResult::failure)
            .collect(Collectors.toList());
    }
}
