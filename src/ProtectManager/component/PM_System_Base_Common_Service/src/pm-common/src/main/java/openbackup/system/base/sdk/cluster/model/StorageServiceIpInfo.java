/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import java.util.List;

/**
 * Storage service ips info
 *
 * @author p30001902
 * @since 2020-12-10
 */
@Data
public class StorageServiceIpInfo {
    private String esn;

    private List<StorageFrontServiceInfo> serviceIps;
}
