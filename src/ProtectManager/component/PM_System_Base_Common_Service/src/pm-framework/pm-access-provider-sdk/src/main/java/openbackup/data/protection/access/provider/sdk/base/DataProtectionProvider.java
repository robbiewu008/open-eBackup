/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.base;

/**
 * DataMover provider base class
 *
 * @param <T> template type
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
 */
public interface DataProtectionProvider<T> {
    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    boolean applicable(T object);
}
