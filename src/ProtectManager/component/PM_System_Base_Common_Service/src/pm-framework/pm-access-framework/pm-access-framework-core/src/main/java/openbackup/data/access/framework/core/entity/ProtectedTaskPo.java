/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.framework.core.entity;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * ProtectedTask Po
 *
 * @author c00631681
 * @since 2024-01-29
 */
@TableName("PROTECTED_TASK")
@Getter
@Setter
public class ProtectedTaskPo {
    @TableId
    private String uuid;
    private String protectedObjectId;
    private String policyId;
    private String scheduleId;
}