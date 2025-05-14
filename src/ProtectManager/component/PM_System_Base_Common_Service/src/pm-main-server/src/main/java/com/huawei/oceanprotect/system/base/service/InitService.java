/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service;

import openbackup.system.base.util.Applicable;

/**
 * dorado service
 *
 * @author g00500588
 * @since 2021-01-11
 */
public interface InitService<T> extends Applicable<String> {
    /**
     * Query dev esn string.
     *
     * @param object object
     * @return the string
     */
    String init(T object);
}
