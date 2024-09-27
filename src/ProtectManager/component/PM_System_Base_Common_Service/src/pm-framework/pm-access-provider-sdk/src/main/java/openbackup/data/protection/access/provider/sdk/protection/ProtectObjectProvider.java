/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.protection;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.protection.model.CheckProtectObjectDto;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * 保护Provider, 根据subType区分
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/11/10
 */
public interface ProtectObjectProvider extends DataProtectionProvider<String> {
    /**
     * 创建保护前
     *
     * @param checkProtectObjectDto checkProtectObjectDto
     */
    void beforeCreate(CheckProtectObjectDto checkProtectObjectDto);

    /**
     * 修改保护前
     *
     * @param checkProtectObjectDto checkProtectObjectDto
     */
    void beforeUpdate(CheckProtectObjectDto checkProtectObjectDto);

    /**
     * 创建/修改等失败
     *
     * @param checkProtectObjectDto checkProtectObjectDto
     */
    void failedOnCreateOrUpdate(CheckProtectObjectDto checkProtectObjectDto);

    /**
     * 移除保护
     *
     * @param protectedResource protectedResource
     */
    void remove(ProtectedResource protectedResource);
}
