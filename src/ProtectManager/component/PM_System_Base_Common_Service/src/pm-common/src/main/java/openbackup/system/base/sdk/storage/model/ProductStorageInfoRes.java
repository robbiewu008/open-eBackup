/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 存储信息
 *
 * @author p00511147
 * @since 2020-12-10
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class ProductStorageInfoRes {
    private String wwn;

    private ProductStorageInfo storageInfo;
}
