/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.backstorage;

import com.huawei.oceanprotect.system.base.initialize.backstorage.beans.InitBackActionResult;

import openbackup.system.base.common.model.repository.StoragePool;

/**
 * 初始化存储池
 *
 * @author w00493811
 * @since 2020-12-28
 */
public interface InitializeStoragePool {
    /**
     * 执行动作:
     * 设置存储池参数
     *
     * @param deviceId deviceId
     * @param username username
     * @param storagePool 存储池
     * @param result 动作结果
     * @return 动作结果
     */
    InitBackActionResult doAction(String deviceId, String username,
        StoragePool storagePool, InitBackActionResult result);
}