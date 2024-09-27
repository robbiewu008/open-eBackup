/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oracle.constants;

import lombok.Data;

/**
 * ubc返回的oracle scn副本信息
 *
 * @author c30038333
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-06
 */
@Data
public class ScnCopy {
    // 副本Id
    private String id;

    // 创建时间
    private long timestamp;

    public ScnCopy(String id, long timestamp) {
        this.id = id;
        this.timestamp = timestamp;
    }

    /**
     * 默认构造函数
     */
    public ScnCopy() {
    }
}
