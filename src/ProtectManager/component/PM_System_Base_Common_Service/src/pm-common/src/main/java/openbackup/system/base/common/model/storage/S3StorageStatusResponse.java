/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.model.storage;

import lombok.Getter;
import lombok.Setter;

/**
 * S3StorageStatusResponse
 *
 * @author y30046482
 * @since 2023-08-17
 */
@Getter
@Setter
public class S3StorageStatusResponse {
    private String id;

    private String esn;

    private String storageId;

    private Integer status;

    private String clusterName;

    private String clusterIp;
}