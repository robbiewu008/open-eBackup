/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resourcegroup.req;

import openbackup.system.base.common.utils.JSONObject;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;

/**
 * 修改资源组保护请求体
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-01-27
 */

@Getter
@Setter
public class UpdateResourceGroupProtectedObjectRequest {
    @Length(min = 1, max = 64)
    @NotNull
    private String resourceGroupId;

    @Length(min = 1, max = 36)
    @NotNull
    private String slaId;

    private JSONObject extParams;
}