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

import openbackup.system.base.lambda.ThrowableSupplier;

import java.util.function.Function;
import java.util.function.Supplier;

/**
 * Thread Cache
 *
 * @param <T> template type
 */
public class ThreadCache<T> {
    private final ThreadLocal<T> cache = new ThreadLocal<>();

    /**
     * get cache data
     *
     * @return cache data
     */
    public T get() {
        T data = cache.get();
        if (data == null) {
            cache.remove();
        }
        return data;
    }

    /**
     * call method
     *
     * @param supplier supplier
     * @param data data
     * @param <E> template type
     * @return result
     */
    public <E> E call(Supplier<E> supplier, T data) {
        return call(supplier::get, value -> data);
    }

    /**
     * call method
     *
     * @param supplier supplier
     * @param function data function
     * @param <E> template type E
     * @param <ET> template type ET
     * @return result
     * @throws ET exception
     */
    public <E, ET extends Throwable> E call(ThrowableSupplier<ET, E> supplier, Function<T, T> function) throws ET {
        T oldData = cache.get();
        cache.set(function.apply(oldData));
        try {
            return supplier.get();
        } finally {
            if (oldData != null) {
                cache.set(oldData);
            } else {
                cache.remove();
            }
        }
    }

    /**
     * run method
     *
     * @param runnable runnable
     * @param data data
     */
    public void run(Runnable runnable, T data) {
        call(
                () -> {
                    runnable.run();
                    return null;
                },
                data);
    }
}
