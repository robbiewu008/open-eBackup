/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.emeistor.console.bean;

import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * redis键过期entity
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/5/24
 */
@Getter
@Setter
@TableName("t_redis_expire")
public class RedisExpireEntity {
    private String key;
    private String userId;
    private String type;
}
