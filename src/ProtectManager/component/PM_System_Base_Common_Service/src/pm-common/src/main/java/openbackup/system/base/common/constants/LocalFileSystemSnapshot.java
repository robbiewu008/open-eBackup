/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.constants;

import lombok.Data;

/**
 * 文件系统快照信息
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-10
 */
@Data
public class LocalFileSystemSnapshot {
    private String id;

    private String name;

    // 是否是安全快照
    private Boolean isSecuritySnap;

    // 是否在保护期内
    private Boolean isInProtectionPeriod;
}
