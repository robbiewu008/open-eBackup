/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

/**
 * DataMover ips object
 *
 * @author p30001902
 * @since 2020-11-19
 */
@Data
public class DataMoverIps {
    private String anyBackup;

    private String dme;
}
