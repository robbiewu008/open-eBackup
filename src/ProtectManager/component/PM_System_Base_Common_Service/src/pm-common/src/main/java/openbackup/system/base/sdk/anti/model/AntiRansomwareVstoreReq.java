/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.anti.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * PM防勒索-更新租户请求
 *
 * @author j00619968
 * @since 2023-01-06
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class AntiRansomwareVstoreReq {
    // 租户ID
    @Size(max = 4, min = 1)
    String vstoreId;

    // 租户名称
    @Size(max = 512, min = 1)
    String vstoreName;

    // 存储设备ID
    @NotNull
    @Size(max = 64, min = 1)
    String storageDeviceId;
}
