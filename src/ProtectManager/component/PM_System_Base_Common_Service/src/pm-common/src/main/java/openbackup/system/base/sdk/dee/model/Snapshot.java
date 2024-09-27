/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.dee.model;

import lombok.Data;

/**
 * 副本相关的快照信息
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-20
 */
@Data
public class Snapshot {
    /**
     * 快照id
     */
    private String id;

    /**
     * 文件系统名
     */
    private String parentName;
}
