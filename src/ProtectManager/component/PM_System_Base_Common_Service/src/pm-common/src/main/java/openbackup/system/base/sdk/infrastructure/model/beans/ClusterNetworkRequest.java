/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model.beans;

import lombok.Data;

/**
 * 功能描述
 *
 * @author x00464136
 * @since 2023-10-18
 */
@Data
public class ClusterNetworkRequest {
    /**
     * 内部通信网络平面ip
     */
    private String ipCheck;
}
