/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;

import org.springframework.web.multipart.MultipartFile;

/**
 * 获取LLD配置信息和并返回
 *
 * @author swx1010572
 * @version: [DataBackup 1.5.0]
 * @since 2023-07-25
 */
public interface InitializeNetworkBodyXls {
    /**
     * 根据过滤条件获取端口列表
     *
     * @param lld 过滤条件
     * @return 所有的端口信息
     */
    InitNetworkBody checkAndReturnInitXls(MultipartFile lld);
}
