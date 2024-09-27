/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.framework.copy.mng.service;

import openbackup.system.base.sdk.copy.model.Copy;

import java.util.List;

/**
 * 功能描述
 *
 * @author z00842230
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-17
 */
public interface CopyAuthVerifyService {
    /**
     * 校验副本查询权限
     *
     * @param copy 副本
     */
    void checkCopyQueryAuth(Copy copy);

    /**
     * 校验副本操作权限
     *
     * @param copy              副本
     * @param authOperationList 操作权限集合
     */
    void checkCopyOperationAuth(Copy copy, List<String> authOperationList);
}
