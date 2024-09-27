/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.pack.lock.entity;

import com.baomidou.mybatisplus.annotation.IdType;
import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.Date;

/**
 * 锁信息表
 *
 * @author w30042425
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-06-10
 */
@Getter
@Setter
@TableName("T_DISTRIBUTED_LOCK")
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class LockEntity {
    @TableId(value = "id", type = IdType.INPUT)
    private String id;

    @TableField("KEY")
    private String key;

    @TableField("DESCRIPTION")
    private String description;

    @TableField("UNLOCK_TIME")
    private Date unlockTime;

    @TableField("LOCK_TIME")
    private Date lockTime;

    @TableField("OWNER")
    private String owner;
}
