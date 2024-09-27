/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * PM防勒索-更新租户请求
 *
 * @since 2022-12-28
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class AntiRansomwareVstoreReq {
    // 租户ID
    @Size(max = 64, min = 1)
    String vstoreId;

    @Size(max = 512, min = 1)
    // 租户名称
    String vstoreName;

    // 存储设备ID
    @NotNull
    @Size(max = 64, min = 1)
    String storageDeviceId;
}
