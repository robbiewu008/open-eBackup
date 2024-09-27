/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.lock.entity;

import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * 功能描述: LockResource
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-07-07
 */
@Getter
@Setter
@TableName(value = "lock_resources")
public class ResourceLock {
    private String lockId;

    private String resourceId;

    private String lockType;

    private String lockState;

    private Integer priority;

    private Long timestamp;
}