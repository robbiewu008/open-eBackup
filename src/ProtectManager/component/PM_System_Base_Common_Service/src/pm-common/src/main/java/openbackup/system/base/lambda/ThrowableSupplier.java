/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.lambda;

/**
 * Throwable Supplier
 *
 * @param <ET> template type ET
 * @param <T> template type T
 * @author l00272247
 * @since 2021-12-14
 */
public interface ThrowableSupplier<ET extends Throwable, T> {
    /**
     * getter
     *
     * @return value
     * @throws ET exception
     */
    T get() throws ET;
}
