/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.lambda.ThrowableSupplier;

import java.util.function.Function;
import java.util.function.Supplier;

/**
 * Thread Cache
 *
 * @param <T> template type
 * @author l00272247
 * @since 2021-11-23
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
